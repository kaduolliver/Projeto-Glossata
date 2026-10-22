"""Conservative post-merge cleanup of generated Glossata build artifacts.

Default is audit-only. Never touches the active CMake build, published evidence,
scientific media, package/, or unrecognized content.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
BUILD_VARIANT = re.compile(r"^(\d+\.\d+\.\d+)-[A-Za-z0-9_-]+$")
SENSITIVE_EXTENSIONS = {".sqlite", ".sqlite3", ".db", ".csv", ".jsonl", ".jpg", ".jpeg", ".webp", ".md", ".key", ".pem", ".p12", ".pfx"}
SENSITIVE_NAMES = {".env", "id_rsa", "id_ed25519", "credentials.json"}


def inventory(path: Path) -> tuple[int, float, bool, bool]:
    """Return size, most recent mtime, links present and valuable material present."""
    size = 0
    newest = path.stat(follow_symlinks=False).st_mtime
    links = path.is_symlink()
    valuable = False
    for directory, dirs, files in os.walk(path, followlinks=False):
        for name in dirs + files:
            child = Path(directory) / name
            if child.is_symlink():
                links = True
                continue
            if child.is_file():
                size += child.stat().st_size
                newest = max(newest, child.stat().st_mtime)
                if child.suffix.lower() in SENSITIVE_EXTENSIONS or child.name.lower() in SENSITIVE_NAMES:
                    valuable = True
            elif child.is_dir():
                newest = max(newest, child.stat().st_mtime)
    return size, newest, links, valuable


def plan_cleanup(root: Path, min_age_days: int = 14, retain: int = 2, include_variants: bool = False,
                 now: float | None = None) -> dict:
    """Plan only allowlisted directories. Never recursively scan unrelated paths."""
    root = root.resolve(strict=True)
    build = root / "build"
    if build.is_symlink():
        raise ValueError("Recusado: build/ é um link simbólico ou junction.")
    if not build.is_dir():
        return {"root": str(root), "candidates": [], "skipped": [], "total_bytes": 0}
    age_cutoff = (time.time() if now is None else now) - min_age_days * 86400
    candidates, skipped = [], []
    pools: list[tuple[str, list[Path]]] = []
    previews = build / "tests" / "previews"
    if previews.is_symlink():
        skipped.append({"path": str(previews.relative_to(root)), "reason": "symbolic link"})
    elif previews.is_dir():
        pools.append(("generated-previews", [p for p in previews.iterdir() if p.is_dir() and not p.is_symlink()]))
    if include_variants:
        groups: dict[str, list[Path]] = {}
        for path in build.iterdir():
            if path.is_symlink():
                continue
            match = BUILD_VARIANT.fullmatch(path.name)
            if path.is_dir() and match:
                groups.setdefault(match.group(1), []).append(path)
        pools.extend((f"old-build-{version}", children) for version, children in groups.items())
    for kind, paths in pools:
        scanned = [(path, *inventory(path)) for path in paths]
        scanned.sort(key=lambda row: row[2], reverse=True)
        for idx, (path, size, newest, linked, valuable) in enumerate(scanned):
            reason = ("symlink/junction detected" if linked else
                      "possibly valuable local files" if kind.startswith("old-build-") and valuable else
                      "retained newest" if idx < retain else
                      "recently modified" if newest >= age_cutoff else "")
            record = {"path": path.relative_to(root).as_posix(), "category": kind, "bytes": size,
                      "last_modified": round(newest)}
            if reason:
                skipped.append({**record, "reason": reason})
            else:
                candidates.append(record)
    return {"root": str(root), "candidates": candidates, "skipped": skipped,
            "total_bytes": sum(c["bytes"] for c in candidates)}


def require_clean_main(root: Path) -> None:
    """Destructive operations only on a clean, integrated main checkout."""
    branch = subprocess.check_output(["git", "-C", str(root), "branch", "--show-current"],
                                     text=True, encoding="utf-8").strip()
    status = subprocess.check_output(["git", "-C", str(root), "status", "--porcelain", "--untracked-files=normal"],
                                     text=True, encoding="utf-8").strip()
    if branch != "main" or status:
        raise ValueError("Execute --apply somente na main limpa, após o merge e sem builds/testes em execução.")


def execute_plan(root: Path, plan: dict) -> int:
    root = root.resolve(strict=True)
    require_clean_main(root)
    count = 0
    for item in plan["candidates"]:
        target = root / item["path"]
        if not target.is_relative_to(root / "build") or target.is_symlink() or not target.is_dir():
            raise ValueError("Alvo inseguro ou alterado: " + str(target))
        # Recheck that no links appeared between planning and deletion.
        size, _latest, links, valuable = inventory(target)
        if links or (item["category"].startswith("old-build-") and valuable) or size != item["bytes"]:
            raise ValueError("Alvo alterado ou com arquivos protegidos: " + str(target))
        shutil.rmtree(target)
        count += 1
    return count


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT, help="Raiz do repositório (testes apenas).")
    parser.add_argument("--min-age-days", type=int, default=14, help="Idade mínima desde a última alteração.")
    parser.add_argument("--retain", type=int, default=2, help="Manter os N conjuntos mais recentes por classe.")
    parser.add_argument("--include-old-builds", action="store_true", help="Incluir builds de validação versionadas, com proteções.")
    parser.add_argument("--apply", action="store_true", help="Excluir os candidatos (padrão: somente simulação).")
    args = parser.parse_args(argv)
    if args.min_age_days < 1 or args.retain < 1:
        parser.error("--min-age-days e --retain devem ser positivos")
    try:
        plan = plan_cleanup(args.root, args.min_age_days, args.retain, args.include_old_builds)
        print(json.dumps({"mode": "apply" if args.apply else "dry-run", **plan}, ensure_ascii=False, indent=2))
        if args.apply:
            print(f"Removidos {execute_plan(args.root, plan)} conjuntos gerados; arquivos originais preservados.")
    except (ValueError, OSError, subprocess.CalledProcessError) as exc:
        print(f"Limpeza recusada: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
