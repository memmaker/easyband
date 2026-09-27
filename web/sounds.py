#!/usr/bin/env python3
"""Write the web build's sound.cfg and copy the used .wavs.
Easyband's own samples (lib/xtra/sound, from upstream) come first; events
its sound.cfg leaves empty or names a missing file for are filled from the
Dubtrain Angband Sound Pack v3.1.0 (~/Downloads, as Zangband; same event
names). 'walk' stays silent (every step).
Usage: sounds.py <sound.cfg to write> <wav dir>"""
import os, shutil, sys
ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
OWN = os.path.join(ROOT, 'lib/xtra/sound')
PACK = os.path.expanduser('~/Downloads/Dubtrain Angband Sound Pack v3.1.0')
# Easyband event -> Dubtrain event, when the names differ
MAP = {'zap': 'zap_rod', 'stairs': 'stairs_down', 'walk': ''}
EVENTS = open(os.path.join(ROOT, 'SRC/variable.c'), encoding='latin-1').read()
EVENTS = EVENTS.split('angband_sound_name[SOUND_MAX][16] =')[1].split('};')[0]
EVENTS = [e for e in EVENTS.replace('"', ' ').replace(',', ' ').split() if e.isidentifier()]

def cfg(d):
    r = {}
    for line in open(os.path.join(d, 'sound.cfg'), encoding='latin-1'):
        if '=' in line and not line.lstrip().startswith('#'):
            k, v = line.split('=', 1)
            r[k.strip()] = [f for f in v.split() if os.path.exists(os.path.join(d, f))]
    return r

own, pack = cfg(OWN), cfg(PACK)
cfg_path, out = sys.argv[1], sys.argv[2]
os.makedirs(out, exist_ok=True)
lines = ['# Easyband web build: own samples, gaps from Dubtrain v3.1.0 (web/sounds.py)', '[Sound]']
n_own = n_pack = 0
for e in EVENTS:
    files = own.get(e, [])
    src = OWN
    if files: n_own += 1
    elif e != 'walk':
        files, src = sorted({f for d in MAP.get(e, e).split() for f in pack.get(d, [])}), PACK
        n_pack += 1
        # DASP names lie: its 'shoot' has the melee swish; firing gets the arrow
        # samples only, and a melee miss (own samples come first) the swish
        if e == 'shoot': files = [f for f in files if f != 'plc_miss_swish.wav']
        if e == 'miss': files = ['plc_miss_swish.wav']
    assert files or e == 'walk', 'no sample for ' + e
    for f in files:
        shutil.copy(os.path.join(src, f), out)
    lines.append(f'{e} = {" ".join(files)}')
open(cfg_path, 'w').write('\n'.join(lines) + '\n')
print(f'sounds.py: {len(EVENTS)} events, {n_own} own, {n_pack} Dubtrain', file=sys.stderr)
