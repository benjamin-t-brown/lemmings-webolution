"""Read-only checks for the repository's agent contracts (standard library only)."""

import argparse
import json
import re
from pathlib import Path
from urllib.parse import unquote


CORE = [
    "AGENTS.md", "docs/ORCHESTRATION.md", "docs/FACILITATOR.md",
    "docs/ROLES.md", "docs/AGENT_SAFETY.md", "docs/agents/README.md",
    "docs/agents/ROUND.md", "docs/agents/EXPLORE.md", "docs/briefs/TEMPLATE.md",
    ".cursor/rules/match-the-original.mdc", ".cursor/rules/orchestration.mdc",
    ".cursor/rules/windows-msys2-build.mdc", ".opencode/README.md",
    "docs/OVERLORD.md", "scripts/overlord/README.md", "docs/RENDERING_METHOD.md",
]
CLIENTS = [(".claude", "agents"), (".cursor", "agents"), (".opencode", "agent")]
FIELDS = [
    "Queue id", "Harness / worker", "Why this tier", "Owns live game",
    "Owned files/paths", "Dependencies", "Investigation/run budget", "Continuation/repair",
]
HEADINGS = [
    "Required reading", "Already settled (do not re-derive)", "Question",
    "Prediction (before any live run)", "Acceptance criteria", "Optional leads",
    "Hypotheses", "Must not touch", "Deliverables", "Report honestly",
]
STATUSES = {"queued", "running", "blocked", "ready-for-verify", "landed"}
MAX_ACTIVE = 3
# PLAN.md is the live plan; round prose goes to docs/plan-history/ (user, 2026-10-24).
PLAN_MAX_BYTES = 30000
PLAN_HISTORY = [f"docs/plan-history/phase-{p}.md" for p in "abcde"]


def read(root, path, errors):
    file = root / path
    if not file.is_file():
        errors.append(f"{path}: missing file")
        return ""
    try:
        return file.read_text(encoding="utf-8")
    except UnicodeError:
        errors.append(f"{path}: invalid UTF-8")
        return ""


def slug(text):
    return re.sub(r"[^\w\- ]", "", text.lower()).replace(" ", "-")


def links(root, path, text, errors):
    # Ignore code examples; these may contain intentionally hypothetical links.
    text = re.sub(r"```.*?```", "", text, flags=re.S)
    for target in re.findall(r"\[[^\]]*\]\(([^)\s]+)\)", text):
        if re.match(r"[a-zA-Z][\w+.-]*:", target):
            continue
        name, _, fragment = unquote(target).partition("#")
        destination = (root / path).parent / name if name else root / path
        if not destination.exists():
            errors.append(f"{path}: broken link {target}")
        elif fragment and destination.is_file() and destination.suffix == ".md":
            headings = re.findall(r"^#{1,6}\s+(.+?)\s*$",
                                  destination.read_text(encoding="utf-8"), re.M)
            if fragment not in {slug(h) for h in headings}:
                errors.append(f"{path}: missing heading anchor {target}")


def repo_path(value):
    return (isinstance(value, str) and bool(value.strip())
            and not re.match(r"[/\\]|[a-zA-Z]:", value)
            and "\\" not in value and ".." not in value.split("/"))


def frontmatter_field(text, name):
    front = re.match(r"\A---\n(.*?)\n---", text, re.S)
    field = re.search(r"^" + re.escape(name) + r":\s*([^\n]+)$",
                      front.group(1), re.M) if front else None
    return field.group(1).strip() if field else None


def frontmatter_rules(text):
    """Read our deliberately small V2 rule-list format, rejecting unsupported shapes.

    This is a repository contract lint, not a general YAML parser or runtime evaluator.
    Resolved OpenCode configuration must also be checked before relying on permissions.
    """
    front = re.match(r"\A---\n(.*?)\n---", text, re.S)
    section = re.search(r"^permissions:\n(.*?)(?=^\S|\Z)",
                        front.group(1), re.M | re.S) if front else None
    if not section:
        return None
    blocks = re.split(r"^  - ", section.group(1), flags=re.M)
    if blocks[0].strip():
        return None
    rules = []
    for block in blocks[1:]:
        match = re.fullmatch(r"action: ([\w*]+)\n    resource: ([^\n]+)\n"
                             r"    effect: (allow|deny|ask)\s*", block)
        if not match:
            return None
        action, resource, effect = match.groups()
        rules.append((action, resource.strip('\"\''), effect))
    return rules if rules else None


FACILITATORS = ["facilitator", "facilitator-astra", "facilitator-mac"]
WORKERS = ["round-agent", "explore-agent", "disasm-agent"]


def check_dispatch_rules(path, text, allowed, errors, extra=()):
    # Exact ordered lists reject broad allows and later overrides as well as missing denies.
    expected = [("subagent", "*", "deny")]
    expected += [("subagent", name, "allow") for name in allowed]
    expected += list(extra)
    if frontmatter_rules(text) != expected:
        errors.append(f"{path}: delegation permissions must deny by default and allow only {allowed}")
    front = re.match(r"\A---\n(.*?)\n---", text, re.S)
    for legacy in ["name", "variant", "permission", "tools"]:
        if front and re.search(r"^" + legacy + r":", front.group(1), re.M):
            errors.append(f"{path}: do not mix native V2 permissions with legacy {legacy}")


def check_opencode_facilitator(root, roles, errors, name="facilitator"):
    path = f".opencode/agent/{name}.md"
    text = read(root, path, errors)
    links(root, path, text, errors)
    if "../../docs/FACILITATOR.md" not in text:
        errors.append(f"{path}: missing facilitator contract pointer")
    model, _, variant = (frontmatter_field(text, "model") or "").partition("#")
    if not model or model == "inherit" or f"`{model}`" not in roles:
        errors.append(f"{path}: explicit model missing or absent from ROLES.md")
    if frontmatter_field(text, "mode") != "all":
        errors.append(f"{path}: facilitator must use mode: all for manual and delegated launches")
    if variant != "high":
        errors.append(f"{path}: facilitator variant must be high")
    suffix = "-mac" if name.endswith("-mac") else ""
    check_dispatch_rules(path, text, [w + suffix for w in WORKERS], errors)

    command_path = f".opencode/commands/{name}.md"
    command = read(root, command_path, errors)
    links(root, command_path, command, errors)
    if frontmatter_field(command, "agent") != name:
        errors.append(f"{command_path}: missing {name} agent binding")
    for target in ["../../docs/FACILITATOR.md", "../../docs/ORCHESTRATION.md#facilitator-invocation"]:
        if target not in command:
            errors.append(f"{command_path}: missing pointer {target}")
    if frontmatter_field(command, "model") is not None:
        errors.append(f"{command_path}: omit model override and use the pinned facilitator agent")
    if frontmatter_field(command, "subtask") != "false":
        errors.append(f"{command_path}: facilitator must run in the current session")


MAC_AGENTS = ["facilitator", "round-agent", "explore-agent", "disasm-agent"]


def check_opencode_overlord(root, errors):
    path = ".opencode/agents/overlord.md"
    text = read(root, path, errors)
    links(root, path, text, errors)
    if "../../docs/OVERLORD.md" not in text:
        errors.append(f"{path}: missing overlord contract pointer")
    if frontmatter_field(text, "mode") != "primary":
        errors.append(f"{path}: overlord must be a primary agent")
    check_dispatch_rules(path, text, FACILITATORS, errors, extra=[
        ("edit", "**/docs/tasks.json", "deny"),
        ("edit", "**/docs/tasks-archive.json", "deny"),
    ])
    command_path = ".opencode/commands/overlord.md"
    command = read(root, command_path, errors)
    if frontmatter_field(command, "agent") != "overlord":
        errors.append(f"{command_path}: missing overlord agent binding")
    if (frontmatter_field(command, "subagent") != "false"
            or frontmatter_field(command, "subtask") is not None):
        errors.append(f"{command_path}: overlord must run in the current session using subagent: false")
    for name, content in [(path, text), (command_path, command)]:
        if frontmatter_field(content, "model") is not None:
            errors.append(f"{name}: overlord retains the selected session model; omit model")
        if "../../docs/FACILITATOR.md" in content or "ORCHESTRATION.md#facilitator-invocation" in content:
            errors.append(f"{name}: overlord must not invoke facilitation")


def frontmatter_without(text, names):
    front = re.match(r"\A---\n(.*?)\n---", text, re.S)
    lines = front.group(1).splitlines() if front else []
    return [line for line in lines if not re.match(r"(" + "|".join(names) + r"):", line)]


def check_opencode_mac(root, roles, errors):
    # The Mac clone's opencode2 only has the llmgw provider; its -mac agents mirror mouv's.
    for role in MAC_AGENTS:
        path = f".opencode/agent/{role}-mac.md"
        text = read(root, path, errors)
        links(root, path, text, errors)
        model_ref = frontmatter_field(text, "model") or ""
        model, _, native_variant = model_ref.partition("#")
        if not model.startswith("llmgw/") or f"`{model}`" not in roles:
            errors.append(f"{path}: Mac agents pin an llmgw model listed in ROLES.md")
        if (native_variant or frontmatter_field(text, "variant")) != "high":
            errors.append(f"{path}: Mac agent variant must be high")
        base = read(root, f".opencode/agent/{role}.md", errors)
        keep = ["name", "description", "model"]
        # Mac facilitator uses the same policy, but only Mac worker IDs.
        mirrored = re.sub(r"(    resource: (?:round|explore|disasm)-agent)-mac$",
                          r"\1", text, flags=re.M) if role == "facilitator" else text
        if frontmatter_without(mirrored, keep) != frontmatter_without(base, keep):
            errors.append(f"{path}: mode, variant and permissions must match {role}.md")
    command_path = ".opencode/commands/facilitator-mac.md"
    command = read(root, command_path, errors)
    links(root, command_path, command, errors)
    if frontmatter_field(command, "agent") != "facilitator-mac":
        errors.append(f"{command_path}: missing facilitator-mac agent binding")
    if frontmatter_field(command, "model") is not None:
        errors.append(f"{command_path}: omit model override and use the pinned agent")
    if frontmatter_field(command, "subtask") != "false":
        errors.append(f"{command_path}: facilitator must run in the current session")
    for target in ["../../docs/FACILITATOR.md", "../../docs/ORCHESTRATION.md#facilitator-invocation"]:
        if target not in command:
            errors.append(f"{command_path}: missing pointer {target}")


def check_opencode_delegation(root, errors):
    try:
        config = json.loads(read(root, ".opencode/opencode.json", errors))
        depth = config.get("experimental", {}).get("subagent_depth")
    except (ValueError, AttributeError):
        depth = None
    if type(depth) is not int or depth != 2:
        errors.append(".opencode/opencode.json: experimental.subagent_depth must be 2")
    for worker in WORKERS:
        for suffix in ["", "-mac"]:
            path = f".opencode/agent/{worker}{suffix}.md"
            text = read(root, path, errors)
            front = re.match(r"\A---\n(.*?)\n---", text, re.S)
            front = front.group(1) if front else ""
            # Existing workers intentionally retain unmixed V1 frontmatter. V2
            # normalizes task: deny to subagent: deny; prevent accidental delegation.
            if (frontmatter_field(text, "mode") != "subagent"
                    or re.findall(r"^  task:\s*([^\n]+)$", front, re.M) != ["deny"]
                    or re.search(r"^(permissions|tools|system|request|disabled):", front, re.M)):
                errors.append(f"{path}: worker must remain a subagent with delegation denied")


def check_brief(root, path, errors):
    text = read(root, path, errors)
    links(root, path, text, errors)
    values = {}
    for field in FIELDS:
        match = re.search(r"^- \*\*" + re.escape(field) + r":\*\*\s*(.+)$", text, re.M)
        if not match:
            errors.append(f"{path}: missing field {field}")
        else:
            values[field] = match.group(1).strip()
    for heading in HEADINGS:
        match = re.search(r"^## " + re.escape(heading) + r"\s*\n(.*?)(?=^## |\Z)",
                          text, re.M | re.S)
        if not match or not match.group(1).strip().strip("-"):
            errors.append(f"{path}: missing/empty section {heading}")
    if values.get("Owns live game") not in {"yes", "no"}:
        errors.append(f"{path}: Owns live game must be yes or no")
    if values.get("Harness / worker") not in {"targeted", "hard", "explore"}:
        errors.append(f"{path}: invalid worker tier")
    owned = re.findall(r"`([^`]+)`", values.get("Owned files/paths", ""))
    explore = values.get("Harness / worker") == "explore"
    if not owned and not (explore and values.get("Owned files/paths") == "none"):
        errors.append(f"{path}: Owned files/paths needs backtick-quoted paths or globs")
    for name in owned:
        if (not repo_path(name) or
                not (name.startswith(("src/", "tools/harness/", "build/tmp/", "scripts/", "cmake/"))
                     or name == "CMakeLists.txt")):
            errors.append(f"{path}: ownership outside worker areas: {name}")
    if values.get("Harness / worker") == "explore" and (
            owned or values.get("Owns live game") == "yes"):
        errors.append(f"{path}: explore is read-only and cannot own the live game")
    reading = re.search(r"^## Required reading\s*\n(.*?)(?=^## |\Z)", text, re.M | re.S)
    if reading and not re.search(r"\[[^\]]+\]\([^)]+\)", reading.group(1)):
        errors.append(f"{path}: Required reading needs source links")


def validate(root, brief=None):
    errors = []
    for path in CORE:
        text = read(root, path, errors)
        links(root, path, text, errors)
    plan = read(root, "PLAN.md", errors)
    links(root, "PLAN.md", plan, errors)
    if len(plan.encode("utf-8")) > PLAN_MAX_BYTES:
        errors.append(f"PLAN.md: over {PLAN_MAX_BYTES} bytes; move round prose to docs/plan-history/")
    for path in PLAN_HISTORY:
        links(root, path, read(root, path, errors), errors)
    roles = read(root, "docs/ROLES.md", errors)
    for client, folder in CLIENTS:
        for role, contract in [("round-agent", "ROUND"), ("explore-agent", "EXPLORE")]:
            path = f"{client}/{folder}/{role}.md"
            text = read(root, path, errors)
            links(root, path, text, errors)
            if f"../../docs/agents/{contract}.md" not in text:
                errors.append(f"{path}: missing shared contract pointer")
            front = re.match(r"\A---\n(.*?)\n---", text, re.S)
            model = re.search(r"^model:\s*(\S+)", front.group(1), re.M) if front else None
            # Claude Code workers inherit the session model; a pin there misroutes.
            if client == ".claude":
                if model:
                    errors.append(f"{path}: Claude Code workers must not pin a model")
            elif not model or model.group(1) == "inherit" or f"`{model.group(1)}`" not in roles:
                errors.append(f"{path}: explicit model missing or absent from ROLES.md")
        path = f"{client}/commands/facilitator.md"
        text = read(root, path, errors)
        links(root, path, text, errors)
        if "../../docs/FACILITATOR.md" not in text:
            errors.append(f"{path}: missing facilitator contract pointer")
        if "../../docs/ORCHESTRATION.md#facilitator-invocation" not in text:
            errors.append(f"{path}: missing facilitator invocation pointer")
    # The overlord directs facilitators; its command must not read as a facilitator invocation.
    for client in [".claude", ".cursor", ".opencode"]:
        path = f"{client}/commands/overlord.md"
        text = read(root, path, errors)
        links(root, path, text, errors)
        if "../../docs/OVERLORD.md" not in text:
            errors.append(f"{path}: missing overlord contract pointer")
        if "ORCHESTRATION.md#facilitator-invocation" in text:
            errors.append(f"{path}: overlord must not invoke facilitation")
    check_opencode_facilitator(root, roles, errors)
    check_opencode_overlord(root, errors)
    # The Astra tier mirrors facilitator.md except for its name, description and model.
    check_opencode_facilitator(root, roles, errors, "facilitator-astra")
    keep = ["name", "description", "model"]
    if (frontmatter_without(read(root, ".opencode/agent/facilitator-astra.md", errors), keep)
            != frontmatter_without(read(root, ".opencode/agent/facilitator.md", errors), keep)):
        errors.append(".opencode/agent/facilitator-astra.md: mode, variant and permissions must match facilitator.md")
    check_opencode_mac(root, roles, errors)
    check_opencode_facilitator(root, roles, errors, "facilitator-mac")
    check_opencode_delegation(root, errors)
    try:
        queue = json.loads(read(root, "docs/tasks.json", errors))
    except (ValueError, TypeError):
        errors.append("docs/tasks.json: invalid JSON")
        queue = {}
    if not isinstance(queue, dict):
        errors.append("docs/tasks.json: expected object")
        queue = {}
    if queue.get("schema_version") != 1:
        errors.append("docs/tasks.json: expected schema_version 1")
    for key in ("active", "queue"):
        if key not in queue:
            errors.append(f"docs/tasks.json: missing {key}")
    # Landed tasks and stopped runs live in docs/tasks-archive.json, not the live queue.
    run = queue.get("standing_run")
    if "recently_landed" in queue or (isinstance(run, dict) and "previous" in run):
        errors.append("docs/tasks.json: move landed tasks and earlier runs to docs/tasks-archive.json")
    pending = queue.get("queue", [])
    if not isinstance(pending, list):
        errors.append("docs/tasks.json: queue must be a list")
        pending = []
    active = queue.get("active")
    # Concurrent offline rounds (ORCHESTRATION.md): active is null, one task, or up to three.
    actives = active if isinstance(active, list) else ([active] if active is not None else [])
    if len(actives) > MAX_ACTIVE:
        errors.append(f"docs/tasks.json: at most {MAX_ACTIVE} active tasks")
    if sum(1 for a in actives if isinstance(a, dict) and a.get("owns_live_game")) > 1:
        errors.append("docs/tasks.json: at most one active task may own the live game")
    ids = set()
    for item in actives + pending:
        if not isinstance(item, dict):
            errors.append("docs/tasks.json: task must be an object")
            continue
        task_id = item.get("id")
        if not isinstance(task_id, str) or not task_id or task_id in ids:
            errors.append("docs/tasks.json: missing or duplicate task id")
        else:
            ids.add(task_id)
        status = item.get("status")
        if not isinstance(status, str) or status not in STATUSES:
            errors.append(f"{task_id}: invalid status")
            status = None
        if status == "blocked" and not item.get("blocked_reason"):
            errors.append(f"{task_id}: blocked task needs blocked_reason")
        if not isinstance(item.get("owns_live_game"), bool):
            errors.append(f"{task_id}: owns_live_game must be boolean")
        if status == "running" and item.get("owns_live_game") and not item.get("live_owner"):
            errors.append(f"{task_id}: running live task needs live_owner")
        if item.get("live_owner") and (not item.get("owns_live_game") or
                                      status in {"queued", "ready-for-verify", "landed"}):
            errors.append(f"{task_id}: inconsistent live_owner")
        for key in ["brief"] + (["worker_output"] if status == "ready-for-verify" else []):
            value = item.get(key)
            if not repo_path(value) or not (root / value).is_file():
                errors.append(f"{task_id}: {key} must name an existing repository file")
        if not any(item is a for a in actives) and status != "queued":
            errors.append(f"{task_id}: waiting queue items must be queued")
    if brief:
        check_brief(root, brief, errors)
    return errors


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--brief", type=Path, help="Strictly validate a new brief before dispatch")
    args = parser.parse_args()
    errors = validate(args.root.resolve(), args.brief)
    for error in errors:
        print("ERROR:", error)
    if not errors:
        print("Agent framework checks passed.")
    return bool(errors)


if __name__ == "__main__":
    raise SystemExit(main())
