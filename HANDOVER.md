# Easyband v2.3: handover

> **Public repo** (this folder, remote `memmaker` = github.com/memmaker/easyband):
> history without the cloud bundle. The brief below and the `rvip/` bundle it
> describes live in the private cloud history, **memmaker/easyband-cloud**
>. Upstream archive commit: `2c3e95b` here,
> `00f2a06` there. The Mac build takes shared files from `~/Games/rvip-tools/web`,
> `~/Games/tactical-angband`, `~/Downloads/Dubtrain Angband Sound Pack v3.1.0`.

## Cloud experiment (read this first)

This repo runs the RVIP import in a Claude Code **cloud** session. Everything
the procedure normally takes from sibling folders on the maintainer's Mac is
bundled under `rvip/`:

- `rvip/RVIP.md` — the procedure (snapshot; the canonical copy lives on the
  Mac). **Write lessons into `rvip/LESSONS.md`** (new file, one bullet per
  lesson naming the RVIP section it belongs to); never edit `rvip/RVIP.md`.
- `rvip/web/rvip-wm.js`, `rvip/web/rvip-sound.js` — shared page code every
  game loads (window manager, sound). Use, don't fork.
- `rvip/templates/zangband/` — the freshest case-A (z-term) web port, done
  through all stages: `main-web.c` (the z-term web frontend), `web/build.sh`
  (emcc + Asyncify + IDBFS), `web/zangband.js` + `web/index.html` (page with
  rvip-wm.js windows, tiles blit, sound), `web/mkgraf-shb.py` (Shockbolt
  pref generator; edit its `SHB`/`GD` paths to
  `rvip/templates/tactical-angband/lib/tiles/shockbolt` and `.../lib/gamedata`),
  `web/tile-coverage.py`, `web/sounds.py` (edit `PACK` to
  `rvip/templates/dubtrain`), `web/make-help.py`, `graf-shb.prf` (example
  output), `HANDOVER.md` (all nine stage sections: copy its solutions).
- `rvip/templates/tactical-angband/` — Shockbolt: `tiles.webp` (the 64x64
  sheet the pages blit), `lib/tiles/shockbolt/` (prefs), `lib/gamedata/`
  (4.2 monster/object lists the generator matches names against), `lib/pref/`.
- `rvip/templates/dubtrain/` — the Dubtrain Angband Sound Pack (wavs +
  sound.cfg) for stage 6.

**The game.** Easyband v2.3 (`readme.txt`, `Easyband23.txt`,
`lib/help/version.txt`): an Angband 2.9.x-era variant, plain C (`SRC/`,
57 files, `z-term.c`, `main-x11.c` etc.). Case **A**. Read RVIP.md Part A
(A0–A6b, A-Zangband, A-FrogComposband, A-Hengband) and Part W. Tiles: own
`lib/xtra/graf/8x8.bmp` + `16x16.bmp` (+ `mask.bmp`) with `lib/pref/graf-*.prf`;
count coverage of the 16x16 set against every `N:` entry of
`lib/edit/r_info.txt`, `k_info.txt`, `f_info.txt` with a script like
`tile-coverage.py`; ≥95% → own 16x16 set, else Shockbolt from the bundle
(the Zangband way, family stand-ins, report the numbers). One set, never mix.

**Differences from a local run**
- No browser pane and no ruzzoli.de deploy key. Stages 1–6 are in scope.
  Tests: Playwright/Chromium (`npx playwright install chromium`) driving
  `web/dist` served by `python3 -m http.server`: character creation, random
  keys, save/reload/restore, explore, stairs, menus, tiles (read canvas
  pixels), screenshots into `web/shots/`. Write `web/deploy.sh` like
  Zangband's (target `ruzzoli.de/roguelikes/easyband/`); never run it.
- Toolchain: Emscripten (`git clone https://github.com/emscripten-core/emsdk
  && ./emsdk install latest && ./emsdk activate latest`), clang/gcc for the
  native ASan build (`-DUSE_GCU` curses + pty + random keys, as A-Zangband
  describes; `pyte` for reading screens). Record every command in
  `web/toolchain.sh`. If Emscripten cannot be installed, write that file
  with what you tried and the errors, commit, push, stop.
- Commit after every stage (`RVIP: stage N <topic>`) and **push to `origin`**
  (github.com/memmaker/easyband, private). Every commit message ends with
  `Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>`. Never force-push.
- Never `fetch()` `.cfg`/`.prf` files from the page: stage them into the
  preload and read them with `Module.FS.readFile` (the template's
  `loadSoundCfg()`).
- The Docs page (stage 6) is not here: write `docs/web/easyband-docs.html`
  in the shape of the template's `make-help.py` output and note it for the
  Mac side.
- The upstream source came as an archive with no history: the first commit
  is the drop, our commits go on top on `main`. Note the version, authors and
  licence from the readme files in the Stage 1 handover.

## RVIP progress

### Stage 1 (get + build): done 2026-09-26 (cloud)
- **Base**: Easyband 2.3 (banner "Easyband 2.3 · The Fun of Angband", Andres
  Zanzani, andres_zanzani@yahoo.it, http://www.majerle.org), built on
  GSN2Band10 (Gwidon S. Naskrent) on **Angband 2.9.3** (Robert Ruehlmann,
  8 July 2000; before him Ben Harrison 2.7.0–2.8.5, Cutler/Astrand/Marsh/
  Hill/Teague/Swiger 2.0–2.6.2; Moria Koeneke 1985, Umoria Wilson 1989).
  Licence: the Angband/Moria notice in the source headers (not-for-profit
  copying; `SRC/main.c` l.3-9), no GPL file. Upstream = the archive drop
  `00f2a06` (no git history; `2c3e95b` in the public repo).
- Folder = repo root; sources in **`SRC/`** (upper case), prefs in
  **`lib/user`** (2.9.3 has no `lib/pref`), edit files `lib/edit`, case **A**.
  `SRC/Makefile.std` is stale (names `cmd-attk.c` etc. that do not exist):
  the source list is every `SRC/*.c` except `main*`, `maid-*`, `Readdib.c`.
- **Web frontend `SRC/main-web.c`** (Zangband template, adapted): 2.9.3
  z-term has `TERM_XTRA_CLEAR` (→ `js_clear`), **no big-tile mode**, plain
  globals `inkey_flag`, `character_generated`, `p_ptr->is_dead`,
  `p_ptr->depth`, window flags in `op_ptr->window_flag[]`, options via
  `option_norm[OPT_*]` (set in `init_web()`: `auto_more`, `center_player`).
  `main.c` is an `if (!done)` chain: `USE_WEB` block calls
  `init_web(argc, argv)` and sets `ANGBAND_SYS = "x11"`; after
  `init_angband()` (which zeroes the window flags) it calls
  `web_init_windows()`. `web_run_end`/beacon not ported (stage 9).
- Port edits (`USE_WEB`): `h-config.h` defines `USE_TRANSPARENCY` (pict hook
  gets terrain `tap/tcp`), `config.h` undefines `SAFE_SETUID` (wasm
  `setuid()` fails → quit at start), `save.c` `web_sync_files()` after a good
  save, `defines.h` `GRAPHICS_SHOCKBOLT 3`, `externs.h` prototypes (also the
  missing `brand_weaponx()`, an implicit declaration = wasm trap risk).
  `lib/user/font-x11.prf` rewritten as a comment-only file (archive damage).
- **Build**: `sh web/build.sh` (needs `emcc` on PATH, `web/toolchain.sh`) →
  `web/dist`: `emcc -O2 -fcommon -std=gnu99 -DUSE_WEB -ISRC -w` + the RVIP
  W7 flags (`-sASYNCIFY -sASYNCIFY_STACK_SIZE=65536 -sSTACK_SIZE=1048576
  -sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=64MB -sFORCE_FILESYSTEM -lidbfs.js
  -sENVIRONMENT=web`), preload `lib/{edit,file,help,user}` +
  empty `data save apex bone info` as `/easyband/lib`. No wasm-ld warnings,
  no `-Wcast-function-type-strict` hits, one implicit declaration (fixed).
- **Page**: `web/index.html`, `web/easyband.js` (from `zangband.js`),
  shared `rvip/web/rvip-wm.js` copied by the build. IDBFS on
  `/easyband/lib/{save,apex,bone}` and **`/easyband/web`** (layout file
  `web-layout.json`): `lib/user` is *not* persisted because the preloaded
  pref files live there (a mount would hide them), so in-game pref dumps are
  lost on reload. Save = `lib/save/0.PLAYER` (`-uPLAYER`).
- **ASan** (native `-DUSE_GCU -DUSE_TPOSIX`, pty + pyte, `web/asan-drive.py`,
  isolated `HOME`): 4 seeds × (2500 keys new character + 2000 restored).
  Fixed in `port:` commit `2f7e980`: `u32b` was `long` → 64-bit hosts looped
  forever in `Rand_div()` at birth (`h-type.h`, `__LP64__`); the Fortune
  cookie (food sval 20) indexed the 20-entry mushroom flavour table and
  scrolls reach sval 52 with `MAX_TITLES` 50 (`object1.c`); macro trigger key
  burst overflow (`cmd4.c`, as Zangband/Frog); `main-gcu.c` used opaque
  ncurses `curscr->_cury`. Then clean.
- **Browser test** (Playwright/Chromium, `web/tests/stage1.js`, 3 seeds):
  splash → birth (Human Warrior, all defaults) → town map, inventory and
  monster list in their windows → 500 random keys → Ctrl-X → "Play again"
  overlay → reload → same character restored; no console errors (only the
  favicon/tiles.webp 404). Screens `web/shots/stage1-*.png`.
- **Tiles decision: Shockbolt.** Own set (`lib/user/graf-new.prf` +
  `lib/xtra/graf/16x16.bmp`, Adam Bolt) covers **1047/1192 = 87.8%**
  (`web/tile-coverage.py new`): r_info 548/677 (Easyband's 129 new monsters
  548–676 have no tile), k_info 435/451, f_info 64/64. Below 95% →
  Shockbolt from `rvip/templates/tactical-angband`. Note: Easyband's
  `r_info.txt` has **no index numbers** (`N:name`, numbered in order from 0,
  `init1.c`).
- Open problems: no big-tile mode in 2.9.3 → with tiles the map term needs
  square cells (`easyband.js` `termShape()` already switches cw = tile /
  tile÷2); Messages window shows a `====` rule when empty (upstream
  `fix_message`); stage-1 page shows "Tiles: on" with no sheet yet.

### Next: stage 2 (explore + stairs)
- Port the explorer from the Zangband template (HANDOVER stage 2 there):
  2.9.3 map is `cave_feat[y][x]`, `cave_info[y][x]` (`CAVE_MARK`,
  `CAVE_GLOW`), objects `cave_o_idx`, monsters `cave_m_idx`; traps are
  features (`FEAT_TRAP_HEAD..TAIL`, `FEAT_INVIS`), doors `FEAT_DOOR_HEAD`
  (locked = `FEAT_DOOR_HEAD+1..+7`), stairs `FEAT_LESS/FEAT_MORE`, shops
  `FEAT_SHOP_HEAD..TAIL`. Main loop `process_player()` in `SRC/dungeon.c`,
  commands in `process_command()` (`dungeon.c`), `disturb()` in `cave.c`/
  `xtra2.c`. Key: `H` if free in `process_command()` and `lib/user/pref.prf`.

### Stage 2 (explore + stairs): done 2026-09-26 (cloud)
- **Explore key `H`** (free in the original keyset and `lib/user/pref.prf`
  `C:0:` keymaps; roguelike `H` stays "run west": no explore key there).
- Code: end of `SRC/cmd2.c`: `auto_explore` (0 / explore / stairs up /
  stairs down), `explore_step()`, `do_cmd_explore()`, `explore_to_stairs()`,
  `explore_reset()`, `explore_new_level()`; prototypes in `externs.h`.
- Hooks: `process_command()` `case 'H'` (`dungeon.c`); `process_player()`
  treats `auto_explore` like running (key abort check, `else if
  (auto_explore) explore_step();` before running); `dungeon()` calls
  `explore_new_level()` after `p_ptr->leaving = FALSE`; `disturb()`
  (`cave.c`) calls `explore_reset()`. `do_cmd_go_up/down()` (`cmd2.c`) walk
  to the nearest known staircase instead of "I see no ... staircase here",
  and take it on arrival.
- **Known grid**: `cave_info[y][x] & CAVE_MARK`, or the explorer's own
  `explore_seen[][]` (every grid with `CAVE_SEEN|CAVE_MARK` at each step;
  2.9.3 forgets torch-lit floors). BFS over `DUNGEON_HGT x DUNGEON_WID`;
  passable: floor, invisible trap, glyph, open/broken door, stairs, rubble
  (dug with `do_cmd_tunnel_aux()`), closed doors (opened with
  `do_cmd_open_aux()`; a locked one is tried once, then skipped: 
  `explore_done[][] = 2`). Never: known traps, shop entrances, walls, a
  visible monster's grid.
- Targets: a known passable grid next to an unknown one, or a grid with a
  seen object (`o_ptr->marked`) not yet stood on. Stops: `disturb()`, a new
  message (`message_num()` changed), a visible monster in line of sight
  (explore only; never-moving ones only when adjacent), a step that did not
  move, no light in the dungeon ("You have no light to explore by."),
  "Nothing left to explore." / "You know of no way up/down.".
- Tested (Playwright, `web/tests/stage2.js`): town `>` walks to the
  entrance and descends ("You enter a maze of down staircases"); DL1 `H`
  walks rooms and corridors, opens doors, picks up gold, tries a locked door
  once ("You failed to pick the lock."), stops on monsters; `<` walks back
  to the up staircase and climbs to town. Screens `web/shots/stage2-*.png`.
  Note: Easyband's starting kit keeps the torches in the pack (wield one
  first, else "You have no light").
- Open problems: a visible erratic monster that never comes closer keeps
  stopping explore (by design, A2); no explore key in the roguelike keyset;
  the move onto an object may prompt (game's own pickup).

### Next: stage 3 (Enter menu + inventory)
- `request_command()` (`SRC/util.c` ~l.3013): keymaps via
  `keymap_act[mode][ch]`, auto-commands in `p_ptr->command_new`; Enter is
  "ignore" in `process_command()`. Item lists `show_inven()`/`show_equip()`
  (`object1.c`), `get_item()` (`object1.c`), `do_cmd_inven()/equip()`
  (`cmd3.c`). Port Zangband's `cmd_menu()` / `inven_screen()` (A3b/A3c).

### Stage 3 (Enter menu + inventory): done 2026-09-26 (cloud)
- **Enter menu**: `cmd_menu()` at the end of `SRC/util.c` (Zangband's
  design): 11 groups in `cmd_menu_groups[]` (as `lib/help/command.txt`
  groups them, plus "Easyband extras": `J` cure all, `K` identify fully),
  incl. `H` explore and the `<`/`>` stair walks. Opened in
  `request_command()` right after `inkey()` when the key is `\r`/`\n`, not
  shopping, and `!keymap_act[mode][key]`. Two boxes (`box_draw()`,
  `box_menu()`), keys shown for the current keyset (`command_key()`
  reverse-looks-up `keymap_act`); 2/8/arrows move, Enter/Space/5/6 choose,
  the key chooses, Esc/4/0 back. Arrow keys arrive as raw X11 keysym
  triggers because the menu reads with `inkey_base` (`menu_inkey()` parses
  `^_..FF52\r`). The chosen underlying command runs past the keymaps
  (`raw` in `request_command()`).
- **Item menus**: `inven_screen()` at the end of `SRC/cmd3.c`;
  `do_cmd_inven()/do_cmd_equip()` call it (not in stores: `character_icky`
  keeps the plain list there). Cursor `>` left of the list (`show_list_col`,
  set by `show_inven()/show_equip()` in `object1.c`). Letter = main action,
  Shift+letter = drop, Ctrl+letter = observe, 2/8/arrows move, Enter/Space/5
  = action box (`inv_action_menu()`), `+ - *` = main/drop/observe, 4/6 or
  `/` = other list, Esc/0/. close, anything else = normal command (as
  before).
- **How item actions run (key queue + preselect)**: `inv_act[]` = {key,
  name, pack/equipment, test} in main-action order (Eat, Quaff, Read, Aim,
  Use, Zap, Activate, Fire, Cast/Pray, Browse, Wear, Take off, Refuel,
  Throw, Drop, Destroy, Observe, Inscribe, Uninscribe). `inv_run()` sets
  `get_item_preselect` (slot index), `p_ptr->command_new = key`,
  `command_new_raw` (no keymap) and `inven_reopen` = `i`/`e`.
  `get_item()` (`object1.c`) returns the preselected slot if the mode and
  `get_item_okay()` accept it, else asks as usual. `request_command()`
  clears the preselect when no command is queued and queues the reopen
  when `inven_may_reopen()` (no moving monster in view).
- Tested (Playwright, `web/tests/stage3.js`, 11 checks pass): menu lists all
  groups, movement group shows `H`, cursor wraps (Up Up → Easyband extras),
  Esc closes, menu runs `V` (version message); `i` list with cursor, food
  letter eats (7→6 rations), list reopens, Enter opens the action box
  (Eat/Throw/Drop/Destroy/Observe/Inscribe), `I` observes, torch letter
  wields it, `e` shows it, Shift+letter drops it. Screens
  `web/shots/stage3-*.png`.
- ASan (native, pty, random keys weighted to `H < > Enter i e`, 4 seeds x
  (2500 new + 2000 restored)): clean.
- Open problems: roguelike keyset not tested in the browser (menu shows
  keys via `command_key()`); no mouse; Ctrl+H/Tab in the list are
  ignored (not letters).

### Next: stage 4 (tiles)
- Decision from stage 1: **Shockbolt** (own 16x16 = 87.8%).
- Generator: port `rvip/templates/zangband/web/mkgraf-shb.py` to 2.9.3
  data (r_info unnumbered, 64 fixed features `FEAT_*` in `defines.h`, no
  t_info/fields; flavours are `S:0x80..0xFF` as in Zangband; bolts
  `S:0x30..0x7F` from `spells1.c bolt_pict()`), write `lib/user/graf-shb.prf`,
  loaded by `graf-x11.prf` `?:[EQU $GRAF shb]`. `main-web.c` already sets
  `GRAPHICS_SHOCKBOLT` + `ANGBAND_GRAF = "shb"`; no big-tile mode: the page
  uses square map cells in tile mode (`termShape()`).

### Stage 4 (tiles): done 2026-09-26 (cloud)
- **Tile set: Shockbolt** 64x64 (Raymond Gaustadnes; Angband 4.2
  `lib/tiles/shockbolt/64x64.png` as lossless `web/tiles.webp`, 13 MB, same
  blob as `rvip/templates/tactical-angband/tiles.webp`), the one set (own
  Adam Bolt 16x16 covers 87.8%). Drawn nearest-neighbour
  (`imageSmoothingEnabled = false`) at the map cell size.
- **Pref** `lib/user/graf-shb.prf`, generated by `python3 web/mkgraf-shb.py`
  (Zangband's generator ported: reads the bundled Shockbolt prefs and 4.2
  gamedata; unnumbered r_info in file order; 2.9.3's 64 fixed features
  mapped by hand incl. the 16 traps in 2.9.3 order, 8 shops, 16 door
  variants; flavours `S:0x80..0xFF`, bolts `S:0x30..0x7F` as Zangband).
  Loaded by `lib/user/graf-x11.prf` `?:[EQU $GRAF shb]`.
- **Coverage** (`web/tile-coverage.py`): **1190/1192 = 99.8%** (r_info
  677/677, k_info 450/451, f_info 63/64; misses are the index-0 entries).
  691 by name, 314 by hand, **185 family stand-ins** (149 monsters, 36
  objects).
- **C**: `main-web.c` `web_graphics()` sets `GRAPHICS_SHOCKBOLT`,
  `use_transparency`, `ANGBAND_GRAF = "shb"` (from `js_tiles_wanted()` at
  start); `web_switch_graphics()` (`reset_visuals(TRUE)` + redraw) at the
  command prompt. `cave.c map_info()`: `graf_shb` shifts floors inside
  Shockbolt's torch/lit/dark triplet (c-1 torch-lit, c+1 dark/blind; only
  with `view_special_lite`/`view_yellow_lite`), wall lighting is off for
  Shockbolt (the text-mode branch would overwrite the tile attr).
- **Page** (`web/easyband.js`): **no big-tile mode in 2.9.3**, so tile mode
  uses square cells (`L.gtile`, own zoom steps 8..64, default fits 80
  columns), text mode `L.tile` x `L.tile/2`; the main term is pinned to
  80x24 (2.9.3's map panel is `SCREEN_WID/HGT` 66x22, fixed). Tiles on/off
  button applies at the next command prompt (cell width changes with it).
- Tested (Playwright `web/tests/stage4.js`): town (brick buildings, shop
  entrances, floor, player tile; 1454 pict calls), DL1 room with a fruit bat
  and a potion, zoom 12→16 px, Tiles off → text 12x24 cells → on again, no
  console errors. Screens `web/shots/stage4-*.png`.
- Open problems: at 1280x800 the tile map is only 12 px per grid (80 square
  cells across the map window; the canvas never scales up): single-window
  mode (Windows ▾) gives 16 px; a real fix is a big-tile mode or a
  term-sized map panel in the C code. Sidebar/status text is spaced out in
  square cells. 185 stand-ins (Easyband's own monsters share family tiles).

### Next: stage 5 (web page)
- Windows: `web_window_flags[]` in `main-web.c` (0 map, 1 Inventory
  `PW_INVEN`, 2 Messages `PW_MESSAGE`, 3 Monsters `PW_M_LIST`, 4 Recall
  `PW_MONSTER|PW_OBJECT`, 5 Equipment `PW_EQUIP`, 6 Character
  `PW_PLAYER_0`), set by `web_init_windows()` after `init_angband()`;
  check `window_flag_desc[]` has entries for all of them. Layout file
  `/easyband/web/web-layout.json`. Game end: `hook_quit` → `js_quit`.
  `web/deploy.sh` exists (never run here).

### Source restored (2026-09-26, Mac side)
The empty drop was a Mac-side extraction failure (p7zip/7zz cannot decode
this solid RAR; The Unarchiver `unar` can). The real source is now in the
repo: 198 files with content. Only the six `lib/user/font-*.prf` files were
unreadable and are still empty; `font-x11.prf` is the only one the web
frontend loads, so write a minimal one (Angband 2.9.3's maps a handful of
wall/floor glyphs; an empty file is acceptable). The game is **Angband 2.9.3**
based (`readme.txt` banner). Continue with stage 1 as planned; the toolchain
recorded in `web/toolchain.sh` is still valid.

### Stage 1 (get + build): BLOCKED 2026-09-26 — upstream source is empty
- **The upstream drop has no content.** Commit `351b2ba` ("upstream Easyband
  v2.3 ...") holds 210 files outside `rvip/` and every one is **0 bytes**
  (blob `e69de29` = git's empty blob): all of `SRC/*.c`/`*.h`/Makefiles,
  `readme.txt`, `Easyband23.txt`, `compile.txt`, `lib/edit/*.txt`,
  `lib/data/*.raw`, `lib/help/*`, `lib/user/*.prf`, `lib/xtra/graf/16x16.bmp`,
  `8x8.bmp`, `mask.bmp`, `lib/xtra/sound/*.wav`. `origin/main` is the same
  (no other branch). Check: `git ls-files -s | grep -v ' e69de29' | grep -v rvip/`
  lists only `HANDOVER.md`. Most likely the `.7z`/`.rar` extraction on the Mac
  wrote names without data (e.g. an extractor that cannot read RAR5/solid 7z).
- Cannot be fetched here: the cloud network policy only allows package
  registries, GitHub and a few download hosts (angband.oook.cz,
  web.archive.org time out), and no other repo is in this session's scope.
- **Consequences:** nothing game-specific could be done in any stage:
  no version/authors/licence (readme files empty), no `src/main-web.c`
  adaptation (z-term quirks unknown), no `web/build.sh` source list, no
  native ASan build, no Playwright birth test, **no tile coverage count**
  (`lib/edit/*_info.txt` and `16x16.bmp` empty) and so no tiles decision.
  Stages 2–6 all need a building game; they were not started.
- **Done anyway:** toolchain installed and verified, recorded in
  `web/toolchain.sh` (emsdk latest = **Emscripten 6.0.10**, Asyncify smoke
  test passed; gcc 13.3 / clang 18.1 for ASan; Playwright Chromium at
  `/opt/pw-browsers`). `web/deploy.sh` (target `ruzzoli.de/roguelikes/easyband/`,
  guard line; never run). Lesson in `rvip/LESSONS.md`.

### Next: stage 1 again (Mac side first) — done, see Stage 1 above
- Re-extract `Easyband (v2.3)[var][src].7z` / `easyband23_src.rar` with a tool
  that reads them (`7zz x`, `unar`), check `find . -size 0` is (nearly) empty
  and `SRC/dungeon.c` has content, commit the real files on top as
  "upstream Easyband v2.3 (content)" (no history rewrite), push.
- Then rerun this routine: stage 1 follows RVIP A0/A1 + A-Zangband with the
  template `rvip/templates/zangband/main-web.c` (2.9.x z-term is closer to
  TinyAngband/Frog: expect `TERM_XTRA_CLEAR`, `inkey_flag`, `dun_level`,
  `p_ptr->is_dead` as plain globals; sources `SRC/`, list from
  `SRC/Makefile.std`), `emcc` via `. ../emsdk/emsdk_env.sh` as in
  `web/toolchain.sh`.

### Stage 5 (web page): done 2026-09-26 (cloud)
- **Windows** (`rvip/web/rvip-wm.js`, `web/index.html` `#t-<id>`, `TERMS`
  in `web/easyband.js` = `WEB_TERMS` 7 in `SRC/main-web.c`): 0 Map,
  1 Inventory `PW_INVEN`, 2 Messages `PW_MESSAGE`, 3 Monsters `PW_M_LIST`,
  4 Recall `PW_MONSTER|PW_OBJECT`, 5 Equipment `PW_EQUIP`, 6 Character
  `PW_PLAYER_0`; all have `window_flag_desc[]` entries (`tables.c`), so
  `load2.c`'s mask keeps them. Set by `web_init_windows()` (after
  `init_angband()`, before the savefile, which brings its own flags).
  Default on: Map, Inventory, Monsters, Messages; Recall, Equipment,
  Character via Windows ▾.
- **Layout file** `/easyband/web/web-layout.json` (IDBFS; wm tree incl.
  which windows are on, zoom, titles, Tiles, audio).
- **Game end**: `quit()` → `quit_aux` = `hook_quit` (set in `init_web()`)
  → `js_quit(msg, p_ptr->is_dead)`; `plog_aux` = `hook_plog`. Ctrl-X →
  "Play again" overlay → reload restores. Death: tombstone/`-more-`/
  "Do you really..." wait for keys in C, then the page syncs and reloads;
  the dead save starts a new birth.
- **Help**: `build.sh` writes a stub `help.html` (stage 6 replaces it).
- **`web/deploy.sh`**: guard line (commit + push first), target
  `ruzzoli.de/roguelikes/easyband/`. **Not run: the cloud has no ssh key
  for ruzzoli.de and no network route there** → Mac side runs
  `sh web/deploy.sh` after `sh web/build.sh`.
- Tested (Playwright `web/tests/stage5.js`, all PASS): new character →
  pack in Inventory, Village idiot/Farmer Maggot in Monsters, empty
  equipment slots in Equipment (Easyband's kit starts in the pack),
  character sheet in Character; Equipment + Character on → Ctrl-X →
  overlay → reload → same windows, character restored; debug `^A y n Great
  Hell Wyrm` → attack it → death → page reloads → birth "Choose a sex"; no
  console errors (only 404s: favicon). Screens `web/shots/stage5-*.png`.
- Open problems: Messages window shows upstream `fix_message`'s `====` rule
  under few messages; with 6 windows at 1280x800 the tile map is small
  (no big-tile mode, stage 4); the top-bar hint text is cut at 1280 px.

### Next: stage 6 (docs + sound)
- Sound: upstream `lib/xtra/sound/sound.cfg` + 20 own wavs (in the
  preload? no: build copies only edit/file/help/user). Port
  `rvip/templates/zangband/web/sounds.py` (events from
  `angband_sound_name[]` in `SRC/tables.c`/`variable.c`), fill from
  `rvip/templates/dubtrain`; hook `TERM_XTRA_SOUND` → `js_sound` exists.
- Help: `web/make-help.py` from the template, `PAGE='easyband.html'`.

### Stage 6 (docs + sound): done 2026-09-26 (cloud; Docs entry left for the Mac)
- **Help** (`web/make-help.py` → `$OUT/help.html` in `build.sh`): About,
  Keyboard controls (keys to remember incl. `H`, Enter menu, item menus,
  `<`/`>` walks; essentials; complete list of 83 keys parsed from
  `lib/help/command.txt`, both keysets), Saving (web), Tips, New player's
  guide, Playing in the browser, Credits (Zanzani, Naskrent, Ruehlmann,
  Harrison, 2.0–2.6.2 team, Koeneke, Wilson; Shockbolt © 2012 Raymond
  Gaustadnes; Dubtrain), About this version (upstream `00f2a06` + compare
  link `memmaker/easyband/compare/00f2a06...main`).
- **Docs page: NOT done here** — `~/Desktop/Games/Roguelikes/Docs` is on
  the Mac. `make-help.py`'s `GAME` dict holds the content in the Docs
  fields; Mac side: add `GAMES` entry `easyband.html` in `build-docs.py`
  (essentials, complete list from `lib/help/command.txt`, Tips, Credits,
  "In the browser"), `GUIDES['easyband.html']` (first section = About) and
  `SAVING['easyband.html']` in `guides.py`, run `python3 build-docs.py`.
  `make-help.py` then picks the Docs entry up automatically.
- **Sound**: `web/sounds.py <cfg> <wavdir>` (events from
  `angband_sound_name[]` in `SRC/variable.c`, 28): Easyband's own samples
  for 15 events, Dubtrain (`rvip/templates/dubtrain`) for 12 empty ones
  (`zap`→`zap_rod`, `stairs`→`stairs_down`), `walk` silent. Web cfg into the
  preload (`/easyband/lib/xtra/sound/sound.cfg`, read by `loadSoundCfg()`
  with `FS.readFile`), 36 wavs (1.5 MB) to `dist/sound`. Upstream cfg
  untouched. Music: `web/music/new_town.ogg` (from the Zangband template,
  originally heavenAndHell) → `dist/music`, loops at depth 0. C/JS hooks
  unchanged (`TERM_XTRA_SOUND` → `js_sound`, `js_depth`).
- Tested (Playwright `web/tests/stage6.js`, all PASS): fresh load Sound
  off / Music off; Help shows the guide, Esc closes; real clicks on Sound
  and Music → eat fires `eat.wav`, quaff `plm_cork_*.wav`,
  `music/new_town.ogg` 200; reload → both still on; no console errors.
  Screen `web/shots/stage6-help.png`.
- Open problems: 2.9.3 calls `sound()` only for ~20 places (hit/miss/kill
  via `message_type`); no hit/kill checked in the browser (same path).

### Next: stage 7 (publish) — Mac side first
- `sh web/build.sh && sh web/deploy.sh` (stage 5's live-URL check is still
  open: https://ruzzoli.de/roguelikes/easyband/), browser check on the live
  page, Docs entry (above), merge `rvip/LESSONS.md` into RVIP.md.

### Stage 7 — publish (done 2026-09-26, Mac)
- **Mac check** (browser pane, own tab, `web/dist` served locally with
  `Cache-Control: no-store`): birth (Human Warrior), town, wield torch,
  `>` walks to the entrance and descends, `H` explores and stops on
  monsters, Enter menu + submenu, `i` list + action box, Windows ▾
  (Equipment, Character on) → zoom → Ctrl-X → "Play again" → reload →
  same windows and character, Help guide (Docs entry), Tiles off/on,
  Sound/Music off at first load, Sound on after a click (`sound/eat.wav`
  on eating), `sound.cfg` read from the preload (no `.cfg` request), no
  console errors (only Chrome's beforeunload notices from forced reloads).
- **What the cloud got wrong** (fixed in `442af2e` "RVIP: stage 5/6
  fixes (Mac)"): the map was 80 square cells across the window (6 px
  grids in the pane, sidebar text spread over square cells, bottom half of
  the Map window empty). Now the map view follows the window
  (`SCREEN_HGT/WID` = `web_view_hgt/wid` under `USE_WEB`, `web_set_view()`
  in `main-web.c`, status row `ROW_MAP + SCREEN_HGT`, `prt_depth()`'s literal
  23), tiles are big tiles over two half-width text cells (`MAP_STEP`,
  filler 255/255 skipped by the page); town layout and detection/magic
  mapping keep the fixed 66x22 (`SCREEN_*_STD`, `DETECT_Y1/X1`).
  `dungeon.c` `fixed_shape` made `Term_resize` fail silently after the
  first layout change (off under `USE_WEB`; full `do_cmd_redraw()` after a
  main resize). Messages: repeats fold to "(xN)" (from the old local stash),
  not the blank birth separator lines. Window "Monsters" → "Visible".
  Explore stop names the monster ("In view: ...").
- **Stash**: kept the message fold, the "In view" message, the local
  tactical-angband paths of `mkgraf-shb.py`/`tile-coverage.py`, "Visible";
  dropped the rest (the cloud had it).
- **Docs**: entry `easyband.html` in `~/Desktop/Games/Roguelikes/Docs`
  (`build-docs.py` GAMES + `parse_easyband()`, `guides.py` GUIDES + SAVING),
  generated from `make-help.py`'s `GAME`; `make-help.py` now reads it.
- **Repos**: public **memmaker/easyband** (this folder, remote `memmaker`,
  branch `main`, `git filter-repo --path rvip --path web/shots
  --invert-paths`); private **memmaker/easyband-cloud** (renamed,
  GitHub only). README with upstream
  `2c3e95b` and the compare view.
- **Live**: https://ruzzoli.de/roguelikes/easyband/ (`sh web/build.sh && sh
  web/deploy.sh`), og block by hand (image `roguelikes/easyband.png`).
  Index `a7d4693`: card before Quickband (`easyband.png`, 60 Shockbolt
  monsters at 32 px, 384x160), count 34, tree Angband → GSN2Band (2000s ·
  Gwidon S. Naskrent, new node) → Easyband (2001 · Andres Zanzani). Year
  from the archive dates (Easyband 2.3 files 2001-08/09; `readme.txt`
  2001-08-07); GSN2Band's own year not checked (stage 8).
- Test IndexedDB `/easyband/...` deleted on localhost:64185 and ruzzoli.de.
- Open problems: at 80 columns minimum (status line) the tiled map in a
  narrow Map window is still small (e.g. with Equipment + Character on);
  a menu box ending on a big tile's filler cell can leave a stale half
  (JS skips the filler, as in Zangband); detection is the fixed 66x22
  centred on the view, so it can cover grids outside a small view; the
  `====` birth separator stays in Messages (upstream).

### Next: stage 8 (shrine)
- Page `~/Games/roguelikes-index/shrine/easyband.html` (+ `shrine/easyband/`),
  Info button on the card, ✦ in the tree, `#bar h1` link in `web/index.html`.
- Material: manual/help = `lib/help/*.txt` (2.9.3 help files: `general.txt`,
  `command.txt`, `birth.txt`, `playing.txt`, `version.txt`, ...), the
  in-game `?` menu; readme `readme.txt` (Angband 2.9.3's); licence = the
  Angband/Moria notice in the source headers (`SRC/main.c` l.3-9; no GPL
  file); changelog `Easyband23.txt` (2.1 → 2.2 → 2.3 changes); credits
  `lib/file/news.txt` (GSN2Band10 line). Walkthrough: none known (look on
  RogueBasin / angband.oook.cz; the archive's author site
  http://www.majerle.org is from 2001). Dates: archive entries 2000-07
  (Angband 2.9.3 base) to 2001-09 (Easyband 2.3).

### Stage 8 — shrine (done 2026-09-26, Mac)
- **Live**: https://ruzzoli.de/roguelikes/shrine/easyband.html (+ `shrine/easyband/`
  `manual.html`, `changelog.txt`, `license.txt`); index `7b6a679`: card Info
  button, tree ✦, game page `#bar h1` links to the shrine (this repo
  `de06045`, deployed by copying `web/index.html` into `dist`).
- **Manual**: `python3 web/mkmanual.py > ~/Games/roguelikes-index/shrine/easyband/manual.html`
  (all 14 `lib/help` files in `help.hlp` menu order, `(file.txt)` refs linked).
  Changelog = `Easyband23.txt` (Latin-1 → UTF-8); licence = the source-header
  notice + readme's NO WARRANTY. No walkthrough exists; cheats: `^W`/`^A` and
  the cheat options do **not** mark the character (Easyband removed it).
- **Lineage (web-checked)**: Easyband 1.0 = 9 Jan 2001, 2.3 = 24 Sep 2001
  (Bablos' variant list, archived; RogueBasin's 2 Nov 2001 is the Amiga
  date). 2.0 adopted GSN2band 1.0 (10 Nov 2000, Gwidon S. Naskrent, "based on
  Angband 2.9.1"). Base version disagrees: GSN2band page/RogueBasin 2.9.1,
  `defines.h` 2.9.2, readme/news 2.9.3. Tree now: GSN2Band 2000 from 2.9.1 →
  Easyband 2001 from GSN2Band 1.0. GSNband (1998, on Zangband 2.2.8) is not
  in the tree (not its code parent).
- Open problems: card text still says "about 130 new monsters" (that number
  was the tile gap; Easyband itself names only 2 new monsters, most extras
  come from GSN2band/GSNband); og block kept the card text.

### Next: stage 9 (graveyard + leaderboard)

### Stage 9 — graveyard + leaderboard (done)
- **Hook**: `SRC/files.c` `close_game()`, top of the `is_dead` branch (before
  `kingly()`/tombstone) → `web_run_end()` in `SRC/main-web.c` (extern in
  `externs.h`) → `js_beacon` EM_JS → `RvipWM.report` (outbox). Win =
  `total_winner` (tested first: `kingly()` rewrites `died_from`); quit =
  `died_from` "Quitting"/"Interrupting"/"Abortion"; else death.
- **Fields**: g=easyband, ev, name (`op_ptr->full_name`, omitted if empty),
  killer (`died_from` = `monster_desc(0x88)`, "a "/"an "/"the "/"The "
  stripped; none for win/quit), depth (`p_ptr->depth`, levels), score
  (`total_points()`, the Hall of Fame's points), turns (game `turn`), lvl.
  Nothing missing.
- **Killer art**: roguelikes-index `4dc6dcb` (`killers/make.py` `easyband()`:
  r_info names in file order from 0 + `lib/user/graf-shb.prf` →
  `web/tiles.webp` 64 px → 32 px; 665 PNGs). Same commit: card/og text
  "676 monsters, many of them from GSN2band" (was "about 130 new monsters");
  also the shrine og and `web/index.html` og.
- **Live test** (browser pane, https://ruzzoli.de/roguelikes/easyband/):
  death (`^A y n` "Great Hell Wyrm", walk into it) → `ev=death&name=Tester&
  killer=Great%20Hell%20Wyrm&depth=0&score=110&turns=18&lvl=1&id=…&at=…` 204;
  quit (`Q y @`) → `ev=quit…` 204; outbox `[]` both times. `/easyband/…`
  IndexedDB deleted on ruzzoli.de. Win path not reachable in a test (same
  branch, `total_winner` checked first).
- Browser pane: dispatched `keydown` Escape/Backspace do not reach the game
  at birth; real keys (`computer key`) do, letters work either way. `^A n`
  with "Morgoth, Lord of Darkness" places nothing in town (unique); a
  non-unique works.
