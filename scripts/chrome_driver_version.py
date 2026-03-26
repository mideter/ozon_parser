"""Согласование major-версии Chromium с undetected-chromedriver (version_main)."""
from __future__ import annotations

import re
import subprocess


def detect_chrome_major_version() -> int | None:
    for bin_name in ("google-chrome", "chromium", "chromium-browser"):
        try:
            out = subprocess.check_output(
                [bin_name, "--version"], text=True, timeout=8
            )
            m = re.search(r"(\d+)\.", out)
            if m:
                return int(m.group(1))
        except Exception:
            continue
    return None
