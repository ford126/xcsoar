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

#include "org_xcsoar_EventBridge.h"
#include "Screen/Android/Event.hpp"
#include "Android/Main.hpp"

void
Java_org_xcsoar_EventBridge_onKeyDown(JNIEnv *env, jclass cls, jint key_code)
{
  event_queue->push(Event(Event::KEY_DOWN, key_code));
}

void
Java_org_xcsoar_EventBridge_onKeyUp(JNIEnv *env, jclass cls, jint key_code)
{
  event_queue->push(Event(Event::KEY_UP, key_code));
}

void
Java_org_xcsoar_EventBridge_onMouseDown(JNIEnv *env, jclass cls,
                                        jint x, jint y)
{
  event_queue->push(Event(Event::MOUSE_DOWN, x, y));
}

void
Java_org_xcsoar_EventBridge_onMouseUp(JNIEnv *env, jclass cls,
                                      jint x, jint y)
{
  event_queue->push(Event(Event::MOUSE_UP, x, y));
}

void
Java_org_xcsoar_EventBridge_onMouseMove(JNIEnv *env, jclass cls,
                                        jint x, jint y)
{
  event_queue->push(Event(Event::MOUSE_MOTION, x, y));
}
