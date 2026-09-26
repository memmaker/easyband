# Shrine manual: Easyband lib/help/*.txt/*.hlp/*.spo (Angband 2.9.3 plain text) -> one HTML page.
# python3 web/mkmanual.py > ~/Games/roguelikes-index/shrine/easyband/manual.html
import os, re, html
H = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'lib', 'help')
def read(f): return open(f'{H}/{f}', encoding='latin-1').read().replace('\r\n', '\n')
order, seen = [], set()
def visit(f):
    if f in seen or not os.path.exists(f'{H}/{f}'): return
    seen.add(f); order.append(f)
    for m in re.finditer(r'^\*\*\*\*\* \[.\] (\S+)', read(f), re.M): visit(m.group(1))
visit('help.hlp')
def fid(f): return f.replace('.', '-')
def body(f):
    lines = [l for l in read(f).split('\n') if not l.startswith('***** ')]  # menu targets: hidden in the game too
    t = html.escape('\n'.join(lines).rstrip())
    return re.sub(r'\(([a-z-]+\.(?:txt|hlp|spo))\)', lambda m: f'(<a href="#{fid(m.group(1))}">{m.group(1)}</a>)' if m.group(1) in seen else m.group(0), t)
print('''<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>Easyband · Manual</title>
<style>body{background:#0b0a09;color:#d8d2c4;font:14px/1.45 "IBM Plex Mono",monospace;margin:0;padding:16px}
a{color:#e0b060}pre{white-space:pre-wrap;overflow-wrap:anywhere;margin:0 0 2em}h2{color:#e0b060;font-size:16px;border-bottom:1px solid #3a342a;padding-top:8px}
nav a{margin-right:1em;white-space:nowrap}</style></head><body>
<p><a href="../easyband.html">&larr; Easyband shrine</a> · In-game help of Easyband 2.3 (<kbd>?</kbd>), <code>lib/help/</code>, unchanged text.</p>''')
print('<nav>' + ' '.join(f'<a href="#{fid(f)}">{f}</a>' for f in order) + '</nav>')
for f in order: print(f'<h2 id="{fid(f)}">{f}</h2>\n<pre>{body(f)}</pre>')
print('</body></html>')
