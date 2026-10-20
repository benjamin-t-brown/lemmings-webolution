"""Exercise malformed framework inputs using isolated repository snapshots."""

import importlib.util
import json
import shutil
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("framework", ROOT / "scripts/check_agent_framework.py")
framework = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(framework)


class FrameworkChecks(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        # Copy only framework reference material; never operate on the real queue or game.
        for name in ["docs", ".claude", ".cursor", ".opencode"]:
            shutil.copytree(ROOT / name, self.root / name,
                            ignore=shutil.ignore_patterns("worktrees", "sessions"))
        for name in ["AGENTS.md", "PLAN.md"]:
            shutil.copy2(ROOT / name, self.root / name)
        self.write("docs/tasks.json", json.dumps({"schema_version": 1, "active": None, "queue": []}))

    def write(self, path, content):
        target = self.root / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(content, encoding="utf-8")

    def test_valid_framework(self):
        self.assertEqual(framework.validate(self.root), [])

    def test_broken_link_and_anchor(self):
        with (self.root / "AGENTS.md").open("a", encoding="utf-8") as stream:
            stream.write("\n[missing](missing.md)\n[anchor](docs/ORCHESTRATION.md#absent)\n")
        errors = framework.validate(self.root)
        self.assertTrue(any("broken link" in e for e in errors))
        self.assertTrue(any("missing heading anchor" in e for e in errors))

    def test_missing_contract_and_inherited_pin(self):
        self.write(".cursor/agents/round-agent.md", "---\nmodel: inherit\n---\n")
        errors = framework.validate(self.root)
        self.assertTrue(any("missing shared contract" in e for e in errors))
        self.assertTrue(any("explicit model" in e for e in errors))

    def test_claude_workers_reject_model_pin(self):
        path = self.root / ".claude/agents/round-agent.md"
        path.write_text(path.read_text(encoding="utf-8").replace(
            "tools:", "model: sonnet\ntools:", 1), encoding="utf-8")
        errors = framework.validate(self.root)
        self.assertTrue(any("must not pin a model" in e for e in errors))

    def test_facilitator_commands_require_invocation_contract(self):
        self.write(".opencode/commands/facilitator.md",
                   "---\ndescription: Read rules\n---\n"
                   "Read [contract](../../docs/FACILITATOR.md).\n")
        errors = framework.validate(self.root)
        self.assertTrue(any("missing facilitator invocation pointer" in e for e in errors))
        self.assertTrue(any("missing facilitator agent binding" in e for e in errors))
        self.assertFalse(any("omit model override" in e for e in errors))

    def test_opencode_facilitator_cannot_be_a_subagent(self):
        path = ".opencode/agent/facilitator.md"
        self.write(path, (self.root / path).read_text(encoding="utf-8")
                   .replace("mode: primary", "mode: subagent"))
        errors = framework.validate(self.root)
        self.assertTrue(any("must be a primary agent" in e for e in errors))

    def test_opencode_facilitator_command_cannot_override_pinned_agent(self):
        path = ".opencode/commands/facilitator.md"
        text = (self.root / path).read_text(encoding="utf-8")
        text = text.replace("agent: facilitator", "agent: facilitator\nmodel: openrouter/other-model#high")
        self.write(path, text)
        errors = framework.validate(self.root)
        self.assertTrue(any("omit model override" in e for e in errors))

    def test_opencode_facilitator_variant_and_session_binding(self):
        path = ".opencode/agent/facilitator.md"
        self.write(path, (self.root / path).read_text(encoding="utf-8")
                   .replace("variant: max", "variant: high"))
        path = ".opencode/commands/facilitator.md"
        self.write(path, (self.root / path).read_text(encoding="utf-8")
                   .replace("subtask: false", "subtask: true"))
        errors = framework.validate(self.root)
        self.assertTrue(any("variant must be max" in e for e in errors))
        self.assertTrue(any("must run in the current session" in e for e in errors))

    def test_opencode_workers_use_luna_and_keep_their_permissions(self):
        for role, edit in [("round-agent", "allow"), ("explore-agent", "deny")]:
            with self.subTest(role=role):
                text = (self.root / f".opencode/agent/{role}.md").read_text(encoding="utf-8")
                self.assertEqual(framework.frontmatter_field(text, "model"), "openai/gpt-6-luna")
                self.assertEqual(framework.frontmatter_field(text, "variant"), "high")
                self.assertEqual(framework.frontmatter_field(text, "mode"), "subagent")
                self.assertIn(f"  edit: {edit}\n", text)
                self.assertIn("  task: deny\n", text)

    def test_opencode_default_uses_luna_without_changing_facilitator(self):
        config = json.loads((self.root / ".opencode/opencode.json").read_text(encoding="utf-8"))
        self.assertEqual(config["model"], {
            "providerID": "openai", "model": "gpt-6-luna", "variant": "max"})
        text = (self.root / ".opencode/agent/facilitator.md").read_text(encoding="utf-8")
        self.assertEqual(framework.frontmatter_field(text, "model"), "openai/gpt-6.1-sol")
        self.assertEqual(framework.frontmatter_field(text, "variant"), "max")

    def test_stale_opencode_deepseek_worker_pin_is_rejected(self):
        for role in ["round-agent", "explore-agent"]:
            with self.subTest(role=role):
                path = f".opencode/agent/{role}.md"
                text = (self.root / path).read_text(encoding="utf-8")
                self.write(path, text.replace("model: openai/gpt-6-luna",
                                              "model: openrouter/deepseek/deepseek-v4.1-flash"))
                errors = framework.validate(self.root)
                self.assertTrue(any(path in e and "explicit model" in e for e in errors), errors)
                self.write(path, text)

    def test_bad_json_and_shape(self):
        for content in ["{", "[]"]:
            self.write("docs/tasks.json", content)
            self.assertTrue(framework.validate(self.root))

    def test_invalid_queue_and_live_ownership(self):
        self.write("brief.md", "legacy brief")
        item = {"id": "example", "status": "blocked", "owns_live_game": False,
                "live_owner": "worker", "brief": "brief.md"}
        self.write("docs/tasks.json", json.dumps(
            {"schema_version": 1, "active": item, "queue": [item]}))
        errors = framework.validate(self.root)
        for expected in ["blocked_reason", "inconsistent live_owner", "duplicate task id",
                         "waiting queue items"]:
            self.assertTrue(any(expected in e for e in errors), errors)

    def test_ready_requires_artifact_and_released_owner(self):
        self.write("brief.md", "legacy brief")
        self.write("docs/tasks.json", json.dumps({"schema_version": 1, "queue": [], "active": {
            "id": "example", "status": "ready-for-verify", "owns_live_game": True,
            "live_owner": "worker", "brief": "brief.md"}}))
        errors = framework.validate(self.root)
        self.assertTrue(any("worker_output" in e for e in errors))
        self.assertTrue(any("inconsistent live_owner" in e for e in errors))

    def brief(self, tier="targeted", owned="`src/example.cpp`"):
        values = {field: "none" for field in framework.FIELDS}
        values.update({"Queue id": "example", "Harness / worker": tier,
                       "Owns live game": "no", "Owned files/paths": owned})
        text = "\n".join(f"- **{key}:** {value}" for key, value in values.items())
        for heading in framework.HEADINGS:
            body = "[Lessons](docs/LESSONS.md)" if heading == "Required reading" else "Explicit scope."
            text += f"\n\n## {heading}\n\n{body}\n"
        return text

    def test_valid_round_and_explore_briefs(self):
        for tier, owned in [("targeted", "`src/example.cpp`"), ("explore", "none")]:
            self.write("brief.md", self.brief(tier, owned))
            self.assertEqual(framework.validate(self.root, Path("brief.md")), [])

    def test_missing_brief_fields_and_forbidden_ownership(self):
        self.write("brief.md", self.brief(owned="`docs/LESSONS.md`, `src/../AGENTS.md`")
                   .replace("- **Dependencies:** none", ""))
        errors = framework.validate(self.root, Path("brief.md"))
        self.assertTrue(any("missing field Dependencies" in e for e in errors))
        self.assertEqual(sum("ownership outside worker areas" in e for e in errors), 2)


if __name__ == "__main__":
    unittest.main()
