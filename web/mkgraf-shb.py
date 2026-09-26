#!/usr/bin/env python3
"""Write lib/user/graf-shb.prf: Shockbolt tiles (Angband 4.2's 64x64 set,
bundled in rvip/templates/tactical-angband) for Easyband's (Angband 2.9.3)
r_info / k_info / f_info entries and the S: slots (bolts, flavours).
Ported from the Zangband web port's generator.

Monsters and objects are matched by name against Shockbolt's 4.2 pref; an
entry without a tile of its own gets its family's tile (monsters: same symbol,
same colour if possible; objects: same tval) and is marked "# stand-in".
Features (2.9.3's fixed FEAT_* list: shops, traps, doors) and the S: slots
are mapped here by hand.  Run from the repo root: python3 web/mkgraf-shb.py"""
import re, os, unicodedata
SHB = 'rvip/templates/tactical-angband/lib/shockbolt'
GD = 'rvip/templates/tactical-angband/lib/gamedata'
ZE = 'lib/edit'
COLS = 'dwsorgbuDWvyRGBU'
CNAME = ['Dark', 'White', 'Slate', 'Orange', 'Red', 'Green', 'Blue', 'Umber', 'Light Dark',
         'Light Slate', 'Violet', 'Yellow', 'Light Red', 'Light Green', 'Light Blue', 'Light Umber']

def rd(p): return open(p, encoding='latin-1').read()
def norm(s):
    s = unicodedata.normalize('NFKD', s).encode('ascii', 'ignore').decode()
    return re.sub(r'[^a-z0-9]+', ' ', s.replace('&', '').replace('~', '').lower()).strip()

# --- Shockbolt's 4.2 pref ---
mon, obj, feat, trap, gf = {}, {}, {}, {}, {}
for l in open(f'{SHB}/graf-shb-dark.prf', encoding='utf-8').read().splitlines():
    p = l.split(':')
    if p[0] == 'monster': mon[norm(p[1])] = (p[2], p[3])
    elif p[0] == 'object': obj.setdefault(p[1], {})[norm(p[2])] = (p[3], p[4])
    elif p[0] == 'feat': feat[(p[1], p[2])] = (p[3], p[4])
    elif p[0] == 'trap': trap[(p[1], p[2])] = (p[3], p[4])
    elif p[0] == 'GF': gf[(p[1].split(' |')[0], p[2])] = (p[3], p[4])
flav = {int(m[1]): (m[2], m[3]) for m in re.finditer(r'^flavor:(\d+):(0x\w+):(0x\w+)', rd(f'{SHB}/flvr-shb.prf'), re.M)}

# 4.2 monster glyph/colour (glyph from monster_base unless overridden) -> family tiles
base = dict(re.findall(r'^name:(.*)\nglyph:(.)', rd(f'{GD}/monster_base.txt'), re.M))
fam = {}
for blk in open(f'{GD}/monster.txt', encoding='utf-8').read().split('\nname:')[1:]:
    name = blk.split('\n')[0]
    g = re.search(r'^glyph:(.)', blk, re.M)
    b = re.search(r'^base:(.*)', blk, re.M)
    c = re.search(r'^color:(.)', blk, re.M)
    g = g[1] if g else base.get(b[1]) if b else None
    if g and norm(name) in mon: fam.setdefault(g, []).append((c[1] if c else 'w', mon[norm(name)]))

# 4.2 flavours: kind -> colour letter -> first tile
fl42 = {}
kind = None
for l in rd(f'{GD}/flavor.txt').splitlines():
    p = l.split(':')
    if p[0] == 'kind': kind = p[1]
    elif p[0] in ('flavor', 'fixed') and int(p[1]) in flav:
        col = p[3] if p[0] == 'fixed' else p[2]
        if col in CNAME: fl42.setdefault(kind, {}).setdefault(COLS[CNAME.index(col)], flav[int(p[1])])

def entries(f):
    """(idx, name, glyph, colour, tval, sval) of every N: block"""
    out = []
    for n_, blk in enumerate(re.split(r'^N:', rd(f'{ZE}/{f}'), flags=re.M)[1:]):
        # Easyband r_info.txt: "N:name", numbered in file order from 0 (init1.c)
        m = re.match(r'(\d+):(.*)', blk) or re.match(r'()(.*)', blk)
        m = (None, int(m[1]) if m[1] else n_, m[2])
        g = re.search(r'^G:(.):(.)', blk, re.M)
        i = re.search(r'^I:(\d+):(\d+)', blk, re.M)
        out.append((m[1], m[2].strip(), g[1] if g else '?', g[2] if g else 'w',
                    int(i[1]) if i and f == 'k_info.txt' else 0, int(i[2]) if i and f == 'k_info.txt' else 0))
    return out

out, n = [], {'exact': 0, 'stand-in': 0, 'hand': 0}
def put(kind, idx, name, t, how):
    n[how] += 1
    out.append(f'# {name}' + (' (stand-in)' if how == 'stand-in' else ''))
    out.append(f'{kind}:{idx}:{t[0]}/{t[1]}')

# --- Monsters ---
# Zangband symbols missing from 4.2 -> a 4.2 family
GLYPH = {'Q': 'G', 'N': 'p', 'x': 'X', 'l': 'l', 'I': 'I', 'z': 'z', 'A': 'A', 'U': 'U', 'Y': 'Y',
         'y': 'y', 'm': 'm', '$': '$', '!': '$', '?': '$', '=': '$', '|': '$', '(': '$', '/': '$',
         '~': '$', '*': '*', '.': 'l', '#': 'l', '+': 'l', ',': ',', 'n': 'n', 'j': 'j'}
# Zangband names of monsters 4.2 has under another name
ALIAS = {'Novice warrior': 'Soldier', 'Novice rogue': 'Cutpurse', 'Novice priest': 'Acolyte',
         'Novice mage': 'Apprentice', 'Novice paladin': 'Warrior', 'Novice ranger': 'Archer',
         'Novice archer': 'Archer', 'Smeagol': 'Sméagol', 'Novice illusionist': 'Illusionist', 'Wolf, Farmer Maggot\'s dog': 'Fang, Farmer Maggot\'s dog'}
out.append('##### Monsters #####')
for idx, name, g, c, _, _ in entries('r_info.txt'):
    if idx == 0: continue
    t = mon.get(norm(ALIAS.get(name, name)))
    if t: put('R', idx, name, t, 'exact'); continue
    f = fam.get(g) or fam.get(GLYPH.get(g, g)) or fam['p']
    t = next((tt for cc, tt in f if cc == c), f[0][1])
    put('R', idx, name, t, 'stand-in')

# The player: warrior tile; per class/race from xtra-shb (no $GENDER in 2.9.3)
out.append('R:0:0x83/0x87')
skip = False
for l in rd(f'{SHB}/xtra-shb.prf').splitlines():
    if l.startswith('?:'):
        skip = 'GENDER' in l
        if not skip: out.append(l)
    elif l.startswith('monster:<player>') and not skip:
        p = l.split(':'); out.append(f'R:0:{p[2]}/{p[3].split()[0]}')
out.append('?:1')

# --- Objects ---
TV = {1: 'skeleton', 2: 'bottle', 3: 'junk', 5: 'spike', 7: 'chest', 8: 'figurine', 9: 'statue',
      16: 'shot', 17: 'arrow', 18: 'bolt', 19: 'bow', 20: 'digger', 21: 'hafted', 22: 'polearm',
      23: 'sword', 30: 'boots', 31: 'gloves', 32: 'helm', 33: 'crown', 34: 'shield', 35: 'cloak',
      36: 'soft armour', 37: 'hard armour', 38: 'dragon armour', 39: 'light', 40: 'amulet',
      45: 'ring', 55: 'staff', 65: 'wand', 66: 'rod', 70: 'scroll', 75: 'potion', 77: 'flask',
      80: 'food', 90: 'prayer book', 91: 'magic book', 92: 'nature book', 93: 'shadow book',
      94: 'necromantic tome', 95: 'magic book', 96: 'magic book', 100: 'gold'}
# tvals without 4.2 objects: hand-picked tiles of the set
OBJ_HAND = {'skeleton': ('0x97', '0x8E'), 'bottle': ('0x88', '0x80'), 'junk': ('0x87', '0xB6'),
            'spike': ('0x86', '0xB5'), 'figurine': ('0x92', '0x82'), 'statue': ('0x92', '0x82'),
            'staff': ('0x8A', '0x80'), 'wand': ('0x88', '0x80'), 'rod': ('0x88', '0x80'),
            'potion': ('0x88', '0x80'), 'necromantic tome': ('0x8F', '0xFD')}
FLV = {40: 'amulet', 45: 'ring', 55: 'staff', 65: 'wand', 66: 'rod', 70: 'scroll', 75: 'potion'}
out.append('##### Objects #####')
books = {}
for idx, name, g, c, tv, sv in entries('k_info.txt'):
    if idx == 0: continue
    tn = TV.get(tv)
    o = obj.get(tn, {})
    if tv == 80 and sv < 20: o = {}           # mushrooms: flavoured, S:0xF0+
    t = o.get(norm(name))
    if t: put('K', idx, name, t, 'exact'); continue
    if 90 <= tv <= 96 and tn in obj:          # books: the n-th book of the 4.2 realm
        bl = list(obj[tn].values()); put('K', idx, name, bl[min(sv, len(bl) - 1)], 'hand'); continue
    if tv in FLV or (tv == 80 and sv < 20):   # drawn from the flavour slot (S: below)
        k = FLV.get(tv, 'mushroom'); t = next(iter(fl42.get(k, {}).values()), None) or OBJ_HAND.get(tn)
        put('K', idx, name, t, 'hand'); continue
    if tn in OBJ_HAND: put('K', idx, name, OBJ_HAND[tn], 'hand'); continue
    t = next(iter(o.values()), obj['none']['unknown item'])
    put('K', idx, name, t, 'stand-in')

# --- Features: 2.9.3 f_info index (FEAT_* in defines.h) -> tile (lit
# variant; cave.c shifts the Shockbolt torch/lit/dark triplet for floors) ---
F = lambda code: feat[(code, 'lit')] if (code, 'lit') in feat else feat[(code, '*')]
T = lambda nm: trap[(nm, 'lit')]
FLOOR = F('FLOOR')
FEAT = {0: F('NONE'), 1: FLOOR, 2: FLOOR, 3: trap[('glyph of warding', 'lit')], 4: F('OPEN'),
        5: F('BROKEN'), 6: F('LESS'), 7: F('MORE'),
        8: F('STORE_GENERAL'), 9: F('STORE_ARMOR'), 10: F('STORE_WEAPON'), 11: F('STORE_BOOK'),
        12: F('STORE_ALCHEMY'), 13: F('STORE_MAGIC'), 14: F('STORE_BLACK'), 15: F('HOME'),
        # 2.9.3 trap order (spells1.c hit_trap): trap door, pits, runes, spots, darts, gases
        16: T('trap door'), 17: T('pit'), 18: T('spiked pit'), 19: T('poison pit'),
        20: T('rune of summoning'), 21: T('teleport rune'), 22: T('fire trap'), 23: T('acid trap'),
        24: T('slow dart'), 25: T('strength loss dart'), 26: T('dexterity loss dart'),
        27: T('constitution loss dart'), 28: T('blinding gas trap'), 29: T('confusion gas trap'),
        30: T('poison gas trap'), 31: T('sleeping gas trap') if ('sleeping gas trap', 'lit') in trap else T('poison gas trap'),
        48: F('GRANITE'), 49: F('RUBBLE'), 50: F('MAGMA'), 51: F('QUARTZ'), 52: F('MAGMA'),
        53: F('QUARTZ'), 54: F('MAGMA_K'), 55: F('QUARTZ_K')}
for i in range(32, 48): FEAT[i] = F('CLOSED')
for i in range(56, 60): FEAT[i] = F('GRANITE')
for i in range(60, 64): FEAT[i] = F('PERM')
out.append('##### Features #####')
for idx, name, *_ in entries('f_info.txt'):
    put('F', idx, name, FEAT[idx], 'hand')

# --- S: slots.  0x30..0x7F bolts (static | - / \ by text colour), 0x80.. flavours ---
GFC = ['DARK_WEAK', 'LIGHT_WEAK', 'SHARD', 'FIRE', 'FIRE', 'POIS', 'COLD', 'GRAVITY', 'DARK_WEAK',
       'SHARD', 'NEXUS', 'LIGHT_WEAK', 'METEOR', 'POIS', 'ELEC', 'SOUND']
out.append('##### Bolts and flavours #####')
for base, d in ((0x30, 'static'), (0x40, '90'), (0x50, '0'), (0x60, '45'), (0x70, '135')):
    for k in range(16):
        t = gf.get((GFC[k], d)) or gf[('*', d)]
        out.append(f'S:0x{base + k:02X}:{t[0]}/{t[1]}')
for base, kind in ((0x80, 'amulet'), (0x90, 'ring'), (0xA0, 'staff'), (0xB0, 'wand'), (0xC0, 'rod'),
                   (0xD0, 'scroll'), (0xE0, 'potion'), (0xF0, 'mushroom')):
    tiles = fl42[kind]
    for k in range(16):
        t = tiles.get(COLS[k]) or next(iter(tiles.values()))
        out.append(f'S:0x{base + k:02X}:{t[0]}/{t[1]}')

hdr = f"""# File: graf-shb.prf
#
# Shockbolt 64x64 tiles (Raymond Gaustadnes; Angband 4.2 lib/tiles/shockbolt)
# for Easyband (Angband 2.9.3).  Generated by web/mkgraf-shb.py -- do not edit.
# {n['exact']} entries by name, {n['hand']} mapped by hand, {n['stand-in']} family stand-ins.
"""
open('lib/user/graf-shb.prf', 'w').write(hdr + '\n'.join(out) + '\n')
print(n)
