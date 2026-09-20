#!/usr/bin/env python3

from datetime import datetime, timezone
from pathlib import Path
import re
import subprocess
import sys


source_dir = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()


def run_git(*args):
    try:
        return subprocess.check_output(
            ["git", "-C", str(source_dir), *args],
            stderr=subprocess.DEVNULL,
            text=True,
        ).strip()
    except (OSError, subprocess.CalledProcessError):
        return ""


def git_succeeds(*args):
    try:
        subprocess.check_call(
            ["git", "-C", str(source_dir), *args],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        return True
    except (OSError, subprocess.CalledProcessError):
        return False


def read_source_metadata(name):
    try:
        return (source_dir / name).read_text(encoding="utf-8").strip()
    except OSError:
        return ""


def describe_version():
    described = run_git("describe", "--tags", "--match", "v*")
    if described:
        return described

    # MSYS Git can fail to walk annotated tags when it is launched by the
    # native Windows Python used by Meson. Reconstruct the same description
    # from commands which are reliable in that mixed environment.
    tag_refs = run_git("show-ref", "--tags").splitlines()
    tags = []
    for tag_ref in tag_refs:
        fields = tag_ref.split()
        if len(fields) != 2 or not fields[1].startswith("refs/tags/"):
            continue
        tag = fields[1].removeprefix("refs/tags/")
        match = re.fullmatch(r"v(\d+)\.(\d+)\.(\d+)", tag)
        if match and git_succeeds("merge-base", "--is-ancestor", tag, "HEAD"):
            tags.append((tuple(int(match.group(i)) for i in range(1, 4)), tag))
    if not tags:
        return ""

    tag = max(tags)[1]
    distance = run_git("rev-list", "--count", f"{tag}..HEAD")
    short_commit = run_git("rev-parse", "--short=10", "HEAD")
    if distance == "0":
        return tag
    if distance.isdigit() and short_commit:
        return f"{tag}-{distance}-g{short_commit}"
    return ""


commit = run_git("rev-parse", "HEAD") or read_source_metadata("XEMU_COMMIT")
described = describe_version()
version = described[1:] if described.startswith("v") else described
if not version:
    version = read_source_metadata("XEMU_VERSION") or "0.0.0"

match = re.fullmatch(
    r"(\d+)\.(\d+)\.(\d+)(?:-(\d+)-g[0-9a-f]+)?(?:-dirty)?", version
)
if match:
    major, minor, patch = (int(match.group(i)) for i in range(1, 4))
    version_commit = int(match.group(4) or 0)
else:
    major = minor = patch = version_commit = 0

date = datetime.now(timezone.utc).strftime("%a %b %d %H:%M:%S UTC %Y")

print(f'#define XEMU_VERSION       "{version}"')
print(f"#define XEMU_VERSION_MAJOR {major}")
print(f"#define XEMU_VERSION_MINOR {minor}")
print(f"#define XEMU_VERSION_PATCH {patch}")
print(f"#define XEMU_VERSION_COMMIT {version_commit}")
print(f'#define XEMU_COMMIT        "{commit}"')
print(f'#define XEMU_DATE          "{date}"')
