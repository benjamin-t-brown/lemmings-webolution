"""Static research profile checks; no model or original binary execution."""
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

import supervise
import rea_static_proxy as proxy


class StaticProfileTests(unittest.TestCase):
    def registry(self):
        return {'tools': {'claude': {'path': sys.executable}},
                'agent_models': {'claude': {'model': 'claude-sonnet-5-5', 'effort': 'high'}}}

    def test_only_authorized_research_gets_mcp(self):
        for task, readonly in [(None, False), ({'kind':'research'}, False),
                ({'kind':'research','user_authorized':True}, True),
                ({'kind':'implementation','user_authorized':True}, True)]:
            cmd = supervise.command_for('claude', self.registry(), Path('.'),
                                         task=task, read_only=readonly)
            self.assertIn('--safe-mode', cmd)
            self.assertNotIn('--mcp-config', cmd)
        cmd = supervise.command_for('claude', self.registry(), Path('.'),
                                     task={'kind':'research','user_authorized':True})
        self.assertNotIn('--safe-mode', cmd)
        self.assertIn('--strict-mcp-config', cmd)
        self.assertEqual(cmd[cmd.index('--setting-sources')+1], '')
        self.assertIn('--disable-slash-commands', cmd)
        self.assertEqual(cmd.count('--allowedTools'), 1)
        self.assertIn('Bash(python tools/agent_control/verify.py --profile all)',cmd)
        self.assertEqual({s.removeprefix('mcp__rea_static__') for s in cmd
                          if s.startswith('mcp__rea_static__')}, proxy.ALLOWED)

    def test_codex_research_profile_is_explicit_and_scoped(self):
        import tomllib
        registry = self.registry()
        registry['tools']['codex'] = {'path': sys.executable}
        registry['agent_models']['codex'] = {'model': 'gpt-6.1-sol', 'effort': 'high'}
        for task, readonly in [(None, False), ({'kind':'research'}, False),
                ({'kind':'research','user_authorized':True}, True)]:
            cmd = supervise.command_for('codex', registry, Path('.'),task=task,read_only=readonly)
            self.assertFalse(any(s.startswith('mcp_servers=') for s in cmd))
        cmd = supervise.command_for('codex', registry, Path('.'),
                task={'kind':'research','user_authorized':True})
        self.assertIn('--ignore-user-config', cmd)
        self.assertIn('approval_policy="never"', cmd)
        self.assertEqual(cmd[cmd.index('--sandbox')+1], 'workspace-write')
        table = tomllib.loads(next(s for s in cmd if s.startswith('mcp_servers=')))['mcp_servers']
        self.assertEqual(set(table), {'rea_static'})
        server = table['rea_static']
        self.assertEqual(set(server['enabled_tools']), proxy.ALLOWED)
        self.assertEqual(set(server['tools']), proxy.ALLOWED)
        self.assertTrue(server['required'])
        self.assertTrue(all(v == {'approval_mode':'approve'} for v in server['tools'].values()))
        self.assertIn('rea_static_proxy.py', server['args'][-1])

    def test_implementation_workers_get_same_static_profile(self):
        registry = self.registry()
        registry['tools']['codex'] = {'path': sys.executable}
        registry['agent_models']['codex'] = {'model':'gpt-6.1-sol','effort':'high'}
        for backend in ('claude', 'codex'):
            for task in ({'id':'impl'}, {'id':'impl','kind':'implementation'}):
                with self.subTest(backend=backend, task=task):
                    actual = supervise.command_for(backend,registry,Path('.'),task=task)
                    expected = supervise.command_for(backend,registry,Path('.'),
                        task={'kind':'research','user_authorized':True})
                    self.assertEqual(actual, expected)
                    review = supervise.command_for(backend,registry,Path('.'),task=task,read_only=True)
                    self.assertFalse(any('mcp_servers=' in a or a == '--mcp-config' for a in review))
        self.assertFalse(supervise.static_rea_enabled({'kind':'planning'},False))
        self.assertFalse(supervise.static_rea_enabled({'id':'unsafe','requires_original_execution':True},False))
        self.assertFalse(supervise.static_rea_enabled({'kind':'research'},False))

    def test_explicit_readonly_roles_get_only_static_access(self):
        import tomllib
        registry = self.registry()
        registry['tools']['codex'] = {'path': sys.executable}
        registry['agent_models']['codex'] = {'model':'gpt-6.1-sol','effort':'high'}
        for backend in ('claude', 'codex'):
            for role in ('plan', 'review', 'review_completion'):
                with self.subTest(backend=backend, role=role):
                    cmd = supervise.command_for(backend, registry, Path('.'), read_only=True, role=role)
                    if backend == 'claude':
                        self.assertEqual(cmd[cmd.index('--tools')+1], 'Read,Glob,Grep')
                        self.assertEqual(cmd[cmd.index('--permission-mode')+1], 'dontAsk')
                        self.assertEqual(cmd.count('--allowedTools'), 1)
                        allowed = cmd[cmd.index('--allowedTools')+1:cmd.index('--setting-sources')]
                        self.assertEqual(set(allowed), {'mcp__rea_static__'+n for n in proxy.ALLOWED})
                        self.assertIn('--strict-mcp-config', cmd)
                        self.assertNotIn('--safe-mode', cmd)
                    else:
                        self.assertEqual(cmd[cmd.index('--sandbox')+1], 'read-only')
                        self.assertIn('approval_policy="never"', cmd)
                        table = tomllib.loads(next(x for x in cmd if x.startswith('mcp_servers=')))['mcp_servers']
                        self.assertEqual(set(table), {'rea_static'})
                        self.assertEqual(set(table['rea_static']['enabled_tools']), proxy.ALLOWED)
        self.assertFalse(supervise.static_rea_enabled(None, True, 'unknown'))
        self.assertFalse(supervise.static_rea_enabled(None, True))
        self.assertFalse(supervise.static_rea_enabled(None, False, 'plan'))

    def test_runtime_and_other_mcp_methods_not_advertised(self):
        result=proxy.filter_tools({'tools':[{'name':'batch_decompile'},
            {'name':'launch_binary'},{'name':'run_script'},{'name':'debugger'}]})
        self.assertEqual(result,{'tools':[{'name':'batch_decompile'}]})
        for name in ('launch_binary','run_script','debugger','rename_procedure'):
            with self.assertRaises(ValueError): proxy.checked_call({'name':name})
        with self.assertRaises(ValueError):
            proxy.checked_call({'name':'close_binary','arguments':{'snapshot_path':'outside'}})

    def test_reference_identity_target_and_provider_enforced(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp); (root/'agent').mkdir(); exe=root/'reference.exe';exe.write_bytes(b'not executable')
            (root/'agent/tools.windows.json').write_text(json.dumps({'reference':{
                'path':'reference.exe','sha256':hashlib.sha256(exe.read_bytes()).hexdigest()}}))
            args={'path':str(exe)}
            self.assertEqual(proxy.checked_call({'name':'open_binary','arguments':args},root)
                             ['arguments']['provider_id'],'ghidra')
            for extra in ({'path':str(root/'other.exe')},{'provider_id':'auto'},
                          {'snapshot_path':'outside'},{'format':'raw-binary'}):
                with self.assertRaises(ValueError):
                    proxy.checked_call({'name':'open_binary','arguments':args|extra},root)
            exe.write_bytes(b'changed')
            with self.assertRaises(ValueError):
                proxy.checked_call({'name':'open_binary','arguments':args},root)

    def test_new_config_is_hash_guarded(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);(root/'agent').mkdir()
            with patch.object(supervise,'ROOT',root):
                for name in ('rea-static.runtime.json','rea-static.mcp.json','claude-research.settings.json'):
                    path=root/'agent'/name;path.write_text('{}')
                    before=supervise.control_hashes();self.assertIn('agent/'+name,before)
                    path.write_text('{"changed":true}')
                    self.assertNotEqual(before,supervise.control_hashes())


if __name__ == '__main__':
    unittest.main(verbosity=2)
