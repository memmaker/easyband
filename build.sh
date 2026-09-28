#!/bin/sh
# Release build. sh build.sh unix  -> ./easyband (curses, -mgcu)
#                sh build.sh win   -> ./easyband.exe (MinGW cross, Windows frontend)
set -e
cd "$(dirname "$0")/SRC"
CORE="birth.c cave.c cmd1.c cmd2.c cmd3.c cmd4.c cmd5.c cmd6.c dungeon.c files.c generate.c
	init1.c init2.c randart.c load1.c load2.c melee1.c melee2.c monster1.c monster2.c
	object1.c object2.c save.c squelch.c spells1.c spells2.c store.c tables.c util.c
	variable.c wizard1.c wizard2.c xtra1.c xtra2.c z-form.c z-rand.c z-term.c z-util.c z-virt.c"
if [ "$1" = win ]; then
	CROSS=${CROSS:-x86_64-w64-mingw32-}
	cp Readdib.h readdib.h     # included in lower case
	${CROSS}windres angband.rc -O coff -o angband.res
	${CROSS}gcc -O2 -w -fcommon -DWINDOWS -I. $CORE -DUSE_TRANSPARENCY main-win.c Readdib.c angband.res \
		-s -static -mwindows -lwinmm -o ../easyband.exe
	rm -f angband.res readdib.h
else
	${CC:-cc} -O2 -w -fcommon -DUSE_GCU -DUSE_NCURSES -DUSE_TRANSPARENCY -I. $EXTRA $CORE main.c main-gcu.c -lncurses -o ../easyband
fi
