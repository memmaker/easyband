**RVIP port** of Easyband 2.3 by Andres Zanzani, from the source archive
`easyband23_src.rar` (`Easyband (v2.3)[var][src].7z`); commit `2c3e95b` is
that archive untouched (extracted with `unar`; the earlier commit `351b2ba`
holds a broken extraction with empty files).
Play: https://ruzzoli.de/roguelikes/easyband/
Our changes: https://github.com/memmaker/easyband/compare/2c3e95b...main

Easyband ("The Fun of Angband") is a gentler, more generous Angband:
Moria (1985) → Umoria (1989) → Angband 2.0–2.9.3 (2000) → GSN2Band10
(Gwidon S. Naskrent) → Easyband. It keeps the classic dive to Morgoth on
dungeon level 100 and adds about 130 monsters, the PowPlayer and Snip races,
new ego weapons and artifacts, cheaper shops, a Magic shop that identifies
and recharges, and the `J` (cure all) and `K` (identify fully) commands.
Upstream notes: `readme.txt`, `Easyband23.txt`, `lib/help/`.

What this port adds:
- **Web frontend** `SRC/main-web.c` (z-term hooks to canvases, Emscripten +
  Asyncify), saves in the browser's IndexedDB, autosave; page
  `web/index.html` + `web/easyband.js` with the shared `rvip-wm.js` windows:
  Map, Inventory, Visible, Messages, Recall, Equipment, Character.
- **Map view that fills the window** and **big tiles** (a tile over two text
  cells), which 2.9.3 lacked (`SRC/defines.h`, `web_set_view()`); gameplay
  areas (town, detection) keep the original 66×22.
- **Explore** `H`, **`<` / `>`** walk to the nearest known staircase and take
  it (`SRC/cmd2.c`).
- **Enter menu** of all commands (`SRC/util.c`), **inventory cursor** with
  item menus (`SRC/cmd3.c`), no `-more-` stops.
- **Tiles**: Shockbolt (Angband 4.2), mapped to Easyband's monsters and
  items by `web/mkgraf-shb.py` (`lib/user/graf-shb.prf`, 99.8%).
- **Sound**: Easyband's own samples plus the Dubtrain pack for the gaps;
  town music; both off by default.
- Upstream bug fixes found with AddressSanitizer (64-bit `u32b`, flavour
  tables, macro buffer) in the `port:` commit.

Controls (original keyset): 1–9 / arrows move, Shift runs, `H` explore,
`<` `>` stairs, Enter command menu, `i` / `e` item menus, `g` pick up,
`E` `q` `r` eat / quaff / read, `m` `p` cast / pray, `f` `v` fire / throw,
`J` cure all, `K` identify, `?` help, Ctrl+S save, Ctrl+X save and quit.
The Help button opens the game guide.

Build: `sh web/build.sh` → `web/dist` (Homebrew emscripten; see
`web/toolchain.sh`). Deploy: `sh web/deploy.sh`. Notes: `HANDOVER.md`.

Credits: Easyband by Andres Zanzani; GSN2Band by Gwidon S. Naskrent;
Angband 2.9.3 by Robert Ruehlmann, 2.7.0–2.8.5 by Ben Harrison, 2.0–2.6.2 by
Alex Cutler, Andy Astrand, Sean Marsh, Geoff Hill, Charles Teague and Charles
Swiger; Moria © 1985 Robert Alan Koeneke, Umoria © 1989 James E. Wilson.
Tiles: Shockbolt © 2012 Raymond Gaustadnes (from Angband 4.2). Sound: the
Dubtrain Angband Sound Pack v3.1.0. Web port: memmaker.

Licence: the Angband/Moria notice in the source files: "may be copied and
distributed for educational, research, and not for profit purposes provided
that this copyright and statement are included in all such copies."
