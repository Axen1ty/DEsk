#!/usr/bin/env python3
"""check-words.py SOURCE_DIR -- the desk's word tables, checked (meson test).

src/i18n.c and src/index.c keep every word the desk prints in a table with a
column for each language (include/i18n.h, enum desk_lang): the English, which
is the key, then the same in each language, or NULL where there is none yet.
A translation that is printed with printf must take what the English takes,
in the same order -- %d where %d, %s where %s -- or it reads the wrong thing
off the stack. This reads the tables as C does and says, for each row that
is wrong, what is wrong with it:

  - not one column for each language
  - a key twice
  - a translation whose printf conversions are not the English one's
  - a translation with a control character in it other than a new line

It also counts, as information only, the TR("...") keys the sources print
that have no row (they are printed in English, which may be meant).
Exit status 1 if anything is wrong."""
import glob
import os
import re
import sys

LANGS = ('en', 'ja', 'zh', 'ko', 'es', 'fr', 'de')
FMT = re.compile(r'%[-+ #0]*\d*(?:\.\d+)?[hlLqjzt]*[diouxXeEfgGcsp%]')
TOKEN = re.compile(r'"((?:[^"\\]|\\.)*)"|(/\*.*?\*/)|([{},])|(NULL)|(\s+)', re.S)


def unescape(body):
    out = bytearray()
    raw = body.encode('utf-8')
    i = 0
    while i < len(raw):
        c = raw[i]
        if c == 0x5c:
            n = chr(raw[i + 1])
            if n == 'x':
                j = i + 2
                while j < len(raw) and chr(raw[j]) in '0123456789abcdefABCDEF':
                    j += 1
                out.append(int(raw[i + 2:j], 16))
                i = j
                continue
            out += {'n': b'\n', 't': b'\t', '"': b'"', '\\': b'\\', "'": b"'"}[n]
            i += 2
            continue
        out.append(c)
        i += 1
    return out.decode('utf-8')


def table(path):
    """The rows of the WORDS table in path: (line, [strings or None])."""
    src = open(path, encoding='utf-8').read()
    m = re.search(r'static const char \*const WORDS\[\]\[DESK_LANG_COUNT\] = \{\n', src)
    if not m:
        sys.exit(f'{path}: no WORDS table')
    end = src.index('\n};\n', m.end())
    body = src[m.end():end]
    base = src[:m.end()].count('\n') + 1
    rows, cur, lit, pos = [], None, None, 0
    row_line = 0
    while pos < len(body):
        t = TOKEN.match(body, pos)
        if not t:
            sys.exit(f'{path}: cannot read the table near line '
                     f'{base + body[:pos].count(chr(10))}')
        s, com, punct, null, _ = t.groups()
        if s is not None:
            lit = (lit or '') + unescape(s)
        elif null:
            cur.append(None)
        elif punct == '{':
            cur = []
            row_line = base + body[:pos].count('\n')
        elif punct is not None and punct in ',}':
            if lit is not None:
                cur.append(lit)
                lit = None
            if punct == '}':
                rows.append((row_line, cur))
                cur = None
        pos = t.end()
    return rows


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(
        os.path.abspath(__file__)))
    bad = 0
    keys = set()
    for name in ('src/i18n.c', 'src/index.c'):
        path = os.path.join(root, name)
        seen = {}
        rows = table(path)
        for line, row in rows:
            where = f'{name}:{line}'
            if len(row) != len(LANGS):
                print(f'{where}: {len(row)} columns, not {len(LANGS)}: {row[:1]}')
                bad += 1
                continue
            key = row[0]
            if key is None:
                print(f'{where}: no English')
                bad += 1
                continue
            if key in seen:
                print(f'{where}: "{key}" is in the table already (line {seen[key]})')
                bad += 1
            seen[key] = line
            want = FMT.findall(key.split('|', 1)[-1])
            for lang, text in zip(LANGS[1:], row[1:]):
                if text is None:
                    continue
                got = FMT.findall(text)
                if [g for g in got if g != '%%'] != [w for w in want if w != '%%'] \
                        or got.count('%%') != want.count('%%'):
                    print(f'{where}: {lang} takes {got}, the English {want}: "{key}"')
                    bad += 1
                if any(ord(ch) < 0x20 and ch != '\n' for ch in text):
                    print(f'{where}: {lang} has a control character in it: "{key}"')
                    bad += 1
        keys |= set(seen)
        print(f'{name}: {len(rows)} rows')
    # (information) what is printed and has no row
    used = set()
    for path in glob.glob(os.path.join(root, 'src', '*.c')):
        for m in re.finditer(r'\b(?:TR|W)\(\s*((?:"(?:[^"\\]|\\.)*"\s*)+)\)',
                             open(path, encoding='utf-8').read()):
            used.add(''.join(unescape(p) for p in re.findall(r'"((?:[^"\\]|\\.)*)"',
                                                            m.group(1))))
    missing = sorted(k for k in used - keys if re.search('[A-Za-z]', k))
    print(f'{len(missing)} printed keys with no row (shown in English)')
    for k in missing:
        print(f'  {k!r}')
    if bad:
        print(f'{bad} wrong')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
