#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[1]

required = [
    "README.md", "PROJECT-SPECIFICATIONS.md", "PROJECT-RECORD.md",
    "SPECIFICATIONS.md", "FEATURES.md", "IMPLEMENTED-FEATURES.md",
    "PLANNED-FEATURES.md", "CHANGELOGS.md", "BENEFITS.md",
    "COMPETITIVE-OBJECTIVES.md", "BRANDING.md", "USER-MANUAL.md",
    "PRIVACY POLICY.md", "NOTES.md", "SECURITY.md", "LICENSE",
    ".gitignore", ".editorconfig", "goreecloud.platform.yaml",
]

systems = [
    "manager", "privacy_shield", "wardveil_security", "everkeep",
    "glaze_ui", "mesh", "identity", "policy", "observability",
]

errors = []
for relative in required:
    path = root / relative
    if not path.is_file():
        errors.append(f"missing required repository file: {relative}")
    elif path.stat().st_size == 0:
        errors.append(f"required repository file is empty: {relative}")

if (root / "FEATURE-ROADMAP.md").exists():
    errors.append("FEATURE-ROADMAP.md is retired")

manifest = root / "goreecloud.platform.yaml"
if manifest.is_file():
    text = manifest.read_text(encoding="utf-8")
    if 'schema_version: "0.4"' not in text:
        errors.append("platform manifest must use schema_version 0.4")
    for system in systems:
        if f"  {system}:" not in text:
            errors.append(f"platform manifest missing system: {system}")

pointer = root / "SPECIFICATIONS.md"
if pointer.is_file() and "PROJECT-SPECIFICATIONS.md" not in pointer.read_text(encoding="utf-8"):
    errors.append("SPECIFICATIONS.md must point to PROJECT-SPECIFICATIONS.md")

if errors:
    for error in errors:
        print(f"ERROR: {error}")
    raise SystemExit(1)

print("Repository governance validation passed.")
