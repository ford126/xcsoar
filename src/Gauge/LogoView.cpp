/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

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

#include "LogoView.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/BitmapCanvas.hpp"
#include "resource.h"
#include "Version.hpp"

void
DrawLogo(Canvas &canvas, const RECT &rc)
{
  const unsigned width = rc.right - rc.left, height = rc.bottom - rc.top;

  // Load logo
  Bitmap bitmap_logo((width >= 290 && height >= 170) ||
                     (width >= 170 && height >= 210)
                     ? IDB_SWIFT_HD : IDB_SWIFT);

  // Adjust the title to larger screens
  Bitmap bitmap_title((width >= 530 && height >= 60) ||
                      (width >= 170 && height >= 250)
                      ? IDB_TITLE_HD : IDB_TITLE);

  BitmapCanvas bitmap_canvas(canvas);

  // Determine logo size
  SIZE logo_size = bitmap_logo.get_size();

  // Determine title image size
  SIZE title_size = bitmap_title.get_size();

  int logox, logoy, titlex, titley;

  bool hidetitle = false;

  // Determine logo and title positions
  if ((unsigned)(logo_size.cx + title_size.cy + title_size.cx) <= width) {
    // Landscape
    logox = (width - (logo_size.cx + title_size.cy + title_size.cx)) / 2;
    logoy = (height - logo_size.cy) / 2;
    titlex = logox + logo_size.cx + title_size.cy;
    titley = (height - title_size.cy) / 2;
  } else if ((unsigned)(logo_size.cy + title_size.cy * 2) <= height) {
    // Portrait
    logox = (width - logo_size.cx) / 2;
    logoy = (height - (logo_size.cy + title_size.cy * 2)) / 2;
    titlex = (width - title_size.cx) / 2;
    titley = logoy + logo_size.cy + title_size.cy;
  } else {
    // Square screen
    logox = (width - logo_size.cx) / 2;
    logoy = (height - logo_size.cy) / 2;
    hidetitle = true;
  }

  // Draw 'XCSoar N.N' title
  if (!hidetitle){
    bitmap_canvas.select(bitmap_title);
    canvas.copy(titlex, titley, title_size.cx, title_size.cy, bitmap_canvas, 0, 0);
  }

  // Draw XCSoar swift logo
  bitmap_canvas.select(bitmap_logo);
  canvas.copy(logox, logoy, logo_size.cx, logo_size.cy, bitmap_canvas, 0, 0);

  // Draw full XCSoar version number
  canvas.set_text_color(Color::BLACK);
  canvas.background_transparent();
  canvas.text(2, 2, XCSoar_ProductToken);
}