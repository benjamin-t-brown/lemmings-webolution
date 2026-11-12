"""Preview/start the three-stage visible-fluid milestone. Never launches the original."""
import argparse
import json
import continuous
import supervise

TASKS = ('fluid-surface-research-001', 'fluid-surface-impl-001', 'fluid-lever-assessment-001')


def milestone_args(queue, execute=False, backend="claude"):
    if backend not in ("codex", "claude"):
        raise ValueError("Unknown milestone backend")
    continuous.validate_queue(queue)
    by_id = {t['id']: t for t in queue['tasks']}
    if any(name not in by_id for name in TASKS):
        raise ValueError('Milestone queue is incomplete; inspect before running')
    for i, name in enumerate(TASKS):
        task = by_id[name]
        if task.get('stop_after_review') is not (i == len(TASKS) - 1):
            raise ValueError('Milestone review boundaries changed; inspect queue')
        expected = [TASKS[i - 1]] if i else ['lever-initial-face-impl-001']
        if task.get('depends_on') != expected or task.get('requires_original_execution') is not False:
            raise ValueError('Milestone dependencies or execution boundary changed')
    if all(by_id[name]['status'] == 'done' for name in TASKS):
        return None
    if any(by_id[name]['status'] in ('blocked', 'interrupted', 'in_progress') for name in TASKS):
        raise ValueError('Milestone has blocked/interrupted/active work; inspect before resuming')
    action, task = continuous.choose_action(queue)
    if action not in ('implement', 'review') or not task or task['id'] not in TASKS:
        raise ValueError('Next action is outside this milestone; inspect queue')
    args = ['--backend', backend, '--minutes', '60', '--verification-minutes', '90',
            '--review-minutes', '20', '--max-attempts', '3', '--max-cycles', '18']
    if execute:
        args += ['--execute', '--allow-dirty']
    return args


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--execute', action='store_true')
    parser.add_argument('--backend', choices=['codex', 'claude'], default='claude')
    args = parser.parse_args()
    queue = json.loads(supervise.QUEUE.read_text(encoding='utf-8'))
    launch = milestone_args(queue, args.execute, args.backend)
    if launch is None:
        print('Visible-fluid milestone already accepted; no further work launched.')
        return 0
    return continuous.main(launch)


if __name__ == '__main__':
    raise SystemExit(main())
