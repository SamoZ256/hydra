#!/usr/bin/env python3
"""Run clang-tidy over a build tree with precompiled headers as well"""

import argparse
import json
import os
import re
import shlex
import shutil
import subprocess
import sys

PCH_RE = re.compile(r"cmake_pch\.[A-Za-z0-9_.]*hxx(\.pch)?$")


def strip_pch(args, cache):
    out = []
    i = 0
    included = False
    while i < len(args):
        a = args[i]
        if a == "-Winvalid-pch":
            i += 1
            continue
        if (
            a == "-Xclang"
            and i + 3 < len(args)
            and args[i + 1] in ("-include", "-include-pch")
            and args[i + 2] == "-Xclang"
            and PCH_RE.search(args[i + 3])
        ):
            if not included:
                out += ["-include", desystem(args[i + 3], cache)]
                included = True
            i += 4
            continue
        if (
            a in ("-include", "-include-pch")
            and i + 1 < len(args)
            and PCH_RE.search(args[i + 1])
        ):
            if not included:
                out += ["-include", desystem(args[i + 1], cache)]
                included = True
            i += 2
            continue
        if PCH_RE.search(a):
            i += 1
            continue
        if a.startswith("@") and a.endswith(".modmap"):
            i += 1
            continue
        out.append(a)
        i += 1
    return out


def desystem(pch, cache):
    if pch in cache.seen:
        return cache.seen[pch]
    src = pch[:-4] if pch.endswith(".pch") else pch
    if not os.path.exists(src):
        sys.exit(
            f"lint: PCH header not found: {src}\nconfigure and build the tree first"
        )
    name = re.sub(r"[^A-Za-z0-9_.]", "_", os.path.relpath(src, cache.root))
    dst = os.path.join(cache.dir, name)
    with open(src) as f:
        text = f.read()
    with open(dst, "w") as f:
        f.write(text.replace("#pragma clang system_header", ""))
    cache.seen[pch] = dst
    return dst


class Cache:
    def __init__(self, directory, root):
        self.dir = directory
        self.root = root
        self.seen = {}


def load(build_dir):
    path = os.path.join(build_dir, "compile_commands.json")
    if not os.path.exists(path):
        sys.exit(f"lint: no compile_commands.json in {build_dir}")
    with open(path) as f:
        return json.load(f)


def rewrite(build_dirs, out_dir, source_re):
    cache = Cache(os.path.join(out_dir, "pch"), "/")
    os.makedirs(cache.dir, exist_ok=True)
    entries = []
    seen = set()
    for build_dir in build_dirs:
        for e in load(build_dir):
            if e["file"].endswith(".swift") or not source_re.search(e["file"]):
                continue
            key = (os.path.realpath(e["file"]), e["directory"])
            if key in seen:
                continue
            seen.add(key)
            args = e.get("arguments") or shlex.split(e["command"])
            e = dict(e)
            e.pop("arguments", None)
            e["command"] = " ".join(shlex.quote(a) for a in strip_pch(args, cache))
            entries.append(e)
    if not entries:
        sys.exit("lint: no translation units matched")
    with open(os.path.join(out_dir, "compile_commands.json"), "w") as f:
        json.dump(entries, f, indent=1)
    return entries


def check_compiles(entry):
    args = shlex.split(entry["command"])
    cmd = [args[0], "-fsyntax-only"]
    skip = False
    for a in args[1:]:
        if skip:
            skip = False
            continue
        if a in ("-o", "-c"):
            skip = a == "-o"
            continue
        if a.endswith(".o"):
            continue
        cmd.append(a)
    r = subprocess.run(cmd, cwd=entry["directory"], capture_output=True, text=True)
    if r.returncode != 0:
        sys.stderr.write(r.stderr)
        sys.exit(
            "lint: the rewritten command does not compile; clang-tidy results "
            "from a broken AST are not trustworthy"
        )


def git_root(start):
    r = subprocess.run(["git", "rev-parse", "--show-toplevel"], cwd=start,
                       capture_output=True, text=True)
    return r.stdout.strip() if r.returncode == 0 else ""


def changed_files(rev, root):
    r = subprocess.run(
        ["git", "diff", "--name-only", "--diff-filter=d", rev],
        cwd=root,
        capture_output=True,
        text=True,
    )
    if r.returncode != 0:
        sys.exit(f"lint: git diff failed\n{r.stderr}")
    names = [n for n in r.stdout.split("\n") if n]
    r = subprocess.run(
        ["git", "ls-files", "--others", "--exclude-standard"],
        cwd=root,
        capture_output=True,
        text=True,
    )
    names += [n for n in r.stdout.split("\n") if n]
    return {os.path.realpath(os.path.join(root, n)) for n in names}


def dependents(build_dirs, changed, tus):
    """TUs whose recorded header dependencies include any changed file."""
    found = set()
    usable = False
    for build_dir in build_dirs:
        if not os.path.exists(os.path.join(build_dir, ".ninja_deps")):
            continue
        r = subprocess.run(
            ["ninja", "-C", build_dir, "-t", "deps"], capture_output=True, text=True
        )
        if r.returncode != 0:
            continue
        usable = True
        block = []
        for line in r.stdout.split("\n") + [""]:
            if line.startswith(" "):
                block.append(os.path.realpath(line.strip()))
                continue
            if block:
                tu = next((d for d in block if d in tus), None)
                if tu and any(d in changed for d in block):
                    found.add(tu)
            block = []
    return found if usable else None


def diff_mode(runner, out_dir, rev, root, a, rest):
    tidy_diff = None
    for base in (os.path.dirname(os.path.realpath(shutil.which("clang-tidy") or "")),):
        cand = os.path.join(base, "..", "share", "clang", "clang-tidy-diff.py")
        if os.path.exists(cand):
            tidy_diff = os.path.realpath(cand)
    if tidy_diff is None:
        sys.exit("lint: clang-tidy-diff.py not found next to clang-tidy")
    diff = subprocess.run(
        ["git", "diff", "-U0", rev], cwd=root, capture_output=True, text=True
    ).stdout
    cmd = [
        sys.executable,
        tidy_diff,
        "-p1",
        "-path",
        out_dir,
        "-j",
        a.j,
        "-quiet",
        "-only-check-in-db",
    ]
    if a.checks:
        cmd.append(f"-checks={a.checks}")
    if a.fix:
        cmd.append("-fix")
    if a.color:
        cmd.append("-use-color")
    cmd += [x for x in rest if x != "--"]
    return subprocess.run(cmd, cwd=root, input=diff, text=True).returncode


def main():
    p = argparse.ArgumentParser()
    p.add_argument("build_dirs", nargs="+")
    p.add_argument("--out")
    p.add_argument("--sources", default=r".*")
    p.add_argument("--header-filter", default=r".*")
    p.add_argument("--checks")
    p.add_argument("-j", default=str(os.cpu_count() or 4))
    p.add_argument("--fix", action="store_true")
    p.add_argument("--cwd")
    p.add_argument("--color", choices=["auto", "always", "never"], default="auto")
    p.add_argument("--changed", nargs="?", const="HEAD", metavar="REV")
    p.add_argument("--diff", nargs="?", const="HEAD", metavar="REV")
    a, rest = p.parse_known_args()
    a.color = a.color == "always" or (a.color == "auto" and sys.stdout.isatty())

    runner = shutil.which("run-clang-tidy")
    if runner is None:
        sys.exit("lint: run-clang-tidy not found; it ships with LLVM, so add "
                 "that toolchain's bin directory to PATH")

    out_dir = a.out or os.path.join(a.build_dirs[0], "lint")
    os.makedirs(out_dir, exist_ok=True)
    source_re = re.compile(a.sources)

    entries = rewrite(a.build_dirs, out_dir, source_re)
    check_compiles(entries[0])
    cwd = a.cwd or os.path.commonpath([os.path.dirname(e["file"]) for e in entries])

    root = git_root(cwd) or git_root(os.path.dirname(os.path.realpath(__file__)))

    if a.diff is not None:
        if not root:
            sys.exit("lint: --diff requires a git repository")
        return diff_mode(runner, out_dir, a.diff, root, a, rest)

    sources = a.sources
    if a.changed is not None:
        if not root:
            sys.exit("lint: --changed requires a git repository")
        tus = {os.path.realpath(e["file"]): e["file"] for e in entries}
        changed = changed_files(a.changed, root)
        picked = dependents(a.build_dirs, changed, set(tus))
        if picked is None:
            sys.stderr.write(
                "lint: no ninja dependency data; "
                "changed headers will not pull in their users\n"
            )
            picked = changed & set(tus)
        if not picked:
            print("lint: nothing to check")
            return 0
        print(f"lint: {len(picked)} of {len(tus)} translation units affected")
        sources = "|".join(re.escape(tus[f]) for f in sorted(picked))

    cmd = [
        runner,
        "-p",
        out_dir,
        "-j",
        a.j,
        "-quiet",
        f"-header-filter={a.header_filter}",
    ]
    if a.checks:
        cmd.append(f"-checks={a.checks}")
    if a.fix:
        cmd.append("-fix")
    if a.color:
        cmd.append("-use-color=1")
    cmd += [x for x in rest if x != "--"]
    cmd.append(sources)
    return subprocess.call(cmd, cwd=cwd)


if __name__ == "__main__":
    sys.exit(main())
