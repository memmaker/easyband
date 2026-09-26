/* Stage 3: Enter menu lists every group/command; item menus run real actions */
const T = require('./lib.js');
(async () => {
	const { browser, page, errors } = await T.open();
	await T.waitText(page, /Press any key/);
	await T.key(page, 'Escape');
	await T.birth(page, 'Menus');
	const ok = [];
	/* Enter menu: groups */
	await T.key(page, 'Enter', {}, 500);
	let s = await T.screen(page);
	console.log('=== Enter menu\n' + s.split('\n').slice(0, 15).join('\n'));
	ok.push(['menu has 11 groups', /Easyband extras/.test(s) && /Inventory/.test(s)]);
	await T.shot(page, 'stage3-menu');
	/* group b (movement) lists H explore */
	await T.key(page, 'b', {}, 400);
	s = await T.screen(page);
	console.log('=== Movement group\n' + s.split('\n').slice(0, 12).join('\n'));
	ok.push(['movement lists H', /H\s+Auto-explore/.test(s)]);
	await T.key(page, 'Escape', {}, 300);
	/* cursor stays on b; Up Up wraps to the last group (Easyband extras), Enter opens it */
	await T.key(page, 'ArrowUp', {}, 200);
	await T.key(page, 'ArrowUp', {}, 200);
	await T.key(page, 'Enter', {}, 300);
	s = await T.screen(page);
	ok.push(['cursor reached Easyband extras', /Cure all/.test(s)]);
	await T.key(page, 'Escape', {}, 200);
	await T.key(page, 'Escape', {}, 300);
	ok.push(['Esc closes', !/Commands \(Esc/.test(await T.screen(page))]);
	/* Menu runs a command by key: game group, V = version */
	await T.key(page, 'Enter', {}, 300); await T.key(page, 'j', {}, 300); await T.key(page, 'V', {}, 600);
	ok.push(['menu ran V (version)', /playing/i.test(await T.lastMsg(page))]);
	await T.key(page, 'Escape', {}, 300);
	/* Item menu: i shows the cursor list; letter of food = eat */
	await T.key(page, 'i', {}, 500);
	s = await T.screen(page);
	console.log('=== inventory menu\n' + s.split('\n').slice(0, 9).join('\n'));
	ok.push(['inventory list with cursor', />\s*a\)/.test(s)]);
	await T.shot(page, 'stage3-inven');
	const food = /([a-w])\) [^\n]*Ration/.exec(s);
	const hp = /Cur HP\s+(\d+)/;
	if (food) {
		await T.key(page, food[1], {}, 800);
		const m = await T.lastMsg(page);
		console.log('eat via letter:', m);
		ok.push(['letter ate food', /You have (\d+) Rations/.test(m) && !new RegExp(food[0].match(/(\d+) Ration/)[1] + ' Ration').test(m)]);
		s = await T.screen(page);
		ok.push(['list reopened after action', /\(Inventory\)/.test(s)]);
	}
	/* Enter on the cursor item opens its action menu; Observe via the menu key I */
	await T.key(page, 'Enter', {}, 500);
	s = await T.screen(page);
	console.log('=== action menu\n' + s.split('\n').slice(0, 12).join('\n'));
	ok.push(['action menu shows Observe', /I\s+Observe/.test(s)]);
	await T.shot(page, 'stage3-action');
	await T.key(page, 'I', {}, 800);
	s = await T.screen(page);
	console.log('=== observe\n' + s.split('\n').slice(0, 6).join('\n'));
	await T.key(page, 'Escape', {}, 400); await T.key(page, 'Escape', {}, 400);
	/* torch: wield via letter in the list */
	await T.key(page, 'Escape', {}, 300);
	await T.key(page, 'i', {}, 500);
	s = await T.screen(page);
	const torch = /([a-w])\) [^\n]*Torch/.exec(s);
	if (torch) { await T.key(page, torch[1], {}, 800); console.log('wield torch:', await T.lastMsg(page)); }
	await T.key(page, 'Escape', {}, 300);
	/* equipment: e, 4/6 switches lists; Shift+letter of the torch = drop it */
	await T.key(page, 'e', {}, 500);
	s = await T.screen(page);
	console.log('=== equipment menu\n' + s.split('\n').slice(0, 14).join('\n'));
	ok.push(['equipment shows the torch', /Torch/.test(s)]);
	const lt = /([a-l])\) [^\n]*Torch/.exec(s);
	if (lt) { await T.key(page, lt[1].toUpperCase(), {}, 800); console.log('drop torch:', await T.lastMsg(page)); ok.push(['Shift+letter dropped', /no more|drop/i.test(await T.lastMsg(page))]); }
	await T.key(page, 'Escape', {}, 300);
	/* roguelike keyset: the menu shows the roguelike keys (T = take off? no: w/T) */
	console.log(ok.map(o => (o[1] ? 'PASS ' : 'FAIL ') + o[0]).join('\n'));
	console.log('errors:', errors.filter(e => !/404/.test(e)));
	await browser.close();
})().catch(e => { console.error(e); process.exit(1); });
