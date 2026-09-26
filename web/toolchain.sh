#!/bin/sh
# Toolchain for the Easyband web port (RVIP cloud run, 2026-09-26, Linux x86_64).
# Everything below was run in the Claude Code cloud container and worked.

# Emscripten (emsdk "latest" = 6.0.10, releases 666337b525e673e769121856d175f6f52b8ead64;
# ~300 MB from storage.googleapis.com, ~2 min; node bundled)
git clone --depth 1 https://github.com/emscripten-core/emsdk ../emsdk
(cd ../emsdk && ./emsdk install latest && ./emsdk activate latest)
. ../emsdk/emsdk_env.sh      # or: export PATH=../emsdk/upstream/emscripten:$PATH
emcc --version               # emcc ... 6.0.10
sh web/build.sh              # -> web/dist (zero warnings with -w off for casts, see HANDOVER)

# Python helpers (tile coverage needs Pillow, the ASan driver needs pyte)
python3 -m venv ../venv && ../venv/bin/pip install pillow pyte
../venv/bin/python web/tile-coverage.py new      # own 16x16 set: 87.8%

# Native ASan build (A1 / A-Zangband), outside the repo ($W = scratch dir):
#   gcc 13.3.0 (Ubuntu 24.04), libncurses-dev present.
#   -DUSE_TPOSIX is needed: main-gcu.c tests _POSIX_VERSION before unistd.h
#   is included, falls back to termio and the tty never goes raw.
#   cp -r SRC lib $W/ && cd $W/SRC && gcc -O1 -g -fsanitize=address -fcommon \
#     -DUSE_GCU -DUSE_TPOSIX -w -o ../angband \
#     $(ls *.c | grep -v '^main\|^maid\|Readdib') main.c main-gcu.c -lncurses
#   ../venv/bin/python web/asan-drive.py $W/angband SEED NKEYS   (new char, then restored)

# Browser tests: Playwright with the preinstalled Chromium
#   (PLAYWRIGHT_BROWSERS_PATH=/opt/pw-browsers, chromium-1194; do NOT run
#   "playwright install"): npm i playwright@1.56 in a scratch dir $S/pw
#   python3 -m http.server 8765 -d web/dist &
#   NODE_PATH=$S/pw/node_modules node web/tests/stage1.js SEED

# Mac side (stage 7, 2026-09-26): Homebrew emscripten on PATH (brew install
# emscripten; /opt/homebrew/bin/emcc), no emsdk. build.sh takes rvip-wm.js from
# ~/Games/rvip-tools/web/, sounds.py the Dubtrain pack from ~/Downloads,
# make-help.py the Docs entry easyband.html from ~/Desktop/Games/Roguelikes/Docs.
#   sh web/build.sh && sh web/deploy.sh
