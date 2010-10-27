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

#ifndef XCSOAR_SCREEN_OPENGL_HPP
#define XCSOAR_SCREEN_OPENGL_HPP

#include <SDL.h>

#ifdef ANDROID
#include <GLES/gl.h>
#else
#include <SDL_opengl.h>
#endif

/**
 * This class represents an OpenGL texture.
 */
class GLTexture {
  GLuint id;

public:
  GLTexture() {
    init();
  }

  GLTexture(SDL_Surface *surface) {
    init();
    load(surface);
  }

  ~GLTexture() {
    glDeleteTextures(1, &id);
  }

protected:
  void init() {
    glGenTextures(1, &id);
    bind();
    configure();
  }

  static inline void configure() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  static void load(SDL_Surface *surface);

public:
  void bind() {
    glBindTexture(GL_TEXTURE_2D, id);
  }

  static void draw(int x_offset, int y_offset,
                   int dest_x, int dest_y,
                   unsigned dest_width, unsigned dest_height,
                   int src_x, int src_y,
                   unsigned src_width, unsigned src_height,
                   unsigned src_full_width, unsigned src_full_height);
};

#endif