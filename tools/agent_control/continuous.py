"""Plan, implement, verify and independently review tasks until done or blocked."""
import argparse
import contextlib
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import time
import uuid

import supervise as worker
from winjob import run_tree


def obj(properties):
    return {'type': 'object', 'properties': properties,
            'required': list(properties), 'additionalProperties': False}


TEXT = {'type': 'string', 'minLength': 1}
STRINGS = {'type': 'array', 'items': TEXT}
EVIDENCE = obj({'path': TEXT, 'detail': TEXT})
CHECKS = {'type': 'array', 'items': obj({
    'criterion': TEXT, 'passed': {'type': 'boolean'},
    'evidence': {'type': 'array', 'items': EVIDENCE}})}
TASK = obj({'id': TEXT, 'title': TEXT, 'scope': TEXT,
            'runtime_change': TEXT,
            'production_files': {'type': 'array', 'items': TEXT, 'minItems': 1},
            'acceptance': {'type': 'array', 'items': TEXT, 'minItems': 1},
            'depends_on': STRINGS, 'requires_original_execution': {'type': 'boolean'},
            'verification': {'enum': ['main', 'recovery', 'all']}})
PLAN = obj({'decision': {'enum': ['task', 'complete', 'blocked']}, 'reason': TEXT,
            'task': {'anyOf': [TASK, {'type': 'null'}]}, 'checks': CHECKS})
REVIEW = obj({'decision': {'enum': ['accept', 'revise', 'blocked']},
              'reason': TEXT, 'checks': CHECKS})


def schema_with_criteria(schema, criteria):
    """Constrain the provider to literal criterion labels, never paraphrases."""
    if not criteria or len(criteria) != len(set(criteria)):
        raise ValueError('Review criteria must be nonempty and distinct')
    bound = json.loads(json.dumps(schema))
    bound['properties']['checks']['items']['properties']['criterion'] = {
        'type': 'string', 'enum': list(criteria)}
    return bound


class StopLoop(RuntimeError):
    def __init__(self, status, reason):
        self.status = status
        super().__init__(reason)


def format_duration(seconds):
    hours, remainder = divmod(max(0, int(seconds)), 3600)
    minutes, seconds = divmod(remainder, 60)
    return f'{hours}h {minutes:02d}m {seconds:02d}s'


def claude_structured_result(response):
    """Claude JSON is a result object, or a verbose array ending in that result."""
    events = response if isinstance(response, list) else [response]
    if not events or any(not isinstance(event, dict) for event in events):
        raise ValueError('Claude response must contain JSON event objects')
    if any(event.get('type') == 'error' or event.get('is_error') for event in events):
        raise ValueError('Claude response reported an error; inspect response.jsonl')
    results = [event for event in events if event.get('type') == 'result']
    if len(results) != 1 or events[-1] is not results[0]:
        raise ValueError('Claude response must end with exactly one result event')
    result = results[0]
    if result.get('subtype') != 'success':
        raise ValueError('Claude result was not successful; inspect response.jsonl')
    answer = result.get('structured_output')
    if not isinstance(answer, dict):
        raise ValueError('Claude result is missing a structured output object')
    return answer


def validate(value, schema):
    """Validate the small closed schema vocabulary used above, without packages."""
    if 'anyOf' in schema:
        for alternative in schema['anyOf']:
            try:
                validate(value, alternative)
                return
            except ValueError:
                pass
        raise ValueError('Response does not match any allowed schema')
    if 'enum' in schema and value not in schema['enum']:
        raise ValueError(f'Invalid decision/value: {value!r}')
    kind = schema.get('type')
    expected = {'object': dict, 'array': list, 'string': str, 'boolean': bool, 'null': type(None)}
    if kind and type(value) is not expected[kind]:
        raise ValueError(f'Expected {kind}')
    if kind == 'object':
        if set(value) != set(schema['required']):
            raise ValueError('Missing or unexpected response fields')
        for key, child in schema['properties'].items():
            validate(value[key], child)
    elif kind == 'array':
        if len(value) < schema.get('minItems', 0):
            raise ValueError('Empty required list')
        for item in value:
            validate(item, schema['items'])
    elif kind == 'string' and not value.strip():
        raise ValueError('Empty required text')


def validate_queue(queue):
    tasks = queue['tasks']
    ids = [task['id'] for task in tasks]
    if len(ids) != len(set(ids)):
        raise ValueError('Duplicate task IDs')
    by_id = {task['id']: task for task in tasks}
    visited, visiting = set(), set()

    def visit(task_id):
        if task_id in visiting:
            raise ValueError('Cyclic task dependencies')
        if task_id in visited:
            return
        if task_id not in by_id:
            raise ValueError(f'Missing dependency: {task_id}')
        visiting.add(task_id)
        task = by_id[task_id]
        if not re.fullmatch(r'[a-zA-Z0-9_-]+', task_id):
            raise ValueError('Invalid task ID')
        if task['status'] not in ('ready', 'queued', 'in_progress', 'needs_review',
                                  'done', 'blocked', 'interrupted'):
            raise ValueError('Invalid task status')
        if 'stop_after_review' in task and type(task['stop_after_review']) is not bool:
            raise ValueError('stop_after_review must be boolean')
        if type(task.get('requires_original_execution')) is not bool:
            raise ValueError('Task must explicitly declare its execution boundary')
        for dependency in task.get('depends_on', []):
            visit(dependency)
        visiting.remove(task_id)
        visited.add(task_id)

    for task_id in ids:
        visit(task_id)


def dependencies_done(task, queue):
    by_id = {t['id']: t for t in queue['tasks']}
    return all(by_id[d]['status'] == 'done' for d in task.get('depends_on', []))


def choose_action(queue):
    validate_queue(queue)
    for task in queue['tasks']:
        if task['status'] in ('in_progress', 'interrupted'):
            return 'interrupted', task
    for task in queue['tasks']:
        if (task['status'] == 'needs_review' and not task['requires_original_execution']
                and dependencies_done(task, queue)):
            return 'review', task
    for task in queue['tasks']:
        if (task['status'] in ('ready', 'queued') and not task['requires_original_execution']
                and not task.get('planning_required') and dependencies_done(task, queue)):
            return 'implement', task
    return 'plan', None


def require_checks(checks, criteria, root):
    if len(checks) != len(criteria) or {c['criterion'] for c in checks} != set(criteria):
        received = [c['criterion'] for c in checks]
        missing = [c for c in criteria if c not in received]
        unexpected = [c for c in received if c not in criteria]
        raise ValueError('Review must cover every exact acceptance criterion once: '
                         f'{len(criteria)} expected, {len(checks)} received; '
                         f'missing={missing!r}; unexpected={unexpected!r}')
    for check in checks:
        if not check['passed'] or not check['evidence']:
            raise ValueError('Acceptance requires passing checks with evidence')
        for evidence in check['evidence']:
            path = Path(evidence['path'])
            resolved = (root / path).resolve()
            if path.is_absolute() or not resolved.is_relative_to(root.resolve()) or not resolved.is_file():
                raise ValueError(f'Evidence must name an existing repo-relative file: {path}')


def apply_plan(queue, proposal, max_tasks):
    validate(proposal, PLAN)
    if proposal['decision'] != 'task' or proposal['task'] is None:
        raise ValueError('Expected one concrete next task')
    task = proposal['task']
    implementation_files = []
    for name in task['production_files']:
        path = Path(name)
        parts = name.replace('\\', '/').split('/')
        if path.is_absolute() or '..' in parts or ':' in name:
            raise ValueError('Production files must be repo-relative paths')
        if any(p in ('tests', '__test__', 'tools', 'build', 'docs') for p in parts):
            raise ValueError('Tests, tools and documentation are not production runtime files')
        normalized = '/'.join(parts)
        if (normalized.startswith(('src/', 'decomp-lemmings/src/'))
                and path.suffix.lower() in ('.cpp', '.cc', '.cxx', '.c')
                and not path.name.startswith('test_')):
            implementation_files.append(name)
    if not implementation_files:
        raise ValueError('Next task must change a production runtime implementation, not only tests or documentation')
    if task['requires_original_execution']:
        raise StopLoop('blocked', 'The proposed task requires original-game execution approval')
    if len(set(task['acceptance'])) != len(task['acceptance']):
        raise ValueError('Acceptance criteria must be distinct')
    existing = next((t for t in queue['tasks'] if t['id'] == task['id']), None)
    if existing:
        if (not existing.get('planning_required') or existing['requires_original_execution']
                or existing['status'] not in ('queued', 'blocked') or existing.get('loop_attempts', 0)):
            raise ValueError('Planner cannot replace completed, protected or attempted tasks')
        # A planning placeholder retains its dependency requirements.
        if not set(existing.get('depends_on', [])) <= set(task['depends_on']):
            raise ValueError('Planner removed a required dependency')
    elif len(queue['tasks']) >= max_tasks:
        raise StopLoop('limit_reached', 'Task queue size limit reached')
    by_id = {t['id']: t for t in queue['tasks']}
    if any(d not in by_id or by_id[d]['status'] != 'done' for d in task['depends_on']):
        raise ValueError('Proposed task must have completed dependencies')
    candidate = dict(task, status='ready', planning_required=False, loop_attempts=0)
    runtime_acceptance = ('Production implementation changes in ' + ', '.join(implementation_files)
                          + ' deliver this runtime behavior: ' + task['runtime_change']
                          + '. Independent review must trace the real call path and before/after '
                          'behavior; test-only changes, dead code, comments or build registration '
                          'alone do not satisfy this criterion.')
    candidate['acceptance'] = list(dict.fromkeys(candidate['acceptance'] + [runtime_acceptance]))
    for other in queue['tasks']:
        if other is not existing and other.get('scope', '').strip() == task['scope'].strip():
            raise ValueError('Planner repeated an existing task scope')
    if existing:
        candidate['acceptance'] = list(dict.fromkeys(existing['acceptance'] + candidate['acceptance']))
        existing.pop('blocker', None)
        existing.update(candidate)
    else:
        queue['tasks'].append(candidate)
    validate_queue(queue)


class Controller:
    def __init__(self, args, registry, lease, run_dir):
        self.started_monotonic = time.monotonic()
        self.started_at = worker.utc()
        self.args, self.registry, self.lease, self.run_dir = args, registry, lease, run_dir
        self.root, self.queue_path = worker.ROOT, worker.QUEUE
        self.expected = self.queue_path.read_bytes()
        self.queue = json.loads(self.expected)
        self.controls = worker.control_hashes()
        self.completion = json.loads((self.root / 'agent/completion.json').read_text())
        self.criteria = self.completion['criteria']
        if not self.criteria or len(self.criteria) != len(set(self.criteria)):
            raise ValueError('Completion criteria must be nonempty and distinct')
        self.stop = self.root / 'build/agent-runs/STOP'
        self.deadline = (time.monotonic() + args.total_minutes * 60
                         if args.total_minutes else float('inf'))
        self.state = {'status': 'running', 'cycles': 0, 'completed_tasks': [], 'events': []}
        self.state['started_at'] = self.started_at
        self.state['requested_models'] = {
            'worker': worker.model_settings(args.backend, registry),
            'planner_and_reviewer': worker.model_settings(args.review_backend or args.backend, registry)}

    def check(self):
        self.lease.check()
        if self.stop.exists():
            raise StopLoop('cancelled', 'STOP requested')
        if time.monotonic() >= self.deadline:
            raise StopLoop('limit_reached', 'Total time limit reached')
        if self.queue_path.read_bytes() != self.expected:
            raise StopLoop('blocked', 'Queue changed externally; preserved without overwrite')
        if worker.control_hashes() != self.controls:
            raise StopLoop('blocked', 'Supervisor controls or completion criteria changed')

    def save(self):
        self.check()
        validate_queue(self.queue)
        worker.atomic_json(self.queue_path, self.queue)
        self.expected = self.queue_path.read_bytes()
        worker.refresh_status(self.root)

    def update_duration(self):
        elapsed = round(max(0.0, time.monotonic() - self.started_monotonic), 3)
        self.state.update(elapsed_seconds=elapsed, elapsed=format_duration(elapsed))

    def event(self, phase, **details):
        self.update_duration()
        event = {'phase': phase, **details, 'elapsed': self.state['elapsed'],
                 'elapsed_seconds': self.state['elapsed_seconds']}
        self.state['events'].append(event)
        worker.atomic_json(self.run_dir / 'result.json', self.state)
        print(json.dumps(event), flush=True)

    def phase_directory(self, phase):
        path = self.run_dir / f'{self.state["cycles"]:04d}-{phase}-{uuid.uuid4().hex[:6]}'
        path.mkdir()
        return path

    def phase_result(self, result, label):
        if result['reason'] == 'cancelled':
            raise StopLoop('cancelled', f'{label} cancelled')
        if result['reason'] == 'timeout':
            raise StopLoop('interrupted', f'{label} timed out; no automatic restart')
        if result['exit_code'] != 0:
            raise StopLoop('blocked', f'{label} failed; inspect the saved logs')

    def verify(self, profile):
        self.check()
        phase = self.phase_directory('verify')
        before = worker.content_fingerprint()
        result = run_tree([self.registry['tools']['python']['path'],
                           str(self.root / 'tools/agent_control/verify.py'), '--profile', profile],
                          cwd=self.root, stdin_path=None, stdout_path=phase / 'verification.log',
                          stderr_path=phase / 'stderr.log', stop_path=self.stop,
                          deadline=min(self.deadline, time.monotonic() + getattr(getattr(self, "args", None), "verification_minutes", 60) * 60))
        self.phase_result(result, 'Verification')
        self.check()
        if worker.content_fingerprint() != before:
            raise StopLoop('blocked', 'Source changed during verification')
        return {'profile': profile, 'log': (phase / 'verification.log').relative_to(self.root).as_posix(),
                'verified_content': before}

    def ask(self, phase_name, payload, schema):
        self.check()
        criteria = (payload['task']['acceptance'] if 'task' in payload
                    else payload['completion_criteria'])
        schema = schema_with_criteria(schema, criteria)
        phase = self.phase_directory(phase_name)
        worker.capture(phase, 'before', self.registry)
        before = worker.source_hashes()
        prompt = ((self.root / 'AGENTS.md').read_text(encoding='utf-8') + '\n\n'
                  'You are a separate READ-ONLY ' + phase_name + ' pass. Read relevant source, '
                  'evidence and logs. Do not edit files or run builds, tests, the original game, '
                  'original-byte oracles, hooks, debuggers or launchers. Do not delegate. '
                  'On the Codex backend your shell runs in a read-only sandbox and is your file reader: '
                  'use it for read commands only (Get-Content, Select-String, rg, dir, git show/diff/log/status) '
                  'and view retained images directly; that is permitted and expected, and lacking another '
                  'reader is not a blocker. '
                  'Return only the requested JSON. Report blockers truthfully. Passing source '
                  'tests alone does not prove original-game fidelity. Never reopen user-parked '
                  'work or relax acceptance criteria. Evidence paths must be existing repo-relative '
                  'files; explain what each proves in detail.\n'
                  'The rea_static MCP profile permits targeted static reference lookups: planners may check feasibility and reviewers may independently check claimed binary behavior. Read retained evidence first; do not turn this pass into open-ended research. Open only the registered reference with provider_id ghidra; analysis may start Ghidra/Java but never the game. Check pseudocode against instructions and close_binary without arguments. Cite addresses and returned findings in JSON; tool results are retained in this phase response.jsonl, so do not write evidence files. Static findings do not prove runtime fidelity. '
                  'For planning: propose one concrete unparked task with completed dependencies, '
                  'or refine an explicitly planning_required placeholder; never replace other tasks. '
                  'In each proposed task scope and plan reason record Difficulty (routine/difficult/exceptional), Recommended model (Sonnet/Opus/Fable), and the evidence-based reason, following AGENTS.md. Recommendations are advisory: explicit user model overrides always win; never change pins or automatically escalate. Recommend bounded Fable use only for a specific unresolved reasoning dependency, with explicit user approval unless already authorized for that scope. Quota, slow tests, tool failures and interruptions do not justify escalation. '
                  'Prioritize missing native runtime functionality and integration. Each task '
                  'must name runtime_change (a concrete before/after behavior) and production_files '
                  '(including a production C++ implementation). Trace the current runtime call '
                  'path and retained evidence before choosing it. Tests and documentation support '
                  'that change; do not propose standalone test expansions, ledger cleanup, '
                  'byte-hash additions, coverage recounts or build-registration-only work. '
                  'Prioritize rendering and gameplay breadth before further audio: evidence-backed scenery roots beyond MM_CLIMB_002, selected-hatch camera entry, serialized teleport links after an installed-level mismatch is established, then evidenced gameplay commands respecting timing parks. Follow agent/IMPLEMENTATION_PLAN.md. Do not remove guards blindly or add per-level offset tables. Accepted scene-roots-001 research must lead to production scenery integration; source tests and injected wins do not prove complete gameplay fidelity. Prioritize the playable milestone: startup, select an existing installed level, play, finish, and retain progress after clone restart. Read accepted reports with docs/STATUS.md; do not redo completed integrations. In task scope name the milestone step, actual affected level/resource or application route, observed or source-traced failure, and why this is the next dependency. Acceptance must demonstrate that route improving. Defer absent variants and synthetic-only edge cases unless they block this milestone. Require an explicit persistence contract; do not equate custom saves with retail compatibility or overwrite installed saves. Surface unresolved decisions instead of unrelated tasks. '
                  'The manually queued, user-authorized playable-baseline-001, playable-expansion-001 and targeted-play-001 assessments are scoped exceptions to production-change requirements. Review its observation criteria without demanding gameplay fixes or fabricated success; executed failures can establish a baseline but cannot establish playability. Do not create further assessment tasks without user authorization. ' 'Explicitly user-authorized queued research is a scoped exception to production-change requirements; review it against its research acceptance, not an invented C++ change. Its acceptance does not prove runtime completion. For new planner proposals, ' 'If only prerequisite research or testing is possible, report the exact '
                  'implementation it would unblock and decision blocked instead of inventing '
                  'functionality or silently returning to housekeeping. '
                  'Use decision blocked if no authorized useful work remains. Complete requires '
                  'all tasks done AND evidence for every completion criterion, not merely an empty queue.\n'
                  'For review: inspect acceptance independently. Use revise for specific fixable '
                  'issues, blocked for missing authorization/access or evidence that cannot be obtained. '
                  'Accept only with every exact criterion covered and supported.\n'
                  'Copy each checks[].criterion verbatim from the schema enum; never '
                  'shorten, paraphrase, combine or duplicate criteria. An accept/complete '
                  'decision requires exactly one passing check per criterion, with evidence.\n'
                  f'Context: {json.dumps(payload, indent=2)}\n'
                  f'Inspection snapshots: {phase.relative_to(self.root).as_posix()}/before-*.txt\n')
        (phase / 'prompt.txt').write_text(prompt, encoding='utf-8')
        worker.atomic_json(phase / 'schema.json', schema)
        backend = self.args.review_backend or self.args.backend
        command = worker.command_for(backend, self.registry, phase, read_only=True,
                                      schema=phase / 'schema.json', role=phase_name)
        minutes = self.args.plan_minutes if phase_name == 'plan' else self.args.review_minutes
        result = run_tree(command, cwd=self.root, stdin_path=phase / 'prompt.txt',
                          stdout_path=phase / 'response.jsonl', stderr_path=phase / 'stderr.log',
                          deadline=min(self.deadline, time.monotonic() + minutes * 60), stop_path=self.stop)
        self.phase_result(result, phase_name)
        self.check()
        if worker.source_hashes() != before:
            raise StopLoop('blocked', f'Read-only {phase_name} changed source or evidence')
        if backend == 'codex':
            if not worker.successful_transcript(backend, phase / 'response.jsonl'):
                raise StopLoop('blocked', f'{phase_name} did not finish successfully')
            answer = json.loads((phase / 'last-message.txt').read_text(encoding='utf-8'))
        else:
            response = json.loads((phase / 'response.jsonl').read_text(encoding='utf-8'))
            answer = claude_structured_result(response)
        validate(answer, schema)
        worker.atomic_json(phase / 'decision.json', answer)
        self.event(phase_name, decision=answer['decision'],
                   evidence=(phase / 'decision.json').relative_to(self.root).as_posix())
        return answer

    def task_verification(self, task):
        if task.get('last_run'):
            path = (self.root / task['last_run'] / 'result.json').resolve()
            if path.is_relative_to(self.root.resolve()) and path.is_file():
                result = json.loads(path.read_text())
                if (result.get('status') == 'needs_review'
                        and result.get('verified_content') == worker.content_fingerprint()):
                    return {'log': str(Path(task['last_run']) / 'verification.log'),
                            'verified_content': result['verified_content']}
        return self.verify(task['verification'])

    def review(self, task):
        verification = self.task_verification(task)
        answer = self.ask('review', {'task': task, 'verification': verification}, REVIEW)
        if answer['decision'] == 'accept':
            require_checks(answer['checks'], task['acceptance'], self.root)
            task.update(status='done', review=answer, accepted_at=worker.utc(),
                        accepted_verification=verification)
            self.state['completed_tasks'].append(task['id'])
            self.save()
            self.event('accepted', task=task['id'])
            if task.get('stop_after_review') is True:
                raise StopLoop('milestone_reached',
                               f'Accepted {task["id"]}; stopped at the requested review boundary')
        elif answer['decision'] == 'revise':
            task['review_feedback'] = answer
            self.retry(task, answer['reason'])
        else:
            task.update(status='blocked', blocker=answer['reason'], review=answer)
            self.save()
            raise StopLoop('blocked', answer['reason'])

    def retry(self, task, reason):
        task['last_failure'] = reason
        if task.get('loop_attempts', 0) >= self.args.max_attempts:
            task.update(status='blocked', blocker='Attempt limit: ' + reason)
            self.save()
            raise StopLoop('blocked', f'Attempt limit reached for {task["id"]}')
        task['status'] = 'ready'
        self.save()
        self.event('retry_queued', task=task['id'], reason=reason)

    def implement(self, task):
        if task.get('loop_attempts', 0) >= self.args.max_attempts:
            raise StopLoop('blocked', f'Attempt limit reached for {task["id"]}')
        minutes = min(self.args.minutes, int((self.deadline - time.monotonic()) / 60)
                      if self.deadline != float('inf') else self.args.minutes)
        if minutes < 1:
            raise StopLoop('limit_reached', 'Less than one minute remains')
        task.update(status='ready', loop_attempts=task.get('loop_attempts', 0) + 1)
        self.save()
        self.event('implement', task=task['id'], attempt=task['loop_attempts'])
        results = []
        phase = self.phase_directory('worker')
        with (phase / 'supervisor.log').open('w', encoding='utf-8') as log, contextlib.redirect_stdout(log):
            worker.main(['--backend', self.args.backend, '--task', task['id'], '--minutes', str(minutes),
                         '--verification-minutes', str(getattr(self.args, 'verification_minutes', 60)),
                         '--execute', '--allow-dirty'], lease=self.lease, result_sink=results,
                        overall_deadline=self.deadline)
        result = results[0]
        if hashlib.sha256(self.queue_path.read_bytes()).hexdigest() != result.get('queue_sha256'):
            raise StopLoop('blocked', 'Queue/control conflict after worker; inspect logs')
        self.expected = self.queue_path.read_bytes()
        self.queue = json.loads(self.expected)
        task = next(t for t in self.queue['tasks'] if t['id'] == task['id'])
        if result['status'] == 'needs_review':
            return
        verification = result.get('verification', {})
        if (verification.get('reason') == 'exited' and verification.get('exit_code') != 0
                and not any(k in result for k in ('error', 'capture_error', 'queue_conflict'))):
            self.retry(task, 'Verification failed; inspect ' + task['last_run'] + '/verification.log')
            return
        reason = result.get('worker', {}).get('reason')
        if reason == 'cancelled' or self.stop.exists():
            raise StopLoop('cancelled', 'Worker cancelled')
        raise StopLoop('interrupted' if result['status'] == 'interrupted' else 'blocked',
                       'Worker stopped; inspect ' + task.get('last_run', 'run logs'))

    def plan(self):
        proposal = self.ask('plan', {'queue': self.queue, 'completion_criteria': self.criteria,
                                     'plan': 'agent/IMPLEMENTATION_PLAN.md'}, PLAN)
        if proposal['decision'] == 'blocked':
            raise StopLoop('blocked', proposal['reason'])
        if proposal['decision'] == 'task':
            apply_plan(self.queue, proposal, self.args.max_tasks)
            self.save()
            return
        if any(t['status'] != 'done' for t in self.queue['tasks']):
            raise StopLoop('blocked', 'Completion was claimed with unfinished tasks')
        require_checks(proposal['checks'], self.criteria, self.root)
        verification = self.verify('all')
        review = self.ask('review_completion', {'completion_criteria': self.criteria,
                                               'proposal': proposal, 'verification': verification,
                                               'queue': self.queue}, REVIEW)
        if review['decision'] != 'accept':
            if review['decision'] == 'blocked':
                raise StopLoop('blocked', 'Completion review: ' + review['reason'])
            self.queue['completion_rejections'] = self.queue.get('completion_rejections', 0) + 1
            self.queue['planning_feedback'] = review
            self.save()
            if self.queue['completion_rejections'] >= self.args.max_attempts:
                raise StopLoop('blocked', 'Repeated completion review failures')
            return
        require_checks(review['checks'], self.criteria, self.root)
        raise StopLoop('complete', 'All tasks, full verification and completion review passed')

    def run(self):
        try:
            while True:
                self.check()
                if self.args.max_cycles and self.state['cycles'] >= self.args.max_cycles:
                    raise StopLoop('limit_reached', 'Cycle limit reached; queue preserved')
                self.state['cycles'] += 1
                action, task = choose_action(self.queue)
                self.event('dispatch', action=action, task=task['id'] if task else None)
                if action == 'interrupted':
                    raise StopLoop('interrupted', f'Review interrupted task {task["id"]} before resuming')
                if action == 'review':
                    self.review(task)
                elif action == 'implement':
                    self.implement(task)
                else:
                    self.plan()
        except StopLoop as stopped:
            self.state.update(status=stopped.status, reason=str(stopped))
        except KeyboardInterrupt:
            self.state.update(status='cancelled', reason='Keyboard interrupt')
        except Exception as error:
            self.state.update(status='blocked', reason=f'{type(error).__name__}: {error}')
        finally:
            self.update_duration()
            self.state['finished_at'] = worker.utc()
            worker.atomic_json(self.run_dir / 'result.json', self.state)
            try:
                worker.refresh_status(self.root)
            except (OSError, ValueError) as error:
                self.state['status_summary_error'] = str(error)
                worker.atomic_json(self.run_dir / 'result.json', self.state)
        return self.state


def positive(value):
    number = int(value)
    if number < 1:
        raise argparse.ArgumentTypeError('Must be positive')
    return number


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--backend', choices=['codex', 'claude'], required=True)
    parser.add_argument('--review-backend', choices=['codex', 'claude'])
    parser.add_argument('--minutes', type=int, choices=range(1, 61), default=60, metavar='1..60')
    parser.add_argument('--verification-minutes', type=int, choices=range(1, 121), default=60, metavar='1..120')
    parser.add_argument('--review-minutes', type=int, choices=range(1, 61), default=10, metavar='1..60')
    parser.add_argument('--plan-minutes', type=int, choices=range(1, 61), default=10, metavar='1..60')
    parser.add_argument('--max-attempts', type=positive, default=3)
    parser.add_argument('--max-tasks', type=positive, default=1000)
    parser.add_argument('--max-cycles', type=int, default=0, help='0 continues until complete or blocked')
    parser.add_argument('--total-minutes', type=positive)
    parser.add_argument('--execute', action='store_true')
    parser.add_argument('--allow-dirty', action='store_true')
    args = parser.parse_args(argv)
    if args.max_cycles < 0:
        parser.error('--max-cycles must be zero or positive')
    queue = json.loads(worker.QUEUE.read_text())
    action, task = choose_action(queue)
    completion = json.loads((worker.ROOT / 'agent/completion.json').read_text())
    registry = json.loads((worker.ROOT / 'agent/tools.windows.json').read_text())
    models = {'worker': worker.model_settings(args.backend, registry),
              'planner_and_reviewer': worker.model_settings(args.review_backend or args.backend, registry)}
    if not args.execute:
        print(json.dumps({'mode': 'preview; no AI launched', 'next_action': action,
                          'task': task['id'] if task else None, 'settings': vars(args),
                          'requested_models': models, 'completion': completion}, indent=2))
        return 0
    dirty = subprocess.check_output([registry['tools']['git']['path'], '-C', str(worker.ROOT),
                                     'status', '--porcelain'], text=True)
    if dirty and not args.allow_dirty:
        raise RuntimeError('Review existing changes and use --allow-dirty to preserve them alongside this run')
    with worker.RepositoryLease(worker.ROOT) as lease:
        run_dir = worker.ROOT / 'build/agent-runs' / ('loop-' + uuid.uuid4().hex)
        run_dir.mkdir()
        controller = Controller(args, registry, lease, run_dir)
        result = controller.run()
    print(json.dumps({'result': result, 'logs': str(run_dir)}, indent=2))
    print(f'Run duration: {result["elapsed"]} | Status: {result["status"]} | '
          f'Tasks completed this run: {len(result["completed_tasks"])}', flush=True)
    return 0 if result['status'] == 'complete' else 1


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, RuntimeError, ValueError) as error:
        print(f'CONTINUOUS RUN REFUSED: {error}', file=sys.stderr)
        sys.exit(2)
