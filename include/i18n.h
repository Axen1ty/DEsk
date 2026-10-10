#pragma once

#include <stdbool.h>

/* What language the desk's own words are in: what is printed on its
 * plates, keys, tabs and slips, and the names programs are filed under.
 * The English is the key: TR("TO DO") is "TO DO" in English and the
 * Japanese for it in Japanese. A key may carry a context before a bar,
 * "clock|DAY", for a word that is two words in another language; what is
 * after the bar is the English. A word with no translation is left in
 * English. */
enum desk_lang {
	DESK_LANG_EN, DESK_LANG_JA, DESK_LANG_ZH, DESK_LANG_KO, DESK_LANG_ES, DESK_LANG_FR,
	DESK_LANG_DE, DESK_LANG_COUNT
};

/* shell.conf's "language = auto | en | ja | zh | ko | es | fr | de"; auto
 * (or anything else) follows the locale. */
enum desk_lang desk_lang_parse(const char *s);
/* LC_ALL, LC_MESSAGES, LANG: ja... is Japanese, zh... Chinese and so on,
 * anything else English */
enum desk_lang desk_lang_from_locale(void);

void desk_lang_set(enum desk_lang l);
enum desk_lang desk_lang_get(void);
/* Chinese, Japanese or Korean: written in wide characters, with no spaces
 * between words */
bool desk_lang_wide(enum desk_lang l);
/* "en", "ja", "zh", ... */
const char *desk_lang_code(enum desk_lang l);

/* A month's name (m 0..11) in the desk's language, as a calendar prints
 * it -- whole, "OCTOBER", "10月"; short, "OCT" -- and a day of the week's
 * (wd 0..6, Sunday first): whole, "THURSDAY", "木曜日"; short, "THU";
 * a letter, "T", "木". A month's letter is its short name. */
enum desk_name { DESK_NAME_LETTER, DESK_NAME_SHORT, DESK_NAME_WHOLE };
const char *desk_month_name(int m, enum desk_name form);
const char *desk_day_name(int wd, enum desk_name form);

const char *desk_tr(const char *key);
#define TR(s) desk_tr(s)
