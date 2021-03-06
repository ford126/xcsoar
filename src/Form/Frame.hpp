/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
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

#ifndef XCSOAR_FORM_FRAME_HPP
#define XCSOAR_FORM_FRAME_HPP

#include "Screen/PaintWindow.hpp"

class WndFrame : public PaintWindow {
  Color background_color;
  const Font *font;
  TCHAR text[256];

public:
  WndFrame(ContainerWindow &parent,
           int X, int Y, int Width, int Height,
           Color background_color,
           const WindowStyle style=WindowStyle());

  void SetAlignCenter();

  void SetFont(const Font &_font) {
    font = &_font;
    invalidate();
  }

  void SetText(const TCHAR *_text);

  const TCHAR *GetCaption() const {
    return text;
  }

  void SetCaption(const TCHAR *_text) {
    SetText(_text);
  }

  unsigned GetTextHeight();

protected:
  unsigned mCaptionStyle;

  /** from class PaintWindow */
  virtual void on_paint(Canvas &canvas);
};

#endif
