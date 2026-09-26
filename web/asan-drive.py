#!/usr/bin/env python3
"""Native ASan run with random keys (RVIP A1 / A-Zangband).

Build (outside the repo, objects never committed):
  cp -r SRC lib $W/ && cd $W/SRC && gcc -O1 -g -fsanitize=address -fcommon \
    -DUSE_GCU -w -o ../angband $(ls *.c | grep -v '^main\|^maid\|Readdib') main.c main-gcu.c -lncurses
Run:  python3 web/asan-drive.py $W/angband SEED NKEYS [weights]
Plays NKEYS random keys on a fresh or restored character (-uTest), sends
Ctrl-X (save+quit) at the end, prints the last screen and any ASan report
(ASAN_OPTIONS log_path=$W/asan)."""
import os, pty, sys, time, random, select, signal
import pyte
exe, seed, n = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
extra = sys.argv[4] if len(sys.argv) > 4 else ''
W = os.path.dirname(os.path.abspath(exe))
random.seed(seed)
# no ^Z ^C ^\\ ^Y ^S(XOFF) ^Q ^O ^V ^X(save+quit) and no Q (suicide/quit at birth): they end the run or stop the tty
ctrl = [chr(c) for c in range(1, 27) if chr(c + 64) not in 'ZCYSQOVX\\']
keys = list('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPRTUVWYZ0123456789 ,.<>;:?!@#$%^&*()-_=+[]{}/\'"~`') \
    + ['\r'] * 8 + ['\x1b'] * 8 + ctrl + list('12346789') * 4 + list(extra) * 6
env = dict(os.environ, HOME=W, TERM='xterm', LINES='24', COLUMNS='80',
           ASAN_OPTIONS=f'log_path={W}/asan:detect_leaks=0:abort_on_error=0')
pid, fd = pty.fork()
if pid == 0:
    os.chdir(W)
    os.execve(exe, [exe, '-uTest', '-mgcu'], env)
scr = pyte.Screen(80, 24); st = pyte.ByteStream(scr)
def pump(t):
    end = time.time() + t
    while True:
        r, _, _ = select.select([fd], [], [], max(0, end - time.time()))
        if not r: return True
        try: d = os.read(fd, 65536)
        except OSError: return False
        if not d: return False
        st.feed(d)
def dead():
    try: return os.waitpid(pid, os.WNOHANG)[0] != 0
    except ChildProcessError: return True
pump(1.0)
alive = True
for i in range(n):
    os.write(fd, random.choice(keys).encode('latin-1'))
    if not pump(0.004) or dead(): alive = False; print('game exited at key', i); break
if alive:
    for k in ['\x1b'] * 6 + ['\x18', '\r', '\x1b', '\r']:
        os.write(fd, k.encode()); pump(0.3)
    time.sleep(1)
    if not dead(): os.kill(pid, signal.SIGKILL); print('killed')
print('\n'.join(l.rstrip() for l in scr.display))
