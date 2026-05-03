/*!
    \file    themes/theme_style.c
    \brief   key=value 文本 parser — 不依赖 stdlib 的 strtok，自己 walk
*/

#include "theme_style.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static void trim(const char ** s, const char ** e)
{
    while (*s < *e && (**s == ' ' || **s == '\t')) (*s)++;
    while (*e > *s && (((*e)[-1]) == ' ' || ((*e)[-1]) == '\t' || ((*e)[-1]) == '\r')) (*e)--;
}

static int key_eq(const char * ks, const char * ke, const char * lit)
{
    size_t klen = (size_t)(ke - ks);
    return klen == strlen(lit) && memcmp(ks, lit, klen) == 0;
}

static uint32_t parse_hex(const char * s, const char * e)
{
    char buf[12]; size_t n = (size_t)(e - s);
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;
    memcpy(buf, s, n); buf[n] = 0;
    return (uint32_t)strtoul(buf, NULL, 0);
}

static void copy_str(char * dst, size_t dst_sz, const char * s, const char * e)
{
    size_t n = (size_t)(e - s);
    if (n >= dst_sz) n = dst_sz - 1;
    memcpy(dst, s, n); dst[n] = 0;
}

int theme_style_parse(theme_style_t * out, const char * text, size_t len)
{
    const char * p = text;
    const char * end = text + len;

    while (p < end) {
        /* find line end */
        const char * line_end = p;
        while (line_end < end && *line_end != '\n') line_end++;

        const char * ls = p;
        const char * le = line_end;
        trim(&ls, &le);

        if (ls < le && *ls != '#') {
            /* split on '=' */
            const char * eq = ls;
            while (eq < le && *eq != '=') eq++;
            if (eq < le) {
                const char * ks = ls,    * ke = eq;
                const char * vs = eq + 1, * ve = le;
                trim(&ks, &ke);
                trim(&vs, &ve);

                if      (key_eq(ks, ke, "bg"))           out->bg           = parse_hex(vs, ve);
                else if (key_eq(ks, ke, "accent"))       out->accent       = parse_hex(vs, ve);
                else if (key_eq(ks, ke, "fg_main"))      out->fg_main      = parse_hex(vs, ve);
                else if (key_eq(ks, ke, "fg_secondary")) out->fg_secondary = parse_hex(vs, ve);
                else if (key_eq(ks, ke, "fg_muted"))     out->fg_muted     = parse_hex(vs, ve);
                else if (key_eq(ks, ke, "banner"))       copy_str(out->banner,   sizeof(out->banner),   vs, ve);
                else if (key_eq(ks, ke, "sub_text"))     copy_str(out->sub_text, sizeof(out->sub_text), vs, ve);
                /* unknown keys silently ignored */
            }
        }

        p = (line_end < end) ? line_end + 1 : line_end;
    }
    return 0;
}
