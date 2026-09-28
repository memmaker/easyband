#!/usr/bin/env python3
"""Writes the in-page game guide (dist/help.html) for the Easyband web build.

On the Mac the game content comes from the desktop key guides in
~/Desktop/Games/Roguelikes/Docs (build-docs.py + guides.py, entry
'easyband.html'), so both guides stay in sync. Until that entry exists (and
in the cloud, where the Docs folder is not reachable) the same content comes
from GAME below, which is written to be pasted into the Docs entry. The key
list is parsed from lib/help/command.txt either way."""
import html, importlib.util, os, re, sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
DOCS = os.path.expanduser('~/Desktop/Games/Roguelikes/Docs')
PAGE = 'easyband.html'
esc = html.escape


def kbd(k):
    return '<kbd>' + esc(k) + '</kbd>'


def command_keys():
    """(key, description) from lib/help/command.txt: 'Name (k)' or
    'Name (k) or Name (rk)' header lines; the second is the roguelike key."""
    out = []
    txt = open(os.path.join(ROOT, 'lib/help/command.txt'), encoding='latin-1').read()
    for line in txt.split('\n'):
        if not line or line[0].isspace() or line.startswith(('---', '===')):
            continue
        shop = ' (in the Magic shop)' if 'only in Magic Shop' in line else ''
        line = re.sub(r'\s+', ' ', line.replace('only in Magic Shop', '')).strip()
        # a first "(...)" that is part of the name: "Walk (with pickup) (;)"
        line = re.sub(r'^([^()]+?) \(([^()]{3,})\) (?=\()', r'\1, \2 ', line)
        line = re.sub(r' or ([^()]+?) \(([^()]{3,})\) (?=\()', r' or \1, \2 ', line)
        m = re.findall(r'([^()]+?) \(([^()]+)\)', line)
        if not m:
            continue
        name, k = m[0]
        k = {'left-paren': '(', 'right-paren': ')'}.get(k, k)
        out.append((k, name.strip() + shop))
        if len(m) > 1 and m[1][1] != k:
            out.append((m[1][1], m[1][0].replace('or ', '', 1).strip() + ' (roguelike keyset)'))
    return out


GAME = {
    'tagline': 'Easyband 2.3 by Andres Zanzani: a gentler, more generous '
               'Angband 2.9.3 variant with extra monsters, items and conveniences.',
    'about': '''<p><strong>Easyband</strong> is Angband made friendlier. Andres Zanzani built it on
Gwidon S. Naskrent's GSN2Band on top of <strong>Angband 2.9.3</strong>, and it keeps the classic
game: descend 100 levels of the Pits of Angband, grow from a town nobody into a hero, and kill
Morgoth, Lord of Darkness.</p>
<ul>
<li><strong>More generous:</strong> better starting gold for some characters, a faster autoroller,
better items and chests, cheaper shops (5% tax), and levels re-rolled until they are worth
exploring.</li>
<li><strong>Handy commands:</strong> <kbd>J</kbd> cures everything, <kbd>K</kbd> fully identifies an
item, <kbd>Ctrl+T</kbd> shows the time; the Magic shop identifies (<kbd>E</kbd>) and
recharges (<kbd>R</kbd>) for gold.</li>
<li><strong>New content:</strong> about 130 new monsters, the PowPlayer and Snip races, new ego
weapons (Crystalie, Holy Fury), scrolls of Brand and Bless Weapon, elven bows, halfling slings,
dwarven crossbows and new artifacts. Thieves can steal from monsters (<kbd>+</kbd>).</li>
</ul>''',
    'essentials': [
        ('Moving', [('1–9 / arrows', 'Move (Shift + direction runs)'), ('H', 'Auto-explore'),
                    ('<  >', 'Walk to the nearest known staircase and take it'),
                    ('Enter', 'Menu of all commands'), ('R', 'Rest'), ('s', 'Search')]),
        ('Items', [('i  e', 'Inventory / equipment (letter = use, Enter = actions)'),
                   ('g', 'Pick up'), ('w  t', 'Wear / take off'), ('E  q  r', 'Eat / quaff / read'),
                   ('d  k', 'Drop / destroy'), ('I', 'Inspect')]),
        ('Fighting', [('walk into it', 'Melee attack'), ('f  v', 'Fire missile / throw'),
                      ('a  u  z', 'Aim wand / use staff / zap rod'), ('m  p', 'Cast / pray'),
                      ('b', 'Browse a book')]),
        ('Knowing', [('l  x', 'Look around'), ('C', 'Character sheet'), ('Ctrl+P', 'Old messages'),
                     ('?', 'Help'), ('J  K', 'Cure all / identify fully (Easyband)'),
                     ('Ctrl+S  Ctrl+X', 'Save / save and quit')]),
    ],
    'tips': '''<ul>
<li>Wield a torch first: Easyband's starting kit keeps the torches in your pack, and without light
you see little and auto-explore refuses to run.</li>
<li>Buy <strong>Flasks of oil</strong> in the General Store: thrown (<kbd>v</kbd>) they are the best
early ranged attack. Also buy Cure Light Wounds potions and a Scroll of Word of Recall.</li>
<li><kbd>H</kbd> explores until something interesting happens; press it again after each stop.
<kbd>&gt;</kbd> walks you to the nearest known down staircase.</li>
<li>Look (<kbd>l</kbd>) at unknown monsters and read their recall before fighting. Run from
anything that is fast, breathes or comes in groups.</li>
<li><kbd>J</kbd> (cure all) and <kbd>K</kbd> (full identify) are Easyband's gifts: use them.</li>
<li>Don't dive too fast: character level about equal to dungeon level is a safe pace early on.</li>
</ul>''',
    'guide': [
        ('Making a character', '''<p>Pick a sex, race and class; the autoroller finds good stats.
Human or Dunadan Warriors are the easiest start; High-Elves see invisible; PowPlayer is
Easyband's own over-powered race for a relaxed game. Mages and Priests are fragile early
but strong later.</p>'''),
        ('The town', '''<p>The numbers on the map are shops: 1 General Store, 2 Armoury,
3 Weaponsmith, 4 Temple, 5 Alchemist, 6 Magic shop, 7 Black market, 8 Home (store items for
free). Walk onto a number to enter. Buy light, food, flasks of oil and healing potions before
going down.</p>'''),
        ('The dungeon', '''<p>Go down the <kbd>&gt;</kbd> staircase. Each level is new every time
you arrive. Explore with <kbd>H</kbd>, fight by walking into monsters, pick up what you find and
identify it by use, by <kbd>K</kbd> or in a shop. Rest (<kbd>R</kbd>, then <kbd>&amp;</kbd>) to
regain hit points before moving on.</p>'''),
        ('Staying alive', '''<p>Keep an escape: Phase Door and Word of Recall scrolls, Cure
potions. When hit points drop to half, leave the fight. Death is permanent: the save is gone
and you start again.</p>'''),
    ],
    'credits': '''<ul>
<li><strong>Easyband 2.3</strong>: Andres Zanzani (andres_zanzani@yahoo.it,
http://www.majerle.org), on <strong>GSN2Band10</strong> by Gwidon S. Naskrent.</li>
<li><strong>Angband 2.9.3</strong>: Robert Ruehlmann; Angband 2.7.0–2.8.5 Ben Harrison;
Angband 2.0–2.6.2 Alex Cutler, Andy Astrand, Sean Marsh, Geoff Hill, Charles Teague,
Charles Swiger. Based on Moria (© 1985 Robert Alan Koeneke) and Umoria (© 1989 James E.
Wilson). Licence: the Angband/Moria notice in the source files (free, not-for-profit
copying).</li>
<li><strong>Tiles</strong>: Shockbolt tileset © 2012 Raymond Gaustadnes (from Angband 4.2).</li>
<li><strong>Sound</strong>: Easyband's own samples; the gaps from the Dubtrain Angband Sound Pack
v3.1.0.</li>
<li>Web port, auto-explore, command menu and item menus: memmaker (RVIP).</li>
</ul>''',
}

# Desktop Docs entry, when it exists, wins (same fields)
if os.path.exists(os.path.join(DOCS, 'build-docs.py')):
    sys.path.insert(0, DOCS)
    spec = importlib.util.spec_from_file_location('build_docs', os.path.join(DOCS, 'build-docs.py'))
    docs = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(docs)
    from guides import GUIDES   # noqa: E402
    game = next((g for g in docs.GAMES if g['file'] == PAGE), None)
    if game and PAGE in GUIDES:
        guide = dict(GUIDES[PAGE])
        info = dict(game['info'])
        GAME = {'tagline': game['tagline'], 'essentials': game['essentials'], 'tips': info['Tips'],
                'credits': info['Credits'], 'about': guide.pop(next(iter(guide))),
                'guide': list(guide.items())}
        kbd = docs.kbd

SAVING = '''<ul>
<li><strong>Saving is automatic.</strong> Every save goes straight into this browser's storage (IndexedDB). The game saves every two minutes while it waits for your next command, and whenever you switch to another tab or window.</li>
<li><kbd>Ctrl+S</kbd> saves and keeps playing. <kbd>Ctrl+X</kbd> saves and quits; reload the page (or press <em>Play again</em>) to continue.</li>
<li>Reloading or closing the tab loses at most the last couple of minutes. The browser asks before you leave a running game.</li>
<li>Each browser keeps <strong>one character</strong>. <em>File ▾ → New game</em> deletes it and starts over. When your character dies, the page starts a new one.</li>
<li><em>Export save</em> downloads your savefile; <em>Import save</em> loads one. Use them to keep a backup or to move a character to another browser or computer.</li>
<li>Your window layout, zoom levels, window titles and the Tiles/Sound/Music buttons are stored in the same browser storage and survive a new character.</li>
<li>Private/incognito windows and "clear site data" delete the stored game. Export first if the character matters.</li>
</ul>'''

WEB = '''<ul>
<li><strong>Windows:</strong> the map fills the big window; Inventory and Visible (the monsters in view) are on the right, Messages along the bottom. Recall, Equipment and Character can be turned on under <em>Windows</em>.</li>
<li><strong>Resize windows</strong> by dragging the gaps between them. The windows always fill the screen and never overlap; the game redraws them at their new size. <em>Reset windows</em> puts everything back.</li>
<li><strong>Zoom:</strong> <em>Zoom −</em> / <em>Zoom +</em> change the size of the map. Hover over a small window's title to show its <em>A−</em> / <em>A+</em> buttons, which change its text size.</li>
<li><strong>Rename a window</strong> by clicking its title, typing a new name and pressing <kbd>Enter</kbd> (<kbd>Esc</kbd> cancels, an empty name restores the default).</li>
<li><strong>Keys:</strong> arrow keys, the numeric keypad or <kbd>1</kbd>–<kbd>9</kbd> move you; <kbd>Shift</kbd> + direction runs. Easyband has no mouse support.</li>
<li><strong>Tiles</strong> switches between Shockbolt tiles and text (at the next command). <strong>Sound</strong> and <strong>Music</strong> are off by default; Music plays a town tune while you are in town.</li>
<li>Browsers keep a few shortcuts for themselves (<kbd>Ctrl+W</kbd>, <kbd>Ctrl+T</kbd>, <kbd>Ctrl+N</kbd>, and <kbd>Cmd</kbd> shortcuts on a Mac), so those never reach the game: Easyband's time command <kbd>Ctrl+T</kbd> is in the <kbd>Enter</kbd> menu.</li>
<li>If the game ever crashes, a message appears at the top; reload the page to continue from your last save.</li>
</ul>'''

KEY_HINTS = [
    ('?', 'In-game help: every command, with explanations'),
    ('H', 'Auto-explore: walk to the nearest unexplored spot (original keyset)'),
    ('Enter', 'Menu of all commands'),
    ('i / e', 'Item menus: letter = main action, Enter = all actions'),
    ('<', 'Go up (walks to the nearest known staircase)'),
    ('>', 'Go down (walks to the nearest known staircase)'),
    ('Ctrl+S', 'Save'),
]


def dl(items):
    return '<dl>' + ''.join(f'<dt>{kbd(k)}</dt><dd>{esc(d)}</dd>' for k, d in items) + '</dl>'


def section(anchor, title, body):
    return f'<h2 id="h-{anchor}">{esc(title)}</h2>{body}'


parts = []
toc = [('about', 'About the game'), ('keys', 'Keyboard controls'), ('saving', 'Saving your game'),
       ('tips', 'Tips'), ('guide', "New player's guide"), ('web', 'Playing in the browser'), ('credits', 'Credits')]
parts.append('<p>' + esc(GAME['tagline']) + '</p><ul class="toc">' +
             ''.join(f'<li><a href="#h-{a}">{esc(t)}</a></li>' for a, t in toc) + '</ul>')
parts.append(section('about', 'About the game', GAME['about']))

ess = ''.join(f'<div class="box"><h3>{esc(cat)}</h3>{dl(items)}</div>' for cat, items in GAME['essentials'])
all_keys = command_keys() + [('H', 'Auto-explore (web port)'), ('Enter', 'Command menu (web port)')]
full = ''.join(f'<div>{kbd(k)}<span>{esc(d)}</span></div>' for k, d in all_keys)
parts.append(section('keys', 'Keyboard controls',
                     '<div class="box key"><h3>The keys to remember</h3>' + dl(KEY_HINTS) + '</div>'
                     '<h3>Essential keys</h3><div class="grid">' + ess + '</div>'
                     '<details><summary>Complete key list (' + str(len(all_keys)) + ' commands)</summary>'
                     '<div class="all">' + full + '</div></details>'))
parts.append(section('saving', 'Saving your game', SAVING))
parts.append(section('tips', 'Tips', GAME['tips']))
parts.append(section('guide', "New player's guide",
                     ''.join(f'<h3>{esc(t)}</h3>{b}' for t, b in GAME['guide'])))
parts.append(section('web', 'Playing in the browser', WEB))
parts.append(section('credits', 'Credits', GAME['credits']))

# RVIP: About this version (Source and changes)
parts.append('<h2 id="h-version">About this version</h2><ul>'
             '<li>Based on <strong>Easyband 2.3</strong> (Angband 2.9.3 / GSN2Band10).</li>'
             '<li>Original source: the Easyband 2.3 source archive (<code>easyband23_src.rar</code>), '
             'commit <code>2c3e95b</code> in the repository.</li>'
             '<li>Our changes (port, auto-explore, command menu, web build): '
             '<a href="https://github.com/memmaker/easyband/compare/2c3e95b...main" target="_blank" rel="noopener">memmaker/easyband</a></li></ul>')
print('\n'.join(parts))
