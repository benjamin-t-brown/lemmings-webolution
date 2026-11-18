"""Preview or run one queue task with a Windows process-tree deadline."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time
import uuid
from datetime import datetime, timezone

from winjob import run_tree
from status_summary import refresh as refresh_status

ROOT = Path(__file__).resolve().parents[2]
QUEUE = ROOT / 'agent/tasks.json'


class RepositoryLease:
    """One writer across single-task and continuous runs."""
    def __init__(self, root):
        self.root = Path(root).resolve()
        self.path = self.root / 'build/agent-runs/supervisor.lock'
        self.identity = {'pid': os.getpid(), 'token': uuid.uuid4().hex}
        self.active = False

    def __enter__(self):
        self.path.parent.mkdir(parents=True, exist_ok=True)
        fd = os.open(self.path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        try:
            os.write(fd, json.dumps(self.identity).encode())
        finally:
            os.close(fd)
        self.active = True
        return self

    def check(self):
        if not self.active or json.loads(self.path.read_text()) != self.identity:
            raise RuntimeError('Repository lease changed or is not active')

    def __exit__(self, *_):
        try:
            self.check()
            self.path.unlink()
        finally:
            self.active = False


def utc():
    return datetime.now(timezone.utc).isoformat()


def atomic_json(path, data):
    temporary = path.with_suffix(path.suffix + '.tmp')
    temporary.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')
    os.replace(temporary, path)


def select_task(queue, task_id):
    tasks = queue['tasks']
    ids = [t['id'] for t in tasks]
    if len(ids) != len(set(ids)):
        raise ValueError('Duplicate task IDs')
    task = next((t for t in tasks if t['id'] == task_id), None)
    if not task or not re.fullmatch(r'[a-zA-Z0-9_-]+', task_id):
        raise ValueError('Unknown or invalid task ID')
    if task['status'] != 'ready':
        raise ValueError(f'Task is {task["status"]}, not ready')
    if task.get('requires_original_execution', True):
        raise ValueError('Original-execution tasks cannot use this supervisor')
    by_id = {t['id']: t for t in tasks}
    if any(by_id.get(dep, {}).get('status') != 'done' for dep in task.get('depends_on', [])):
        raise ValueError('Dependencies are not reviewed and done')
    if task.get('verification') not in ('smoke', 'main', 'recovery', 'all'):
        raise ValueError('Invalid verification profile')
    return task


def model_settings(backend, registry):
    settings = registry.get('agent_models', {}).get(backend, {})
    model, effort = settings.get('model'), settings.get('effort')
    if not isinstance(model, str) or not model.strip() or model != model.strip():
        raise ValueError(f'Missing explicit model for {backend} in agent/tools.windows.json')
    allowed = {'codex': ('low', 'medium', 'high', 'xhigh', 'max'),
               'claude': ('low', 'medium', 'high', 'xhigh', 'max')}
    if effort not in allowed[backend]:
        raise ValueError(f'Missing or unsupported explicit effort for {backend}')
    if model.lower() in ('default', 'auto', 'opus', 'sonnet', 'haiku', 'best', 'opusplan'):
        raise ValueError('Use a versioned model ID, not a moving alias')
    if backend == 'claude':
        override = os.environ.get('CLAUDE_CODE_EFFORT_LEVEL')
        if override and override != effort:
            raise ValueError('CLAUDE_CODE_EFFORT_LEVEL conflicts with the pinned effort; unset it before running')
    return {'model': model, 'effort': effort}


def static_rea_enabled(task, read_only, role=None):
    if read_only:
        return role in ('plan', 'review', 'review_completion')
    if not task or task.get('requires_original_execution', False):
        return False
    kind = task.get('kind', 'implementation')
    return (kind == 'implementation' or
            (kind == 'research' and task.get('user_authorized') is True))


def command_for(backend, registry, run_dir, *, read_only=False, schema=None, task=None, role=None):
    selected = model_settings(backend, registry)
    executable = registry['tools'][backend]['path']
    if not Path(executable).is_file():
        raise ValueError(f'Missing {backend} executable')
    if backend == 'codex':
        command = [executable, 'exec', '--ignore-user-config', '--model', selected['model'],
                '-c', 'model_reasoning_effort=' + json.dumps(selected['effort']),
                '-c', 'windows.sandbox="elevated"',
                '--sandbox', 'read-only' if read_only else 'workspace-write', '-c', 'approval_policy="never"',
                '--cd', str(ROOT), '--json', '--color', 'never',
                '--output-last-message', str(run_dir / 'last-message.txt'), '-']
        if static_rea_enabled(task, read_only, role):
            from rea_static_proxy import ALLOWED
            profile = json.loads((ROOT / 'agent/rea-static.mcp.json').read_text(encoding='utf-8'))['mcpServers']['rea_static']
            # One explicit MCP table replaces inherited servers for this worker.
            policy = ', '.join(name + ' = { approval_mode = "approve" }' for name in sorted(ALLOWED))
            server = ('{ rea_static = { command = ' + json.dumps(profile['command'])
                      + ', args = ' + json.dumps(profile['args'])
                      + ', enabled_tools = ' + json.dumps(sorted(ALLOWED))
                      + ', required = true, startup_timeout_sec = 60, tool_timeout_sec = 300'
                      + ', tools = { ' + policy + ' } } }')
            command[-1:-1] = ['-c', 'mcp_servers=' + server]
        if schema:
            command[-1:-1] = ['--output-schema', str(schema)]
        return command
    # Workers can invoke only explicitly allowed build/history commands without prompts.
    # Planning/review retain read-only file tools plus explicitly scoped static MCP.
    # Safe mode skips local hooks/plugins; shared context is provided explicitly.
    command = [executable, '--print', '--safe-mode', '--model', selected['model'],
               '--effort', selected['effort'], '--permission-mode',
               'dontAsk' if read_only else 'acceptEdits',
               '--permission-prompts', 'none', '--tools',
               'Read,Glob,Grep' if read_only else 'Read,Glob,Grep,Edit,Write,Bash',
               '--output-format', 'json' if schema else 'stream-json', '--verbose']
    if not read_only:
        command += ['--allowedTools',
                    'Bash(git --no-pager show --no-ext-diff --no-textconv HEAD:src/app/main.cpp)',
                    'Bash(python tools/agent_control/verify.py --help)',
                    'Bash(python tools/agent_control/verify.py --profile smoke)',
                    'Bash(python tools/agent_control/verify.py --profile main)',
                    'Bash(python tools/agent_control/verify.py --profile recovery)',
                    'Bash(python tools/agent_control/verify.py --profile all)']
    if static_rea_enabled(task, read_only, role):
        from rea_static_proxy import ALLOWED
        # Explicit local profile: no inherited hooks/plugins/MCP servers.
        command.remove('--safe-mode')
        if read_only:
            command += ['--allowedTools']
        command += ['mcp__rea_static__' + name for name in sorted(ALLOWED)]
        command += ['--setting-sources', '', '--disable-slash-commands',
                    '--settings', str(ROOT / 'agent/claude-research.settings.json'),
                    '--strict-mcp-config', '--mcp-config', str(ROOT / 'agent/rea-static.mcp.json')]
    if schema:
        command += ['--json-schema', schema.read_text(encoding='utf-8')]
    return command


def make_prompt(task, minutes, backend):
    """AGENTS.md carries the rules; this adds the task and per-run mechanics only."""
    instructions = (ROOT / 'AGENTS.md').read_text(encoding='utf-8')
    bash = ('Claude workers have Bash for these exact commands, run verbatim and separately from the '
            'repo root with no pipes, echo or wrappers: '
            'git --no-pager show --no-ext-diff --no-textconv HEAD:src/app/main.cpp; '
            'python tools/agent_control/verify.py --profile smoke (or main, recovery or all). '
            'For verifier calls set timeout=600000 and run_in_background=false. '
            if backend == 'claude' else '')
    return (f'{instructions}\n\nAssigned task:\n{json.dumps(task, indent=2)}\n\n'
            f'Complete only this task. Worker budget is {minutes:.1f} minutes. '
            'After you exit the supervisor runs the smoke gate with its own budget: report it as '
            'pending and do not run it just to finish the report. Run a profile during development '
            'only to answer a concrete question; if it is killed or backgrounded, record that and do '
            'not restart it. Cite completed runs by exact path. Do not retry a denied command or claim '
            'unrun checks passed. If permissions, auth or tools are missing, stop with the blocker. '
            'rea_static: open_binary with the registered reference\'s absolute path and provider_id '
            'ghidra, then targeted decompilation, xrefs and instruction reads; call close_binary '
            'without arguments when finished. Opening it may start Ghidra/Java, never the game. '
            + bash +
            'Save git-show output under the task build directory with file tools. '
            'End with changed files, evidence, checks, unknowns and next action.\n')


def successful_transcript(backend, path):
    """CLI exit zero alone can still carry an API/auth/turn failure."""
    completed = False
    with path.open(encoding='utf-8', errors='replace') as stream:
        for line in stream:
            try:
                event = json.loads(line)
            except json.JSONDecodeError:
                continue
            if backend == 'codex':
                if event.get('type') in ('turn.failed', 'error'):
                    return False
                completed |= event.get('type') == 'turn.completed'
            elif event.get('type') == 'result':
                completed = event.get('subtype') == 'success' and not event.get('is_error', False)
                if not completed:
                    return False
    return completed


def control_hashes():
    paths = [ROOT / 'AGENTS.md', ROOT / 'CLAUDE.md', ROOT / 'agent/tools.windows.json',
             ROOT / 'agent/completion.json', ROOT / 'agent/rea-static.runtime.json',
             ROOT / 'agent/rea-static.mcp.json', ROOT / 'agent/claude-research.settings.json']
    paths += list((ROOT / 'tools/agent_control').glob('*.py'))
    return {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in paths if p.exists()}


def source_hashes():
    """Source and evidence fingerprint; generated output stays under build/."""
    manifest = {}
    for folder in ('src', 'cmake', 'scripts', 'tools', 'agent', 'decomp-lemmings', 'docs'):
        base = ROOT / folder
        for directory, dirs, files in os.walk(base):
            dirs[:] = [d for d in dirs if d not in ('build', '.git', '__pycache__', 'node_modules')]
            for name in files:
                path = Path(directory) / name
                if path.suffix.lower() in ('.exe', '.dll', '.box', '.avi', '.bin'):
                    continue
                manifest[path.relative_to(ROOT).as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
    for name in ('AGENTS.md', 'CLAUDE.md', 'CMakeLists.txt', 'CMakePresets.json',
                 'README.md', '.gitignore', '.gitattributes'):
        path = ROOT / name
        if path.exists():
            manifest[name] = hashlib.sha256(path.read_bytes()).hexdigest()
    return manifest


def content_fingerprint():
    manifest = source_hashes()
    manifest.pop('agent/tasks.json', None)
    manifest.pop('agent/RUN_STATUS.md', None)
    manifest.pop('agent/RUN_STATUS.md.tmp', None)
    return hashlib.sha256(json.dumps(manifest, sort_keys=True).encode()).hexdigest()


def capture(run_dir, label, registry):
    git = registry['tools']['git']['path']
    for name, args in [('status', ['status', '--porcelain=v1', '--untracked-files=all']),
                       ('diff', ['diff', '--binary']), ('staged', ['diff', '--cached', '--binary'])]:
        result = subprocess.run([git, '-C', str(ROOT), *args], capture_output=True)
        (run_dir / f'{label}-{name}.txt').write_bytes(result.stdout + result.stderr)
        if result.returncode:
            raise RuntimeError(f'Git {name} capture failed')
    manifest = source_hashes()
    atomic_json(run_dir / f'{label}-source-hashes.json', manifest)


def main(argv=None, *, lease=None, result_sink=None, overall_deadline=float("inf")):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--backend', choices=['claude', 'codex'], required=True)
    parser.add_argument('--task', required=True)
    parser.add_argument('--minutes', type=int, choices=range(1, 61), default=60, metavar='1..60')
    parser.add_argument('--verification-minutes', type=int, choices=range(1, 121), default=60, metavar='1..120')
    parser.add_argument('--execute', action='store_true', help='Actually launch the AI worker')
    parser.add_argument('--allow-dirty', action='store_true', help='Preserve and work alongside existing edits')
    args = parser.parse_args(argv)
    registry = json.loads((ROOT / 'agent/tools.windows.json').read_text())
    queue_bytes = QUEUE.read_bytes()
    queue = json.loads(queue_bytes)
    task = select_task(queue, args.task)
    run_id = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S') + f'-{os.getpid()}-{uuid.uuid4().hex[:8]}'
    run_dir = ROOT / 'build/agent-runs' / run_id
    command = command_for(args.backend, registry, run_dir, task=task)
    reserve = min(600, args.minutes * 60 / 3)
    worker_seconds = args.minutes * 60 - reserve
    if not args.execute:
        print(json.dumps({'mode': 'preview; no worker launched', 'task': task,
                          'command': command, 'worker_budget_setting_minutes': args.minutes,
                          'worker_seconds': worker_seconds,
                          'verification_seconds': args.verification_minutes * 60,
                          'logs': str(run_dir)}, indent=2))
        return 0
    if os.name != 'nt':
        raise RuntimeError('Execution requires Windows')
    stop = ROOT / 'build/agent-runs/STOP'
    if stop.exists():
        raise RuntimeError(f'Remove the existing cancellation file after review: {stop}')
    dirty = subprocess.check_output([registry['tools']['git']['path'], '-C', str(ROOT),
                                     'status', '--porcelain'], text=True)
    if dirty and not args.allow_dirty:
        raise RuntimeError('Repository has edits. Review them, then use --allow-dirty if appropriate.')
    run_dir.parent.mkdir(parents=True, exist_ok=True)
    owned_lease = lease is None
    if owned_lease:
        lease = RepositoryLease(ROOT).__enter__()
    elif lease.root != ROOT.resolve():
        raise RuntimeError('Repository lease belongs to another checkout')
    lease.check()
    result = {'run_id': run_id, 'task': args.task, 'backend': args.backend,
              'started': utc(), 'command': command, 'status': 'interrupted'}
    expected_queue = queue_bytes
    try:
        # Recheck after lock acquisition to avoid dispatching a changed task.
        if QUEUE.read_bytes() != queue_bytes:
            raise RuntimeError('Queue changed before dispatch')
        run_dir.mkdir()
        capture(run_dir, 'before', registry)
        prompt_file = run_dir / 'prompt.txt'
        prompt_file.write_text(make_prompt(task, worker_seconds / 60, args.backend), encoding='utf-8')
        task.update(status='in_progress', run_id=run_id, started=utc())
        atomic_json(QUEUE, queue)
        expected_queue = QUEUE.read_bytes()
        controls_before = control_hashes()
        deadline = min(overall_deadline, time.monotonic() + worker_seconds)
        result['worker'] = run_tree(command, cwd=ROOT, stdin_path=prompt_file,
                                    stdout_path=run_dir / 'worker.jsonl',
                                    stderr_path=run_dir / 'worker.stderr.log',
                                    deadline=deadline, stop_path=stop)
        worker = result['worker']
        if control_hashes() != controls_before:
            raise RuntimeError('Worker changed supervisor controls/instructions; verification refused')
        if QUEUE.read_bytes() != expected_queue:
            raise RuntimeError('Queue changed during worker execution; verification refused')
        if (worker['reason'] == 'exited' and worker['exit_code'] == 0
                and successful_transcript(args.backend, run_dir / 'worker.jsonl')):
            verification = [registry['tools']['python']['path'],
                            str(ROOT / 'tools/agent_control/verify.py'),
                            '--profile', task['verification']]
            verified_content = content_fingerprint()
            result['verification'] = run_tree(verification, cwd=ROOT, stdin_path=None,
                                              stdout_path=run_dir / 'verification.log',
                                              stderr_path=run_dir / 'verification.stderr.log',
                                              deadline=min(overall_deadline, time.monotonic() + args.verification_minutes * 60),
                                              stop_path=stop)
            verified = result['verification']
            if content_fingerprint() != verified_content:
                raise RuntimeError('Source changed during verification')
            result['status'] = ('needs_review' if verified['reason'] == 'exited'
                                and verified['exit_code'] == 0 else 'blocked')
            if result['status'] == 'needs_review':
                result['verified_content'] = verified_content
        elif worker['reason'] == 'exited':
            result['status'] = 'blocked'
        else:
            result['status'] = 'interrupted'
    except KeyboardInterrupt:
        result['error'] = 'Cancelled by keyboard interrupt'
    except Exception as error:
        result['status'] = 'blocked'
        result['error'] = f'{type(error).__name__}: {error}'
    finally:
        result['finished'] = utc()
        if run_dir.exists():
            try:
                capture(run_dir, 'after', registry)
            except Exception as error:
                result['capture_error'] = str(error)
                result['status'] = 'blocked'
            if QUEUE.read_bytes() == expected_queue:
                task.update(status=result['status'], finished=utc(),
                            last_run=run_dir.relative_to(ROOT).as_posix())
                atomic_json(QUEUE, queue)
                result['queue_sha256'] = hashlib.sha256(QUEUE.read_bytes()).hexdigest()
            else:
                result['queue_conflict'] = 'Queue changed externally; preserved without overwrite'
                result['status'] = 'blocked'
            atomic_json(run_dir / 'result.json', result)
            try:
                refresh_status(ROOT)
            except (OSError, ValueError) as error:
                result['status_summary_error'] = str(error)
                atomic_json(run_dir / 'result.json', result)
        if owned_lease:
            lease.__exit__()
    if result_sink is not None:
        result_sink.append(result)
    print(json.dumps(result, indent=2))
    return 0 if result['status'] == 'needs_review' else 1


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, RuntimeError, ValueError, subprocess.CalledProcessError) as error:
        print(f'SUPERVISOR REFUSED: {error}', file=sys.stderr)
        sys.exit(2)
