/* The faces DEsk sets its text in (fonts.h).
 *
 * The toy font API cairo has takes one family and gives every letter of a
 * string from that one face: there is no falling back to another face for
 * a letter it has not got. "sans-serif" under a Japanese locale is a
 * gothic whose Latin letters are all one width, as on a typewriter, and a
 * Latin face asked for Japanese draws boxes. So each role has two lists,
 * and a string is set in the Latin face only if that face has every one of
 * its letters; anything else -- Japanese, an arrow it lacks -- goes to the
 * second, which is chosen for having nearly everything.
 *
 * Chinese, Japanese and Korean share most of their characters but print
 * them differently, so the wide faces are chosen again for each: Noto's
 * and Source Han's own cut for the region, and the faces only one of them
 * has (a Japanese hand, a Korean gothic) only for that one. A Latin face
 * made in Japan (Klee One) has the Japanese shapes too: for Chinese and
 * Korean it sets only what has no such characters in it. */

#include "fonts.h"

#include <fontconfig/fontconfig.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

struct role {
	const char *name;
	const char *const *latin;
	const char *const *wide;
};

/* Each list in order of preference, ending in a generic name fontconfig
 * always answers. The URW faces (fonts-urw-base35: Palatino, Helvetica,
 * Courier) and Noto CJK (fonts-noto-cjk) are what DEsk is drawn with;
 * the rest are near enough. */
#define WIDE_SANS "Noto Sans CJK JP", "Source Han Sans JP", "Noto Sans CJK SC", \
	"IPAexGothic", "IPAPGothic", "VL PGothic", "WenQuanYi Zen Hei", "WenQuanYi Micro Hei", \
	"NanumGothic", "UnDotum", "Droid Sans Fallback", "sans-serif"
#define WIDE_SERIF "Noto Serif CJK JP", "Source Han Serif JP", "Noto Serif CJK SC", \
	"IPAexMincho", "IPAPMincho", "AR PL UMing CN", "NanumMyeongjo", "UnBatang", \
	"Noto Sans CJK JP", "serif"

static const char *const UI_L[] = { "Noto Sans CJK JP", "Source Han Sans JP", "Noto Sans",
	"Source Sans 3", "Lato", "DejaVu Sans", "sans-serif", NULL };
static const char *const UI_W[] = { WIDE_SANS, NULL };
static const char *const PRINT_L[] = { "Noto Serif CJK JP", "Source Han Serif JP",
	"Noto Serif", "Source Serif 4", "C059", "DejaVu Serif", "serif", NULL };
static const char *const PRINT_W[] = { WIDE_SERIF, NULL };
static const char *const DISPLAY_L[] = { "P052", "TeX Gyre Pagella", "Palatino Linotype",
	"URW Palladio L", "C059", "Noto Serif", "DejaVu Serif", "serif", NULL };
static const char *const ENGRAVE_L[] = { "Nimbus Sans", "TeX Gyre Heros", "Liberation Sans",
	"Arimo", "Noto Sans", "DejaVu Sans", "sans-serif", NULL };
static const char *const TYPE_L[] = { "Nimbus Mono PS", "Courier Prime", "TeX Gyre Cursor",
	"Liberation Mono", "Noto Sans Mono", "DejaVu Sans Mono", "monospace", NULL };
static const char *const TYPE_W[] = { "Noto Sans Mono CJK JP", WIDE_SANS, NULL };
static const char *const SCREEN_L[] = { "DejaVu Sans Mono", "Liberation Mono", "Noto Sans Mono",
	"monospace", NULL };
static const char *const HAND_L[] = { "Klee One", "Caveat", "Kalam", "Patrick Hand",
	"Noto Serif CJK JP", "Source Han Serif JP", "Noto Serif", "serif", NULL };
static const char *const HAND_W[] = { "Klee One", "Zen Kurenaido", "Yomogi", "LXGW WenKai",
	"Nanum Pen Script", "Gaegu", WIDE_SERIF, NULL };

static const struct role ROLES[DESK_FONT_COUNT] = {
	[DESK_FONT_UI] = { "ui", UI_L, UI_W },
	[DESK_FONT_PRINT] = { "print", PRINT_L, PRINT_W },
	[DESK_FONT_DISPLAY] = { "display", DISPLAY_L, PRINT_W },
	[DESK_FONT_ENGRAVE] = { "engrave", ENGRAVE_L, UI_W },
	[DESK_FONT_TYPE] = { "type", TYPE_L, TYPE_W },
	[DESK_FONT_SCREEN] = { "screen", SCREEN_L, TYPE_W },
	[DESK_FONT_HAND] = { "hand", HAND_L, HAND_W },
};

/* the regions the wide faces are cut for */
enum { REG_JP, REG_SC, REG_KR, REGIONS };

/* faces made for one region's way of writing, and left out of the others'
 * wide lists */
static const struct {
	const char *face;
	int region;
} ONLY_FOR[] = {
	{ "IPAexGothic", REG_JP }, { "IPAPGothic", REG_JP }, { "VL PGothic", REG_JP },
	{ "IPAexMincho", REG_JP }, { "IPAPMincho", REG_JP }, { "Klee One", REG_JP },
	{ "Zen Kurenaido", REG_JP }, { "Yomogi", REG_JP },
	{ "WenQuanYi Zen Hei", REG_SC }, { "WenQuanYi Micro Hei", REG_SC },
	{ "AR PL UMing CN", REG_SC }, { "LXGW WenKai", REG_SC },
	{ "NanumGothic", REG_KR }, { "UnDotum", REG_KR }, { "NanumMyeongjo", REG_KR },
	{ "UnBatang", REG_KR }, { "Nanum Pen Script", REG_KR }, { "Gaegu", REG_KR },
};

/* a family's Japanese cut, and the same in each region's */
static const char *const CUTS[][REGIONS] = {
	{ "Noto Sans CJK JP", "Noto Sans CJK SC", "Noto Sans CJK KR" },
	{ "Noto Serif CJK JP", "Noto Serif CJK SC", "Noto Serif CJK KR" },
	{ "Noto Sans Mono CJK JP", "Noto Sans Mono CJK SC", "Noto Sans Mono CJK KR" },
	{ "Source Han Sans JP", "Source Han Sans SC", "Source Han Sans K" },
	{ "Source Han Serif JP", "Source Han Serif SC", "Source Han Serif K" },
};

struct chosen {
	const char *latin, *wide;
	FcCharSet *cs;          /* the letters the Latin face has */
};
static struct chosen g_chosen[REGIONS][DESK_FONT_COUNT];
static atomic_bool g_ready[REGIONS];
static pthread_mutex_t g_choosing = PTHREAD_MUTEX_INITIALIZER;
/* the region of the desk's language: Japanese's for any that is not
 * Chinese or Korean */
static atomic_int g_region = REG_JP;
/* text whose words are not known when its face is chosen: in the desk's
 * language, which may be one the Latin faces have no letters for */
static bool g_wide_default;

void desk_font_set_wide_default(bool wide)
{
	g_wide_default = wide;
}

void desk_font_set_region(enum desk_lang lang)
{
	atomic_store(&g_region, lang == DESK_LANG_ZH ? REG_SC : lang == DESK_LANG_KO ? REG_KR
		: REG_JP);
}

/* A family as it is for region r: its cut for there, or (in a wide list)
 * NULL if it is not for there at all. */
static const char *regional(const char *f, int r, bool wide)
{
	for (size_t i = 0; wide && i < sizeof ONLY_FOR / sizeof ONLY_FOR[0]; i++) {
		if (!strcmp(f, ONLY_FOR[i].face)) {
			return ONLY_FOR[i].region == r ? f : NULL;
		}
	}
	for (size_t i = 0; i < sizeof CUTS / sizeof CUTS[0]; i++) {
		if (!strcmp(f, CUTS[i][REG_JP])) {
			return CUTS[i][r];
		}
	}
	return f;
}

static bool generic(const char *f)
{
	return strcmp(f, "sans-serif") == 0 || strcmp(f, "serif") == 0
		|| strcmp(f, "monospace") == 0;
}

/* Is a face of this family installed? Not "what would fontconfig give for
 * it", which is always something. */
static bool installed(const char *family)
{
	FcPattern *p = FcPatternCreate();
	FcPatternAddString(p, FC_FAMILY, (const FcChar8 *)family);
	FcObjectSet *os = FcObjectSetBuild(FC_FAMILY, (char *)NULL);
	FcFontSet *fs = FcFontList(NULL, p, os);
	bool ok = fs && fs->nfont > 0;
	if (fs) {
		FcFontSetDestroy(fs);
	}
	FcObjectSetDestroy(os);
	FcPatternDestroy(p);
	return ok;
}

static const char *first_installed(const char *const *list, int r, bool wide)
{
	for (int i = 0; list[i]; i++) {
		const char *f = regional(list[i], r, wide);
		if (f && (generic(f) || installed(f))) {
			return f;
		}
	}
	return "sans-serif";
}

static FcCharSet *charset_of(const char *family)
{
	FcPattern *p = FcPatternCreate();
	FcPatternAddString(p, FC_FAMILY, (const FcChar8 *)family);
	FcConfigSubstitute(NULL, p, FcMatchPattern);
	FcDefaultSubstitute(p);
	FcResult r;
	FcPattern *m = FcFontMatch(NULL, p, &r);
	FcCharSet *cs = NULL, *got = NULL;
	if (m && FcPatternGetCharSet(m, FC_CHARSET, 0, &got) == FcResultMatch && got) {
		cs = FcCharSetCopy(got);
	}
	if (m) {
		FcPatternDestroy(m);
	}
	FcPatternDestroy(p);
	return cs;
}

/* The desk's region's faces, chosen the first time they are wanted. */
static const struct chosen *chosen(int r)
{
	if (!atomic_load(&g_ready[r])) {
		pthread_mutex_lock(&g_choosing);
		if (!atomic_load(&g_ready[r])) {
			FcInit();
			struct chosen *c = g_chosen[r];
			for (int i = 0; i < DESK_FONT_COUNT; i++) {
				c[i].latin = first_installed(ROLES[i].latin, r, false);
				c[i].wide = first_installed(ROLES[i].wide, r, true);
				c[i].cs = strcmp(c[i].latin, c[i].wide) == 0 ? NULL
					: charset_of(c[i].latin);
			}
			atomic_store(&g_ready[r], true);
		}
		pthread_mutex_unlock(&g_choosing);
	}
	return g_chosen[r];
}

/* the next letter of s, and where the one after it starts */
static unsigned next_letter(const unsigned char **s)
{
	const unsigned char *p = *s;
	unsigned c = p[0];
	int n = c < 0x80 ? 0 : c < 0xe0 ? 1 : c < 0xf0 ? 2 : 3;
	if (n) {
		c &= 0x3f >> n;
	}
	p++;
	for (int i = 0; i < n && (*p & 0xc0) == 0x80; i++, p++) {
		c = (c << 6) | (*p & 0x3f);
	}
	*s = p;
	return c;
}

const char *desk_font_family(enum desk_font f, const char *text)
{
	int r = atomic_load(&g_region);
	const struct chosen *ch = chosen(r);
	if (f < 0 || f >= DESK_FONT_COUNT) {
		f = DESK_FONT_UI;
	}
	if (!text) {
		return g_wide_default ? ch[f].wide : ch[f].latin;
	}
	if (!ch[f].cs) {
		return ch[f].latin;
	}
	for (const unsigned char *p = (const unsigned char *)text; *p;) {
		unsigned c = next_letter(&p);
		/* (from U+2E80 on: the CJK radicals, kana, hangul, ideographs) */
		if (c >= 0x20 && (!FcCharSetHasChar(ch[f].cs, c) || (r != REG_JP && c >= 0x2e80))) {
			return ch[f].wide;
		}
	}
	return ch[f].latin;
}

void desk_font_select(cairo_t *cr, enum desk_font f, const char *text, bool bold,
	bool italic)
{
	cairo_select_font_face(cr, desk_font_family(f, text),
		italic ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL,
		bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
}

void desk_font_describe(char *buf, size_t n)
{
	const struct chosen *ch = chosen(atomic_load(&g_region));
	size_t o = 0;
	buf[0] = '\0';
	for (int i = 0; i < DESK_FONT_COUNT && o < n; i++) {
		int k = snprintf(buf + o, n - o, "%s%s=%s/%s", i ? ", " : "", ROLES[i].name,
			ch[i].latin, ch[i].wide);
		if (k < 0) {
			break;
		}
		o += (size_t)k;
	}
}
