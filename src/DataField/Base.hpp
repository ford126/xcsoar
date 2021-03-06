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

#ifndef XCSOAR_DATA_FIELD_BASE_HPP
#define XCSOAR_DATA_FIELD_BASE_HPP

#include "Compiler.h"

#include <tchar.h>

#define FORMATSIZE 32
#define UNITSIZE 10
#define OUTBUFFERSIZE 128

class ComboList;

class DataField
{
public:
  typedef enum
  {
    daChange,
    daInc,
    daDec,
    daSpecial,
  } DataAccessKind_t;

  typedef void (*DataAccessCallback_t)(DataField * Sender, DataAccessKind_t Mode);

  DataField(const TCHAR *EditFormat, const TCHAR *DisplayFormat,
            DataAccessCallback_t OnDataAccess = NULL);
  virtual ~DataField(void) {}

  void Special(void);
  virtual void Inc(void);
  virtual void Dec(void);

  gcc_pure
  virtual int GetAsInteger() const;

  gcc_pure
  virtual const TCHAR *GetAsString() const;

  gcc_pure
  virtual const TCHAR *GetAsDisplayString() const;

  virtual void SetAsInteger(int Value);
  virtual void SetAsString(const TCHAR *Value);

  void SetUnits(const TCHAR *text) { _tcscpy(mUnits, text); }

  void
  Use(void)
  {
    mUsageCounter++;
  }

  int
  Unuse(void)
  {
    mUsageCounter--;
    return mUsageCounter;
  }

  void SetDisplayFormat(TCHAR *Value);
  // allows combolist to iterate all values
  void SetDisableSpeedUp(bool bDisable) { mDisableSpeedup = bDisable; }
  bool GetDisableSpeedUp(void) { return mDisableSpeedup; }
  // allows combolist to iterate all values w/out triggering external events
  void SetDetachGUI(bool bDetachGUI) { mDetachGUI = bDetachGUI; }
  bool GetDetachGUI(void) { return mDetachGUI; }

  gcc_malloc
  virtual ComboList *CreateComboList() const { return NULL; }

  virtual void
  SetFromCombo(int iDataFieldIndex, TCHAR *sValue)
  {
    SetAsInteger(iDataFieldIndex);
  }

  void CopyString(TCHAR * szStringOut, bool bFormatted);
  bool SupportCombo;  // all Types dataField support combolist except DataFieldString.

protected:
  DataAccessCallback_t mOnDataAccess;
  TCHAR mEditFormat[FORMATSIZE + 1];
  TCHAR mDisplayFormat[FORMATSIZE + 1];
  TCHAR mUnits[UNITSIZE + 1];

private:
  int mUsageCounter;
  bool mDisableSpeedup;
  bool mDetachGUI;
};

#endif
