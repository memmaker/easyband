#!/bin/sh
# Toolchain for the Easyband web port (RVIP cloud run, 2026-09-26, Linux x86_64).
# Everything below was run in the Claude Code cloud container and worked.

# Emscripten (emsdk "latest" = 6.0.10, emsdk commit e566f7b,
# releases 666337b525e673e769121856d175f6f52b8ead64; node 24.19.0 bundled)
git clone --depth 1 https://github.com/emscripten-core/emsdk ../emsdk
(cd ../emsdk && ./emsdk install latest && ./emsdk activate latest)
. ../emsdk/emsdk_env.sh
emcc --version        # emcc ... 6.0.10 (d6c521a7f05449857c76bd99e396895583cf2083)

# Smoke test (passed): Asyncify + emscripten_sleep under node
#   printf '#include <stdio.h>\n#include <emscripten.h>\nint main(){emscripten_sleep(1);puts("hi");return 0;}\n' > t.c
#   emcc -O2 -sASYNCIFY t.c -o t.js && node t.js   -> hi

# Native ASan build (A-Zangband): present in the image, not yet used
#   gcc 13.3.0 (Ubuntu 24.04), clang 18.1.3; curses: libncurses-dev (apt) when needed
#   python3 -m venv ../venv && ../venv/bin/pip install pyte   # screen reader for the pty driver

# Browser tests: Playwright with the preinstalled Chromium
#   (PLAYWRIGHT_BROWSERS_PATH=/opt/pw-browsers, chromium-1194; do NOT run
#   "playwright install"); npm i -D playwright in a scratch dir, serve web/dist
#   with python3 -m http.server.

# NOT BUILT: the Easyband source in this repo is empty (see HANDOVER.md,
# "Stage 1: blocked"), so web/build.sh does not exist yet.
