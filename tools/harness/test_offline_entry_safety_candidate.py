import concurrent.futures
import hashlib
import importlib.util
import json
import shutil
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
SRC = HERE / 'offline_entry_safety_candidate.py'
WORKER = HERE / 'offline_entry_safety_worker.py'
WORKER_SCRATCH = HERE.parents[1] / 'build/tmp/offline-evidence-preservation-2026-10-28/worker'
spec = importlib.util.spec_from_file_location('candidate', SRC)
candidate = importlib.util.module_from_spec(spec); spec.loader.exec_module(candidate)

# Frozen approval fixture: update only as a deliberate, reviewed fixture change.
APPROVED_LAUNCHER = '7a866b7bfdc5c978b5a3a78b5fa2dc3aef8f9cd0c73e8e25e794cb3bc12c11b1'
APPROVED_WORKER = 'fbb40446cb9e7494707438897231e0431cac304361e686119e32e20362084aee'


def digest(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()


class BoundaryTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='safety-', dir=WORKER_SCRATCH)
        self.root = Path(self.tmp.name)
        self.effects = candidate.OfflineEffects(self.root / 'state', 'SYNTHETIC-TOKEN')
        self.expected = {'launcher_path': str(SRC), 'launcher': APPROVED_LAUNCHER, 'worker_path': str(WORKER), 'worker': APPROVED_WORKER, 'dependencies': {'python': __import__('sys').version_info.major, 'worker_api': 1}, 'approved_args': {'attempt':'a1','test_only':False,'mode':'ok','worker':str(WORKER.resolve())}}

    def tearDown(self): self.tmp.cleanup()

    def retain(self, name, source):
        destination = WORKER_SCRATCH/'evidence'/f'{name}-{self.root.name}'
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(source, destination)
        return destination

    def call(self, *extra):
        return candidate.run(['--attempt', 'a1', '--worker', str(WORKER), *extra], effects=self.effects, expected=self.expected)

    def test_actual_launcher_worker_success_and_testonly(self):
        self.expected['approved_args'] = dict(self.expected['approved_args'], test_only=True)
        self.assertEqual(self.call('--test-only'), 0)
        self.assertEqual(self.effects.events, ['identity-ok'])
        self.expected['approved_args'] = dict(self.expected['approved_args'], test_only=False)
        self.assertEqual(self.call(), 0)
        self.assertEqual(self.effects.events, ['identity-ok', 'identity-ok', 'identity-ok', 'consumed', 'attempt-created', 'stage-00-started', 'mock-launch', 'stage-01-launched', 'stage-02-capture-started', 'mock-capture', 'synthetic-output', 'stage-03-output-persisted', 'receipt'])
        self.assertEqual(self.effects.launch_events, [{'claim_present': True}])
        receipt = json.loads((self.root/'state/attempts/a1/receipt.jsonl').read_text())
        self.assertEqual((receipt['branch'], receipt['result'], receipt['disposition']), ('success','synthetic-result','owned-mock-complete'))
        self.retain('success', self.root/'state')

    def test_supported_argument_drift_refuses_before_effects(self):
        with self.assertRaisesRegex(RuntimeError, 'argument-identity-mismatch'):
            candidate.run(['--attempt','UNAPPROVED-ARGUMENT','--worker',str(WORKER)], effects=self.effects, expected=self.expected)
        self.assertFalse({'consumed','attempt-created','mock-capture'} & set(self.effects.events))

    def test_defaults_and_flags_fail_closed(self):
        with self.assertRaisesRegex(RuntimeError, 'explicit offline mocks'):
            candidate.main(['--attempt','a','--worker',str(WORKER)])
        with self.assertRaises(SystemExit):
            candidate.run(['--attempt','a','--worker',str(WORKER),'--live'])
        spec = importlib.util.spec_from_file_location('worker_flags', WORKER); w=importlib.util.module_from_spec(spec); spec.loader.exec_module(w)
        with self.assertRaises(SystemExit):
            w.main(['--attempt','a','--unknown'], effects=self.effects, expected=self.expected)

    def test_fixed_approval_source_and_supported_arg_controls(self):
        wrong_source = dict(self.expected, launcher='0'*64)
        with self.assertRaisesRegex(RuntimeError, 'source-identity-mismatch'):
            candidate.run(['--attempt','a1','--worker',str(WORKER)], effects=self.effects, expected=wrong_source)
        wrong_mode = dict(self.expected, approved_args=dict(self.expected['approved_args'], mode='geometry'))
        with self.assertRaisesRegex(RuntimeError, 'argument-identity-mismatch'):
            candidate.run(['--attempt','a1','--worker',str(WORKER)], effects=self.effects, expected=wrong_mode)
        self.assertEqual(self.effects.events, [])

    def test_stale_source_worker_failed_copy_and_dependency(self):
        stale = self.root/'stale.py'; stale.write_text('raise RuntimeError("import side effect")\n')
        changed = dict(self.expected); changed['approved_args'] = dict(self.expected['approved_args'], worker=str(stale.resolve()))
        with self.assertRaisesRegex(RuntimeError, 'source-identity-mismatch'):
            candidate.run(['--attempt','a1','--worker',str(stale)], effects=self.effects, expected=changed)
        self.assertNotIn('consumed', self.effects.events)
        self.expected['launcher'] = APPROVED_LAUNCHER
        self.expected['dependencies'] = {'python': 0, 'worker_api': 1}
        with self.assertRaisesRegex(RuntimeError, 'dependency-identity-mismatch'): self.call()
        self.assertNotIn('consumed', self.effects.events)

    def test_worker_direct_testonly_does_nothing(self):
        spec = importlib.util.spec_from_file_location('worker', WORKER); w=importlib.util.module_from_spec(spec); spec.loader.exec_module(w)
        self.expected['approved_args'] = {'attempt':'x','test_only':True,'mode':'ok','worker':str(WORKER.resolve())}
        self.assertEqual(w.main(['--attempt','x','--test-only'], effects=self.effects, expected=self.expected), 0)
        self.assertEqual(self.effects.events, ['identity-ok'])
        self.assertFalse((self.root/'state/authorization.json').exists())

    def test_direct_worker_requires_fixed_approval_source_and_args(self):
        spec = importlib.util.spec_from_file_location('worker', WORKER); w=importlib.util.module_from_spec(spec); spec.loader.exec_module(w)
        with self.assertRaisesRegex(RuntimeError, 'offline effect adapter'):
            w.main(['--attempt','a1'], effects=self.effects)
        with self.assertRaisesRegex(RuntimeError, 'offline effect adapter'):
            w.main(['--attempt','a1'], effects=self.effects, expected=None)
        with self.assertRaisesRegex(RuntimeError, 'argument-identity-mismatch'):
            w.main(['--attempt','UNAPPROVED'], effects=self.effects, expected=self.expected)
        self.assertEqual(self.effects.events, [])
        stale = self.root/'stale-direct-worker.py'
        shutil.copyfile(WORKER, stale)
        stale_spec = importlib.util.spec_from_file_location('stale_worker', stale)
        stale_module = importlib.util.module_from_spec(stale_spec); stale_spec.loader.exec_module(stale_module)
        approval = dict(self.expected)
        approval['approved_args'] = {'attempt':'a1','test_only':False,'mode':'ok','worker':str(stale.resolve())}
        with self.assertRaisesRegex(RuntimeError, 'source-identity-mismatch'):
            stale_module.main(['--attempt','a1'], effects=self.effects, expected=approval)
        self.assertEqual(self.effects.events, [])

    def test_testonly_launcher_stale_worker_fails_closed_before_effects(self):
        stale = self.root/'stale-testonly-worker.py'
        stale.write_text('def main(*a, **k): return 0\n')
        approval = dict(self.expected)
        approval['approved_args'] = dict(approval['approved_args'], test_only=True, worker=str(stale.resolve()))
        with self.assertRaisesRegex(RuntimeError, 'source-identity-mismatch'):
            candidate.run(['--attempt','a1','--worker',str(stale),'--test-only'], effects=self.effects, expected=approval)
        self.assertFalse({'consumed','attempt-created','mock-capture'} & set(self.effects.events))
        self.assertFalse((self.root/'state/authorization.json').exists())

    def invoke(self, attempt, effects, *, worker=WORKER, mode='ok', module=candidate):
        expected = dict(self.expected)
        expected['launcher_path'] = str(Path(module.__file__).resolve())
        expected['launcher'] = digest(module.__file__)
        expected['worker_path'] = str(Path(worker).resolve())
        expected['worker'] = digest(worker)
        expected['approved_args'] = {'attempt': attempt, 'test_only': False, 'mode': mode,
                                     'worker': str(Path(worker).resolve())}
        return module.run(['--attempt', attempt, '--worker', str(worker), '--mode', mode],
                          effects=effects, expected=expected)

    def test_actual_boundary_sequential_restart_output_rename_and_capture_failure(self):
        claim = self.root/'stable-grant-store'
        outputs = self.root/'outputs-one'
        first = candidate.OfflineEffects(claim, 'AUTH-ONE', outputs)
        self.assertEqual(self.invoke('attempt-one', first), 0)
        state = json.loads((claim/'authorization.json').read_text())
        self.assertEqual(state, {'token':'AUTH-ONE','domain':'synthetic-offline-only'})
        self.assertEqual(first.launch_events, [{'claim_present': True}])

        # Fresh module/effects context and a distinct valid output name, same stable grant.
        spec = importlib.util.spec_from_file_location('candidate_restarted', SRC)
        restarted_module = importlib.util.module_from_spec(spec); spec.loader.exec_module(restarted_module)
        restarted = candidate.OfflineEffects(claim, 'AUTH-ONE', self.root/'renamed-output-root')
        with self.assertRaisesRegex(RuntimeError, 'authorization-already-consumed'):
            self.invoke('attempt-renamed', restarted, module=restarted_module)
        self.assertEqual(len(first.launch_events)+len(restarted.launch_events), 1)

        fail_claim = self.root/'capture-failure-store'
        failed = candidate.OfflineEffects(fail_claim, 'AUTH-FAIL', self.root/'capture-failure-output')
        failed.capture = lambda: (_ for _ in ()).throw(OSError('synthetic-capture-failure'))
        self.assertEqual(self.invoke('capture-attempt', failed), 1)
        self.assertEqual(failed.events.count('mock-launch'), 1)
        failure = json.loads((self.root/'capture-failure-output/attempts/capture-attempt/receipt.jsonl').read_text())
        self.assertEqual((failure['branch'], failure['stage'], failure['error']),
                         ('capture-failure', 'capture', 'OSError:synthetic-capture-failure'))
        self.assertEqual(failure['approved_args']['attempt'], 'capture-attempt')
        self.retain('capture-error', self.root/'capture-failure-output')
        retry = candidate.OfflineEffects(fail_claim, 'AUTH-FAIL', self.root/'capture-retry-new-output')
        with self.assertRaisesRegex(RuntimeError, 'authorization-already-consumed'):
            self.invoke('capture-retry', retry)
        self.assertEqual(len(failed.launch_events)+len(retry.launch_events), 1)

    def test_actual_boundary_concurrent_entries_claim_once(self):
        claim, outputs = self.root/'concurrent-store', self.root/'concurrent-outputs'
        def invoke(i):
            effects = candidate.OfflineEffects(claim, 'AUTH-RACE', outputs)
            try:
                result = self.invoke(f'race-{i}', effects)
                return ('launched', effects.launch_events, result)
            except RuntimeError as exc:
                return (str(exc), effects.launch_events, None)
        with concurrent.futures.ThreadPoolExecutor(12) as pool:
            results = list(pool.map(invoke, range(12)))
        self.assertEqual(sum(len(r[1]) for r in results), 1)
        self.assertEqual(sum(r[0] == 'launched' for r in results), 1)
        state = json.loads((claim/'authorization.json').read_text())
        self.assertEqual(state['token'], 'AUTH-RACE')

    def test_interruption_after_claim_before_launch_is_not_refunded(self):
        claim = self.root/'interrupted-store'
        effects = candidate.OfflineEffects(claim, 'AUTH-INTERRUPT', self.root/'interrupted-output')
        def interrupt(): raise KeyboardInterrupt('synthetic-interruption-before-launch')
        effects.launch = interrupt
        with self.assertRaisesRegex(KeyboardInterrupt, 'before-launch'):
            self.invoke('interrupted-attempt', effects)
        self.assertTrue((claim/'authorization.json').is_file())
        self.assertEqual(effects.launch_events, [])
        interrupted_dir = self.root/'interrupted-output/attempts/interrupted-attempt'
        self.assertIn('incomplete-unknown', (interrupted_dir/'stages.jsonl').read_text())
        self.assertFalse((interrupted_dir/'receipt.jsonl').exists())
        self.retain('interrupt-before-launch', self.root/'interrupted-output')
        restarted = candidate.OfflineEffects(claim, 'AUTH-INTERRUPT', self.root/'interrupted-retry-output')
        with self.assertRaisesRegex(RuntimeError, 'authorization-already-consumed'):
            self.invoke('interrupted-retry', restarted)
        self.assertEqual(restarted.launch_events, [])

    def test_executed_singleuse_only_mutant_reaches_second_launch(self):
        worker_root = WORKER_SCRATCH
        artifact_dir = worker_root / f'singleuse-mutant-{self.root.name}'
        artifact_dir.mkdir()
        mutant = artifact_dir/'offline_entry_safety_worker.py'
        mutant.write_text(WORKER.read_text().replace('effects.consume(ns.attempt)', "effects.events.append('consumed')"), newline='\n')
        self.assertNotEqual(digest(mutant), APPROVED_WORKER)
        # Same actual entry, fixed valid launcher/args/worker approval on each fresh output.
        mutant_effects = candidate.OfflineEffects(artifact_dir/'trusted-store', 'AUTH-MUTANT', artifact_dir/'outputs')
        self.assertEqual(self.invoke('mutant-output-a', mutant_effects, worker=mutant), 0)
        self.assertEqual(self.invoke('mutant-output-b', mutant_effects, worker=mutant), 0)
        self.assertEqual(mutant_effects.events.count('mock-launch'), 2)
        self.assertEqual(mutant_effects.events.count('mock-capture'), 2)
        self.assertFalse((artifact_dir/'trusted-store/authorization.json').exists())
        (artifact_dir/'result.json').write_text(json.dumps({
            'mutant_sha256': digest(mutant), 'single_change': 'effects.consume(ns.attempt) → no-op',
            'approved_args': ['mutant-output-a', 'mutant-output-b'],
            'returns': [0, 0], 'mock_launches': mutant_effects.events.count('mock-launch'),
            'mock_captures': mutant_effects.events.count('mock-capture'),
            'authorization_claim_file_exists': False, 'events': mutant_effects.events,
        }, indent=2) + '\n', newline='\n')

    def test_evidence_exclusive_first_bytes_preserved_and_distinct_diagnostics(self):
        self.effects.geometry = lambda: (404, 261)
        self.expected['approved_args'] = dict(self.expected['approved_args'], mode='geometry')
        self.assertEqual(self.call('--mode','geometry'),1)
        p=self.root/'state/attempts/a1/receipt.jsonl'; first=p.read_bytes()
        with self.assertRaises(FileExistsError): self.effects.append_receipt(p.parent, {'error':'overwrite'}, 'bad')
        self.assertEqual(p.read_bytes(),first)
        # Separate attempt/state for injected exception exact branch (not geometry refusal).
        other=candidate.OfflineEffects(self.root/'other','TOKEN2')
        exp=dict(self.expected)
        exp['approved_args']=dict(exp['approved_args'],attempt='b1',mode='exception')
        self.assertEqual(candidate.run(['--attempt','b1','--worker',str(WORKER),'--mode','exception'],effects=other,expected=exp),1)
        rec=json.loads((self.root/'other/attempts/b1/receipt.jsonl').read_text())
        self.assertEqual((rec['branch'],rec['error']),('exception','LookupError:injected-capture-exception'))
        self.assertEqual(json.loads(first)['branch'],'geometry-refusal')
        self.retain('geometry-refusal', self.root/'state')
        self.retain('injected-exception', self.root/'other')

    def test_actual_boundary_distinct_grants_same_destination_preserve_and_reject(self):
        output = self.root / 'shared-destination'
        one = candidate.OfflineEffects(self.root/'grant-one', 'SYNTHETIC-GRANT-ONE', output)
        self.assertEqual(self.invoke('shared-attempt', one), 0)
        attempt_dir = output/'attempts/shared-attempt'
        original = {p.relative_to(output).as_posix(): p.read_bytes()
                    for p in output.rglob('*') if p.is_file()}
        receipt = json.loads((attempt_dir/'receipt.jsonl').read_text())
        self.assertEqual(receipt['approved_args']['attempt'], 'shared-attempt')
        self.assertEqual(receipt['grant_id'], hashlib.sha256(b'SYNTHETIC-GRANT-ONE').hexdigest())
        self.assertEqual(json.loads((attempt_dir/'synthetic-output.json').read_text())['value'], 'synthetic-result')
        two = candidate.OfflineEffects(self.root/'grant-two', 'SYNTHETIC-GRANT-TWO', output)
        self.assertEqual(self.invoke('shared-attempt', two), 1)
        after = {p.relative_to(output).as_posix(): p.read_bytes()
                 for p in output.rglob('*') if p.is_file() and 'rejections' not in p.parts}
        self.assertEqual(after, original)
        rejected = list((output/'rejections').glob('*.json'))
        self.assertEqual(len(rejected), 1)
        rejection = json.loads(rejected[0].read_text())
        self.assertEqual((rejection['attempt'], rejection['grant_id'], rejection['outcome']),
                         ('shared-attempt', hashlib.sha256(b'SYNTHETIC-GRANT-TWO').hexdigest(), 'rejected'))
        self.retain('distinct-grant-rejection', output)

    def test_actual_boundary_interruption_retains_incomplete_stage_and_output(self):
        during = candidate.OfflineEffects(self.root/'interrupt-store', 'INTERRUPT-GRANT', self.root/'interrupt-output')
        def interrupt_capture(): raise KeyboardInterrupt('synthetic-during-capture')
        during.capture = interrupt_capture
        with self.assertRaisesRegex(KeyboardInterrupt, 'during-capture'):
            self.invoke('during-capture', during)
        d = self.root/'interrupt-output/attempts/during-capture'
        stages = [json.loads(line)['stage_record'] for line in (d/'stages.jsonl').read_text().splitlines()]
        self.assertEqual(stages, ['00-started', '01-launched', '02-capture-started'])
        self.assertFalse((d/'receipt.jsonl').exists())
        self.assertFalse((d/'synthetic-output.json').exists())
        self.assertIn('incomplete-unknown', (d/'stages.jsonl').read_text())

        after_output = candidate.OfflineEffects(self.root/'output-interrupt-store', 'OUTPUT-GRANT', self.root/'output-interrupt')
        def interrupt_final(*a, **k): raise KeyboardInterrupt('synthetic-after-output-before-receipt')
        after_output.append_receipt = interrupt_final
        with self.assertRaisesRegex(KeyboardInterrupt, 'after-output'):
            self.invoke('after-output', after_output)
        d2 = self.root/'output-interrupt/attempts/after-output'
        self.assertEqual(json.loads((d2/'synthetic-output.json').read_text())['value'], 'synthetic-result')
        self.assertIn('incomplete-until-final-record', (d2/'stages.jsonl').read_text())
        self.assertFalse((d2/'receipt.jsonl').exists())
        self.retain('interrupt-during-capture', self.root/'interrupt-output')
        self.retain('interrupt-after-output', self.root/'output-interrupt')

    def test_executed_nonoverwrite_only_mutant_breaks_same_preservation_property(self):
        mutant = WORKER_SCRATCH/'nonoverwrite-mutant-launcher.py'
        source = SRC.read_text()
        old = '        p.mkdir()\n'
        new = '        p.mkdir(exist_ok=True)\n'
        self.assertEqual(source.count(old), 1)
        mutant.write_text(source.replace(old, new), newline='\n')
        mutant_hash = digest(mutant)
        spec = importlib.util.spec_from_file_location('nonoverwrite_mutant_launcher', mutant)
        mutant_module = importlib.util.module_from_spec(spec); spec.loader.exec_module(mutant_module)
        out = self.root/'mutant-shared-destination'
        first = mutant_module.OfflineEffects(self.root/'mutant-grant-one', 'MUTANT-GRANT-ONE', out)
        self.assertEqual(self.invoke('same-attempt', first, module=mutant_module), 0)
        target = out/'attempts/same-attempt/stages.jsonl'
        first_bytes = target.read_bytes()
        evidence = WORKER_SCRATCH/'evidence'/f'nonoverwrite-mutant-{self.root.name}'
        evidence.mkdir(parents=True, exist_ok=False)
        (evidence/'earlier-stages-before.jsonl').write_bytes(first_bytes)
        second = mutant_module.OfflineEffects(self.root/'mutant-grant-two', 'MUTANT-GRANT-TWO', out)
        with self.assertRaises(FileExistsError):
            self.invoke('same-attempt', second, module=mutant_module)
        self.assertNotEqual(target.read_bytes(), first_bytes)
        shutil.copytree(out, evidence/'mutated-output')
        (WORKER_SCRATCH/'nonoverwrite-mutant-result.json').write_text(json.dumps({
            'mutant_sha256': mutant_hash, 'single_guard_change': 'exclusive attempt-directory mkdir to exist_ok=True',
            'delivered_worker_unchanged': digest(WORKER) == APPROVED_WORKER,
            'both_grants_distinct': True, 'same_valid_attempt_and_output_destination': True,
            'earlier_stage_bytes_changed': target.read_bytes() != first_bytes,
            'earlier_stage_before_sha256': hashlib.sha256(first_bytes).hexdigest(),
            'earlier_stage_after_sha256': digest(target),
            'reached_actual_launcher_worker': True,
        }, indent=2)+'\n', newline='\n')

    def test_isolated_guard_bypass_behavior_controls(self):
        # Each disposable worker differs in one guard only and is re-approved by
        # fixed synthetic fixture fields, so identity checks cannot mask behavior.
        worker_dir = WORKER_SCRATCH
        worker_dir.mkdir(parents=True, exist_ok=True)
        testonly = worker_dir/'testonly-guard-mutant.py'
        testonly.write_text(WORKER.read_text().replace('if ns.test_only or test_only:', 'if False:'), newline='\n')
        thash = digest(testonly)
        mutant_effects = candidate.OfflineEffects(self.root/'testonly-mutant-state', 'MUTANT-TOKEN')
        approved = dict(self.expected, worker_path=str(testonly), worker=thash,
                        approved_args={'attempt':'x','test_only':True,'mode':'ok','worker':str(testonly.resolve())})
        spec = importlib.util.spec_from_file_location('testonly_mutant', testonly)
        mutant = importlib.util.module_from_spec(spec); spec.loader.exec_module(mutant)
        self.assertEqual(mutant.main(['--attempt','x','--test-only'], effects=mutant_effects, expected=approved), 0)
        self.assertTrue({'consumed','attempt-created','mock-capture'} <= set(mutant_effects.events))
        testonly_result = {'guard':'worker TestOnly no-op only','sha256':thash,'return':0,'events':list(mutant_effects.events)}

        approval_mutant = worker_dir/'worker-approval-guard-mutant.py'
        approval_mutant.write_text(WORKER.read_text().replace('if approved != actual:', 'if False:'), newline='\n')
        ahash = digest(approval_mutant)
        approval_effects = candidate.OfflineEffects(self.root/'approval-mutant-state', 'MUTANT-TOKEN-2')
        approved = dict(self.expected, worker_path=str(approval_mutant), worker=ahash,
                        approved_args={'attempt':'approved','test_only':False,'mode':'ok','worker':str(approval_mutant.resolve())})
        spec = importlib.util.spec_from_file_location('approval_mutant', approval_mutant)
        mutant = importlib.util.module_from_spec(spec); spec.loader.exec_module(mutant)
        self.assertEqual(mutant.main(['--attempt','unapproved'], effects=approval_effects, expected=approved), 0)
        self.assertTrue({'consumed','attempt-created','mock-capture'} <= set(approval_effects.events))
        approval_result = {'guard':'worker approval comparison only','sha256':ahash,'return':0,'events':list(approval_effects.events)}
        (worker_dir/'mutant-results.json').write_text(json.dumps([testonly_result,approval_result],indent=2)+'\n', newline='\n')
