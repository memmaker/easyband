/* Stage 2: '>' walks to the town's down staircase and descends, 'H' explores DL1, '<' walks back up */
const T = require('./lib.js');
const depth = s => { const m = /Lev (\d+)\s*$/m.exec(s); return /Town\s*$/m.test(s) ? 0 : m ? +m[1] : -1; };
const at = s => { const L = s.split('\n'); for (let y = 0; y < L.length; y++) { const x = L[y].indexOf('@'); if (x >= 13) return [y, x]; } return null; };
(async () => {
	const { browser, page, errors } = await T.open();
	await T.waitText(page, /Press any key/);
	await T.key(page, 'Escape');
	let s = await T.birth(page, 'Walker');
	console.log('town, depth', depth(s));
	/* Easyband's starting kit has the torches in the pack: wield one */
	await T.key(page, 'w', {}, 500);
	let inv = await T.screen(page);
	if (!/Torch/.test(inv)) { await T.key(page, '*', {}, 500); inv = await T.screen(page); }
	console.log(inv.split('\n').slice(0, 8).join('\n'));
	const tl = /([a-z])\) [^\n]*Torch/.exec(inv);
	console.log('wield torch:', tl && tl[1]);
	if (tl) await T.key(page, tl[1], {}, 500); else await T.key(page, 'Escape');
	await T.key(page, '>', {}, 3000);
	s = await T.screen(page);
	console.log('after > : depth', depth(s), '| msg:', await T.lastMsg(page));
	await T.shot(page, 'stage2-dl1');
	/* explore: press H until nothing left or 60 presses; handle monsters by stepping on */
	let moves = 0, last = at(s), stops = {};
	for (let i = 0; i < 120; i++) {
		await T.key(page, 'H', {}, 1200);
		s = await T.screen(page);
		const m = await T.lastMsg(page);
		stops[m.replace(/\d+/g, '#').slice(0, 40)] = (stops[m.replace(/\d+/g, '#').slice(0, 40)] || 0) + 1;
		const p = at(s);
		if (p && last && (p[0] !== last[0] || p[1] !== last[1])) moves++;
		last = p;
		if (/-more-/.test(s)) await T.key(page, 'Escape');
		if (/Nothing left to explore/.test(m)) break;
		if (/Cur HP\s+(\d+)/.test(s) && +/Cur HP\s+(\d+)/.exec(s)[1] < 12) await T.key(page, 'J', {}, 300);	/* Easyband: cure all */
		if (/You see a monster/.test(m)) { for (let f = 0; f < 15 && await T.fight(page); f++); if (!await T.fight(page)) await T.key(page, ',', {}, 200); }
		if (/You die/.test(s)) break;
	}
	console.log('H presses moved the player', moves, 'times; stop messages:', JSON.stringify(stops));
	await T.shot(page, 'stage2-explored');
	/* '<' walks to a known up staircase (the one we arrived on) */
	const d1 = depth(s);
	for (let i = 0; i < 6 && depth(await T.screen(page)) === d1; i++) await T.key(page, '<', {}, 2500);
	s = await T.screen(page);
	console.log('after < : depth', depth(s), '| msg:', await T.lastMsg(page));
	await T.shot(page, 'stage2-up');
	console.log(s);
	console.log('errors:', errors.filter(e => !/404/.test(e)));
	await browser.close();
})().catch(e => { console.error(e); process.exit(1); });
