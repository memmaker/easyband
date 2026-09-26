/* Stage 1: birth, random keys, Ctrl-X save, reload restores the character */
const T = require('./lib.js');
(async () => {
	const seed = +(process.argv[2] || 1);
	let rnd = seed; const rand = n => { rnd = (rnd * 1103515245 + 12345) & 0x7fffffff; return rnd % n; };
	let { browser, ctx, page, errors } = await T.open();
	await T.waitText(page, /Press any key/);
	await T.key(page, 'Escape');
	const s = await T.birth(page, 'Tester');
	console.log('=== after birth\n' + s);
	await T.shot(page, 'stage1-birth');
	/* random keys: no Q (suicide), no ^X (quit), no ^S/^Z */
	const pool = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPRTUVWXYZ0123456789 ,.<>;:?!@#$%^&*()-_=+[]{}/~'.split('')
		.concat(['Enter', 'Escape', 'Escape', 'Escape', 'ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown']);
	for (let i = 0; i < 500; i++) await T.key(page, pool[rand(pool.length)], {}, 15);
	for (let i = 0; i < 8; i++) await T.key(page, 'Escape', {}, 50);
	const before = await T.screen(page);
	console.log('=== after random keys\n' + before);
	const side = t => t.split('\n').slice(1, 18).map(l => l.slice(0, 12)).join('|');
	await T.shot(page, 'stage1-random');
	const dead = /You die|Killed by|RIP/.test(await T.screen(page));
	/* Save and quit */
	await T.key(page, 'x', { ctrl: true }, 800);
	for (let i = 0; i < 4 && !(await page.evaluate(() => !document.getElementById('overlay').hidden)); i++) { await T.key(page, 'Escape', {}, 400); }
	const over = await page.evaluate(() => document.getElementById('overlay-msg').textContent + ' visible=' + !document.getElementById('overlay').hidden);
	console.log('=== overlay: ' + over);
	await page.waitForTimeout(1500);
	await page.reload();
	await T.waitText(page, /Press any key/);
	await T.key(page, 'Escape', {}, 1500);
	const r = await T.screen(page);
	console.log('=== after reload\n' + r);
	await T.shot(page, 'stage1-restored');
	console.log('restored:', side(r) === side(before) || dead, dead ? '(died)' : '', 'errors:', errors.filter(e => !/404/.test(e)));
	await browser.close();
})().catch(e => { console.error(e); process.exit(1); });
