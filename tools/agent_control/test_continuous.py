"""Synthetic loop tests: no provider calls or original-game execution."""
import argparse
import contextlib
import copy
import io
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

import continuous as loop
import supervise as worker

MODELS = {'codex': {'model': 'gpt-6.1-sol', 'effort': 'high'},
          'claude': {'model': 'claude-opus-5', 'effort': 'high'}}


def task(name='first', status='queued', deps=()):
    return dict(id=name, title=name, scope='Implement ' + name,
                runtime_change='Connect ' + name + ' to the native application',
                production_files=['src/app/feature.cpp'],
                acceptance=['Evidence for ' + name], depends_on=list(deps),
                requires_original_execution=False, verification='all', status=status)


def checks(criteria):
    return [dict(criterion=c, passed=True,
                 evidence=[dict(path='src/evidence.txt', detail='Inspected supporting source')])
            for c in criteria]


class Harness:
    def __init__(self, root, tasks, backend='codex', **options):
        self.root, self.backend = root, backend
        for folder in ('agent', 'tools/agent_control', 'src', 'build/agent-runs/loop-test'):
            (root / folder).mkdir(parents=True, exist_ok=True)
        (root / 'AGENTS.md').write_text('Do not execute the original game.')
        (root / 'src/evidence.txt').write_text('source evidence')
        self.registry = {'tools': {name: {'path': sys.executable}
                                  for name in ('python', 'git', 'codex', 'claude')}}
        self.registry['agent_models'] = copy.deepcopy(MODELS)
        worker.atomic_json(root / 'agent/tools.windows.json', self.registry)
        worker.atomic_json(root / 'agent/completion.json', {'criteria': ['Native goal']})
        worker.atomic_json(root / 'agent/tasks.json', {'tasks': tasks})
        self.args = argparse.Namespace(backend=backend, review_backend=None, minutes=1,
            review_minutes=1, plan_minutes=1, total_minutes=None, max_cycles=30,
            max_attempts=3, max_tasks=100, **{})
        for key, value in options.items():
            setattr(self.args, key, value)
        self.calls, self.worker_count, self.verify_count = [], 0, 0
        self.review_decision = 'accept'
        self.plan_hook = self.review_hook = self.worker_hook = None
        self.verify_failures = 0
        self.fail_worker = False
        self.claude_verbose = True

    def command(self, backend, registry, phase, **kwargs):
        return [backend, str(phase), 'read' if kwargs.get('read_only') else 'write']

    def runner(self, command, **kw):
        out = kw['stdout_path']
        result = {'reason': 'exited', 'exit_code': 0, 'pid': 1}
        if '--profile' in command:
            self.verify_count += 1
            out.write_text('Synthetic verification\n')
            if self.verify_count <= self.verify_failures:
                result['exit_code'] = 1
            return result
        backend, phase, mode = command
        phase = Path(phase)
        prompt = kw['stdin_path'].read_text()
        if mode == 'write':
            self.worker_count += 1
            assigned, _ = json.JSONDecoder().raw_decode(prompt.split('Assigned task:\n')[1])
            self.calls.append(('worker', assigned['id']))
            (self.root / 'src/evidence.txt').write_text('implementation ' + str(self.worker_count))
            if self.worker_hook:
                self.worker_hook(self, assigned)
            if self.fail_worker:
                out.write_text('{"type":"error"}\n')
            else:
                out.write_text(json.dumps({'type': 'turn.completed'} if backend == 'codex'
                    else {'type': 'result', 'subtype': 'success'}) + '\n')
            return result
        payload, _ = json.JSONDecoder().raw_decode(prompt.split('Context: ')[1])
        schema = json.loads((phase / 'schema.json').read_text())
        criteria = payload['task']['acceptance'] if 'task' in payload else payload['completion_criteria']
        assert schema['properties']['checks']['items']['properties']['criterion']['enum'] == criteria
        if 'completion_criteria' in payload and 'proposal' not in payload:
            self.calls.append(('plan', None))
            answer = dict(decision='complete', reason='All evidence inspected', task=None,
                          checks=checks(payload['completion_criteria']))
            if self.plan_hook:
                answer = self.plan_hook(self, payload, answer)
        else:
            self.calls.append(('review', payload.get('task', {}).get('id', 'completion')))
            criteria = payload['task']['acceptance'] if 'task' in payload else payload['completion_criteria']
            answer = dict(decision=self.review_decision, reason='Independent inspection', checks=checks(criteria))
            if self.review_hook:
                answer = self.review_hook(self, payload, answer)
        if backend == 'codex':
            out.write_text('{"type":"turn.completed"}\n')
            (phase / 'last-message.txt').write_text(json.dumps(answer))
        else:
            response = {'type': 'result', 'subtype': 'success', 'is_error': False,
                        'structured_output': answer}
            if self.claude_verbose:
                response = [{'type': 'system', 'subtype': 'init'},
                            {'type': 'assistant', 'message': {'content': []}}, response]
            out.write_text(json.dumps(response, indent=2))
        return result

    def run(self):
        with contextlib.ExitStack() as stack:
            for name, value in [('ROOT', self.root), ('QUEUE', self.root / 'agent/tasks.json')]:
                stack.enter_context(patch.object(worker, name, value))
            stack.enter_context(patch.object(worker, 'command_for', self.command))
            stack.enter_context(patch.object(worker, 'capture', lambda *a: None))
            stack.enter_context(patch.object(worker.subprocess, 'check_output', return_value=''))
            stack.enter_context(patch.object(worker, 'run_tree', self.runner))
            stack.enter_context(patch.object(loop, 'run_tree', self.runner))
            stack.enter_context(contextlib.redirect_stdout(io.StringIO()))
            lease = stack.enter_context(worker.RepositoryLease(self.root))
            controller = loop.Controller(self.args, self.registry, lease,
                                         self.root / 'build/agent-runs/loop-test')
            self.result = controller.run()
            assert self.result['elapsed_seconds'] >= 0
            assert self.result['elapsed'] == loop.format_duration(self.result['elapsed_seconds'])
            assert self.result['started_at'] and self.result['finished_at']
            saved = json.loads((self.root / 'build/agent-runs/loop-test/result.json').read_text())
            assert saved['elapsed_seconds'] == self.result['elapsed_seconds']
        self.queue = json.loads((self.root / 'agent/tasks.json').read_text())
        return self.result


class LoopTests(unittest.TestCase):
    def test_assessment_stops_after_independent_acceptance(self):
        baseline = task('playable-baseline-001', status='needs_review')
        baseline.update(kind='assessment', user_authorized=True, stop_after_review=True)
        baseline.pop('runtime_change')
        baseline.pop('production_files')
        h = Harness(self.root, [baseline], backend='claude')
        result = h.run()
        self.assertEqual(result['status'], 'milestone_reached')
        self.assertEqual(result['completed_tasks'], ['playable-baseline-001'])
        self.assertEqual(h.queue['tasks'][0]['status'], 'done')
        self.assertEqual(h.worker_count, 0)
        self.assertEqual(h.verify_count, 1)
        self.assertEqual(h.calls, [('review', 'playable-baseline-001')])

    def test_assessment_rejection_does_not_reach_milestone(self):
        baseline = task('playable-baseline-001', status='needs_review')
        baseline.update(kind='assessment', user_authorized=True, stop_after_review=True,
                        loop_attempts=3)
        h = Harness(self.root, [baseline], backend='claude')
        h.review_decision = 'revise'
        self.assertEqual(h.run()['status'], 'blocked')
        self.assertEqual(h.queue['tasks'][0]['status'], 'blocked')
        self.assertEqual(h.result['completed_tasks'], [])


    def test_fresh_post_worker_verification_budget(self):
        for total, allowance, expected in [(None, 60, 3600), (None, 90, 5400), (60, 60, 660)]:
            with self.subTest(total=total, allowance=allowance), tempfile.TemporaryDirectory() as tmp:
                item = task(status='ready')
                item['stop_after_review'] = True
                h = Harness(Path(tmp), [item], minutes=60, total_minutes=total,
                            verification_minutes=allowance)
                clock = [100.0]
                h.worker_hook = lambda *a: clock.__setitem__(0, clock[0] + 2940)
                original = h.runner
                budgets = []
                def runner(command, **kwargs):
                    if '--profile' in command:
                        budgets.append(kwargs['deadline'] - clock[0])
                    return original(command, **kwargs)
                h.runner = runner
                with patch.object(loop.time, 'monotonic', side_effect=lambda: clock[0]):
                    result = h.run()
                self.assertEqual(result['status'], 'milestone_reached')
                self.assertEqual(budgets, [expected])
                ledger = (h.root / 'agent/RUN_STATUS.md').read_text(encoding='utf-8')
                self.assertIn('milestone_reached', ledger)
                self.assertIn(h.queue['tasks'][0]['accepted_verification']['log'], ledger)

    def test_generated_ledger_uses_acceptance_not_stale_worker(self):
        item = task(status='needs_review')
        item.update(stop_after_review=True, last_run='build/old-worker')
        h = Harness(self.root, [item, task('parked', status='blocked')])
        self.assertEqual(h.run()['status'], 'milestone_reached')
        ledger = (self.root / 'agent/RUN_STATUS.md').read_text(encoding='utf-8')
        self.assertIn(h.queue['tasks'][0]['accepted_verification']['log'], ledger)
        self.assertNotIn('old-worker', ledger)
        self.assertIn('parked: **blocked**', ledger)
        with patch.object(worker, 'ROOT', self.root):
            before = worker.content_fingerprint()
            (self.root / 'agent/RUN_STATUS.md').write_text('Derived update')
            self.assertEqual(worker.content_fingerprint(), before)
            controls = worker.control_hashes()
            (self.root / 'tools/agent_control/status_summary.py').write_text('# changed generator')
            self.assertNotEqual(worker.control_hashes(), controls)

    def test_review_verification_budget_and_total_deadline(self):
        # Exercise the actual review-verification launch without real builds.
        for overall, expected in [(float('inf'), 3700), (400, 400)]:
            with self.subTest(overall=overall):
                controller = object.__new__(loop.Controller)
                controller.root = self.root
                controller.registry = {'tools': {'python': {'path': sys.executable}}}
                controller.deadline = overall
                controller.stop = self.root / 'STOP'
                with patch.object(controller, 'check'), \
                     patch.object(controller, 'phase_directory', return_value=self.root), \
                     patch.object(worker, 'content_fingerprint', return_value='unchanged'), \
                     patch.object(loop.time, 'monotonic', return_value=100), \
                     patch.object(loop, 'run_tree', return_value={
                         'reason': 'exited', 'exit_code': 0}) as runner:
                    result = controller.verify('all')
                self.assertEqual(runner.call_args.kwargs['deadline'], expected)
                self.assertEqual(runner.call_args.kwargs['stop_path'], controller.stop)
                self.assertEqual(result['verified_content'], 'unchanged')

    def test_review_verification_timeout_still_stops_without_retry(self):
        controller = object.__new__(loop.Controller)
        controller.root = self.root
        controller.registry = {'tools': {'python': {'path': sys.executable}}}
        controller.deadline = float('inf')
        controller.stop = self.root / 'STOP'
        with patch.object(controller, 'check'), \
             patch.object(controller, 'phase_directory', return_value=self.root), \
             patch.object(worker, 'content_fingerprint', return_value='unchanged'), \
             patch.object(loop, 'run_tree', return_value={
                 'reason': 'timeout', 'exit_code': None}) as runner:
            with self.assertRaises(loop.StopLoop) as caught:
                controller.verify('all')
        self.assertEqual(caught.exception.status, 'interrupted')
        self.assertEqual(runner.call_count, 1)

    def test_duration_format(self):
        for seconds, expected in [(0, '0h 00m 00s'), (65.9, '0h 01m 05s'),
                                  (3661, '1h 01m 01s'), (90000, '25h 00m 00s')]:
            self.assertEqual(loop.format_duration(seconds), expected)

    def test_duration_uses_monotonic_clock_and_increases_in_events(self):
        h = Harness(self.root, [task()])
        ticks = iter(range(1000, 1000000, 2))
        with patch.object(loop.time, 'monotonic', side_effect=lambda: next(ticks)):
            result = h.run()
        self.assertEqual(result['status'], 'complete')
        times = [event['elapsed_seconds'] for event in result['events']]
        self.assertEqual(times, sorted(times))
        self.assertGreater(result['elapsed_seconds'], times[-1])

    def setUp(self):
        env = patch.dict(os.environ)
        env.start()
        self.addCleanup(env.stop)
        os.environ.pop('CLAUDE_CODE_EFFORT_LEVEL', None)
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def test_dependency_chain_and_new_planned_work(self):
        h = Harness(self.root, [task(), task('second', deps=['first'])])
        def plan(h, payload, answer):
            if len(payload['queue']['tasks']) == 2:
                new = task('third', deps=['second'])
                new.pop('status')
                return dict(decision='task', reason='Next integration slice', task=new, checks=[])
            return answer
        h.plan_hook = plan
        self.assertEqual(h.run()['status'], 'complete')
        self.assertEqual(h.worker_count, 3)
        self.assertEqual(h.verify_count, 4)  # cached task results plus full completion check
        self.assertTrue(all(t['status'] == 'done' for t in h.queue['tasks']))
        self.assertEqual(h.calls[-1], ('review', 'completion'))

    def test_claude_structured_output_and_existing_review(self):
        h = Harness(self.root, [task(status='needs_review')], backend='claude')
        self.assertEqual(h.run()['status'], 'complete')
        self.assertEqual(h.worker_count, 0)
        self.assertEqual(h.verify_count, 2)

    def test_claude_single_result_object(self):
        h = Harness(self.root, [task(status='needs_review')], backend='claude')
        h.claude_verbose = False
        self.assertEqual(h.run()['status'], 'complete')

    def test_claude_result_rejects_missing_ambiguous_and_failed_output(self):
        good = dict(type='result', subtype='success', is_error=False,
                    structured_output=dict(decision='accept', reason='Inspected', checks=[]))
        invalid = [None, 'text', [], [1], {},
                   [{'type': 'assistant', 'structured_output': good['structured_output']}],
                   [good, good], [good, {'type': 'assistant'}],
                   dict(good, subtype='error_max_turns'), dict(good, is_error=True),
                   dict(good, structured_output=None), dict(good, structured_output=[]),
                   [{'type': 'error'}, good],
                   [{'type': 'result', 'subtype': 'error'}, good]]
        for response in invalid:
            with self.subTest(response=response), self.assertRaises(ValueError):
                loop.claude_structured_result(response)
        for response in (good, [{'type': 'system'}, good]):
            self.assertEqual(loop.claude_structured_result(response), good['structured_output'])

    def test_verification_retry(self):
        h = Harness(self.root, [task()])
        h.verify_failures = 1
        self.assertEqual(h.run()['status'], 'complete')
        self.assertEqual(h.worker_count, 2)

    def test_review_revision_limit(self):
        h = Harness(self.root, [task()])
        h.review_decision = 'revise'
        self.assertEqual(h.run()['status'], 'blocked')
        self.assertEqual(h.worker_count, 3)
        self.assertEqual(h.queue['tasks'][0]['status'], 'blocked')

    def test_worker_auth_failure_does_not_retry(self):
        h = Harness(self.root, [task()])
        h.fail_worker = True
        self.assertEqual(h.run()['status'], 'blocked')
        self.assertEqual(h.worker_count, 1)
        self.assertEqual(h.verify_count, 0)

    def test_protected_task_never_runs_or_completes(self):
        protected = task()
        protected['requires_original_execution'] = True
        h = Harness(self.root, [protected])
        self.assertEqual(h.run()['status'], 'blocked')
        self.assertEqual(h.worker_count, 0)
        self.assertIn('unfinished', h.result['reason'])

    def test_empty_queue_requires_completion_evidence(self):
        h = Harness(self.root, [])
        h.plan_hook = lambda h, p, a: dict(a, checks=[])
        self.assertEqual(h.run()['status'], 'blocked')
        self.assertEqual(h.verify_count, 0)

    def test_readonly_source_mutation_stops(self):
        h = Harness(self.root, [task(status='needs_review')])
        def mutate(h, p, a):
            (h.root / 'src/evidence.txt').write_text('changed during review')
            return a
        h.review_hook = mutate
        self.assertEqual(h.run()['status'], 'blocked')
        self.assertIn('changed source', h.result['reason'])

    def test_external_queue_change_preserved(self):
        h = Harness(self.root, [task()])
        def mutate(h, t):
            worker.atomic_json(h.root / 'agent/tasks.json', {'tasks': [], 'external': True})
        h.worker_hook = mutate
        self.assertEqual(h.run()['status'], 'blocked')
        self.assertTrue(h.queue['external'])
        self.assertEqual(h.verify_count, 0)

    def test_completion_control_change_stops(self):
        h = Harness(self.root, [task()])
        h.worker_hook = lambda h, t: worker.atomic_json(h.root / 'agent/completion.json', {'criteria': ['weaker']})
        self.assertEqual(h.run()['status'], 'blocked')
        self.assertEqual(h.verify_count, 0)

    def test_stop_and_interrupted_tasks(self):
        h = Harness(self.root, [task()])
        (self.root / 'build/agent-runs/STOP').touch()
        self.assertEqual(h.run()['status'], 'cancelled')
        self.assertEqual(h.calls, [])
        (self.root / 'build/agent-runs/STOP').unlink()
        worker.atomic_json(self.root / 'agent/tasks.json', {'tasks': [task(status='interrupted')]})
        self.assertEqual(h.run()['status'], 'interrupted')
        self.assertEqual(h.worker_count, 0)

    def test_cycle_checkpoint_resumes_at_review(self):
        h = Harness(self.root, [task()], max_cycles=1)
        self.assertEqual(h.run()['status'], 'limit_reached')
        self.assertEqual(h.queue['tasks'][0]['status'], 'needs_review')
        h.args.max_cycles = 30
        self.assertEqual(h.run()['status'], 'complete')
        self.assertEqual(h.worker_count, 1)

    def test_completion_revisions_are_bounded(self):
        h = Harness(self.root, [])
        h.review_decision = 'revise'
        self.assertEqual(h.run()['status'], 'blocked')
        self.assertEqual(h.queue['completion_rejections'], 3)

    def test_total_budget_stops_before_worker(self):
        h = Harness(self.root, [task()], total_minutes=0.001)
        self.assertEqual(h.run()['status'], 'limit_reached')
        self.assertEqual(h.worker_count, 0)

    def test_readonly_timeout_does_not_retry(self):
        h = Harness(self.root, [task(status='needs_review')])
        original = h.runner
        def timeout(command, **kw):
            if command[-1] == 'read':
                return {'reason': 'timeout', 'exit_code': None, 'pid': 1}
            return original(command, **kw)
        h.runner = timeout
        self.assertEqual(h.run()['status'], 'interrupted')
        self.assertEqual(h.worker_count, 0)
        self.assertEqual(h.queue['tasks'][0]['status'], 'needs_review')

    def test_placeholder_and_dependency_preservation(self):
        placeholder = task('next', 'blocked', ['first'])
        placeholder['planning_required'] = True
        queue = {'tasks': [task(status='done'), placeholder]}
        proposed = task('next', deps=['first'])
        proposed.pop('status')
        proposed['acceptance'] = ['Concrete additional check']
        proposal = dict(decision='task', reason='Selected concrete slice', task=proposed, checks=[])
        bad = copy.deepcopy(proposal)
        bad['task']['depends_on'] = []
        with self.assertRaises(ValueError):
            loop.apply_plan(queue, bad, 100)
        loop.apply_plan(queue, proposal, 100)
        self.assertIn('Evidence for next', queue['tasks'][1]['acceptance'])
        self.assertEqual(loop.choose_action(queue)[0], 'implement')
        with self.assertRaises(ValueError):
            loop.apply_plan(queue, proposal, 100)

    def test_planner_requires_production_implementation(self):
        for paths in (['decomp-lemmings/tests/test_feature.cpp'],
                      ['docs/evidence/feature.md'], ['tools/verify_reference.py'],
                      ['src/__test__/feature.cpp'], ['src/app/feature.hpp'],
                      ['../src/feature.cpp']):
            proposed = task('new')
            proposed.pop('status')
            proposed['production_files'] = paths
            with self.subTest(paths=paths), self.assertRaises(ValueError):
                loop.apply_plan({'tasks': []}, dict(decision='task', reason='Next',
                                task=proposed, checks=[]), 100)
        proposed = task('new')
        proposed.pop('status')
        queue = {'tasks': []}
        loop.apply_plan(queue, dict(decision='task', reason='Next', task=proposed, checks=[]), 100)
        self.assertIn('before/after behavior', queue['tasks'][0]['acceptance'][-1])

    def test_schema_evidence_and_cycle_rejections(self):
        with self.assertRaises(ValueError):
            loop.validate({'decision': 'accept'}, loop.REVIEW)
        with self.assertRaises(ValueError):
            loop.validate_queue({'tasks': [task('a', deps=['b']), task('b', deps=['a'])]})
        evidence = checks(['criterion'])
        evidence[0]['evidence'][0]['path'] = '../outside.txt'
        with self.assertRaises(ValueError):
            loop.require_checks(evidence, ['criterion'], self.root)

    def test_bound_schema_rejects_paraphrases_without_mutating_shared_schema(self):
        criteria = ['Preserve exact source types and offsets', 'Record all remaining fidelity limits']
        original = copy.deepcopy(loop.REVIEW)
        bound = loop.schema_with_criteria(loop.REVIEW, criteria)
        answer = dict(decision='accept', reason='Reviewed', checks=checks(criteria))
        loop.validate(answer, bound)
        answer['checks'][0]['criterion'] = 'Preserve types and offsets'
        with self.assertRaises(ValueError):
            loop.validate(answer, bound)
        self.assertEqual(loop.REVIEW, original)
        other = loop.schema_with_criteria(loop.PLAN, ['Whole project is complete'])
        self.assertEqual(other['properties']['checks']['items']['properties']['criterion']['enum'],
                         ['Whole project is complete'])

    def test_duplicate_checks_cannot_substitute_for_missing_criterion(self):
        criteria = ['First requirement', 'Second requirement']
        answer = dict(decision='accept', reason='Reviewed', checks=checks([criteria[0], criteria[0]]))
        loop.validate(answer, loop.schema_with_criteria(loop.REVIEW, criteria))
        with self.assertRaisesRegex(ValueError, 'Second requirement'):
            loop.require_checks(answer['checks'], criteria, self.root)

    def test_shared_lock_and_role_permissions(self):
        with worker.RepositoryLease(self.root):
            with self.assertRaises(FileExistsError):
                with worker.RepositoryLease(self.root):
                    pass
        registry = {'tools': {b: {'path': sys.executable} for b in ('codex', 'claude')}}
        registry['agent_models'] = copy.deepcopy(MODELS)
        schema = self.root / 'schema.json'
        worker.atomic_json(schema, loop.REVIEW)
        codex = worker.command_for('codex', registry, self.root, read_only=True, schema=schema)
        self.assertEqual(codex[codex.index('--sandbox') + 1], 'read-only')
        self.assertIn('--output-schema', codex)
        self.assertEqual(codex[codex.index('--model') + 1], 'gpt-6.1-sol')
        self.assertIn('model_reasoning_effort="high"', codex)
        claude = worker.command_for('claude', registry, self.root, read_only=True, schema=schema)
        self.assertEqual(claude[claude.index('--tools') + 1], 'Read,Glob,Grep')
        self.assertIn('--json-schema', claude)
        self.assertEqual(claude[claude.index('--model') + 1], 'claude-opus-5')
        self.assertEqual(claude[claude.index('--effort') + 1], 'high')

    def test_claude_worker_has_scoped_verification_commands(self):
        registry = {'agent_models': copy.deepcopy(MODELS),
                    'tools': {b: {'path': sys.executable} for b in MODELS}}
        cmd = worker.command_for('claude', registry, self.root)
        self.assertIn('Bash', cmd[cmd.index('--tools') + 1].split(','))
        allowed = cmd[cmd.index('--allowedTools') + 1:]
        self.assertEqual(set(allowed), {
            'Bash(git --no-pager show --no-ext-diff --no-textconv HEAD:src/app/main.cpp)',
            'Bash(python tools/agent_control/verify.py --help)',
            'Bash(python tools/agent_control/verify.py --profile main)',
            'Bash(python tools/agent_control/verify.py --profile recovery)',
            'Bash(python tools/agent_control/verify.py --profile all)'})
        self.assertNotIn('bypassPermissions', cmd)
        self.assertEqual(cmd[cmd.index('--permission-prompts') + 1], 'none')
        review = worker.command_for('claude', registry, self.root, read_only=True)
        self.assertNotIn('--allowedTools', review)
        self.assertEqual(review[review.index('--tools') + 1], 'Read,Glob,Grep')

    def test_model_pins_fail_closed(self):
        for backend in ('codex', 'claude'):
            with self.assertRaises(ValueError):
                worker.model_settings(backend, {})
            registry = {'agent_models': copy.deepcopy(MODELS)}
            registry['agent_models'][backend]['model'] = 'default'
            with self.assertRaises(ValueError):
                worker.model_settings(backend, registry)
        with patch.dict(os.environ, {'CLAUDE_CODE_EFFORT_LEVEL': 'low'}):
            with self.assertRaises(ValueError):
                worker.model_settings('claude', {'agent_models': MODELS})

    def test_worker_and_review_commands_use_registry_pins(self):
        registry = {'agent_models': copy.deepcopy(MODELS),
                    'tools': {b: {'path': sys.executable} for b in MODELS}}
        registry['agent_models']['codex'] = {'model': 'gpt-6.1-terra', 'effort': 'medium'}
        for readonly in (True, False):
            command = worker.command_for('codex', registry, self.root, read_only=readonly)
            self.assertEqual(command[command.index('--model') + 1], 'gpt-6.1-terra')
            self.assertIn('model_reasoning_effort="medium"', command)
            self.assertIn('windows.sandbox="elevated"', command)
            self.assertEqual(command[command.index('--sandbox') + 1],
                             'read-only' if readonly else 'workspace-write')
            self.assertIn('approval_policy="never"', command)
            command = worker.command_for('claude', registry, self.root, read_only=readonly)
            self.assertEqual(command[command.index('--model') + 1], 'claude-opus-5')
            self.assertEqual(command[command.index('--effort') + 1], 'high')


if __name__ == '__main__':
    unittest.main(verbosity=2)
