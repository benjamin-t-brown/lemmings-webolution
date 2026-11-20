"""Safety/lifecycle tests use synthetic Python workers, never an AI or game."""
import contextlib
import ctypes
import io
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import unittest
from unittest.mock import patch

import supervise as supervisor
from verify import check_test_inventory, repository_path
from winjob import run_tree


def is_alive(pid):
    from ctypes import wintypes as W
    kernel = ctypes.WinDLL('kernel32', use_last_error=True)
    kernel.OpenProcess.argtypes = [W.DWORD, W.BOOL, W.DWORD]
    kernel.OpenProcess.restype = W.HANDLE
    kernel.WaitForSingleObject.argtypes = [W.HANDLE, W.DWORD]
    kernel.WaitForSingleObject.restype = W.DWORD
    kernel.CloseHandle.argtypes = [W.HANDLE]
    handle = kernel.OpenProcess(0x100000, False, pid)
    if not handle:
        return False
    try:
        return kernel.WaitForSingleObject(handle, 0) == 0x102
    finally:
        kernel.CloseHandle(handle)


@unittest.skipUnless(os.name == 'nt', 'Windows Job Object tests')
class JobTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='agent_control_')
        self.root = Path(self.temp.name)

    def tearDown(self):
        self.temp.cleanup()

    def run_script(self, script, seconds=4, stop=None):
        path = self.root / 'worker.py'
        path.write_text(script)
        return run_tree([sys.executable, str(path)], cwd=self.root, stdin_path=None,
                        stdout_path=self.root / 'stdout.log',
                        stderr_path=self.root / 'stderr.log',
                        deadline=time.monotonic() + seconds, stop_path=stop)

    def test_exit_code_and_output(self):
        result = self.run_script('import sys\nprint("synthetic worker")\nsys.exit(7)')
        self.assertEqual(result['exit_code'], 7)
        self.assertEqual(result['reason'], 'exited')
        self.assertIn('synthetic worker', (self.root / 'stdout.log').read_text())

    def test_timeout_kills_child_tree(self):
        result = self.run_script(
            'import subprocess,sys,time\nfrom pathlib import Path\n'
            'child=subprocess.Popen([sys.executable,"-c","import time;time.sleep(30)"])\n'
            'Path("child.pid").write_text(str(child.pid))\ntime.sleep(30)', seconds=1.5)
        self.assertEqual(result['reason'], 'timeout')
        child = int((self.root / 'child.pid').read_text())
        time.sleep(0.2)
        self.assertFalse(is_alive(result['pid']))
        self.assertFalse(is_alive(child))

    def test_active_cancellation(self):
        stop = self.root / 'STOP'
        result = self.run_script('import time\nfrom pathlib import Path\n'
                                 'Path("STOP").touch()\ntime.sleep(30)', stop=stop)
        self.assertEqual(result['reason'], 'cancelled')
        self.assertFalse(is_alive(result['pid']))

    def test_root_exit_cleans_background_child(self):
        result = self.run_script(
            'import subprocess,sys\nfrom pathlib import Path\n'
            'p=subprocess.Popen([sys.executable,"-c","import time;time.sleep(30)"])\n'
            'Path("child.pid").write_text(str(p.pid))')
        self.assertEqual(result['exit_code'], 0)
        time.sleep(0.2)
        self.assertFalse(is_alive(int((self.root / 'child.pid').read_text())))

    def test_supervisor_crash_closes_job(self):
        module_dir = Path(__file__).resolve().parent
        grandchild = self.root / 'long_worker.py'
        grandchild.write_text('import os,time\nfrom pathlib import Path\n'
                              'Path("crash-child.pid").write_text(str(os.getpid()))\n'
                              'time.sleep(30)')
        launcher = self.root / 'crash_launcher.py'
        launcher.write_text(
            f'import sys\nsys.path.insert(0,{str(module_dir)!r})\n'
            'import threading,time,os\nfrom pathlib import Path\nfrom winjob import run_tree\n'
            f'worker={str(grandchild)!r}\n'
            't=threading.Thread(target=lambda:run_tree([sys.executable,worker],cwd=Path.cwd(),'
            'stdin_path=None,stdout_path="out",stderr_path="err",deadline=time.monotonic()+30))\n'
            't.start()\n'
            'while not Path("crash-child.pid").exists():time.sleep(0.05)\n'
            'os._exit(0)\n')
        subprocess.run([sys.executable, str(launcher)], cwd=self.root, check=True, timeout=10)
        time.sleep(0.3)
        self.assertFalse(is_alive(int((self.root / 'crash-child.pid').read_text())))


class QueueTests(unittest.TestCase):
    def task(self, **overrides):
        task = {'id': 'safe-001', 'status': 'ready', 'depends_on': [],
                'requires_original_execution': False, 'verification': 'main'}
        return task | overrides

    def test_original_execution_is_rejected(self):
        with self.assertRaises(ValueError):
            supervisor.select_task({'tasks': [self.task(requires_original_execution=True)]}, 'safe-001')

    def test_dependencies_and_duplicate_ids(self):
        with self.assertRaises(ValueError):
            supervisor.select_task({'tasks': [self.task(depends_on=['missing'])]}, 'safe-001')
        with self.assertRaises(ValueError):
            supervisor.select_task({'tasks': [self.task(), self.task()]}, 'safe-001')

    def test_transcript_failure_overrides_zero_exit(self):
        with tempfile.TemporaryDirectory() as directory:
            transcript = Path(directory) / 'events'
            transcript.write_text('{"type":"result","subtype":"error_during_execution","is_error":true}\n')
            self.assertFalse(supervisor.successful_transcript('claude', transcript))
            transcript.write_text('{"type":"turn.completed"}\n{"type":"turn.failed"}\n')
            self.assertFalse(supervisor.successful_transcript('codex', transcript))

    def test_codex_reconnect_notices_are_not_failures(self):
        with tempfile.TemporaryDirectory() as directory:
            transcript = Path(directory) / 'events'
            transcript.write_text('{"type":"error","message":"Reconnecting... 2/5 (503)"}\n'
                                  '{"type":"turn.completed"}\n')
            self.assertTrue(supervisor.successful_transcript('codex', transcript))
            transcript.write_text('{"type":"error","message":"Reconnecting... 5/5 (503)"}\n')
            self.assertFalse(supervisor.successful_transcript('codex', transcript))
            transcript.write_text('{"type":"error","message":"unauthorized"}\n{"type":"turn.completed"}\n')
            self.assertFalse(supervisor.successful_transcript('codex', transcript))

    def test_verification_rejects_oracles_but_accepts_source_array_test(self):
        original = Path('C:/game/Lemmings Revolution.exe')
        check_test_inventory([{'name': 'native_int_array',
                               'command': ['C:/build/test_native_int_array.exe']}], original)
        for tests in ([], [{'name': 'native_actor_update', 'command': ['oracle.exe']}],
                      [{'name': 'renamed', 'command': ['python.exe', str(original)]}]):
            with self.assertRaises(RuntimeError):
                check_test_inventory(tests, original)

    def test_reference_paths_are_relative_to_repository_not_caller(self):
        from verify import ROOT
        expected = ROOT / 'Lemmings Revolution/Lemmings Revolution.exe'
        with tempfile.TemporaryDirectory() as directory:
            previous = Path.cwd()
            try:
                os.chdir(directory)
                self.assertEqual(repository_path('Lemmings Revolution/Lemmings Revolution.exe'), expected)
                with self.assertRaises(RuntimeError):
                    repository_path('../outside-game.exe')
            finally:
                os.chdir(previous)

    def test_one_hour_cap(self):
        with patch.object(sys, 'argv', ['supervise', '--backend', 'codex', '--task', 'safe-001', '--minutes', '61']):
            with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as result:
                supervisor.main()
            self.assertEqual(result.exception.code, 2)

    def test_synthetic_end_to_end_and_lock(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'agent').mkdir()
            (root / 'tools/agent_control').mkdir(parents=True)
            (root / 'AGENTS.md').write_text('Synthetic test instructions')
            git = json.loads((supervisor.ROOT / 'agent/tools.windows.json').read_text())['tools']['git']['path']
            subprocess.run([git, 'init', '-q', str(root)], check=True)
            registry = {'tools': {'git': {'path': git}, 'python': {'path': sys.executable}}}
            (root / 'agent/tools.windows.json').write_text(json.dumps(registry))
            queue = root / 'agent/tasks.json'
            queue.write_text(json.dumps({'tasks': [self.task() | {'scope': 'synthetic fixture'}]}))
            (root / 'tools/agent_control/verify.py').write_text('print("synthetic verification passed")\n')
            fake = root / 'fake.py'
            fake.write_text('import sys\nsys.stdin.read()\nprint(\'{"type":"turn.completed"}\')\n')
            argv = ['supervise', '--backend', 'codex', '--task', 'safe-001',
                    '--execute', '--allow-dirty', '--minutes', '1']
            with patch.object(supervisor, 'ROOT', root), patch.object(supervisor, 'QUEUE', queue), \
                    patch.object(supervisor, 'command_for', return_value=[sys.executable, str(fake)]), \
                    patch.object(sys, 'argv', argv), contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(supervisor.main(), 0)
                self.assertEqual(json.loads(queue.read_text())['tasks'][0]['status'], 'needs_review')
                runs = list((root / 'build/agent-runs').glob('*/result.json'))
                self.assertEqual(len(runs), 1)
                self.assertEqual(json.loads(runs[0].read_text())['verification']['exit_code'], 0)
                data = json.loads(queue.read_text())
                data['tasks'][0]['status'] = 'ready'
                queue.write_text(json.dumps(data))
                lock = root / 'build/agent-runs/supervisor.lock'
                lock.write_text('other supervisor')
                with self.assertRaises(FileExistsError):
                    supervisor.main()
                self.assertEqual(lock.read_text(), 'other supervisor')

    def test_failed_workers_checks_and_control_edits_stay_blocked(self):
        git = json.loads((supervisor.ROOT / 'agent/tools.windows.json').read_text())['tools']['git']['path']
        for scenario in ('worker_error', 'verify_error', 'control_edit', 'queue_conflict'):
            with self.subTest(scenario=scenario), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                (root / 'agent').mkdir()
                (root / 'tools/agent_control').mkdir(parents=True)
                (root / 'AGENTS.md').write_text('Synthetic task guidance')
                subprocess.run([git, 'init', '-q', str(root)], check=True)
                registry = {'tools': {'git': {'path': git}, 'python': {'path': sys.executable}}}
                (root / 'agent/tools.windows.json').write_text(json.dumps(registry))
                queue = root / 'agent/tasks.json'
                queue.write_text(json.dumps({'tasks': [self.task()]}))
                verifier = root / 'tools/agent_control/verify.py'
                verifier.write_text('from pathlib import Path\nPath("verification-ran").touch()\n'
                                    + ('raise SystemExit(1)\n' if scenario == 'verify_error' else ''))
                script = 'import sys,json\nfrom pathlib import Path\nsys.stdin.read()\n'
                if scenario == 'control_edit':
                    script += 'Path("AGENTS.md").write_text("changed controls")\n'
                if scenario == 'queue_conflict':
                    script += ('p=Path("agent/tasks.json")\nd=json.loads(p.read_text())\n'
                               'd["external_change"]=True\np.write_text(json.dumps(d))\n')
                script += 'print(\'{"type":"turn.completed"}\')\n'
                if scenario == 'worker_error':
                    script += 'raise SystemExit(1)\n'
                fake = root / 'fake.py'
                fake.write_text(script)
                with patch.object(supervisor, 'ROOT', root), patch.object(supervisor, 'QUEUE', queue), \
                        patch.object(supervisor, 'command_for', return_value=[sys.executable, str(fake)]), \
                        patch.object(sys, 'argv', ['supervise', '--backend', 'codex', '--task', 'safe-001',
                                                  '--execute', '--allow-dirty', '--minutes', '1']), \
                        contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(supervisor.main(), 1)
                result_path = next((root / 'build/agent-runs').glob('*/result.json'))
                result = json.loads(result_path.read_text())
                self.assertEqual(result['status'], 'blocked')
                self.assertEqual((root / 'verification-ran').exists(), scenario == 'verify_error')
                if scenario == 'queue_conflict':
                    self.assertTrue(json.loads(queue.read_text())['external_change'])
                    self.assertIn('queue_conflict', result)
                else:
                    self.assertEqual(json.loads(queue.read_text())['tasks'][0]['status'], 'blocked')


if __name__ == '__main__':
    unittest.main(verbosity=2)
