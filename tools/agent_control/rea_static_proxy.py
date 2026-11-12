"""Expose only static REA operations on the repository's identified reference.

This filters MCP, not the worker's filesystem or OS permissions. REA may create
analysis databases and start Ghidra/Java; it never receives runtime tools here.
"""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import threading

ROOT = Path(__file__).resolve().parents[2]
ALLOWED = frozenset(('binary_session', 'open_binary', 'close_binary',
    'batch_decompile', 'procedure_pseudo_code', 'procedure_assembly',
    'procedure_info', 'procedure_callers', 'procedure_callees',
    'resolve_containing_procedure', 'inspect_native_instruction',
    'read_function_instructions', 'xrefs', 'read_bytes'))


def checked_call(params, root=ROOT):
    name = params.get('name')
    args = params.get('arguments', {})
    if name not in ALLOWED or not isinstance(args, dict):
        raise ValueError('Only explicitly allowed static REA operations are available')
    args = dict(args)
    if name == 'open_binary':
        if set(args) - {'path', 'provider_id'}:
            raise ValueError('Snapshots and raw target interpretation are not enabled')
        registry = json.loads((root / 'agent/tools.windows.json').read_text(encoding='utf-8'))
        target = (root / registry['reference']['path']).resolve()
        supplied = Path(args.get('path', ''))
        if not supplied.is_absolute() or supplied.resolve() != target or not target.is_relative_to(root.resolve()):
            raise ValueError('Only the registered repository reference may be opened')
        if hashlib.sha256(target.read_bytes()).hexdigest() != registry['reference']['sha256']:
            raise ValueError('Reference identity changed')
        if args.get('provider_id', 'ghidra') != 'ghidra':
            raise ValueError('This profile uses Ghidra only')
        args['provider_id'] = 'ghidra'
    if name == 'close_binary' and args:
        raise ValueError('Close accepts no snapshot/export arguments in this profile')
    return {'name': name, 'arguments': args}


def filter_tools(result):
    return {'tools': [t for t in result.get('tools', []) if t.get('name') in ALLOWED]}


def main():
    config = json.loads((ROOT / 'agent/rea-static.runtime.json').read_text(encoding='utf-8'))
    env = os.environ.copy()
    env.update(config['env'])
    child = subprocess.Popen(config['command'], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             stderr=sys.stderr, cwd=ROOT, env=env, text=True,
                             encoding='utf-8', bufsize=1)
    pending = {}
    guard = threading.Lock()
    def emit(message):
        with guard:
            sys.stdout.write(json.dumps(message, ensure_ascii=True) + '\n')
            sys.stdout.flush()
    def relay():
        for line in child.stdout:
            try:
                msg = json.loads(line)
                if 'method' in msg and 'id' in msg:
                    # No server-initiated sampling, elicitation or client execution.
                    child.stdin.write(json.dumps({'jsonrpc':'2.0','id':msg['id'],
                        'error':{'code':-32601,'message':'Client requests disabled'}})+'\n')
                    child.stdin.flush()
                    continue
                if pending.pop(msg.get('id'), None) == 'tools/list' and 'result' in msg:
                    msg['result'] = filter_tools(msg['result'])
                emit(msg)
            except (ValueError, OSError):
                print('Invalid upstream MCP output', file=sys.stderr)
    thread = threading.Thread(target=relay, daemon=True)
    thread.start()
    try:
        for line in sys.stdin:
            msg = json.loads(line)
            method = msg.get('method')
            try:
                if method == 'tools/call':
                    msg['params'] = checked_call(msg.get('params', {}))
                elif method == 'initialize':
                    msg.setdefault('params', {})['capabilities'] = {}
                elif method not in ('tools/list', 'ping', 'notifications/initialized', 'notifications/cancelled'):
                    raise ValueError('MCP method not enabled in the static profile')
                if 'id' in msg:
                    pending[msg['id']] = method
                child.stdin.write(json.dumps(msg)+'\n')
                child.stdin.flush()
            except ValueError as error:
                if 'id' in msg:
                    emit({'jsonrpc':'2.0','id':msg['id'],
                          'error':{'code':-32602,'message':str(error)}})
    finally:
        child.stdin.close()
        try:
            child.wait(timeout=10)
        except subprocess.TimeoutExpired:
            child.terminate()
        thread.join(timeout=2)


if __name__ == '__main__':
    main()
