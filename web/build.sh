#!/bin/sh
# Build Easyband for the browser (Emscripten + Asyncify).
# Output goes to web/dist; deploy with web/deploy.sh (Mac side only).
# emcc: . ../emsdk/emsdk_env.sh first (see web/toolchain.sh).
set -e
cd "$(dirname "$0")/.."
OUT=web/dist
rm -rf "$OUT" web/stage && mkdir -p "$OUT" web/stage/lib

# Game files: edit/file/help and the pref files (in lib/user for 2.9.3);
# lib/data/*.raw are rebuilt by the game at start, BMP tiles are not used
for d in edit file help user; do cp -R lib/$d web/stage/lib/; done
mkdir -p web/stage/lib/data web/stage/lib/save web/stage/lib/apex web/stage/lib/bone web/stage/lib/info

# Sources: every game file of SRC/ except the other front ends
SRCS=$(ls SRC/*.c | grep -v '/main\|/maid-\|/Readdib')

emcc -O2 -fcommon -std=gnu99 -DUSE_WEB -ISRC -w \
	$SRCS SRC/main.c SRC/main-web.c \
	-o "$OUT/easyband-core.js" \
	-sASYNCIFY -sASYNCIFY_STACK_SIZE=65536 -sSTACK_SIZE=1048576 \
	-sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=64MB \
	-sEXPORTED_FUNCTIONS=_main,_web_request_save \
	-sEXPORTED_RUNTIME_METHODS=FS,IDBFS,HEAPU8,addRunDependency,removeRunDependency \
	-sFORCE_FILESYSTEM -lidbfs.js -sENVIRONMENT=web \
	--preload-file web/stage/lib@/easyband/lib

cp web/index.html rvip/web/rvip-wm.js web/easyband.js "$OUT/"
# Shockbolt tiles (Angband 4.2 lib/tiles/shockbolt/64x64.png as lossless WebP,
# from rvip/templates/tactical-angband); mapping lib/user/graf-shb.prf
# (python3 web/mkgraf-shb.py), drawn nearest-neighbour at cell size
cp web/tiles.webp "$OUT/"
rm -rf web/stage
ls -la "$OUT"
