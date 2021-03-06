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

#ifndef XCSOAR_OS_FILEUTIL_HPP
#define XCSOAR_OS_FILEUTIL_HPP

#include <tchar.h>

#ifdef HAVE_POSIX
#include <unistd.h>
#include <stdio.h>
#else
#include <windows.h>
#endif

namespace Directory
{
  bool Exists(const TCHAR* path);
  void Create(const TCHAR* path);
}

namespace File
{
  bool Exists(const TCHAR* path);

  static inline bool
  Delete(const TCHAR *path)
  {
#ifdef HAVE_POSIX
    return unlink(path) == 0;
#else
    return DeleteFile(path);
#endif
  }

  static inline bool
  Rename(const TCHAR *oldpath, const TCHAR *newpath)
  {
#ifdef HAVE_POSIX
    /* XXX handle EXDEV; cross-filesystem moves are not supported by
       rename() */
    return rename(oldpath, newpath) == 0;
#else
    return MoveFile(oldpath, newpath) != 0;
#endif
  }
}

#endif
