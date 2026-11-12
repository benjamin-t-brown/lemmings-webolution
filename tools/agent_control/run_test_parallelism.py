"""Preview/start the bounded test-isolation task; stop after review."""
import argparse
import json
import continuous
import supervise

TASK = 'test-parallel-isolation-001'


def run_args(queue, execute=False, backend='claude'):
    if backend not in ('claude', 'codex'):
        raise ValueError('Unknown backend')
    continuous.validate_queue(queue)
    task = next((t for t in queue['tasks'] if t['id'] == TASK), None)
    if not task or task.get('stop_after_review') is not True or task['requires_original_execution']:
        raise ValueError('Task or stop boundary changed; inspect before running')
    if task['status'] == 'done':
        return None
    if task['status'] in ('blocked', 'interrupted', 'in_progress'):
        raise ValueError('Task requires inspection before resuming')
    action, selected = continuous.choose_action(queue)
    if action not in ('implement', 'review') or not selected or selected['id'] != TASK:
        raise ValueError('Next action is outside the scoped workflow task')
    args = ['--backend', backend, '--minutes', '60', '--verification-minutes', '90',
            '--review-minutes', '20', '--max-attempts', '3', '--max-cycles', '8']
    return args + (['--execute', '--allow-dirty'] if execute else [])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--execute', action='store_true')
    parser.add_argument('--backend', choices=['claude', 'codex'], default='claude')
    args = parser.parse_args()
    launch = run_args(json.loads(supervise.QUEUE.read_text(encoding='utf-8')), args.execute, args.backend)
    if launch is None:
        print('Test-isolation task already accepted; no further work launched.')
        return 0
    return continuous.main(launch)


if __name__ == '__main__':
    raise SystemExit(main())
