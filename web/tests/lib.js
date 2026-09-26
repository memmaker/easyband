/*
 * Playwright helpers for the Easyband web port (cloud RVIP run).
 * Serve web/dist (python3 -m http.server 8765 -d web/dist) and run a test
 * with NODE_PATH pointing at a node_modules that has playwright:
 *   NODE_PATH=$S/pw/node_modules node web/tests/stage1.js
 * The page is read through a text shadow of term 0 (wraps Module.qb.text/
 * wipe/clear/pict, A-Zangband "Browser testing") and canvas pixels.
 */
const { chromium } = require('playwright');
const fs = require('fs');
const path = require('path');
const URL = process.env.EB_URL || 'http://127.0.0.1:8765/';
const SHOTS = path.join(__dirname, '..', 'shots');

async function open(opts = {}) {
	const browser = opts.browser || await chromium.launch({ executablePath: process.env.PW_CHROME || undefined });
	const ctx = opts.ctx || await browser.newContext({ viewport: { width: 1280, height: 800 } });
	const page = await ctx.newPage();
	const errors = [];
	page.on('pageerror', e => errors.push(String(e)));
	page.on('console', m => { if (m.type() === 'error') errors.push(m.text()); if (process.env.EB_LOG) console.log('[console]', m.type(), m.text()); });
	await page.addInitScript(() => {
		window.__rows = []; window.__terms = [[], [], [], [], [], [], []];
		const hook = () => {
			const qb = window.Module && window.Module.qb;
			if (!qb || qb.__hooked) return;
			qb.__hooked = true;
			const put = (x, y, str, t = 0) => {
				const R = t ? window.__terms[t] : window.__rows;
				const r = (R[y] || '').padEnd(x + str.length, ' ');
				R[y] = r.slice(0, x) + str + r.slice(x + str.length);
			};
			const text = qb.text, wipe = qb.wipe, clear = qb.clear, pict = qb.pict;
			qb.text = function (t, x, y, n, a, s) {
				{ const H = Module.HEAPU8; let o = ''; for (let i = 0; i < n; i++) o += String.fromCharCode(H[s + i] || 32); put(x, y, o, t); }
				return text.apply(this, arguments);
			};
			qb.pict = function (t, x, y, n, ap, cp) {
				if (!t) { const H = Module.HEAPU8; let o = ''; for (let i = 0; i < n; i++) { const k = H[cp + i]; o += (k & 0x80) ? '█' : String.fromCharCode(k || 32); } put(x, y, o); }
				window.__picts = (window.__picts || 0) + 1;
				return pict.apply(this, arguments);
			};
			qb.wipe = function (t, x, y, n) { put(x, y, ' '.repeat(n), t); return wipe.apply(this, arguments); };
			qb.clear = function (t) { if (!t) window.__rows = []; else window.__terms[t] = []; return clear.apply(this, arguments); };
		};
		const iv = setInterval(() => { hook(); if (window.Module && window.Module.qb && window.Module.qb.__hooked) clearInterval(iv); }, 5);
	});
	await page.goto(URL);
	return { browser, ctx, page, errors };
}

async function screen(page) {
	return (await page.evaluate(() => window.__rows.map(r => (r || '').replace(/\s+$/, '')))).join('\n');
}

/* Text of sub-window t (1 Inventory, 2 Messages, 3 Monsters, ...) */
async function term(page, t) {
	return (await page.evaluate(t => (window.__terms[t] || []).map(r => (r || '').replace(/\s+$/, '')), t)).join('\n');
}

/* Last non-empty line of the Messages window */
async function lastMsg(page) {
	const l = (await term(page, 2)).split('\n').filter(x => x.trim());
	return l.length ? l[l.length - 1] : '';
}

/* Step into a monster next to '@' (map rows below the top line); true if one was there */
async function fight(page) {
	const L = (await screen(page)).split('\n');
	for (let y = 1; y < L.length; y++) {
		const x = L[y].indexOf('@', 13);
		if (x < 0) continue;
		const dirs = { '-1,-1': '7', '-1,0': '8', '-1,1': '9', '0,-1': '4', '0,1': '6', '1,-1': '1', '1,0': '2', '1,1': '3' };
		for (const k in dirs) {
			const [dy, dx] = k.split(',').map(Number);
			const c = (L[y + dy] || '')[x + dx];
			if (c && /[a-ln-zA-Z&,]/.test(c)) { await key(page, dirs[k], {}, 150); return true; }
		}
	}
	return false;
}

async function waitText(page, re, ms = 20000) {
	const t0 = Date.now();
	while (Date.now() - t0 < ms) {
		const s = await screen(page);
		if (re.test(s)) return s;
		await page.waitForTimeout(100);
	}
	throw new Error('timeout waiting for ' + re + '\n' + await screen(page));
}

/* key: a character, or a KeyboardEvent key name; mods {ctrl, shift} */
async function key(page, k, mods = {}, delay = 60) {
	await page.evaluate(([k, m]) => {
		document.dispatchEvent(new KeyboardEvent('keydown', { key: k, code: m.code || '', ctrlKey: !!m.ctrl, shiftKey: !!m.shift, bubbles: true }));
	}, [k, mods]);
	await page.waitForTimeout(delay);
}

async function keys(page, str, delay = 60) {
	for (const ch of str) await key(page, ch === '\r' ? 'Enter' : ch === '\x1b' ? 'Escape' : ch, {}, delay);
}

async function shot(page, name) {
	fs.mkdirSync(SHOTS, { recursive: true });
	await page.screenshot({ path: path.join(SHOTS, name + '.png') });
}

/* Birth: accept whatever the questions offer until the map shows */
async function birth(page, name = 'Tester') {
	for (let i = 0; i < 80; i++) {
		const s = await screen(page);
		if (/Cur HP\s+\d+/.test(s) && /LEVEL\s+\d+/.test(s) && !/ESC to/.test(s)) return s;
		if (/-more-/.test(s)) await key(page, 'Escape');
		else if (/Choose a sex/.test(s)) await key(page, 'b');
		else if (/Choose a race/.test(s)) await key(page, 'a');
		else if (/Choose a class/.test(s)) await key(page, 'a');
		else if (/\[y\/n\]|\(y\/n\)/.test(s)) await key(page, 'n');
		else if (/What is your name|Enter a name/i.test(s)) { await keys(page, name); await key(page, 'Enter'); }
		else if (/ESC to continue|ESC to accept/.test(s)) await key(page, 'Escape');
		else if (/Cur HP\s+\d+/.test(s) && /LEVEL\s+\d+/.test(s)) return s;
		else await key(page, 'Enter');
		await page.waitForTimeout(250);
	}
	throw new Error('birth did not finish\n' + await screen(page));
}

module.exports = { open, screen, term, lastMsg, fight, waitText, key, keys, shot, birth, URL };
