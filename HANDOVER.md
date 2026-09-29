# Easyband v2.3: handover

Web port, all RVIP stages 1–9 done (procedure: `~/Games/rvip-tools/RVIP.md`).
Public repo **memmaker/easyband** (remote `memmaker`, branch `main`). The
import started in a cloud session (private `memmaker/easyband-cloud`, deleted
2026-09-27); its `rvip/` bundle is gone.
Live: https://ruzzoli.de/roguelikes/easyband/ · shrine
https://ruzzoli.de/roguelikes/shrine/easyband.html

## The game
- Easyband 2.3 (Andres Zanzani, 2001; 1.0 = 9 Jan 2001, 2.3 = 24 Sep 2001),
  from GSN2band 1.0 (Gwidon S. Naskrent, 2000) on Angband 2.9.x (source
  says 2.9.2/2.9.3, GSN2band page 2.9.1). Licence: the Angband/Moria notice in
  the source headers (`SRC/main.c` l.3-9), no GPL file.
- Upstream = archive drop, no history: `2c3e95b` in this repo (README compare
  view). The solid RAR only extracts with `unar`; the six `lib/user/font-*.prf`
  were unreadable and are restored from Angband v2.9.3 (byte sizes match).
  The web build uses `$SYS x11`, so `font-x11.prf`'s X11 palette applies
  (decision: keep it).
- Sources in **`SRC/`**, prefs in **`lib/user`** (no `lib/pref`), edit files
  `lib/edit`. Case A (z-term). `SRC/Makefile.std` is stale: sources = every
  `SRC/*.c` except `main*`, `maid-*`, `Readdib.c`. `r_info.txt` has **no index
  numbers** (numbered in file order from 0).
- Native builds: `build.sh unix|win`, used by `.github/workflows/release.yml`
  (tag `v*`).

## Build, test, deploy
- `sh web/build.sh` (needs `emcc`; see `web/toolchain.sh`) → `web/dist`,
  preload `lib/{edit,file,help,user}` + empty `data save apex bone info` as
  `/easyband/lib`. Page `web/index.html` + `web/easyband.js`, loads shared
  `../rvip-wm.js` and `../rvip-app.js` (not copied).
- `sh web/deploy.sh` (guard: commit + push first).
- IDBFS: `/easyband/lib/{save,apex,bone}` and `/easyband/web`
  (`web-layout.json`). `lib/user` is not persisted (a mount would hide the
  preloaded prefs), so in-game pref dumps are lost on reload. Save =
  `lib/save/0.PLAYER`.
- Tests: Playwright `web/tests/stage1..6.js` + `lib.js` (panes via
  `__pane(p)`, `status()` = pane 8); native ASan random-key driver
  `web/asan-drive.py` (`-DUSE_GCU -DUSE_TPOSIX`, pty + pyte).

## File map (port code)
- `SRC/main-web.c`: web frontend. `init_web()` (options `auto_more`,
  `center_player`; `hook_quit` → `js_quit`), `web_init_windows()` after
  `init_angband()`, `web_graphics()`/`web_switch_graphics()` (Shockbolt,
  `ANGBAND_GRAF = "shb"`), `web_set_view()`, `web_run_end()` (beacon).
  `USE_WEB` edits: `h-config.h` `USE_TRANSPARENCY`, `config.h` no
  `SAFE_SETUID`, `save.c` `web_sync_files()`, `h-type.h` `u32b` fix for LP64.
- Windows: 0 Map, 1 Inventory, 2 Messages, 3 Visible, 4 Recall, 5 Equipment,
  6 Character (`WEB_TERMS`, all have `window_flag_desc[]` entries), 7 pop-up,
  8 Status. Sub-terms are HTML `<pre>` rows (`web_sub_fresh()`); the map
  canvas shows term 0's map area only. Pop-up = term 0 while
  `!character_generated || character_icky` (only the diff vs `Term->mem` when
  every icky level is a screen_save; `screen_depth` is no longer static).
- Map view follows the window: `SCREEN_HGT/WID` = `web_view_hgt/wid`; town
  layout and detection keep the fixed 66x22 (`SCREEN_*_STD`); `fixed_shape`
  off under `USE_WEB`. Tiles are big tiles `MAP_STEP x MAP_VSTEP` cells
  (fillers 255/255 skipped by the page). Status row `ROW_BOTTOM` =
  `web_row_bottom`.
- Explore `H` / stair walks `<` `>`: end of `SRC/cmd2.c` (`auto_explore`,
  `explore_step()` …), hooks in `dungeon.c` `process_command()`/
  `process_player()`/`dungeon()` and `cave.c disturb()`. No explore key in
  the roguelike keyset.
- Enter menu `cmd_menu()` end of `SRC/util.c`; item menus `inven_screen()`
  end of `SRC/cmd3.c` (key queue + `get_item_preselect` in `object1.c`).
- Tiles: Shockbolt 64x64 `web/tiles.webp`; own Adam Bolt 16x16 covered only
  87.8%. Pref `lib/user/graf-shb.prf` from `python3 web/mkgraf-shb.py`
  (99.8% by `web/tile-coverage.py`, 185 family stand-ins), loaded by
  `graf-x11.prf` `?:[EQU $GRAF shb]`. `cave.c map_info()` shifts floors in
  Shockbolt's lit/dark triplet.
- Sound: `web/sounds.py` (Easyband's samples + Dubtrain fill-ins), cfg in
  the preload (read with `FS.readFile`, never `fetch()`); music
  `web/music/new_town.ogg`.
- Help: `web/make-help.py` (reads the Docs entry `easyband.html` in
  `~/Desktop/Games/Roguelikes/Docs`). Shrine manual:
  `python3 web/mkmanual.py > ~/Games/roguelikes-index/shrine/easyband/manual.html`.
- Map fonts "Easyband original: WxH": `lib/xtra/font/*.fon` (X11 misc-fixed,
  public domain) → `web/fonts/Easyband_*.woff` by `web/mkfon.py`; shipped in
  the game's own `dist/fonts/`; `termShape()` snaps cells to whole pixels.
- Beacon: `SRC/files.c close_game()` → `web_run_end()`; win =
  `total_winner`, quit = `died_from` "Quitting"/"Interrupting"/"Abortion".
  Killer art: roguelikes-index `killers/make.py easyband()`.

## Gotchas
- Easyband's start kit keeps the torches in the pack: wield one before
  exploring ("You have no light").
- Cheats (`^W`/`^A`, cheat options) do not mark the character.
- Browser pane: dispatched Escape/Backspace don't reach the game at birth;
  real keys do. `^A n` with a unique places nothing in town.

## Open
- A menu box ending on a big tile's filler cell can leave a stale half.
- Detection is the fixed 66x22 centred on the view, so it can cover grids
  outside a small view.
- 2.9.3 calls `sound()` in only ~20 places.
