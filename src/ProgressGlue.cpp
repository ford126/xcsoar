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

#include "ProgressGlue.hpp"
#include "Screen/ProgressWindow.hpp"
#include "Interface.hpp"
#include "MainWindow.hpp"

static ProgressWindow *global_progress_window;

static void
handle_paint_messages()
{
  assert_none_locked();

#ifdef WIN32
  MSG msg;
  while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }
#endif
}

void
ProgressGlue::Create(const TCHAR *text)
{
  if (global_progress_window == NULL)
    global_progress_window = new ProgressWindow(XCSoarInterface::main_window);

  global_progress_window->set_message(text);
  global_progress_window->set_pos(0);

  handle_paint_messages();
}

void
ProgressGlue::Close()
{
  delete global_progress_window;
  global_progress_window = NULL;
}

void
ProgressGlue::Step()
{
  if (global_progress_window != NULL)
    global_progress_window->step();
}

void
ProgressGlue::SetValue(unsigned value)
{
  if (global_progress_window != NULL)
    global_progress_window->set_pos(value);
}

void
ProgressGlue::SetRange(unsigned value)
{
  if (global_progress_window != NULL)
    global_progress_window->set_range(0, value);
}

void
ProgressGlue::SetStep(int step)
{
  if (global_progress_window != NULL)
    global_progress_window->set_step(step);
}
