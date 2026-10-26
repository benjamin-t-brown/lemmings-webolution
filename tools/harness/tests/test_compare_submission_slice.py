import copy
import json
import tempfile
import unittest
from pathlib import Path

from tools.harness.compare_submission_slice import SliceError, compare, load, record_raw


ROOT = Path(__file__).resolve().parents[1]
FIX = ROOT / "fixtures" / "submission-slice"


def read_rows():
    return [json.loads(line) for line in (FIX / "baseline-a.jsonl").read_text(encoding="utf-8").splitlines()]


def result(mutator=None):
    left = read_rows()
    right = copy.deepcopy(left)
    if mutator:
        mutator(right)
    with tempfile.TemporaryDirectory() as directory:
        a, b = Path(directory) / "a.jsonl", Path(directory) / "b.jsonl"
        a.write_text("".join(json.dumps(x, separators=(",", ":")) + "\n" for x in left), encoding="utf-8")
        b.write_text("".join(json.dumps(x, separators=(",", ":")) + "\n" for x in right), encoding="utf-8")
        return compare(a, b)


def identical(mutator):
    rows = read_rows()
    mutator(rows)
    with tempfile.TemporaryDirectory() as directory:
        paths = [Path(directory) / (str(i) + ".jsonl") for i in range(2)]
        payload = "".join(json.dumps(x, separators=(",", ":")) + "\n" for x in rows)
        for path in paths:
            path.write_text(payload, encoding="utf-8")
        return compare(*paths)


class ComparatorTests(unittest.TestCase):
    def test_committed_baselines_complete_equal_and_raw_hashes(self):
        r = compare(FIX / "baseline-a.jsonl", FIX / "baseline-b.jsonl")
        self.assertEqual(r["status"], "complete_equal")
        a, b = load(FIX / "baseline-a.jsonl"), load(FIX / "baseline-b.jsonl")
        self.assertEqual(len(a["raw_bytes"]), (FIX / "baseline-a.jsonl").stat().st_size)
        self.assertEqual(len(a["raw_sha256"]), 64)
        self.assertEqual(a["raw_bytes"], (FIX / "baseline-a.jsonl").read_bytes())
        self.assertEqual(a["raw_sha256"], b["raw_sha256"])
        self.assertEqual(a["raw_records"][1]["draw"]["vertices"][0]["specular"], "unknown")
        self.assertNotIn("specular", a["comparable"][1]["draw"]["vertices"][0])

    def test_raw_recorder_is_byte_exact_and_create_only(self):
        source = (FIX / "baseline-a.jsonl").read_bytes()
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory) / "raw.jsonl"
            metadata = record_raw(target, (source[:37], source[37:]))
            self.assertEqual(target.read_bytes(), source)
            self.assertEqual(metadata["byte_count"], len(source))
            with self.assertRaises(FileExistsError):
                record_raw(target, (source,))

    def test_each_discriminator_alone(self):
        mutations = [
            lambda r: r[1]["draw"]["vertices"][0]["xyzw"].__setitem__(0, 1.125),
            lambda r: r[0]["state_history"][0]["value"].__setitem__("blend", True),
            None,
            lambda r: r[1]["draw"]["resource"].__setitem__("content_version", 5),
            lambda r: r[1]["draw"]["resource"].__setitem__("generation", 3),
            lambda r: r[1]["draw"]["viewport"].__setitem__("width", 639),
            lambda r: r[1]["draw"]["clear"].__setitem__("color", 0),
        ]
        for index, mutate in enumerate(mutations):
            with self.subTest(field=index):
                if index == 2:
                    def swap_draw_payloads(rows):
                        rows[1]["draw"], rows[2]["draw"] = rows[2]["draw"], rows[1]["draw"]
                    self.assertEqual(result(swap_draw_payloads)["status"], "complete_different")
                elif mutate is not None:
                    self.assertNotEqual(result(mutate)["status"], "complete_equal")

    def test_coverage_mutations_are_indeterminate(self):
        for key, value in (("dropped", 1), ("seen", 1), ("serialized", 1),
                           ("decode_failed", 1), ("filtered", 1), ("end_sequence", 12)):
            with self.subTest(key=key):
                self.assertEqual(result(lambda r: r[0]["coverage"].__setitem__(key, value))["status"], "indeterminate")

    def test_identical_incomplete_logs_not_equal(self):
        r = read_rows()
        r[0]["coverage"]["dropped"] = 1
        with tempfile.TemporaryDirectory() as d:
            paths = [Path(d) / (str(i) + ".jsonl") for i in range(2)]
            for p in paths:
                p.write_text("".join(json.dumps(x) + "\n" for x in r), encoding="utf-8")
            self.assertEqual(compare(*paths)["status"], "indeterminate")

    def test_four_av_false_positives_rejected_identically(self):
        cases = [
            lambda r: r[0]["state_history"][0].__setitem__("value", {}),
            lambda r: r[0]["state_history"][0].__setitem__("sequence", 999),
            lambda r: r[1]["draw"].__setitem__("resource", {}),
            lambda r: r[1]["draw"].__setitem__("space", "unknown"),
        ]
        for case in cases:
            with self.subTest(case=case):
                with self.assertRaises(SliceError):
                    identical(case)

    def test_state_missing_failed_unknown_and_missing_checkpoint(self):
        changes = [lambda r: r[0].__setitem__("state_history", []),
                   lambda r: r[0]["state_history"][0].__setitem__("return", "failure"),
                   lambda r: r[0]["state_history"][0].__setitem__("return", "unknown"),
                   lambda r: r[0]["state_history"][0].__setitem__("value", None)]
        for mutate in changes:
            with self.assertRaises(SliceError):
                result(mutate)

    def test_boundary_coordinate_space_and_unknown_specular_scope(self):
        self.assertEqual(result(lambda r: r[0].__setitem__("boundary", "other"))["status"], "complete_different")
        with self.assertRaises(SliceError):
            result(lambda r: r[1]["draw"].__setitem__("space", "world"))

    def test_unknown_only_specular_domain(self):
        def set_known(rows):
            rows[1]["draw"]["vertices"][0]["specular"] = 0x12345678

        # One-sided and identical known values cannot enter the unknown-only compare view.
        with self.assertRaises(SliceError):
            result(set_known)
        with self.assertRaises(SliceError):
            identical(set_known)

        for malformed in ({"unknown": True}, 1, True, None):
            with self.subTest(value=malformed):
                with self.assertRaises(SliceError):
                    result(lambda r: r[1]["draw"]["vertices"][0].__setitem__("specular", malformed))

        # The exact supported unknown sentinel remains accepted.
        self.assertEqual(compare(FIX / "baseline-a.jsonl", FIX / "baseline-b.jsonl")["status"], "complete_equal")

    def test_unsupported_event_is_indeterminate(self):
        with self.assertRaises(SliceError):
            result(lambda r: r[1].__setitem__("kind", "mystery"))

    def test_strict_typed_comparison_and_malformed_inputs(self):
        # JSON booleans cannot masquerade as integer texture IDs.
        with self.assertRaises(SliceError):
            result(lambda r: r[0]["state_history"][0]["value"].__setitem__("texture", True))
        self.assertEqual(result(lambda r: r[1]["draw"]["vertices"][0]["xyzw"].__setitem__(0, 1))["status"], "complete_different")
        malformed = [
            lambda r: r[1]["draw"].__setitem__("primitive_count", 2),
            lambda r: r[1]["draw"]["resource"].__setitem__("generation", True),
            lambda r: r[1]["draw"]["viewport"].__setitem__("width", False),
            lambda r: r[1]["draw"]["vertices"][0].__setitem__("uv", [float("nan"), 0]),
            lambda r: r[1]["draw"]["vertices"][0]["xyzw"].__setitem__(0, 1 << 4096),
        ]
        for mutate in malformed:
            with self.subTest(mutate=mutate):
                with self.assertRaises((SliceError, ValueError)):
                    result(mutate)

    def test_duplicate_json_key_rejected(self):
        raw = (FIX / "baseline-a.jsonl").read_text(encoding="utf-8")
        raw = raw.replace('"capture":"synthetic-scene-fan-v1"',
                          '"capture":"synthetic-scene-fan-v1","capture":"duplicate"', 1)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "duplicate.jsonl"
            path.write_text(raw, encoding="utf-8")
            with self.assertRaises(SliceError):
                load(path)


if __name__ == "__main__":
    unittest.main()
