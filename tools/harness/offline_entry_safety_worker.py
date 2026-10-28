"""Synthetic-only worker boundary. It has no platform or process adapters."""
import argparse
import hashlib
import json
import sys
import hashlib
from pathlib import Path


def main(argv=None, *, effects=None, expected=None, test_only=False):
    p = argparse.ArgumentParser(add_help=False)
    p.add_argument('--attempt', required=True)
    p.add_argument('--test-only', action='store_true')
    p.add_argument('--mode', choices=('ok', 'geometry', 'exception'), default='ok')
    ns = p.parse_args(argv)
    if effects is None or expected is None:
        raise RuntimeError('no offline effect adapter configured')
    approved = expected.get('approved_args')
    actual = {'attempt': ns.attempt, 'test_only': bool(ns.test_only or test_only),
              'mode': ns.mode, 'worker': str(Path(__file__).resolve())}
    if approved != actual:
        raise RuntimeError('argument-identity-mismatch')
    if str(Path(__file__).resolve()) != str(Path(expected.get('worker_path', '')).resolve()):
        raise RuntimeError('source-identity-mismatch')
    if hashlib.sha256(Path(__file__).read_bytes()).hexdigest() != expected.get('worker'):
        raise RuntimeError('source-identity-mismatch')
    effects.audit_identity(expected)
    # Direct TestOnly is also a validated no-op; validation above has no effects.
    if ns.test_only or test_only:
        return 0
    effects.consume(ns.attempt)
    diag = {'attempt': ns.attempt, 'source': expected['launcher'], 'worker': expected['worker'],
            'mode': ns.mode, 'approved_args': approved,
            'grant_id': hashlib.sha256(str(effects.token).encode()).hexdigest()}
    try:
        attempt_dir = effects.create_attempt(ns.attempt)
    except FileExistsError as exc:
        effects.rejection(dict(diag, stage='attempt-destination', outcome='rejected',
                               error=f'{type(exc).__name__}:{exc}'))
        return 1
    effects.stage(attempt_dir, '00-started', dict(diag, stage='started', outcome='incomplete-unknown'))
    try:
        effects.launch()
        effects.stage(attempt_dir, '01-launched', dict(diag, stage='launched', outcome='incomplete-unknown'))
        if ns.mode == 'geometry':
            observed = effects.geometry()
            if observed != (640, 480):
                raise ValueError(f'wrong-geometry:{observed[0]}x{observed[1]}')
        elif ns.mode == 'exception':
            raise LookupError('injected-capture-exception')
        effects.stage(attempt_dir, '02-capture-started', dict(diag, stage='capture-started', outcome='incomplete-unknown'))
        result = effects.capture()
        effects.write_output(attempt_dir, result)
        diag.update(branch='success', result=result)
        effects.stage(attempt_dir, '03-output-persisted', dict(diag, stage='output-persisted', outcome='incomplete-until-final-record'))
        disposition = 'owned-mock-complete'
        code = 0
    except Exception as exc:
        branch = ('exception' if ns.mode == 'exception' else
                  'geometry-refusal' if ns.mode == 'geometry' else 'capture-failure')
        stage = ('injected-exception' if ns.mode == 'exception' else
                 'geometry-check' if ns.mode == 'geometry' else 'capture')
        diag.update(branch=branch, stage=stage, error=f'{type(exc).__name__}:{exc}')
        effects.stage(attempt_dir, '03-error', dict(diag, outcome='failed'))
        disposition, code = 'owned-mock-failed', 1
    effects.append_receipt(attempt_dir, diag, disposition)
    return code


if __name__ == '__main__':
    raise SystemExit(main())
