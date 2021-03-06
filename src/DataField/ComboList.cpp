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

#include "DataField/ComboList.hpp"

#include <stdlib.h>
#include <string.h>

ComboList::Item::Item(int _DataFieldIndex,
                        const TCHAR *_StringValue,
                        const TCHAR *_StringValueFormatted)
  :DataFieldIndex(_DataFieldIndex),
   StringValue(_tcsdup(_StringValue)),
   StringValueFormatted(_tcsdup(_StringValueFormatted)) {}

ComboList::Item::~Item() {
  free(StringValue);
  free(StringValueFormatted);
}

void
ComboList::Clear()
{
  for (unsigned i = 0; i < items.size(); i++)
    delete items[i];

  items.clear();
}

unsigned
ComboList::Append(ComboList::Item *item)
{
  unsigned i = items.size();
  items.append(item);
  return i;
}
