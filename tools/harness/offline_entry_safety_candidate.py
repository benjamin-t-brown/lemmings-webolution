"""Unwired fail-closed offline launcher/evidence boundary; never starts a process."""
import argparse
import hashlib
import json
import os
import sys
import importlib.util
import uuid
from pathlib import Path



def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


class OfflineEffects:
    """Scratch filesystem ledger and synthetic effects; deliberately no OS launch APIs."""
    def __init__(self, root, token, output_root=None):
        # root is the stable, trusted local authorization store. Output naming is
        # deliberately separate: a caller-selected attempt directory cannot mint a grant.
        self.root, self.token = Path(root), token
        self.output_root = Path(output_root) if output_root is not None else self.root
        self.root.mkdir(parents=True, exist_ok=True)
        self.output_root.mkdir(parents=True, exist_ok=True)
        self.events = []
        self.launch_events = []

    def audit_identity(self, expected):
        if sha(expected['launcher_path']) != expected['launcher'] or sha(expected['worker_path']) != expected['worker']:
            raise RuntimeError('source-identity-mismatch')
        if expected.get('dependencies') != {'python': sys.version_info.major, 'worker_api': 1}:
            raise RuntimeError('dependency-identity-mismatch')
        self.events.append('identity-ok')

    def consume(self, attempt):
        state = self.root / 'authorization.json'
        payload = {'token': self.token, 'domain': 'synthetic-offline-only'}
        try:
            fd = os.open(state, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
        except FileExistsError:
            raise RuntimeError('authorization-already-consumed')
        with os.fdopen(fd, 'w') as f:
            f.write(json.dumps(payload, sort_keys=True)); f.flush(); os.fsync(f.fileno())
        self.events.append('consumed')

    def create_attempt(self, attempt):
        p = self.output_root / 'attempts' / attempt
        p.parent.mkdir(parents=True, exist_ok=True)
        p.mkdir()
        self.events.append('attempt-created')
        return p

    def write_exclusive_json(self, path, data):
        raw = (json.dumps(data, sort_keys=True, separators=(',', ':')) + '\n').encode()
        fd = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
        with os.fdopen(fd, 'wb') as f:
            f.write(raw); f.flush(); os.fsync(f.fileno())

    def stage(self, directory, name, data):
        raw = (json.dumps(dict(data, stage_record=name), sort_keys=True, separators=(',', ':')) + '\n').encode()
        fd = os.open(Path(directory) / 'stages.jsonl', os.O_CREAT | os.O_APPEND | os.O_WRONLY, 0o600)
        with os.fdopen(fd, 'ab') as f:
            f.write(raw); f.flush(); os.fsync(f.fileno())
        self.events.append('stage-' + name)

    def rejection(self, data):
        directory = self.output_root / 'rejections'
        directory.mkdir(parents=True, exist_ok=True)
        data = dict(data, grant_id=hashlib.sha256(str(self.token).encode()).hexdigest())
        path = directory / f"{data.get('attempt', 'unknown')}-{uuid.uuid4().hex}.json"
        self.write_exclusive_json(path, data)
        self.events.append('rejection')
        return path

    def write_output(self, directory, result):
        self.write_exclusive_json(Path(directory) / 'synthetic-output.json',
                                  {'kind': 'synthetic-output', 'value': result})
        self.events.append('synthetic-output')

    def geometry(self): return (640, 480)
    def launch(self):
        # This represents the actual launcher→worker operational boundary, not capture.
        # The test records disk claim state at precisely this synthetic launch event.
        claimed = (self.root / 'authorization.json').is_file()
        self.launch_events.append({'claim_present': claimed})
        self.events.append('mock-launch')
    def capture(self):
        self.events.append('mock-capture')
        return 'synthetic-result'

    def append_receipt(self, directory, data, disposition):
        data['disposition'] = disposition
        raw = (json.dumps(data, sort_keys=True, separators=(',', ':')) + '\n').encode()
        path = Path(directory) / 'receipt.jsonl'
        fd = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
        with os.fdopen(fd, 'wb') as f:
            f.write(raw); f.flush(); os.fsync(f.fileno())
        self.events.append('receipt')


def run(argv=None, *, effects=None, expected=None):
    parser = argparse.ArgumentParser(prog='offline-entry-safety')
    parser.add_argument('--attempt', required=True)
    parser.add_argument('--test-only', action='store_true')
    parser.add_argument('--mode', choices=('ok', 'geometry', 'exception'), default='ok')
    parser.add_argument('--worker', required=True)
    args = parser.parse_args(argv)
    if effects is None or expected is None:
        raise RuntimeError('live execution disabled: explicit offline mocks required')
    # Approval is fixed independently of invocation. Compare parsed semantics, not raw argv
    # spelling/order; worker location is also an approved supported argument.
    actual = {'attempt': args.attempt, 'test_only': args.test_only,
              'mode': args.mode, 'worker': str(Path(args.worker).resolve())}
    if actual != expected.get('approved_args'):
        raise RuntimeError('argument-identity-mismatch')
    if str(Path(__file__).resolve()) != str(Path(expected['launcher_path']).resolve()):
        raise RuntimeError('source-identity-mismatch')
    effects.audit_identity(expected)
    # Verify exact worker identity before loading/calling its boundary.
    if sha(args.worker) != expected['worker']:
        raise RuntimeError('source-identity-mismatch')
    # TestOnly is a validated no-op: verify this launcher, worker and parsed
    # approval first, but never load the worker or touch operational effects.
    if args.test_only:
        return 0
    spec = importlib.util.spec_from_file_location('approved_offline_worker', args.worker)
    if spec is None or spec.loader is None:
        raise RuntimeError('worker-load-failed')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.main(['--attempt', args.attempt, '--mode', args.mode] +
                       (['--test-only'] if args.test_only else []),
                       effects=effects, expected=expected)


def main(argv=None):
    # No adapter, token, native/imported desktop component, or live authority by default.
    return run(argv)


if __name__ == '__main__':
    raise SystemExit(main())
