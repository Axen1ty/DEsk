#!/usr/bin/env python3
"""Wrap a GLSL file as a C string, so the shader ships inside the binary.

    glsl2c.py NAME in.glsl out.h

Run by meson with the Python meson itself runs on, so the build needs
nothing it did not already have. The shader is kept as a .glsl file rather
than as a C string because it is two thousand lines of code that a person
has to read against draw.c, and quotes on every line do not help with that.
"""

import os
import sys


def main():
    name, src, dst = sys.argv[1:4]
    with open(src, encoding="ascii") as f:
        text = f.read()
    out = [
        "/* generated from %s by glsl2c.py -- edit that, not this */"
        % os.path.basename(src),
        "static const char %s[] =" % name,
    ]
    for line in text.splitlines():
        # backslash and quote for C; '?' because -std=c11 still reads
        # trigraphs, and a "??/" in a comment would eat the next character
        esc = line.replace("\\", "\\\\").replace('"', '\\"').replace("?", "\\?")
        out.append('\t"%s\\n"' % esc)
    out.append("\t;")
    with open(dst, "w", encoding="ascii") as f:
        f.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
