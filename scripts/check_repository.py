"""Check the files selected for publication; never print local credentials."""
from pathlib import Path
import json
import re
import subprocess
import sys
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]

def files_for_publication():
    result = subprocess.run(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"],
        cwd=ROOT, capture_output=True, check=True,
    )
    return sorted({p.decode("utf-8") for p in result.stdout.split(b"\0") if p})

def main():
    files = files_for_publication()
    errors = []
    text_files = {}
    generated = {".o", ".d", ".crf", ".axf", ".hex", ".map", ".lst", ".lnp", ".dep", ".iex", ".uvoptx", ".dbgconf"}
    for name in files:
        path = ROOT / name
        if not path.is_file():
            errors.append("Missing selected file: " + name)
            continue
        if path.name in {"app_config.local.h", "config.local.js", "project.private.config.json"} or path.suffix.lower() in generated or ".uvguix" in name:
            errors.append("Private/generated file selected: " + name)
        if path.suffix.lower() in {".c", ".h", ".s", ".js", ".json", ".wxml", ".wxss", ".md", ".ioc", ".uvprojx", ".py"}:
            try:
                text_files[name] = path.read_text(encoding="utf-8")
            except UnicodeDecodeError:
                errors.append("Non-UTF-8 text: " + name)
    for name, text in text_files.items():
        # These patterns identify actual credential formats, not field names.
        if re.search(r"version=\d{4}-\d{2}-\d{2}&res=", text):
            errors.append("OneNET credential literal found: " + name)
        if re.search(r"\b(?:ghp_|github_pat_)[A-Za-z0-9_]{20,}", text):
            errors.append("GitHub credential literal found: " + name)
        if name.endswith(".json"):
            try:
                json.loads(text)
            except json.JSONDecodeError:
                errors.append("Invalid JSON: " + name)
        if name.endswith(".md"):
            for target in re.findall(r"\]\(([^)]+)\)", text):
                if target.startswith(("https://", "http://", "#", "mailto:")):
                    continue
                target = target.split("#", 1)[0]
                if target and not ((ROOT / name).parent / target).exists():
                    errors.append("Broken document link in " + name + ": " + target)
    project = ROOT / "firmware/MDK-ARM/wulianwang.uvprojx"
    for element in ET.parse(project).iter("FilePath"):
        target = project.parent / element.text.replace("\\", "/")
        if not target.exists():
            errors.append("Missing Keil source: " + element.text)
    for page in (ROOT / "miniprogram/pages").rglob("*.wxml"):
        for asset in re.findall(r'src="(/[^"{}]+)"', page.read_text(encoding="utf-8")):
            if not (ROOT / "miniprogram" / asset.lstrip("/")).exists():
                errors.append("Missing mini-program asset: " + asset)
    local_files = ["firmware/USER/app_config.local.h", "miniprogram/config.local.js"]
    for name in local_files:
        ignored = subprocess.run(["git", "check-ignore", "--quiet", name], cwd=ROOT)
        if ignored.returncode != 0:
            errors.append("Local configuration is not ignored: " + name)
    if errors:
        for error in errors:
            print("ERROR:", error)
        return 1
    print("PASS:", len(files), "publication files checked; local configurations ignored.")
    print("PASS: UTF-8, JSON, document links, Keil references and image references.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
