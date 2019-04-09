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

#pragma once

#include <pango/pango.h>
#include <gdk/gdk.h>
#include <errno.h>

#ifdef VTE_DEBUG
#define IFDEF_DEBUG(str) str
#else
#define IFDEF_DEBUG(str)
#endif

namespace vte {

namespace grid {

        typedef long row_t;
        typedef long column_t;

        struct coords {
        public:
                coords() = default;
                coords(row_t r, column_t c) : m_row(r), m_column(c) { }

                inline void set_row(row_t r)       { m_row = r; }
                inline void set_column(column_t c) { m_column = c; }

                inline row_t row()       const { return m_row; }
                inline column_t column() const { return m_column; }

                inline bool operator == (coords const& rhs) const { return m_row == rhs.m_row && m_column == rhs.m_column; }
                inline bool operator != (coords const& rhs) const { return m_row != rhs.m_row || m_column != rhs.m_column; }

                inline bool operator <  (coords const& rhs) const { return m_row < rhs.m_row || (m_row == rhs.m_row && m_column <  rhs.m_column); }
                inline bool operator <= (coords const& rhs) const { return m_row < rhs.m_row || (m_row == rhs.m_row && m_column <= rhs.m_column); }
                inline bool operator >  (coords const& rhs) const { return m_row > rhs.m_row || (m_row == rhs.m_row && m_column >  rhs.m_column); }
                inline bool operator >= (coords const& rhs) const { return m_row > rhs.m_row || (m_row == rhs.m_row && m_column >= rhs.m_column); }

                IFDEF_DEBUG(char const* to_string() const);

        private:
                row_t m_row;
                column_t m_column;
        };

        struct span {
        public:
                span() = default;
                span(coords const& s, coords const& e) : m_start(s), m_end(e) { }
                span(row_t sr, column_t sc, row_t er, column_t ec) : m_start(sr, sc), m_end(er, ec) { }

                inline void set(coords const&s, coords const& e) { m_start = s; m_end = e; }
                inline void set_start(coords const& s) { m_start = s; }
                inline void set_end(coords const& e) { m_end = e; }

                inline bool operator == (span const& rhs) const { return m_start == rhs.m_start && m_end == rhs.m_end; }
                inline bool operator != (span const& rhs) const { return m_start != rhs.m_start || m_end != rhs.m_end; }

                inline coords const& start() const { return m_start; }
                inline coords const& end()   const { return m_end; }
                inline row_t start_row()       const { return m_start.row(); }
                inline row_t end_row()         const { return m_end.row(); }
                inline column_t start_column() const { return m_start.column(); }
                inline column_t end_column()   const { return m_end.column(); }

                inline void clear() { m_start = coords(-1, -1); m_end = coords(-2, -2); }
                inline bool empty() const { return m_start > m_end; }
                inline explicit operator bool() const { return !empty(); }

                inline bool contains(coords const& p) const { return m_start <= p && p <= m_end; }
                inline bool box_contains(coords const& p) const { return m_start.row() <= p.row() && p.row() <= m_end.row() &&
                                                                         m_start.column() <= p.column() && p.column() <= m_end.column(); }

                inline bool contains(row_t row, column_t column) { return contains(coords(row, column)); }

                IFDEF_DEBUG(char const* to_string() const);

        private:
                coords m_start;
                coords m_end;
        };

} /* namespace grid */

namespace view {

        /* FIXMEchpe: actually 32-bit int would be sufficient here */
        typedef long coord_t;

        class coords {
        public:
                coords() = default;
                coords(coord_t x_, coord_t y_) : x(x_), y(y_) { }

                inline bool operator == (coords const& rhs) const { return x == rhs.x && y == rhs.y; }
                inline bool operator != (coords const& rhs) const { return x != rhs.x || y != rhs.y; }

                void swap(coords &rhs) { coords tmp = rhs; rhs = *this; *this = tmp; }

                IFDEF_DEBUG(char const* to_string() const);

        public:
                coord_t x;
                coord_t y;
        };

        class extents {
        public:
                extents() = default;
                extents(coord_t w, coord_t h) : m_width(w), m_height(h) { }

                inline coord_t width() const { return m_width; }
                inline coord_t height() const { return m_height; }

                inline bool operator == (extents const& rhs) const { return m_width == rhs.m_width && m_height == rhs.m_height; }
                inline bool operator != (extents const& rhs) const { return m_width != rhs.m_width || m_height != rhs.m_height; }

                IFDEF_DEBUG(char const* to_string() const);

        private:
                coord_t m_width;
                coord_t m_height;
        };

} /* namespace view */

namespace color {

        /* 24-bit (8 bit per channel) packed colour */
        /* FIXME: we could actually support 10 bit per channel */
        typedef guint32 packed;

        class rgb : public PangoColor {
        public:
                rgb() = default;
                rgb(PangoColor const& c) { *static_cast<PangoColor*>(this) = c; }
                rgb(GdkRGBA const* c);
                rgb(GdkRGBA const& c) : rgb(&c) { }

                bool parse(char const* spec);

                void from_pango(PangoColor const& c) { *static_cast<PangoColor*>(this) = c; }

                inline bool operator == (rgb const& rhs) const {
                        return red == rhs.red && green == rhs.green && blue == rhs.blue;
                }

                IFDEF_DEBUG(char const* to_string() const);
        };

} /* namespace color */

namespace util {

        class restore_errno {
        public:
                restore_errno() { m_errsv = errno; }
                ~restore_errno() { errno = m_errsv; }
                operator int () const { return m_errsv; }
        private:
                int m_errsv;
        };

        class smart_fd {
        public:
                smart_fd() : m_fd(-1) { }
                explicit smart_fd(int fd) : m_fd(fd) { }
/* FIXME */
#if 0
                ~smart_fd() { if (m_fd != -1) { restore_errno errsv; close(m_fd); } }

                inline smart_fd& operator = (int rhs) { if (m_fd != -1) { restore_errno errsv; close(m_fd); } m_fd = rhs; return *this; }
                inline smart_fd& operator = (smart_fd& rhs) { if (&rhs != this) { if (m_fd != -1) { restore_errno errsv; close(m_fd); } m_fd = rhs.m_fd; rhs.m_fd = -1; } return *this; }
#endif
                inline operator int () const { return m_fd; }
                inline operator int* () { g_assert(m_fd == -1); return &m_fd; }

                int steal() { auto d = m_fd; m_fd = -1; return d; }

                /* Prevent accidents */
                smart_fd(smart_fd const&) = delete;
                smart_fd(smart_fd&&) = delete;
                smart_fd& operator = (smart_fd const&) = delete;
                smart_fd& operator = (smart_fd&&) = delete;

        private:
                int m_fd;
        };

} /* namespace util */

} /* namespace vte */
