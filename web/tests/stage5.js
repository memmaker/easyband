/* Stage 5: window contents, layout survives a reload, Ctrl-X overlay,
   death (debug: summon a Great Hell Wyrm) -> page reloads -> new birth */
const T = require('./lib.js');
(async () => {
	let { browser, ctx, page, errors } = await T.open();
	const ok = (name, v, info = '') => console.log((v ? 'PASS ' : 'FAIL ') + name + (info ? '  ' + info : ''));
	await T.waitText(page, /Press any key/);
	await T.key(page, 'Escape');
	await T.birth(page, 'Winny');
	await T.key(page, 'Escape', {}, 500);
	/* Equipment + Character windows on (Windows menu checkboxes) */
	await page.evaluate(() => ['eqp', 'chr'].forEach(id => {
		const i = document.querySelector('#btn-layout input[data-id="' + id + '"]') || document.querySelector('input[data-id="' + id + '"]');
		if (i && !i.checked) { i.checked = true; i.dispatchEvent(new Event('change')); }
	}));
	await page.waitForTimeout(800);
	await T.key(page, 'i', {}, 400); await T.key(page, 'Escape', {}, 400);
	const W = {};
	for (const [t, n] of [[1, 'inv'], [2, 'msg'], [3, 'mon'], [5, 'eqp'], [6, 'chr']]) W[n] = await T.term(page, t);
	console.log(JSON.stringify(W, null, 1));
	ok('inventory window', /a\) .*(Ration|Torch|Potion|Flask|Scroll)/i.test(W.inv));
	ok('equipment window', /(Weapon|Body|Light|Wielding)|a\) /i.test(W.eqp));
	ok('character window', /Name|Race|Class/.test(W.chr));
	const shown = await page.evaluate(() => ['main', 'inv', 'msg', 'mon', 'rec', 'eqp', 'chr'].filter(id => !document.getElementById('t-' + id).classList.contains('wm-off')));
	ok('windows shown', shown.includes('eqp') && shown.includes('chr'), shown.join(','));
	await T.shot(page, 'stage5-windows');
	/* Save + quit, reload: layout and character kept */
	await T.key(page, 'x', { ctrl: true }, 1000);
	for (let i = 0; i < 4 && !(await page.evaluate(() => !document.getElementById('overlay').hidden)); i++) await T.key(page, 'Escape', {}, 400);
	ok('Ctrl-X overlay', await page.evaluate(() => !document.getElementById('overlay').hidden));
	await page.waitForTimeout(1500);
	await page.reload();
	await T.waitText(page, /Press any key/);
	await T.key(page, 'Escape', {}, 1500);
	const shown2 = await page.evaluate(() => ['main', 'inv', 'msg', 'mon', 'rec', 'eqp', 'chr'].filter(id => !document.getElementById('t-' + id).classList.contains('wm-off')));
	ok('layout after reload', shown2.join() === shown.join(), shown2.join(','));
	ok('character restored', /Winny/.test(await T.term(page, 6)) || /Winny/.test(await T.screen(page)));
	/* Death: debug mode, summon a Great Hell Wyrm next to the player */
	await T.key(page, 'a', { ctrl: true }, 500);
	let s = await T.screen(page);
	if (/\[y\/n\]|y\/n/.test(s)) await T.key(page, 'y', {}, 500);
	s = await T.screen(page);
	if (/\[y\/n\]|y\/n/.test(s)) await T.key(page, 'y', {}, 500);
	await T.keys(page, 'n'); await page.waitForTimeout(300);
	await T.keys(page, 'Great Hell Wyrm'); await T.key(page, 'Enter', {}, 500);
	/* text map so fight() can find the 'D' next to '@' */
	await page.click('#btn-tiles'); await T.key(page, 'Escape', {}, 800);
	console.log(await T.term(page, 3));
	let reloaded = false;
	page.on('framenavigated', f => { if (f === page.mainFrame()) reloaded = true; });
	for (let i = 0; i < 300 && !reloaded; i++) {
		s = await T.screen(page);
		if (i % 40 === 0) console.log('--- ' + i + '\n' + s.split('\n').slice(0, 3).join('\n'));
		if (/Do you really|\[y\/n\]/.test(s)) await T.key(page, 'y', {}, 200);
		else if (/-more-/.test(s)) await T.key(page, 'Escape', {}, 150);
		else if (/You die|Killed|RIP|tomb|Goodbye/i.test(s)) await T.key(page, 'Escape', {}, 200);
		else if (!(await T.fight(page))) await T.key(page, 's', {}, 150);   /* attack it (wakes it) or wait */
		if (i === 5) await T.shot(page, 'stage5-wyrm');
	}
	ok('page reloaded after death', reloaded);
	if (reloaded) {
		await page.waitForTimeout(2000);
		await T.waitText(page, /Press any key/, 30000);
		await T.key(page, 'Escape', {}, 1500);
		s = await T.screen(page);
		ok('new birth after death', !/Winny/.test(s) && /(sex|race|character|Choose|Name)/i.test(s), s.split('\n').slice(0, 4).join('|'));
		await T.shot(page, 'stage5-newbirth');
	}
	ok('no console errors', !errors.filter(e => !/404/.test(e)).length, JSON.stringify(errors));
	await browser.close();
})().catch(e => { console.error(e); process.exit(1); });
