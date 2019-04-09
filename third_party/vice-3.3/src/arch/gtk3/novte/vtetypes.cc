/*
 * Copyright © 2015 Christian Persch
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vice.h"

#include <glib.h>

#include "vtetypes.hh"

#include <type_traits>

static_assert(std::is_pod<vte::grid::coords>::value, "vte::grid::coords not POD");
static_assert(sizeof(vte::grid::coords) == 2 * sizeof(long), "vte::grid::coords size wrong");

static_assert(std::is_pod<vte::grid::span>::value, "vte::grid::span not POD");
static_assert(sizeof(vte::grid::span) == 4 * sizeof(long), "vte::grid::span size wrong");

static_assert(std::is_pod<vte::view::coords>::value, "vte::view::coords not POD");
static_assert(sizeof(vte::view::coords) == 2 * sizeof(vte::view::coord_t), "vte::view::coords size wrong");

static_assert(std::is_pod<vte::color::rgb>::value, "vte::color::rgb not POD");
static_assert(sizeof(vte::color::rgb) == sizeof(PangoColor), "vte::color::rgb size wrong");

static_assert(sizeof(vte::util::smart_fd) == sizeof(int), "vte::util::smart_fd size wrong");

vte::color::rgb::rgb(GdkRGBA const* rgba) {
    g_assert(rgba);
    /* FIXME: equal distribution! */
    red   = rgba->red   * 65535.;
    green = rgba->green * 65535.;
    blue  = rgba->blue  * 65535.;
}

bool vte::color::rgb::parse(char const* spec)
{
    char *spec_copy = (char *)spec;
    bool retval = false;

    /* gdk_color_parse doesnt handle all XParseColor formats.  It only
     * supports the #RRRGGGBBB format, not the rgb:RRR/GGG/BBB format.
     * See: man XParseColor */

    if (g_ascii_strncasecmp(spec_copy, "rgb:", 4) == 0) {
        char *cur, *ptr;

        spec_copy = g_strdup(spec);
        cur = spec_copy;
        ptr = spec_copy + 3;

        *cur++ = '#';
        while (*ptr++) {
            if (*ptr != '/') {
                *cur++ = *ptr;
            }
        }
        *cur++ = '\0';
    }

    retval = pango_color_parse(this, spec_copy) != FALSE;

    if (spec_copy != spec) {
        g_free(spec_copy);
    }

    return retval;
}

#ifdef VTE_DEBUG

#define DEBUG_STRING_SIZE (256)
#define DEBUG_STRING_SLICES (64)

static char* debug_get_buf(void)
{
    static char *buf = NULL;
    static gsize offset = 0;

    if (buf != NULL) {
        offset = (offset + 1) % DEBUG_STRING_SLICES;
    } else {
        buf = g_new0(char, DEBUG_STRING_SIZE * DEBUG_STRING_SLICES);
    }
    return buf + offset * DEBUG_STRING_SIZE;
}

char const* vte::grid::coords::to_string() const
{
    char *buf = debug_get_buf();
    g_snprintf(buf, DEBUG_STRING_SIZE, "grid[%ld,%ld]", row(), column());
    return buf;
}

char const* vte::grid::span::to_string() const
{
    if (empty()) {
        return "grid[empty]";
    }

    char *buf = debug_get_buf();
    g_snprintf(buf, DEBUG_STRING_SIZE, "grid[%ld,%ld .. %ld,%ld]",
                start_row(), start_column(), end_row(), end_column());
    return buf;
}

char const* vte::view::coords::to_string() const
{
    char *buf = debug_get_buf();
    g_snprintf(buf, DEBUG_STRING_SIZE, "view[%ld,%ld]", x, y);
    return buf;
}

char const* vte::view::extents::to_string() const
{
    char *buf = debug_get_buf();
    g_snprintf(buf, DEBUG_STRING_SIZE, "view::extents[%ld x %ld]", width(), height());
    return buf;
}

char const* vte::color::rgb::to_string() const
{
    char *buf = debug_get_buf();
    g_snprintf(buf, DEBUG_STRING_SIZE, "rgb(%04x,%04x,%04x)", red, green, blue);
    return buf;
}

#endif /* VTE_DEBUG */

#ifdef MAIN

#include <glib.h>

using namespace vte::grid;

static void test_grid_coords (void)
{
    /* Default constructor */
    coords p1;

    /* Construction and assignment */

    coords p2(256,16);
    g_assert_cmpint(p2.row(), ==, 256);
    g_assert_cmpint(p2.column(), ==, 16);

    p2.set_row(512);
    g_assert_cmpint(p2.row(), ==, 512);

    p2.set_column(32);
    g_assert_cmpint(p2.column(), ==, 32);

    coords p3(256,16);
    coords p4 = p3;
    g_assert_cmpint(p3.row(), ==, p4.row());
    g_assert_cmpint(p3.column(), ==, p4.column());

    /* Comparision operators */

    g_assert_true(p3 == p4);
    g_assert_false(p3 != p4);

    p4.set_row(32);
    g_assert_false(p3 == p4);
    g_assert_true(p3 != p4);

    g_assert_true (coords(42, 21) <= coords(42, 21));
    g_assert_false(coords(42, 21) >  coords(42, 21));
    g_assert_false(coords(42, 21) <  coords(42, 21));
    g_assert_true (coords(42, 21) >= coords(42, 21));

    g_assert_true (coords(42, 42) <= coords(43, 16));
    g_assert_true (coords(42, 42) <  coords(43, 16));
    g_assert_false(coords(42, 42) >= coords(43, 16));
    g_assert_false(coords(42, 42) >  coords(43, 16));

    g_assert_true (coords(42, 42) <= coords(43, 160));
    g_assert_true (coords(42, 42) <  coords(43, 160));
    g_assert_false(coords(42, 42) >= coords(43, 160));
    g_assert_false(coords(42, 42) >  coords(43, 160));

#ifdef VTE_DEBUG
    /* to_string() */
    g_assert_cmpstr(vte::grid::coords(17, 42).to_string(), ==, "grid[17,42]");
#endif
}

static void test_grid_span (void)
{
    /* Default constructor */
    span s1;

    /* Construction and assignment */

    coords s2s(16, 16), s2e(32, 32);
    span s2(s2s, s2e);
    g_assert_true(s2.start() == s2s);
    g_assert_true(s2.end() == s2e);
    g_assert_cmpint(s2.start_row(), ==, s2s.row());
    g_assert_cmpint(s2.start_column(), ==, s2s.column());
    g_assert_cmpint(s2.end_row(), ==, s2e.row());
    g_assert_cmpint(s2.end_column(), ==, s2e.column());

    span s3 = s2;
    g_assert_true(s2 == s3);
    g_assert_false(s2 != s3);

    span s4(16, 16, 32, 32);
    g_assert_true(s2 == s4);
    g_assert_false(s2 != s4);

    coords p4s(24, 24);
    s4.set_start(p4s);
    g_assert_true(s4.start() == p4s);

    coords p4e(80, 80);
    s4.set_end(p4e);
    g_assert_true(s4.end() == p4e);

    /* Empty and operator bool */
    span s5 = s2;
    g_assert_true(s5);
    g_assert_false(s5.empty());

    s5.clear();
    g_assert_false(s5);
    g_assert_true(s5.empty());

    s5 = span(coords(32, 32), coords(16, 16));
    g_assert_false(s5);
    g_assert_true(s5.empty());

    /* Contains */

    span s6(16, 16, 16, 32);
    g_assert_false(s6.contains(coords(15, 24)));
    g_assert_false(s6.contains(coords(16, 15)));
    g_assert_true (s6.contains(coords(16, 16)));
    g_assert_true (s6.contains(coords(16, 31)));
    g_assert_true (s6.contains(coords(16, 32)));
    g_assert_false(s6.contains(coords(16, 33)));
    g_assert_false(s6.contains(coords(17, 15)));
    g_assert_false(s6.contains(coords(17, 16)));

    span s7(16, 16, 32, 8);
    g_assert_false(s7.contains(coords(15, 4)));
    g_assert_false(s7.contains(coords(16, 15)));
    g_assert_true (s7.contains(coords(16, 16)));
    g_assert_true (s7.contains(coords(16, 42)));
    g_assert_true (s7.contains(coords(17, 42)));
    g_assert_true (s7.contains(coords(31, 100)));
    g_assert_true (s7.contains(coords(32, 8)));
    g_assert_false(s7.contains(coords(32, 9)));
    g_assert_false(s7.contains(coords(33, 2)));

    span s8(16, 16, 32, 32);
    g_assert_false(s8.box_contains(coords(15, 15)));
    g_assert_false(s8.box_contains(coords(15, 24)));
    g_assert_false(s8.box_contains(coords(15, 42)));
    g_assert_false(s8.box_contains(coords(16, 15)));
    g_assert_true (s8.box_contains(coords(16, 16)));
    g_assert_true (s8.box_contains(coords(16, 24)));
    g_assert_true (s8.box_contains(coords(16, 32)));
    g_assert_false(s8.box_contains(coords(16, 33)));
    g_assert_false(s8.box_contains(coords(24, 15)));
    g_assert_true (s8.box_contains(coords(24, 16)));
    g_assert_true (s8.box_contains(coords(24, 24)));
    g_assert_true (s8.box_contains(coords(24, 32)));
    g_assert_false(s8.box_contains(coords(24, 33)));
    g_assert_false(s8.box_contains(coords(32, 15)));
    g_assert_true (s8.box_contains(coords(32, 16)));
    g_assert_true (s8.box_contains(coords(32, 24)));
    g_assert_true (s8.box_contains(coords(32, 32)));
    g_assert_false(s8.box_contains(coords(32, 33)));
    g_assert_false(s8.box_contains(coords(33, 15)));
    g_assert_false(s8.box_contains(coords(33, 24)));
    g_assert_false(s8.box_contains(coords(3, 42)));

#ifdef VTE_DEBUG
    /* to_string() */
    g_assert_cmpstr(vte::grid::span(17, 42, 18, 3).to_string(), ==, "grid[17,42 .. 18,3]");
#endif
}

static void test_view_coords (void)
{
    /* Default constructor */
    vte::view::coords p1;

    /* Construction and assignment */
    vte::view::coords p2(256, 512);

    /* Comparision operators */

    vte::view::coords p3 = p2;
    vte::view::coords p4(1024, 2048);
    vte::view::coords p5 = p4;

    g_assert_true(p3 == p2);
    g_assert_false(p3 != p2);
    g_assert_true(p3 != p4);
    g_assert_false(p3 == p4);

    /* Swap */

    p5.swap(p3);
    g_assert_true(p3 == p4);
    g_assert_true(p5 == p2);

#ifdef VTE_DEBUG
    /* to_string() */
    g_assert_cmpstr(vte::view::coords(256, 512).to_string(), ==, "view[256,512]");
#endif
}


static void test_color_rgb (void)
{
}

static void test_util_restore_errno(void)
{
    errno = -42;
    {
        vte::util::restore_errno errsv;
        errno = 36;
    }
    g_assert_cmpint(errno, ==, -42);
}

static void test_util_smart_fd(void)
{
    vte::util::smart_fd fd2;
    g_assert_true(fd2 == -1);

    fd2 = 42;
    g_assert_true(fd2 == 42);

    vte::util::smart_fd fd3(STDERR_FILENO);
    g_assert_true(fd3 != -1);
    g_assert_true(fd3 == STDERR_FILENO);

    g_assert_cmpint(fd3.steal(), ==, STDERR_FILENO);
    g_assert_true(fd3 == -1);

    int *v = fd3;
    *v = 42;
    g_assert_true(fd3 == 42);
}

int main(int argc, char *argv[])
{
    g_test_init (&argc, &argv, nullptr);

    g_test_add_func("/vte/c++/grid/coords", test_grid_coords);
    g_test_add_func("/vte/c++/grid/span", test_grid_span);
    g_test_add_func("/vte/c++/color/rgb", test_color_rgb);
    g_test_add_func("/vte/c++/view/coords", test_view_coords);
    g_test_add_func("/vte/c++/util/restore-errno", test_util_restore_errno);
    g_test_add_func("/vte/c++/util/smart-fd", test_util_smart_fd);

    return g_test_run();
}

#endif /* MAIN */
