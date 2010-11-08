/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2010 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Screen/Canvas.hpp"
#include "Screen/Util.hpp"
#include "Compatibility/gdi.h"
#include "Asset.hpp" /* for needclipping */

// TODO: ClipPolygon is not thread safe (uses a static array)!
// We need to make it so.

void
Canvas::clipped_polygon(const POINT* lppt, unsigned cPoints)
{
  ::ClipPolygon(*this, lppt, cPoints, true);
}

void
Canvas::clipped_polyline(const POINT* lppt, unsigned cPoints)
{
  ::ClipPolygon(*this, lppt, cPoints, false);
}

void
Canvas::autoclip_polygon(const POINT* lppt, unsigned cPoints)
{
  if (need_clipping())
    clipped_polygon(lppt, cPoints);
  else
    polygon(lppt, cPoints);
}

void
Canvas::autoclip_polyline(const POINT* lppt, unsigned cPoints)
{
  if (need_clipping())
    clipped_polyline(lppt, cPoints);
  else
    polyline(lppt, cPoints);
}

void
Canvas::line(int ax, int ay, int bx, int by)
{
#ifndef NOLINETO
  ::MoveToEx(dc, ax, ay, NULL);
  ::LineTo(dc, bx, by);
#else
  POINT p[2] = {{ax, ay}, {bx, by}};
  polyline(p, 2);
#endif
}

void
Canvas::two_lines(int ax, int ay, int bx, int by, int cx, int cy)
{
#ifndef NOLINETO
  ::MoveToEx(dc, ax, ay, NULL);
  ::LineTo(dc, bx, by);
  ::LineTo(dc, cx, cy);
#else
  POINT p[2];

  p[0].x = ax;
  p[0].y = ay;
  p[1].x = bx;
  p[1].y = by;
  polyline(p, 2);

  p[0].x = cx;
  p[0].y = cy;
  polyline(p, 2);
#endif
}

void
Canvas::move_to(int x, int y)
{
#ifdef NOLINETO
  cursor.x = x;
  cursor.y = y;
#else
  ::MoveToEx(dc, x, y, NULL);
#endif
}

void
Canvas::line_to(int x, int y)
{
#ifdef NOLINETO
  line(cursor.x, cursor.y, x, y);
  move_to(x, y);
#else
  ::LineTo(dc, x, y);
#endif
}

void
Canvas::segment(int x, int y, unsigned radius,
                Angle start, Angle end, bool horizon)
{
  ::Segment(*this, x, y, radius, start, end, horizon);
}

const SIZE
Canvas::text_size(const TCHAR *text, size_t length) const
{
  SIZE size;
  ::GetTextExtentPoint(dc, text, length, &size);
  return size;
}

const SIZE
Canvas::text_size(const TCHAR *text) const
{
  return text_size(text, _tcslen(text));
}

unsigned
Canvas::text_height(const TCHAR *text) const
{
  TEXTMETRIC tm;
  GetTextMetrics(dc, &tm);
  return tm.tmHeight;
}

void
Canvas::text(int x, int y, const TCHAR *text)
{
  ::ExtTextOut(dc, x, y, 0, NULL, text, _tcslen(text), NULL);
}

void
Canvas::text(int x, int y, const TCHAR *text, size_t length)
{
  ::ExtTextOut(dc, x, y, 0, NULL, text, length, NULL);
}

void
Canvas::text_opaque(int x, int y, const RECT &rc, const TCHAR *text)
{
  ::ExtTextOut(dc, x, y, ETO_OPAQUE, &rc, text, _tcslen(text), NULL);
}

void
Canvas::text_clipped(int x, int y, const RECT &rc, const TCHAR *text)
{
  ::ExtTextOut(dc, x, y, ETO_CLIPPED, &rc, text, _tcslen(text), NULL);
}

void
Canvas::text_clipped(int x, int y, unsigned width, const TCHAR *text)
{
  const SIZE size = text_size(text);

  RECT rc;
  ::SetRect(&rc, x, y, x + min(width, (unsigned)size.cx), y + size.cy);
  text_clipped(x, y, rc, text);
}

void
Canvas::copy(int dest_x, int dest_y,
             unsigned dest_width, unsigned dest_height,
             const Canvas &src, int src_x, int src_y)
{
  ::BitBlt(dc, dest_x, dest_y, dest_width, dest_height,
           src.dc, src_x, src_y, SRCCOPY);
}

void
Canvas::copy(const Canvas &src, int src_x, int src_y)
{
  copy(0, 0, width, height, src, src_x, src_y);
}

void
Canvas::copy(const Canvas &src)
{
  copy(src, 0, 0);
}

void
Canvas::copy_transparent_black(const Canvas &src)
{
#ifdef _WIN32_WCE
  ::TransparentImage(dc, 0, 0, get_width(), get_height(),
                     src.dc, 0, 0, get_width(), get_height(),
                     Color::BLACK);
#else
  ::TransparentBlt(dc, 0, 0, get_width(), get_height(),
                   src.dc, 0, 0, get_width(), get_height(),
                   Color::BLACK);
#endif
}

void
Canvas::copy_transparent_white(const Canvas &src)
{
#ifdef _WIN32_WCE
  ::TransparentImage(dc, 0, 0, get_width(), get_height(),
                     src.dc, 0, 0, get_width(), get_height(),
                     Color::WHITE);
#else
  ::TransparentBlt(dc, 0, 0, get_width(), get_height(),
                   src.dc, 0, 0, get_width(), get_height(),
                   Color::WHITE);
#endif
}

void
Canvas::stretch_transparent(const Canvas &src, Color key)
{
#ifdef _WIN32_WCE
  ::TransparentImage(dc, 0, 0, get_width(), get_height(),
                     src.dc, 0, 0, src.get_width(), src.get_height(),
                     key);
#else
  ::TransparentBlt(dc, 0, 0, get_width(), get_height(),
                   src.dc, 0, 0, src.get_width(), src.get_height(),
                   key);
#endif
}

void
Canvas::stretch(int dest_x, int dest_y,
                unsigned dest_width, unsigned dest_height,
                const Canvas &src,
                int src_x, int src_y,
                unsigned src_width, unsigned src_height)
{
  ::StretchBlt(dc, dest_x, dest_y, dest_width, dest_height,
               src.dc, src_x, src_y, src_width, src_height,
               SRCCOPY);
}

void
Canvas::stretch(const Canvas &src,
                int src_x, int src_y,
                unsigned src_width, unsigned src_height)
{
  stretch(0, 0, width, height, src, src_x, src_y, src_width, src_height);
}

void
Canvas::copy_or(int dest_x, int dest_y,
                unsigned dest_width, unsigned dest_height,
                const Canvas &src, int src_x, int src_y)
{
  ::BitBlt(dc, dest_x, dest_y, dest_width, dest_height,
           src.dc, src_x, src_y, SRCPAINT);
}

void
Canvas::copy_and(int dest_x, int dest_y,
                 unsigned dest_width, unsigned dest_height,
                 const Canvas &src, int src_x, int src_y)
{
  ::BitBlt(dc, dest_x, dest_y, dest_width, dest_height,
           src.dc, src_x, src_y, SRCAND);
}

void
Canvas::stretch_or(int dest_x, int dest_y,
                   unsigned dest_width, unsigned dest_height,
                   const Canvas &src,
                   int src_x, int src_y,
                   unsigned src_width, unsigned src_height)
{
  ::StretchBlt(dc, dest_x, dest_y, dest_width, dest_height,
               src.dc, src_x, src_y, src_width, src_height,
               SRCPAINT);
}

void
Canvas::stretch_and(int dest_x, int dest_y,
                    unsigned dest_width, unsigned dest_height,
                    const Canvas &src,
                    int src_x, int src_y,
                    unsigned src_width, unsigned src_height)
{
  ::StretchBlt(dc, dest_x, dest_y, dest_width, dest_height,
               src.dc, src_x, src_y, src_width, src_height,
               SRCAND);
}