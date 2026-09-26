/* Stage 4: Shockbolt tiles on the map (canvas pixels), text/tiles toggle, dungeon tiles */
const T = require('./lib.js');
/* Map cells of term 0: how many are drawn as tiles (many colours) vs text */
async function census(page) {
	return page.evaluate(() => {
		const cv = document.querySelector('#t-main canvas'), c = cv.getContext('2d');
		const qb = Module.qb, T0 = { cw: cv.width / qb.termCols(0), ch: cv.height / qb.termRows(0) };
		let tile = 0, text = 0, blank = 0;
		for (let y = 1; y < qb.termRows(0) - 1; y++) for (let x = 13; x < qb.termCols(0); x++) {
			const d = c.getImageData(Math.floor(x * T0.cw), Math.floor(y * T0.ch), Math.max(1, Math.floor(T0.cw)), Math.max(1, Math.floor(T0.ch))).data;
			const cols = new Set();
			for (let i = 0; i < d.length; i += 4) cols.add((d[i] >> 4) + ',' + (d[i + 1] >> 4) + ',' + (d[i + 2] >> 4));
			if (cols.size <= 1) blank++; else if (cols.size <= 12) text++; else tile++;
		}
		return { tile, text, blank, cell: T0.cw + 'x' + T0.ch };
	});
}
(async () => {
	const { browser, page, errors } = await T.open();
	await T.waitText(page, /Press any key/);
	await T.key(page, 'Escape');
	await T.birth(page, 'Tiles');
	await page.waitForTimeout(800);
	console.log('town, tiles on:', JSON.stringify(await census(page)), 'picts', await page.evaluate(() => window.__picts));
	await T.shot(page, 'stage4-town-tiles');
	/* torch + down */
	await T.key(page, 'w', {}, 400);
	let inv = await T.screen(page); if (!/Torch/.test(inv)) { await T.key(page, '*', {}, 400); inv = await T.screen(page); }
	const tl = /([a-z])\) [^\n]*Torch/.exec(inv); if (tl) await T.key(page, tl[1], {}, 400); else await T.key(page, 'Escape');
	await T.key(page, '>', {}, 3000);
	await T.key(page, 'H', {}, 3000);
	console.log('DL1, tiles on:', JSON.stringify(await census(page)));
	await T.shot(page, 'stage4-dl1-tiles');
	/* zoom in: bigger cells */
	await page.click('#btn-zoom-in'); await page.click('#btn-zoom-in');
	await T.key(page, 'Escape', {}, 800);
	console.log('zoomed:', JSON.stringify(await census(page)));
	await T.shot(page, 'stage4-dl1-zoom');
	/* Tiles off: text at the next command prompt */
	await page.click('#btn-tiles');
	await T.key(page, 'Escape', {}, 1000);
	console.log('tiles off:', JSON.stringify(await census(page)), await page.evaluate(() => document.getElementById('btn-tiles').textContent));
	await T.shot(page, 'stage4-dl1-text');
	await page.click('#btn-tiles');
	await T.key(page, 'Escape', {}, 1000);
	console.log('tiles on again:', JSON.stringify(await census(page)));
	console.log('errors:', errors.filter(e => !/404/.test(e)));
	await browser.close();
})().catch(e => { console.error(e); process.exit(1); });
