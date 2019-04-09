/*
 * Copyright (C) 2003 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* The interfaces in this file are subject to change at any time. */

#include "vice.h"

#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <glib.h>
#include "buffer.h"
#include "vteconv.h"
#include "vtedefines.hh"

typedef size_t (*convert_func)(GIConv converter,
                                const guchar **inbuf,
                                gsize *inbytes_left,
                                guchar **outbuf,
                                gsize *outbytes_left);
struct _VteConv {
    GIConv conv;
    convert_func convert;
    gint (*close)(GIConv converter);
    gboolean in_unichar, out_unichar;
    VteByteArray *in_scratch, *out_scratch;
};

/* We can't use g_utf8_strlen as that's not nul-safe :( */
static gsize _vte_conv_utf8_strlen(const gchar *p, gssize max)
{
    const gchar *q = p + max;
    gsize length = 0;
    while (p < q) {
        p = g_utf8_next_char(p);
        length++;
    }
    return length;
}

/* A variant of g_utf8_validate() that allows NUL characters.
 * Requires that max_len >= 0 && end != NULL. */
static gboolean _vte_conv_utf8_validate(const gchar *str,
                                        gssize max_len,
                                        const gchar **end)
{
    gboolean ret;
    do {
        ret = g_utf8_validate(str, max_len, end);
        max_len -= *end - str;
        str = *end;
        /* Hitting a NUL is okay. Clear the error and iterate over them. */
        while (max_len > 0 && *str == '\0') {
            ret = TRUE;
            max_len--;
            str++;
            *end = str;
        }
    } while (ret && max_len > 0);
    return ret;
}

/* A variant of g_utf8_get_char_validated() that allows NUL characters.
 * Requires that max_len >= 0. */
static gunichar _vte_conv_utf8_get_char_validated(const gchar *p, gssize max_len) {
    gunichar ret;
    /* Handle NUL at the beginning. */
    if (max_len > 0 && p[0] == '\0') {
        return 0;
    }
    ret = g_utf8_get_char_validated(p, max_len);
    /* If a partial match is returned but there's a NUL in the buffer
        * then this is a wrong error, we're facing an invalid character. */
    if (ret == (gunichar) -2 && memchr(p, '\0', max_len) != NULL) {
        ret = (gunichar) -1;
    }
    return ret;
}

/* A bogus UTF-8 to UTF-8 conversion function which attempts to provide the
 * same semantics as g_iconv(). */
static size_t _vte_conv_utf8_utf8(GIConv converter,
                                    const gchar **inbuf,
                                    gsize *inbytes_left,
                                    gchar **outbuf,
                                    gsize *outbytes_left)
{
    gboolean validated;
    const gchar *endptr;
    size_t bytes;

    /* We don't tolerate shenanigans! */
    g_assert_cmpuint(*outbytes_left, >=, *inbytes_left);

    /* The only error we can throw is EILSEQ, so check for that here. */
    validated = _vte_conv_utf8_validate(*inbuf, *inbytes_left, &endptr);

    /* Copy whatever data was validated. */
    bytes = endptr - *inbuf;
    memcpy(*outbuf, *inbuf, bytes);
    *inbuf += bytes;
    *outbuf += bytes;
    *outbytes_left -= bytes;
    *inbytes_left -= bytes;

    /* Return 0 (number of non-reversible conversions performed) if everything
     * looked good, else EILSEQ. */
    if (validated) {
        return 0;
    }

    /* Determine why the end of the string is not valid. */
    if (_vte_conv_utf8_get_char_validated(*inbuf, *inbytes_left) == (gunichar) -2) {
        /* Prefix of a valid UTF-8 */
        errno = EINVAL;
    } else {
        /* We had enough bytes to validate the character, and
         * it failed.  It just doesn't look right. */
        errno = EILSEQ;
    }
    return (size_t) -1;
}

/* Open a conversion descriptor which, in addition to normal cases, provides
 * UTF-8 to UTF-8 conversions and a gunichar-compatible source and target
 * encoding. */
VteConv _vte_conv_open(const char *target, const char *source)
{
    VteConv ret;
    GIConv conv;
    gboolean in_unichar, out_unichar, utf8;
    const char *real_target, *real_source;

    /* No shenanigans. */
    g_assert(target != NULL);
    g_assert(source != NULL);
    g_assert(strlen(target) > 0);
    g_assert(strlen(source) > 0);

    /* Assume normal iconv usage. */
    in_unichar = FALSE;
    out_unichar = FALSE;
    real_source = source;
    real_target = target;

    /* Determine if we need to convert gunichars to UTF-8 on input. */
    if (strcmp(target, VTE_CONV_GUNICHAR_TYPE) == 0) {
        real_target = "UTF-8";
        out_unichar = TRUE;
    }

    /* Determine if we need to convert UTF-8 to gunichars on output. */
    if (strcmp(source, VTE_CONV_GUNICHAR_TYPE) == 0) {
        real_source = "UTF-8";
        in_unichar = TRUE;
    }

    /* Determine if this is a UTF-8 to UTF-8 conversion. */
    utf8 = ((g_ascii_strcasecmp(real_target, "UTF-8") == 0) &&
            (g_ascii_strcasecmp(real_source, "UTF-8") == 0));

    /* If we're doing UTF-8 to UTF-8, just use a dummy function which
     * checks for bad data. */
    conv = NULL;
    if (!utf8) {
        char *translit_target = g_strdup_printf ("%s//translit", real_target);
        conv = g_iconv_open(translit_target, real_source);
        g_free (translit_target);
        if (conv == ((GIConv) -1)) {
            conv = g_iconv_open(real_target, real_source);
        }
        if (conv == ((GIConv) -1)) {
            return VTE_INVALID_CONV;
        }
    }

    /* Set up the descriptor. */
    ret = g_slice_new0(struct _VteConv);
    if (utf8) {
        ret->conv = NULL;
        ret->convert = (convert_func) _vte_conv_utf8_utf8;
        ret->close = NULL;
    } else {
        g_assert((conv != NULL) && (conv != ((GIConv) -1)));
        ret->conv = conv;
        ret->convert = (convert_func) g_iconv;
        ret->close = g_iconv_close;
    }

    /* Initialize other elements. */
    ret->in_unichar = in_unichar;
    ret->out_unichar = out_unichar;

    /* Create scratch buffers. */
    ret->in_scratch = _vte_byte_array_new();
    ret->out_scratch = _vte_byte_array_new();

    return ret;
}

gint _vte_conv_close(VteConv converter)
{
    g_assert(converter != NULL);
    g_assert(converter != VTE_INVALID_CONV);

    /* Close the underlying descriptor, if there is one. */
    if (converter->conv != NULL) {
        g_assert(converter->close != NULL);
        converter->close(converter->conv);
    }

    /* Free the scratch buffers. */
    _vte_byte_array_free(converter->in_scratch);
    _vte_byte_array_free(converter->out_scratch);

    /* Free the structure itself. */
    g_slice_free(struct _VteConv, converter);

    return 0;
}

size_t _vte_conv(VteConv converter,
      const guchar **inbuf, gsize *inbytes_left,
      guchar **outbuf, gsize *outbytes_left)
{
    size_t ret, tmp;
    const guchar *work_inbuf_start, *work_inbuf_working;
    guchar *work_outbuf_start, *work_outbuf_working;
    gsize work_inbytes, work_outbytes;

    g_assert(converter != NULL);
    g_assert(converter != VTE_INVALID_CONV);

    work_inbuf_start = work_inbuf_working = *inbuf;
    work_outbuf_start = work_outbuf_working = *outbuf;
    work_inbytes = *inbytes_left;
    work_outbytes = *outbytes_left;

    /* Possibly convert the input data from gunichars to UTF-8. */
    if (converter->in_unichar) {
        int i, char_count;
        guchar *p, *end;
        gunichar *g;

        /* Make sure the scratch buffer has enough space. */
        char_count = *inbytes_left / sizeof(gunichar);
        _vte_byte_array_set_minimum_size(converter->in_scratch, (char_count + 1) * VTE_UTF8_BPC);

        /* Convert the incoming text. */
        g = (gunichar*) *inbuf;
        p = converter->in_scratch->data;
        end = p + (char_count + 1) * VTE_UTF8_BPC;
        for (i = 0; i < char_count; i++) {
            p += g_unichar_to_utf8(g[i], (gchar *)p);
            g_assert(p <= end);
        }
        /* Update our working pointers. */
        work_inbuf_start = converter->in_scratch->data;
        work_inbuf_working = work_inbuf_start;
        work_inbytes = p - work_inbuf_start;
    }

    /* Possibly set the output pointers to point at our scratch buffer. */
    if (converter->out_unichar) {
        work_outbytes = *outbytes_left * VTE_UTF8_BPC;
        _vte_byte_array_set_minimum_size(converter->out_scratch, work_outbytes);
        work_outbuf_start = converter->out_scratch->data;
        work_outbuf_working = work_outbuf_start;
    }

    /* Call the underlying conversion. */
    ret = 0;
    do {
        tmp = converter->convert(converter->conv,
                                    &work_inbuf_working,
                                    &work_inbytes,
                                    &work_outbuf_working,
                                    &work_outbytes);
        if (tmp == (size_t) -1) {
            /* Check for zero bytes, which we pass right through. */
            if (errno == EILSEQ) {
                if ((work_inbytes > 0) &&
                    (work_inbuf_working[0] == '\0') &&
                    (work_outbytes > 0)) {
                    work_outbuf_working[0] = '\0';
                    work_outbuf_working++;
                    work_inbuf_working++;
                    work_outbytes--;
                    work_inbytes--;
                    ret++;
                } else {
                    /* No go. */
                    ret = -1;
                    break;
                }
            } else {
                ret = -1;
                break;
            }
        } else {
            ret += tmp;
            break;
        }
    } while (work_inbytes > 0);

    /* We can't handle this particular failure, and it should
     * never happen.  (If it does, our caller needs fixing.)  */
    g_assert((ret != (size_t)-1) || (errno != E2BIG));

    /* Possibly convert the output from UTF-8 to gunichars. */
    if (converter->out_unichar) {
        int  left = *outbytes_left;
        gunichar *g;
        gchar *p;

        g = (gunichar*) *outbuf;
        for(p = (gchar *)work_outbuf_start; p < (gchar *)work_outbuf_working; p = g_utf8_next_char(p)) {
               g_assert(left>=0);
               *g++ = g_utf8_get_char(p);
               left -= sizeof(gunichar);
        }
        *outbytes_left = left;
        *outbuf = (guchar*) g;
    } else {
        /* Pass on the output results. */
        *outbuf = work_outbuf_working;
        *outbytes_left -= (work_outbuf_working - work_outbuf_start);
    }

    /* Advance the input pointer to the right place. */
    if (converter->in_unichar) {
        /* Get an idea of how many characters were converted, and
         * advance the pointer as required. */
        gsize chars;
        chars = _vte_conv_utf8_strlen((const gchar *)work_inbuf_start,
                                        work_inbuf_working - work_inbuf_start);
        *inbuf += (sizeof(gunichar) * chars);
        *inbytes_left -= (sizeof(gunichar) * chars);
    } else {
        /* Pass on the input results. */
        *inbuf = work_inbuf_working;
        *inbytes_left -= (work_inbuf_working - work_inbuf_start);
    }

    return ret;
}

size_t _vte_conv_cu(VteConv converter, const guchar **inbuf, gsize *inbytes_left,
                    gunichar **outbuf, gsize *outbytes_left)
{
    return _vte_conv(converter, inbuf, inbytes_left, (guchar**)outbuf, outbytes_left);
}

size_t _vte_conv_uu(VteConv converter, const gunichar **inbuf, gsize *inbytes_left,
                    gunichar **outbuf, gsize *outbytes_left)
{
    return _vte_conv(converter, (const guchar**)inbuf, inbytes_left, (guchar**)outbuf, outbytes_left);
}

size_t _vte_conv_uc(VteConv converter, const gunichar **inbuf, gsize *inbytes_left, guchar **outbuf, gsize *outbytes_left)
{
    return _vte_conv(converter, (const guchar**)inbuf, inbytes_left, outbuf, outbytes_left);
}

#ifdef VTECONV_MAIN

static gsize ucs4_strlen(const gunichar *p, gsize max_len)
{
    const gunichar *q = p + max_len;
    gsize length = 0;
    while (p < q && *p++ != 0) {
        length++;
    }
    return length;
}
static void clear(gunichar wide[5], gchar narrow[5])
{
    wide[0] = 'T';
    wide[1] = 'E';
    wide[2] = 'S';
    wide[3] = 'T';
    wide[4] = '\0';
    strcpy(narrow, "test");
}

static int mixed_strcmp(const gunichar *wide, const guchar *narrow)
{
    while (*wide && *narrow) {
        if (*wide != (gunichar)*narrow) {
            return -1;
        }
        wide++;
        narrow++;
    }
    return 0;
}

/* Test _vte_conv_utf8_strlen, especially where it differs from g_utf8_strlen. */
static void test_utf8_strlen (void)
{
    g_assert_cmpuint(_vte_conv_utf8_strlen("", 0), ==, 0);
    g_assert_cmpuint(_vte_conv_utf8_strlen("\0\0\0\0", 4), ==, 4);
    g_assert_cmpuint(_vte_conv_utf8_strlen("\0A\0\0", 4), ==, 4);
    g_assert_cmpuint(_vte_conv_utf8_strlen("\0A\0B", 4), ==, 4);
    g_assert_cmpuint(_vte_conv_utf8_strlen("A\0B\0", 4), ==, 4);
    g_assert_cmpuint(_vte_conv_utf8_strlen("ABCD", 4), ==, 4);
    g_assert_cmpuint(_vte_conv_utf8_strlen("ABCDE", 4), ==, 4);
    g_assert_cmpuint(_vte_conv_utf8_strlen("\xC2\xA0\xC2\xA0", 4), ==, 2);
}

static void test_utf8_validate (void)
{
    static const struct {
        char input[16];
        gsize ilen;
        gsize endlen;
        gboolean validates;
    } tests[] = {
        { "\0\0\0", 0, 0, TRUE },
        { "\0\0\0", 1, 1, TRUE },
        { "\0\0\0", 3, 3, TRUE },

        { "ab\0cd\0\0ef", 6, 6, TRUE },
        { "ab\0cd\0\0ef", 7, 7, TRUE },
        { "ab\0cd\0\0ef", 9, 9, TRUE },

        { "ab\xE2\x94\x80\0\xE2\x94\x80yz", 11, 11, TRUE },

        { "ab\x80\0cd", 6, 2, FALSE },

        { "ab\xE2\0cd", 6, 2, FALSE },
    };
    guint i;
    const char *end;

    for (i = 0; i < G_N_ELEMENTS (tests); i++) {
        g_assert(_vte_conv_utf8_validate(tests[i].input, tests[i].ilen, &end) == tests[i].validates);
        g_assert_cmpuint((gsize)(end - tests[i].input), ==, tests[i].endlen);
    }
}

/* Test _vte_conv_utf8_get_char_validated. */
static void test_utf8_get_char_validated (void)
{
        static const guchar mbyte_test_u[] = { 0xe2, 0x94, 0x80 };
        static const guchar mbyte_test_break_u[] = { 0xe2, 0xe2, 0xe2 };
        const char *mbyte_test = (const char *)mbyte_test_u;
        const char *mbyte_test_break = (const char *)mbyte_test_break_u;

        g_assert_cmpuint(_vte_conv_utf8_get_char_validated("", 0), ==, (gunichar)-2);
        g_assert_cmpuint(_vte_conv_utf8_get_char_validated("\0", 1), ==, 0);
        g_assert_cmpuint(_vte_conv_utf8_get_char_validated(mbyte_test, 1), ==, (gunichar)-2);
        g_assert_cmpuint(_vte_conv_utf8_get_char_validated(mbyte_test, 2), ==, (gunichar)-2);
        g_assert_cmpuint(_vte_conv_utf8_get_char_validated(mbyte_test, 3), ==, 0x2500);
        g_assert_cmpuint(_vte_conv_utf8_get_char_validated(mbyte_test_break, 1), ==, (gunichar)-2);
        g_assert_cmpuint(_vte_conv_utf8_get_char_validated(mbyte_test_break, 2), ==, (gunichar)-1);
        g_assert_cmpuint(_vte_conv_utf8_get_char_validated(mbyte_test_break, 3), ==, (gunichar)-1);
        g_assert_cmpuint(_vte_conv_utf8_get_char_validated("\x80\0", 2), ==, (gunichar)-1);
        g_assert_cmpuint(_vte_conv_utf8_get_char_validated("\xE2\0", 2), ==, (gunichar)-1);
        g_assert_cmpuint(_vte_conv_utf8_get_char_validated("\xE2\x94\0", 3), ==, (gunichar)-1);
        g_assert_cmpuint(_vte_conv_utf8_get_char_validated("\xE2\x94\x80\0", 4), ==, 0x2500);
}

typedef struct {
    gunichar wide[8];
    gssize widelen;
    gchar narrow[8];
    gssize narrowlen;
    char target[16];
    char source[16];
} TestData;

static void test_narrow_narrow (const TestData *tests, gsize n_tests)
{
    VteConv conv;
    guchar buf[10];
    const guchar *inbuf;
    guchar *outbuf;
    gsize inbytes, outbytes, ret;
    gsize i;

    for (i = 0; i < n_tests; i++) {
        memset(buf, 0, sizeof(buf));
        inbuf = (const guchar *)tests[i].narrow;
        inbytes = tests[i].narrowlen >= 0 ? tests[i].narrowlen : strlen(tests[i].narrow);
        outbuf = buf;
        outbytes = sizeof(buf);
        conv = _vte_conv_open(tests[i].target, tests[i].source);
        ret = _vte_conv(conv, &inbuf, &inbytes, &outbuf, &outbytes);
        g_assert_cmpuint(ret, ==, 0);
        g_assert_cmpuint(inbytes, ==, 0);
        g_assert_cmpstr(tests[i].narrow, ==, (char *)buf);
        _vte_conv_close(conv);
    }
}

static void test_narrow_to_wide (const TestData *tests, gsize n_tests)
{
    gunichar widebuf[5];
    VteConv conv;
    const guchar *inbuf;
    guchar *outbuf;
    gsize inbytes, outbytes, ret;
    gsize i;

    for (i = 0; i < n_tests; i++) {
        memset(widebuf, 0, sizeof(widebuf));
        inbuf = (const guchar *)tests[i].narrow;
        inbytes = tests[i].narrowlen >= 0 ? tests[i].narrowlen : strlen(tests[i].narrow);
        outbuf = (guchar*) widebuf;
        outbytes = sizeof(widebuf);
        conv = _vte_conv_open(VTE_CONV_GUNICHAR_TYPE, tests[i].source);
        ret = _vte_conv(conv, &inbuf, &inbytes, &outbuf, &outbytes);
        g_assert_cmpuint(ret, ==, 0);
        g_assert_cmpuint(inbytes, ==, 0);
        g_assert_cmpint(mixed_strcmp(widebuf, inbuf), ==, 0);
        _vte_conv_close(conv);
    }
}

static void test_wide_to_narrow (const TestData *tests, gsize n_tests)
{
    char buf[10];
    VteConv conv;
    const guchar *inbuf;
    guchar *outbuf;
    gsize inbytes, outbytes, ret;
    gsize i;

    for (i = 0; i < n_tests; i++) {
        memset(buf, 0, sizeof(buf));
        inbuf = (const guchar *)tests[i].wide;
        inbytes = tests[i].widelen >= 0 ? tests[i].widelen
                : ucs4_strlen(tests[i].wide, sizeof(tests[i].wide)) * sizeof(gunichar);
        outbuf = (guchar *)buf;
        outbytes = sizeof(buf);
        conv = _vte_conv_open(tests[i].target, VTE_CONV_GUNICHAR_TYPE);
        ret = _vte_conv(conv, &inbuf, &inbytes, &outbuf, &outbytes);
        g_assert_cmpuint(ret, ==, 0);
        g_assert_cmpuint(inbytes, ==, 0);
        g_assert_cmpint(mixed_strcmp(tests[i].wide, outbuf), ==, 0);
        _vte_conv_close(conv);
    }
}

static void test_g_iconv_narrow_narrow (void)
{
    static const TestData tests[] = {
        { { 0, }, -1, "test", -1, "UTF-8", "ISO-8859-1" },
        { { 0, }, -1, "test", -1, "ISO-8859-1", "UTF-8" },
    };

    test_narrow_narrow (tests, G_N_ELEMENTS(tests));
}

static void test_g_iconv_narrow_to_wide (void)
{
    static const TestData tests[] = {
        { { 0, }, -1, "test", -1, VTE_CONV_GUNICHAR_TYPE, "ISO-8859-1" },
    };

    test_narrow_to_wide (tests, G_N_ELEMENTS(tests));
}

static void test_g_iconv_wide_to_narrow (void)
{
    static const TestData tests[] = {
        { { 'T', 'E', 'S', 'T', 0 }, -1, "", -1, "ISO-8859-1", VTE_CONV_GUNICHAR_TYPE },
    };

    test_wide_to_narrow (tests, G_N_ELEMENTS(tests));
}

static void test_utf8_to_utf8 (void)
{
    static const TestData tests[] = {
        { { 0, }, -1, "test", -1, "UTF-8", "UTF-8" },
    };

    test_narrow_narrow (tests, G_N_ELEMENTS (tests));
}

static void test_zero_byte_passthrough (void)
{
    gunichar wide_test[5];
    gchar narrow_test[5];
    VteConv conv;
    const guchar *inbuf;
    guchar *outbuf;
    gsize inbytes, outbytes;
    int i;

    /* Test zero-byte pass-through. */
    clear(wide_test, narrow_test);
    memset(wide_test, 0, sizeof(wide_test));
    inbuf = (guchar *)wide_test;
    inbytes = 3 * sizeof(gunichar);
    outbuf = (guchar *)narrow_test;
    outbytes = sizeof(narrow_test);
    conv = _vte_conv_open("UTF-8", VTE_CONV_GUNICHAR_TYPE);
    i = _vte_conv(conv, &inbuf, &inbytes, &outbuf, &outbytes);
    g_assert(inbytes == 0);
    if ((narrow_test[0] != 0) ||
        (narrow_test[1] != 0) ||
        (narrow_test[2] != 0)) {
        g_error("Conversion 6 failed.\n");
    }
    _vte_conv_close(conv);

    /* Test zero-byte pass-through. */
    clear(wide_test, narrow_test);
    memset(wide_test, 'A', sizeof(wide_test));
    memset(narrow_test, 0, sizeof(narrow_test));
    inbuf = (guchar *)narrow_test;
    inbytes = 3;
    outbuf = (guchar *)wide_test;
    outbytes = sizeof(wide_test);
    conv = _vte_conv_open(VTE_CONV_GUNICHAR_TYPE, "UTF-8");
    i = _vte_conv(conv, &inbuf, &inbytes, &outbuf, &outbytes);
    g_assert(inbytes == 0);
    if ((wide_test[0] != 0) ||
        (wide_test[1] != 0) ||
        (wide_test[2] != 0)) {
        g_error("Conversion 7 failed.\n");
    }
    _vte_conv_close(conv);

    /* Test zero-byte pass-through. */
    clear(wide_test, narrow_test);
    memset(wide_test, 'A', sizeof(wide_test));
    memset(narrow_test, 0, sizeof(narrow_test));
    inbuf = (guchar *)narrow_test;
    inbytes = 3;
    outbuf = (guchar *)wide_test;
    outbytes = sizeof(wide_test);
    conv = _vte_conv_open(VTE_CONV_GUNICHAR_TYPE, "ISO-8859-1");
    i = _vte_conv(conv, &inbuf, &inbytes, &outbuf, &outbytes);
    g_assert(inbytes == 0);
    if ((wide_test[0] != 0) ||
        (wide_test[1] != 0) ||
        (wide_test[2] != 0)) {
        g_error("Conversion 8 failed.\n");
    }
    _vte_conv_close(conv);
}

static void test_utf8_to_utf8_error (void)
{
    static const guchar mbyte_test[] = { 0xe2, 0x94, 0x80 };
    static const guchar mbyte_test_break[] = { 0xe2, 0xe2, 0xe2 };
    gchar buf[10];
    VteConv conv;
    const guchar *inbuf;
    guchar *outbuf;
    gsize inbytes, outbytes;
    gsize i;

    /* Test UTF-8 to UTF-8 error reporting, valid multibyte. */
    for (i = 0; i < sizeof(mbyte_test); i++) {
        int ret;
        inbuf = mbyte_test;
        inbytes = i + 1;
        outbuf = (guchar *)buf;
        outbytes = sizeof(buf);
        conv = _vte_conv_open("UTF-8", "UTF-8");
        ret = _vte_conv(conv, &inbuf, &inbytes, &outbuf, &outbytes);
        switch (i) {
            case 0:
                g_assert_cmpint(ret, ==, -1);
                g_assert_cmpint(errno, ==, EINVAL);
                break;
            case 1:
                g_assert_cmpint(ret, ==, -1);
                g_assert_cmpint(errno, ==, EINVAL);
                break;
            case 2:
                g_assert_cmpint(ret, !=, -1);
                break;
            default:
                g_assert_not_reached();
                break;
        }
        _vte_conv_close(conv);
    }

    /* Test UTF-8 to UTF-8 error reporting, invalid multibyte. */
    for (i = 0; i < sizeof(mbyte_test_break); i++) {
        int ret;
        inbuf = mbyte_test_break;
        inbytes = i + 1;
        outbuf = (guchar *)buf;
        outbytes = sizeof(buf);
        conv = _vte_conv_open("UTF-8", "UTF-8");
        ret = _vte_conv(conv, &inbuf, &inbytes, &outbuf, &outbytes);
        _vte_conv_close(conv);
        switch (i) {
            case 0:
                g_assert_cmpint(ret, ==, -1);
                g_assert_cmpint(errno, ==, EINVAL);
                break;
            case 1:
                g_assert_cmpint(ret, ==, -1);
                g_assert_cmpint(errno, ==, EILSEQ);
                break;
            case 2:
                g_assert_cmpint(ret, ==, -1);
                g_assert_cmpint(errno, ==, EILSEQ);
                break;
            default:
                g_assert_not_reached();
                break;
        }
    }
}

int main (int argc, char *argv[])
{
        g_test_init (&argc, &argv, nullptr);

        g_test_add_func ("/vte/conv/utf8/strlen", test_utf8_strlen);
        g_test_add_func ("/vte/conv/utf8/validate", test_utf8_validate);
        g_test_add_func ("/vte/conv/utf8/get-char", test_utf8_get_char_validated);
        g_test_add_func ("/vte/conv/utf8/conversion", test_utf8_to_utf8);
        g_test_add_func ("/vte/conv/utf8/conversion-with-error", test_utf8_to_utf8_error);
        g_test_add_func ("/vte/conv/narrow-narrow", test_g_iconv_narrow_narrow);
        g_test_add_func ("/vte/conv/narrow-to-wide", test_g_iconv_narrow_to_wide);
        g_test_add_func ("/vte/conv/wide-to-narrow", test_g_iconv_wide_to_narrow);
        g_test_add_func ("/vte/conv/zero-byte-passthrough", test_zero_byte_passthrough);

    return g_test_run ();
}
#endif
