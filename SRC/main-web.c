/* File: main-web.c */

/*
 * Browser (Emscripten/WASM) front end for Easyband (Angband 2.9.3 z-term).
 *
 * Copied from the Zangband web port (RVIP template) and adapted: 2.9.3
 * has TERM_XTRA_CLEAR, no big-tile mode, and plain globals (inkey_flag,
 * p_ptr->is_dead, p_ptr->depth, op_ptr->window_flag).
 *
 * The map (term 0's map area) is the only <canvas> (web/easyband.js).
 * Every other term, the sidebar and status line (the Status pane), and
 * whatever term 0 shows over the map while the game is "icky" (item lists,
 * menus, stores, the character sheet; before the character exists: the
 * whole birth screen) go to the page as HTML text lines built here (RVIP W0
 * rule 6, web_send_rows()).  Blocking input uses Asyncify: when the game waits
 * for a key we sleep in emscripten_sleep(), which yields to the browser.
 *
 * The module registers itself as "x11" so that the same pref files
 * (keymaps, window layout, graphics) as the X11 build are used; special
 * keys are sent in the X11 keysym macro format.
 */

#include "angband.h"

#ifdef USE_WEB

#include <emscripten.h>

#define WEB_TERMS 7		/* terms 1-6: see web_window_flags[] */

static term web_term[WEB_TERMS];

/* Pending "save now" request from the page (tab hidden / closing) */
static int web_want_save = 0;

/* Last time we yielded to the browser */
static double web_last_yield = 0;

static void web_switch_graphics(int on);
static void web_set_view(void);


/* ---- JavaScript side (implemented in web/easyband.js) ---- */

EM_JS(void, js_text, (int t, int x, int y, int n, int a, const char *s), {
	Module.qb.text(t, x, y, n, a, s);
});

EM_JS(void, js_wipe, (int t, int x, int y, int n), {
	Module.qb.wipe(t, x, y, n);
});

EM_JS(void, js_clear, (int t), {
	Module.qb.clear(t);
});

EM_JS(void, js_curs, (int t, int x, int y), {
	Module.qb.curs(t, x, y);
});

EM_JS(void, js_pict, (int t, int x, int y, int n, const byte *ap, const char *cp,
                      const byte *tap, const char *tcp), {
	Module.qb.pict(t, x, y, n, ap, cp, tap, tcp);
});

/* Tiles (1) or text (0) as the page's Tiles button says */
EM_JS(int, js_tiles_wanted, (void), {
	return Module.qb.tilesWanted();
});

/* Map zoom in tile mode (1..4): a grid is 2m x m cells */
static int web_mult = 1;

EM_JS(int, js_tile_mult, (void), {
	return Module.qb.tileMult();
});

EM_JS(void, js_mult_applied, (int m), {
	Module.qb.multApplied(m);
});

/* -1: no change, else the new Tiles setting */
EM_JS(int, js_tiles_switch, (void), {
	return Module.qb.tilesSwitch();
});

EM_JS(void, js_fresh, (int t), {
	Module.qb.fresh(t);
});

/*
 * Text panes (RVIP W0 rule 6): pane 1..6 = web terms 1..6, WEB_POP = the
 * pop-up, WEB_STAT = Status.  line(): one row, trimmed, colour runs
 * "\x05#rrggbb" .. "\x06", "\x01" .. "\x06" = the cursor cell, "\x07" + 8 hex
 * (a c ta tc) + width digit = a tile icon (web_row()).  rows(): the rows in
 * use (after the lines).
 */
EM_JS(void, js_line, (int p, int y, const char *s), {
	Module.qb.line(p, y, UTF8ToString(s));
});

EM_JS(void, js_rows, (int p, int n), {
	Module.qb.rows(p, n);
});

/* The pop-up's box on term 0 (cells): shown at its place over the map */
EM_JS(void, js_pop_at, (int x, int y), {
	Module.qb.popAt(x, y);
});

/*
 * The map canvas shows only term 0's map area: cells from (x, y) on, minus
 * the bottom status rows; the sidebar and the status line are the Status pane
 */
EM_JS(void, js_origin, (int x, int y, int bottom), {
	Module.qb.origin(x, y, bottom);
});

/* Term 0 row 0 (messages, questions): RvipWM.prompt */
EM_JS(void, js_prompt, (const char *s), {
	Module.qb.prompt(UTF8ToString(s));
});

EM_JS(void, js_bell, (void), {
	Module.qb.bell();
});

EM_JS(void, js_sound, (const char *name), {
	Module.qb.sound(UTF8ToString(name));
});

EM_JS(void, js_depth, (int depth), {
	Module.qb.depth(depth);
});

EM_JS(void, js_color, (int i, int r, int g, int b), {
	Module.qb.color(i, r, g, b);
});

EM_JS(int, js_term_cols, (int t), {
	return Module.qb.termCols(t);
});

EM_JS(int, js_term_rows, (int t), {
	return Module.qb.termRows(t);
});

/* Layout changes after a browser resize */
EM_JS(int, js_layout_pending, (int t), {
	return Module.qb.layoutPending(t);
});

EM_JS(int, js_pending_cols, (int t), {
	return Module.qb.pendingCols(t);
});

EM_JS(int, js_pending_rows, (int t), {
	return Module.qb.pendingRows(t);
});

EM_JS(void, js_apply_layout, (int t, int cols, int rows), {
	Module.qb.applyLayout(t, cols, rows);
});

/* Next queued input: -1 none, else key */
EM_JS(int, js_next_event, (int at_cmd), {
	return Module.qb.nextEvent(at_cmd);
});

EM_JS(void, js_quit, (const char *msg, int dead), {
	Module.qb.quit(msg ? UTF8ToString(msg) : "", dead);
});

EM_JS(void, js_plog, (const char *msg), {
	Module.qb.plog(UTF8ToString(msg));
});

EM_JS(void, js_sync, (void), {
	Module.qb.sync();
});


/* Run report (roguelikes-index/server/CONTRACT.md) through RvipWM's outbox;
   never throws, offline it waits in the outbox. Negative ints are omitted. */
EM_JS(void, js_beacon, (const char *ev, const char *name, const char *killer, int depth, int score, int turns, int lvl), {
	try {
		var p = [['g', 'easyband'], ['ev', UTF8ToString(ev)], ['name', name ? UTF8ToString(name) : ''],
		         ['killer', killer ? UTF8ToString(killer) : ''], ['depth', depth], ['score', score], ['turns', turns], ['lvl', lvl]];
		var q = p.filter(function (a) { return a[1] !== '' && !(a[1] < 0); })
		         .map(function (a) { return a[0] + '=' + encodeURIComponent(a[1]); }).join('&');
		if (window.RvipWM && RvipWM.report) RvipWM.report(q); else fetch('/roguelikes/beacon?' + q, { keepalive: true, mode: 'no-cors' }).catch(function () {});
	} catch (e) {}
});

/* Called from close_game() (files.c) when the run is over, before kingly()
   and the tombstone. Suicide (Q) and signals die too ("Quitting",
   "Interrupting", "Abortion"); retiring as a winner keeps total_winner. */
void web_run_end(void)
{
	cptr k = p_ptr->died_from, ev = "death";

	if (p_ptr->total_winner) ev = "win", k = NULL;
	else if (streq(k, "Quitting") || streq(k, "Interrupting") || streq(k, "Abortion")) ev = "quit", k = NULL;
	else if (prefix(k, "a ")) k += 2;
	else if (prefix(k, "an ")) k += 3;
	else if (prefix(k, "the ") || prefix(k, "The ")) k += 4;
	js_beacon(ev, op_ptr->full_name, k, p_ptr->depth, (int)total_points(), (int)turn, p_ptr->lev);
}


/* Persist the save directories (called after every save) */
void web_sync_files(void)
{
	js_sync();
}


/* Called from JS when the page is about to be hidden or closed */
EMSCRIPTEN_KEEPALIVE void web_request_save(void)
{
	web_want_save = 1;
}


/* Waiting for a command (the only safe moment for layout/save/tiles) */
static bool web_at_cmd(void)
{
	return (inkey_flag && character_generated);
}


/*
 * Resize the terms to the layout the page computed after a browser resize.
 * Term_resize() runs the resize hooks, which redraw the contents.  The main
 * window changes its size only at the command prompt; a cell-size change
 * alone applies at once.
 */
static void web_apply_layout(void)
{
	int i, main_resized = 0;
	term *old = Term;

	/* Only the map follows its window; text terms have a fixed size */
	for (i = 0; i < 1; i++)
	{
		term *t = &web_term[i];
		int cols, rows;

		if (!js_layout_pending(i)) continue;

		cols = js_pending_cols(i);
		rows = js_pending_rows(i);
		if (cols < 1) cols = 1;
		if (rows < 1) rows = 1;

		if (!i)
		{
			if (cols < 80) cols = 80;
			if (rows < 24) rows = 24;

			if (((cols != t->wid) || (rows != t->hgt)) && !web_at_cmd()) continue;
		}

		/* New canvas size and cell size (the canvas starts blank) */
		js_apply_layout(i, cols, rows);

		Term_activate(t);
		if ((cols == t->wid) && (rows == t->hgt)) Term_redraw();
		else
		{
			Term_resize(cols, rows);
			if (!i) web_set_view(), main_resized = 1;
			Term_redraw();
		}
	}

	Term_activate(old);

	/* New map view size: the game redraws the whole screen (command prompt) */
	if (main_resized && character_generated) do_cmd_redraw();
}


/* Move queued browser input into the main term's key queue */
static int web_pump(void)
{
	int k, got = 0;
	term *old = Term;

	web_apply_layout();

	Term_activate(&web_term[0]);

	while ((k = js_next_event(web_at_cmd())) >= 0)
	{
		Term_keypress(k);
		got = 1;
	}

	/* Tiles <-> text: only while waiting for a command */
	if (web_at_cmd() && !got)
	{
		int on = js_tiles_switch();

		if ((on >= 0) && (on != (use_graphics != GRAPHICS_NONE)))
		{
			web_switch_graphics(on);
			got = 1;
		}
		else if (js_tile_mult() != web_mult)
		{
			web_mult = js_tile_mult();
			js_mult_applied(web_mult);
			web_set_view();
			do_cmd_redraw();
			got = 1;
		}
	}

	/* Safe autosave: only while waiting for a command */
	if (web_want_save && web_at_cmd() && !p_ptr->is_dead && !got &&
	    (Term->key_head == Term->key_tail))
	{
		web_want_save = 0;
		Term_keypress(KTRL('S'));
		got = 1;
	}

	Term_activate(old);
	return got;
}

static void web_yield(int ms)
{
	emscripten_sleep(ms);
	web_last_yield = emscripten_get_now();
}

static errr web_check_events(int wait)
{
	if (web_pump()) return (0);

	if (!wait)
	{
		/* Let the browser paint now and then during long actions */
		if (emscripten_get_now() - web_last_yield > 50) web_yield(0);
		return (web_pump() ? 0 : 1);
	}

	while (1)
	{
		web_yield(10);
		if (web_pump()) return (0);
	}
}

static void web_react(void)
{
	int i;

	for (i = 0; i < 16; i++)
		js_color(i, angband_color_table[i][1], angband_color_table[i][2],
		         angband_color_table[i][3]);
}

static int web_idx(void)
{
	return (int)(Term - web_term);
}

/*
 * Text terms have a fixed size, list widths near the default window's
 * (RVIP W0 rule 3: the window's size never changes the text); a window
 * smaller than its text scrolls.  Messages keep 200 lines (fix_message()
 * fills from the top).
 */
#define WEB_PAD(A, C)	(((A) == 255) && ((byte)(C) == 255))
#define WEB_TILE(A, C)	(((A) & 0x80) && ((byte)(C) & 0x80) && !WEB_PAD(A, C))
#define WEB_POP		WEB_TERMS
#define WEB_STAT	(WEB_TERMS + 1)
static const int web_cols[WEB_TERMS] = { 0, 80, 120, 50, 64, 80, 80 };	/* lists: weights at column 71 */
static const int web_rows[WEB_TERMS] = { 0, 26, 200, 60, 60, 16, 24 };

/* A hash of what the page shows per pane row, and rows in use */
static u32b web_hash[WEB_TERMS + 2][256];
static int web_nrows[WEB_TERMS + 2];
static bool web_sent[WEB_TERMS + 2];	/* a line went out: rows() ends the batch */
static int web_pop_x = -1, web_pop_y = -1;
static u32b web_prompt_hash = 1;

/* Row buffer: 255 cells, each at most a colour run + an icon */
static char web_buf[255 * 24 + 16];

extern int screen_depth;	/* util.c */

/*
 * Term 0 shows a pop-up (text over the map, or a whole screen before play):
 * while "icky" (screen_save(), stores, the death screens)
 */
static bool web_pop_on(void)
{
	return (!character_generated || (character_icky > 0));
}

/*
 * The screen under the pop-up: the map saved by the outer screen_save(),
 * unless the game is icky for another reason too (a store): then the whole
 * screen is the pop-up
 */
static term_win *web_under(void)
{
	if (character_generated && (screen_depth > 0) && (character_icky == screen_depth))
		return (Term->mem);
	return (NULL);
}

static u32b web_fnv(cptr s)
{
	u32b h = 2166136261U;

	while (*s) h = (h ^ (byte)*s++) * 16777619U;
	return (h | 1);
}

static void web_send(int p, int y, cptr s)
{
	u32b h = web_fnv(s);

	if ((y > 255) || (web_hash[p][y] == h)) return;
	web_hash[p][y] = h;
	web_sent[p] = TRUE;
	js_line(p, y, s);
}

static void web_send_rows(int p, int n)
{
	if ((web_nrows[p] == n) && !web_sent[p]) return;
	web_nrows[p] = n;
	web_sent[p] = FALSE;
	js_rows(p, n);
}

/*
 * Row y of term window w, cells x0..cols-1, as a pane line: cells equal to
 * the saved screen "under" (the map below a pop-up) are blank; trailing
 * blanks dropped; the cursor cell (cx, cy) marked.  Returns the length.
 */
static int web_row(term_win *w, term_win *under, int cols, int y, int x0, int cx, int cy)
{
	char *b = web_buf;
	char *end = b;	/* after the last shown cell */
	int x, cur = -1, open_end = 0;

	for (x = x0; x < cols; x++)
	{
		byte a = w->a[y][x], ta = w->ta[y][x];
		byte c = (byte)w->c[y][x], tc = (byte)w->tc[y][x];
		bool curs = ((x == cx) && (y == cy));

		if (under && (a == under->a[y][x]) && (c == (byte)under->c[y][x]) &&
		    (ta == under->ta[y][x]) && (tc == (byte)under->tc[y][x]))
			a = TERM_WHITE, c = ' ';

		/* A tile: an icon over two cells if a blank follows (lists), else one */
		if (WEB_TILE(a, c))
		{
			int wd = 1;

			if ((x + 1 < cols) && ((byte)w->c[y][x + 1] == ' ') && !(w->a[y][x + 1] & 0x80)) wd = 2;
			if (cur >= 0) *b++ = '\x06', cur = -1;
			b += sprintf(b, "\x07%02x%02x%02x%02x%d", a, c, ta, tc, wd);
			x += wd - 1;
			end = b, open_end = 0;
			continue;
		}
		if (WEB_PAD(a, c) || (c == 127) || ((c < 32) && (c != 1) && (c != 2))) a = TERM_WHITE, c = ' ';

		/* Glyphs as the canvas draws them (font-x11.prf's DEC graphics) */
		if (curs || (c != ' '))
		{
			int k = a & 0x0F;

			if (curs)
			{
				if (cur >= 0) *b++ = '\x06';
				*b++ = '\x01';
				cur = -1;
			}
			else if (k != cur)
			{
				if (cur >= 0) *b++ = '\x06';
				b += sprintf(b, "\x05#%02x%02x%02x", angband_color_table[k][1],
				             angband_color_table[k][2], angband_color_table[k][3]);
				cur = k;
			}
		}

		if (c == 1) b += sprintf(b, "\xe2\x97\x86");
		else if (c == 2) b += sprintf(b, "\xe2\x96\x92");
		else if (c >= 128) *b++ = (char)(0xC0 | (c >> 6)), *b++ = (char)(0x80 | (c & 0x3F));
		else *b++ = (char)c;

		if (curs) *b++ = '\x06';
		if (curs || (c != ' ')) end = b, open_end = (cur >= 0);
	}

	b = end;
	if (open_end) *b++ = '\x06';
	*b = '\0';
	return (int)(b - web_buf);
}

static errr Term_text_web(int x, int y, int n, byte a, cptr s);
static errr Term_pict_web(int x, int y, int n, const byte *ap, const char *cp,
                          const byte *tap, const char *tcp);

/*
 * After a pop-up the canvas shows term 0 again: paint it whole from
 * Term->scr (z-term's own redraw skips what the pop-up "left unchanged").
 */
static void web_repaint(void)
{
	term_win *w = Term->scr;
	int y, x, n;

	js_clear(0);
	for (y = 0; y < Term->hgt; y++)
	{
		for (x = 0; x < Term->wid; x += n)
		{
			byte a = w->a[y][x];

			n = 1;
			if ((a & 0x80) && (w->c[y][x] & 0x80))
			{
				(void)Term_pict_web(x, y, 1, &w->a[y][x], &w->c[y][x], &w->ta[y][x], &w->tc[y][x]);
				continue;
			}
			while ((x + n < Term->wid) && (w->a[y][x + n] == a) && !((a & 0x80) && (w->c[y][x + n] & 0x80))) n++;
			(void)Term_text_web(x, y, n, a, &w->c[y][x]);
		}
	}
}

/* A sub-window's rows after its Term_fresh() */
static void web_sub_fresh(int i)
{
	term_win *w = Term->scr;
	int y, n = 0;

	for (y = 0; y < Term->hgt; y++)
	{
		if (web_row(w, NULL, Term->wid, y, 0, -1, -1)) n = y + 1;
		web_send(i, y, web_buf);
	}
	web_send_rows(i, n);
}

/*
 * The Status pane: the sidebar (term 0 rows ROW_MAP.., columns left of the
 * map) down to its last used row, then the status line (last row) as its
 * groups, one per line (runs of cells split at two or more blanks).
 */
static void web_status(void)
{
	term_win *w = Term->scr;
	int y, x, n = 0, last = 0, st = Term->hgt - 1;

	for (y = ROW_MAP; y < st; y++)
	{
		if (web_row(w, NULL, COL_MAP, y, 0, -1, -1)) last = y - ROW_MAP + 1;
		web_send(WEB_STAT, y - ROW_MAP, web_buf);
	}
	n = last;

	for (x = 0; x < Term->wid; )
	{
		int e, gap;

		if (((byte)w->c[st][x] == ' ') && !(w->a[st][x] & 0x80)) { x++; continue; }
		for (e = x, gap = 0; (e < Term->wid) && (gap < 2); e++)
			gap = (((byte)w->c[st][e] == ' ') && !(w->a[st][e] & 0x80)) ? gap + 1 : 0;
		if (last && (n == last)) web_send(WEB_STAT, n++, "");	/* a blank row before the groups */
		(void)web_row(w, NULL, e, st, x, -1, -1);
		web_send(WEB_STAT, n++, web_buf);
		x = e;
	}
	web_send_rows(WEB_STAT, n);
}

/*
 * Term 0 after its Term_fresh(): row 0 to the prompt line; a pop-up (the
 * cells that differ from the saved screen, rows 1..) to the pop-up pane.
 */
static void web_main_fresh(void)
{
	term_win *w = Term->scr, *under = NULL;
	int y, x, y0 = -1, y1 = -1, x0 = Term->wid;
	char row0[256];
	static bool was_pop = TRUE;

	/* A pop-up ended: the map canvas shows term 0 again */
	if (!web_pop_on() && was_pop) web_repaint();
	was_pop = web_pop_on();
	if (!was_pop) web_status();

	/* The prompt line: row 0 as plain text */
	for (x = 0; x < Term->wid; x++)
	{
		byte c = (byte)w->c[0][x];
		row0[x] = ((c < 32) || (c >= 127) || WEB_TILE(w->a[0][x], c)) ? ' ' : (char)c;
	}
	while ((x > 0) && (row0[x - 1] == ' ')) x--;
	row0[x] = '\0';
	if (web_fnv(row0) != web_prompt_hash)
	{
		web_prompt_hash = web_fnv(row0);
		js_prompt(row0);
	}

	if (web_pop_on())
	{
		under = web_under();

		/* The pop-up's box: rows and first column that differ */
		for (y = 1; y < Term->hgt; y++)
		{
			for (x = 0; x < Term->wid; x++)
			{
				byte a = w->a[y][x];
				char c = w->c[y][x];

				if (under ? ((a == under->a[y][x]) && (c == under->c[y][x]) &&
				             (w->ta[y][x] == under->ta[y][x]) && (w->tc[y][x] == under->tc[y][x]))
				          : ((c == ' ') && !(a & 0x80)))
					continue;
				if (y0 < 0) y0 = y;
				y1 = y;
				if (x < x0) x0 = x;
				break;
			}
			/* (a row's first difference; x0 is the least of them) */
		}
	}

	/* No pop-up (any more) */
	if (y0 < 0)
	{
		if (web_nrows[WEB_POP] || (web_pop_x >= 0))
		{
			web_send_rows(WEB_POP, 0);
			(void)memset(web_hash[WEB_POP], 0, sizeof(web_hash[WEB_POP]));
			web_pop_x = web_pop_y = -1;
			js_pop_at(-1, -1);
		}
		return;
	}

	if ((x0 != web_pop_x) || (y0 != web_pop_y))
	{
		web_pop_x = x0, web_pop_y = y0;
		js_pop_at(x0, y0);
	}
	for (y = y0; y <= y1; y++)
	{
		(void)web_row(w, under, Term->wid, y, x0, w->cv ? w->cx : -1, w->cy);
		web_send(WEB_POP, y - y0, web_buf);
	}
	web_send_rows(WEB_POP, y1 - y0 + 1);
}

/* Draw on the map canvas: term 0 without a pop-up (text terms: at their fresh) */
static bool web_canvas(void)
{
	return (!web_idx() && !web_pop_on());
}

static errr Term_xtra_web(int n, int v)
{
	switch (n)
	{
		case TERM_XTRA_CLEAR: if (web_canvas()) js_clear(0); return (0);
		case TERM_XTRA_NOISE: js_bell(); return (0);
		case TERM_XTRA_SOUND:
			if ((v > 0) && (v < SOUND_MAX)) js_sound(angband_sound_name[v]);
			return (0);
		case TERM_XTRA_FRESH:
			if (web_idx()) { web_sub_fresh(web_idx()); return (0); }
			web_main_fresh();
			js_fresh(0);

			/* The page's Sound button is the only switch (off by default) */
			use_sound = TRUE;

			/* The page plays town music at depth 0 */
			js_depth(character_generated ? p_ptr->depth : -1);
			return (0);
		case TERM_XTRA_BORED: return (web_check_events(0));
		case TERM_XTRA_EVENT: return (web_check_events(v));
		case TERM_XTRA_FLUSH:
			while (js_next_event(0) >= 0) ;
			return (0);
		case TERM_XTRA_DELAY:
			if (!web_idx()) js_fresh(0);
			if (v > 0) web_yield(v);
			return (0);
		case TERM_XTRA_REACT: web_react(); return (0);
	}

	return (1);
}

static errr Term_curs_web(int x, int y)
{
	if (!web_canvas()) return (0);
	js_curs(0, x, y);
	return (0);
}

static errr Term_wipe_web(int x, int y, int n)
{
	if (!web_canvas()) return (0);
	js_wipe(0, x, y, n);
	return (0);
}

static errr Term_text_web(int x, int y, int n, byte a, cptr s)
{
	if (!web_canvas()) return (0);
	js_text(0, x, y, n, a, s);
	return (0);
}

static errr Term_pict_web(int x, int y, int n, const byte *ap, const char *cp,
                          const byte *tap, const char *tcp)
{
	if (!web_canvas()) return (0);
	js_pict(0, x, y, n, ap, cp, tap, tcp);
	return (0);
}


/*
 * The map view (SCREEN_HGT/WID in defines.h) fills the main term: below the
 * message row, above the status row, right of the sidebar.  With tiles each
 * grid takes two text cells (big tiles), so the sidebar keeps normal text.
 */
int web_view_hgt = SCREEN_HGT_STD, web_view_wid = SCREEN_WID_STD, web_map_step = 1, web_map_vstep = 1;
int web_row_bottom = ROW_MAP + SCREEN_HGT_STD;

static void web_set_view(void)
{
	term *t = &web_term[0];

	web_map_vstep = (use_graphics != GRAPHICS_NONE) ? web_mult : 1;
	web_map_step = 2 * web_map_vstep - (use_graphics == GRAPHICS_NONE);
	web_view_hgt = MIN(DUNGEON_HGT, (t->hgt - ROW_MAP - 1) / web_map_vstep);
	web_view_wid = MIN(DUNGEON_WID, (t->wid - COL_MAP) / web_map_step);
	web_row_bottom = t->hgt - 1;

	if (character_generated) verify_panel();
}

/* Shockbolt tiles (lib/user/graf-shb.prf), or text */
static void web_graphics(int on)
{
	use_graphics = arg_graphics = on ? GRAPHICS_SHOCKBOLT : GRAPHICS_NONE;
	use_transparency = on;
	ANGBAND_GRAF = "shb";
	if (web_term[0].hgt) web_set_view();
}

/* The page's Tiles button, applied at the command prompt */
static void web_switch_graphics(int on)
{
	web_graphics(on);
	reset_visuals(TRUE);
	do_cmd_redraw();
}


static void hook_plog(cptr str)
{
	if (str) js_plog(str);
}

static void hook_quit(cptr str)
{
	int i;

	for (i = 0; i < WEB_TERMS; i++) (void)term_nuke(&web_term[i]);

	/* After death the tombstone and scores already waited for a key */
	js_sync();
	js_quit(str, p_ptr->is_dead);
}


/*
 * What each sub-window shows (TERMS in web/easyband.js).  init_angband()
 * clears the flags, main.c then calls web_init_windows(); a savefile
 * brings its own flags.
 */
static const u32b web_window_flags[WEB_TERMS] =
{
	0,
	PW_INVEN,					/* 1 Inventory */
	PW_MESSAGE,					/* 2 Messages */
	PW_M_LIST,					/* 3 Visible (monster list) */
	PW_MONSTER | PW_OBJECT,		/* 4 Recall */
	PW_EQUIP,					/* 5 Equipment */
	PW_PLAYER_0					/* 6 Character */
};

void web_init_windows(void)
{
	int i;

	for (i = 0; i < WEB_TERMS; i++) op_ptr->window_flag[i] = web_window_flags[i];
}


errr init_web(int argc, char **argv)
{
	int i;

	(void)argc;
	(void)argv;

	/* RVIP defaults for new characters: no -more- stops, centred map */
	option_norm[OPT_auto_more] = TRUE;
	option_norm[OPT_center_player] = TRUE;

	web_react();

	/* The canvas: term 0's map area only (before the page asks for sizes) */
	js_origin(COL_MAP, ROW_MAP, 1);

	/* Shockbolt tiles unless the page says text */
	web_mult = js_tile_mult();
	js_mult_applied(web_mult);
	web_graphics(js_tiles_wanted());

	for (i = 0; i < WEB_TERMS; i++)
	{
		term *t = &web_term[i];
		int cols = i ? web_cols[i] : js_term_cols(0), rows = i ? web_rows[i] : js_term_rows(0);

		if (!i)
		{
			if (cols < 80) cols = 80;
			if (rows < 24) rows = 24;
		}

		term_init(t, cols, rows, (i == 0) ? 1024 : 16);

		t->soft_cursor = TRUE;
		t->attr_blank = TERM_WHITE;
		t->char_blank = ' ';

		t->xtra_hook = Term_xtra_web;
		t->curs_hook = Term_curs_web;
		t->wipe_hook = Term_wipe_web;
		t->text_hook = Term_text_web;
		t->pict_hook = Term_pict_web;
		t->higher_pict = TRUE;

		Term_activate(t);
		angband_term[i] = t;
	}

	Term_activate(&web_term[0]);
	web_set_view();

	web_last_yield = emscripten_get_now();

	quit_aux = hook_quit;
	plog_aux = hook_plog;

	return (0);
}

#endif /* USE_WEB */
