"""Check the short, current documentation path; historical records are not policy.

Standard library only. No build, game, desktop or installation access.
Run: python scripts/check_docs.py
"""

import re
import unicodedata
from pathlib import Path
from urllib.parse import unquote


ROOT = Path(__file__).resolve().parents[1]
BUDGETS = {
    "PLAN.md": (120, 10_000),
    "docs/LESSONS.md": (120, 10_000),
    "docs/STATUS.md": (150, 16_000),
    "docs/README.md": (120, 10_000),
}
CURRENT = [*BUDGETS, "AGENTS.md", "README.md", "docs/AGENT_SAFETY.md",
           "docs/acceptance/legacy-gates.md"]


def anchors(text):
    """Heading slugs and explicit HTML anchors used by these Markdown documents."""
    result = set(re.findall(r'<a\s+(?:id|name)=["\']([^"\']+)', text))
    counts = {}
    for heading in re.findall(r"^#{1,6}\s+(.+?)\s*$", text, re.M):
        heading = re.sub(r"<[^>]*>", "", heading).lower()
        base = "".join(c for c in heading if c in "-_ "
                       or unicodedata.category(c)[0] in "LN").replace(" ", "-")
        n = counts.get(base, 0)
        result.add(f"{base}-{n}" if n else base)
        counts[base] = n + 1
    return result


def check(root=ROOT):
    errors = []
    for name in CURRENT:
        path = root / name
        if not path.is_file():
            errors.append(f"{name}: missing file")
            continue
        data = path.read_bytes()
        try:
            text = data.decode("utf-8")
        except UnicodeDecodeError:
            errors.append(f"{name}: not UTF-8")
            continue
        if b"\r" in data:
            errors.append(f"{name}: use LF endings")
        if name in BUDGETS:
            lines, size = BUDGETS[name]
            if len(text.splitlines()) > lines or len(data) > size:
                errors.append(f"{name}: exceeds {lines} lines / {size} bytes; extract detail")
        # Validate navigational Markdown links, not inline code paths or old logs.
        prose = re.sub(r"```.*?```", "", text, flags=re.S)
        for target in re.findall(r"\[[^\]]*\]\(([^)\s]+)\)", prose):
            if re.match(r"[a-zA-Z][\w+.-]*:", target):
                continue
            dest, _, fragment = unquote(target).partition("#")
            linked = path.parent / dest if dest else path
            if not linked.exists():
                errors.append(f"{name}: missing link {target}")
            elif fragment and linked.suffix == ".md":
                if fragment not in anchors(linked.read_text(encoding="utf-8")):
                    errors.append(f"{name}: missing anchor {target}")
    return errors


if __name__ == "__main__":
    failures = check()
    for failure in failures:
        print(failure)
    print(f"Current documentation: {'FAIL' if failures else 'PASS'} "
          f"({len(CURRENT)} files; size budgets, links, UTF-8/LF only)")
    raise SystemExit(bool(failures))
