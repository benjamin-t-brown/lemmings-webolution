"""Derived run ledger. Narrative roadmap and acceptance evidence stay authoritative."""
import json
import os
from pathlib import Path


def refresh(root):
    root = Path(root)
    queue_path = root / 'agent/tasks.json'
    queue = json.loads(queue_path.read_text(encoding='utf-8'))
    tasks = queue['tasks']
    lines = ['# Current run status (generated)', '',
             'Derived from tasks.json and the latest loop result; do not edit manually.',
             'Task acceptance is not project completion or proof of gameplay fidelity.',
             'Read IMPLEMENTATION_PLAN.md for priorities and docs/STATUS.md for the fidelity ledger.', '',
             '## Queue', '']
    pending = [t for t in tasks if t['status'] != 'done']
    for t in pending:
        lines.append(f'- {t["id"]}: **{t["status"]}** — {t.get("title", t["id"])}')
    if not pending:
        lines.append('No unfinished queue entries. This does not establish project completion.')
    lines += ['', '## Recently completed queue entries', '']
    for t in [t for t in tasks if t['status'] == 'done'][-5:]:
        lines.append(f'- {t["id"]}: {t.get("title", t["id"])}')
        if t.get('accepted_at'):
            lines.append(f'  Accepted: {t["accepted_at"]}')
        # The acceptance verifier may be newer than last_run after a review-only resume.
        logs = set()
        verification = t.get('accepted_verification', {})
        if verification.get('log'):
            logs.add(verification['log'])
        for check in t.get('review', {}).get('checks', []):
            for evidence in check.get('evidence', []):
                if evidence.get('path', '').endswith('verification.log'):
                    logs.add(evidence['path'])
        for log in sorted(logs):
            lines.append(f'  Verification evidence: `{log}`')
    results = list((root / 'build/agent-runs').glob('loop-*/result.json'))
    if results:
        path = max(results, key=lambda p: p.stat().st_mtime_ns)
        result = json.loads(path.read_text(encoding='utf-8'))
        lines += ['', '## Latest loop', '', f'- Result: `{path.relative_to(root).as_posix()}`',
                  f'- Status: **{result.get("status", "unknown")}**',
                  f'- Duration: {result.get("elapsed", "not recorded")}',
                  '- Completed in this invocation: ' + (', '.join(result.get('completed_tasks', [])) or 'none')]
        reason = result.get('reason', '')
        if reason:
            # Keep the overview bounded; full reason remains in the linked result.
            lines.append('- Reason: ' + ' '.join(reason.split())[:500])
    lines += ['', '## Documentation handoff', '',
              'At each accepted milestone or priority change, reconcile IMPLEMENTATION_PLAN.md and',
              'docs/STATUS.md with the accepted review and its verification log. Keep detailed',
              'per-run reports historical; append corrections rather than replacing their evidence.',
              'A generated ledger does not update narrative conclusions or historical fidelity gates.', '']
    destination = root / 'agent/RUN_STATUS.md'
    temporary = destination.with_suffix('.md.tmp')
    temporary.write_text('\n'.join(lines), encoding='utf-8', newline='\n')
    os.replace(temporary, destination)
