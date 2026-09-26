# Easyband v2.3: handover

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

(nothing yet — start with stage 1)
