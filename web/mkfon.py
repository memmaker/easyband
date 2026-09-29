"""Windows .fon bitmap fonts (lib/xtra/font) -> pixel-exact .woff for the map.

Each set pixel becomes a square of U units, UPM = height * U, so the font is
crisp at font-size = n * height px. Needs fontTools (pip install fonttools).
Usage: python3 web/mkfon.py lib/xtra/font/*.fon  -> web/fonts/Easyband_<name>.woff (then move them to roguelikes-index/fonts/, the shared list is in rvip-wm.js)
"""
import os, struct, sys
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen

U = 64

def fnt(data):
    """The first RT_FONT (0x8008) resource of an NE file."""
    ne = struct.unpack_from('<H', data, 0x3C)[0]
    rt = ne + struct.unpack_from('<H', data, ne + 0x24)[0]
    shift = struct.unpack_from('<H', data, rt)[0]
    p = rt + 2
    while True:
        typ, cnt = struct.unpack_from('<HH', data, p)
        if typ == 0:
            raise ValueError('no font resource')
        p += 8
        if typ == 0x8008:
            off, ln = struct.unpack_from('<HH', data, p)
            return data[off << shift:(off << shift) + (ln << shift)]
        p += 12 * cnt

def glyphs(f):
    ver, = struct.unpack_from('<H', f, 0)
    asc, = struct.unpack_from('<H', f, 74)
    charset = f[85]
    h, = struct.unpack_from('<H', f, 88)
    first, last = f[95], f[96]
    tab, ent = (118, 4) if ver == 0x200 else (148, 6)
    out = {}
    for i in range(last - first + 1):
        w, = struct.unpack_from('<H', f, tab + i * ent)
        off = struct.unpack_from('<H' if ent == 4 else '<I', f, tab + i * ent + 2)[0]
        rows = []
        for y in range(h):
            bits = 0
            for c in range((w + 7) // 8):
                bits = (bits << 8) | f[off + c * h + y]
            rows.append([(bits >> ((w + 7) // 8 * 8 - 1 - x)) & 1 for x in range(w)])
        out[first + i] = (w, rows)
    return out, h, asc, charset

def build(path, dst):
    name = os.path.splitext(os.path.basename(path))[0]
    gl, h, asc, charset = glyphs(fnt(open(path, 'rb').read()))
    enc = 'cp437' if charset == 255 else 'cp1252'
    order, cmap, metrics, shapes = ['.notdef'], {}, {'.notdef': (0, 0)}, {}
    for code, (w, rows) in sorted(gl.items()):
        # X11 fixed-font DEC graphics as the page draws them: 1 diamond, 2 wall
        uni = {1: 0x25C6, 2: 0x2592}.get(code) or (
            code >= 32 and code != 127 and ord(bytes([code]).decode(enc, 'replace')))
        if not uni:
            continue
        if uni == 0xFFFD or uni in cmap:
            continue
        g = 'g%d' % code
        pen = TTGlyphPen(None)
        for y, row in enumerate(rows):
            x = 0
            while x < w:
                if row[x]:
                    x0 = x
                    while x < w and row[x]:
                        x += 1
                    t, b = (asc - y) * U, (asc - y - 1) * U
                    pen.moveTo((x0 * U, b)); pen.lineTo((x0 * U, t))
                    pen.lineTo((x * U, t)); pen.lineTo((x * U, b)); pen.closePath()
                x += 1
        order.append(g); cmap[uni] = g; shapes[g] = pen.glyph(); metrics[g] = (w * U, 0)
    shapes['.notdef'] = TTGlyphPen(None).glyph()
    fam = 'Easyband_' + name
    fb = FontBuilder(h * U, isTTF=True)
    fb.setupGlyphOrder(order)
    fb.setupCharacterMap(cmap)
    fb.setupGlyf(shapes)
    fb.setupHorizontalMetrics(metrics)
    fb.setupHorizontalHeader(ascent=asc * U, descent=-(h - asc) * U)
    fb.setupNameTable({'familyName': fam, 'styleName': 'Regular'})
    fb.setupOS2(sTypoAscender=asc * U, sTypoDescender=-(h - asc) * U, sTypoLineGap=0,
                usWinAscent=asc * U, usWinDescent=(h - asc) * U, fsSelection=0x40)
    fb.setupPost(isFixedPitch=1)
    fb.font.flavor = 'woff'
    fb.save(os.path.join(dst, fam + '.woff'))
    print(fam, 'cells', max(w for w, _ in gl.values()), 'x', h, enc, len(order) - 1, 'glyphs')

if __name__ == '__main__':
    for p in sys.argv[1:]:
        build(p, os.path.join(os.path.dirname(os.path.abspath(__file__)), 'fonts'))
