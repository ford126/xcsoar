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

#include "Dialogs/Internal.hpp"
#include "Engine/Task/TaskEvents.hpp"
#include "Engine/Task/Factory/AbstractTaskFactory.hpp"
#include "Protection.hpp"
#include "Math/Earth.hpp"
#include "Profile/Profile.hpp"
#include "LocalTime.hpp"
#include "UtilsText.hpp"
#include "OS/PathName.hpp"
#include "Math/SunEphemeris.hpp"
#include "Blackboard.hpp"
#include "SettingsComputer.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Key.h"
#include "Math/FastMath.h"
#include "MainWindow.hpp"
#include "Components.hpp"
#include "DrawThread.hpp"
#include "GlideSolvers/GlidePolar.hpp"
#include "Task/TaskManager.hpp"
#include "Task/Tasks/TaskSolvers/TaskSolution.hpp"
#include "Task/Tasks/BaseTask/UnorderedTaskPoint.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "WayPoint/WayPointGlue.hpp"
#include "Compiler.h"
#include "Compatibility/path.h"

#include <assert.h>
#include <stdio.h>

#include <memory>

enum task_edit_result {
  SUCCESS,
  UNMODIFIED,
  INVALID,
};

static int page = 0;
static WndForm *wf = NULL;
static WndListFrame *wDetails = NULL;
static WndFrame *wInfo = NULL;
static WndFrame *wCommand = NULL;
static WndOwnerDrawFrame *wImage = NULL;
static bool hasimage1 = false;
static bool hasimage2 = false;
static const Waypoint *selected_waypoint = NULL;

static Bitmap jpgimage1, jpgimage2;

static TCHAR path_modis[MAX_PATH];
static TCHAR path_google[MAX_PATH];
static TCHAR szWaypointFile[MAX_PATH];

#define MAXLINES 100
static int LineOffsets[MAXLINES];
static unsigned nTextLines = 0;

static void
NextPage(int Step)
{
  assert(selected_waypoint);
  bool page_ok = false;
  page += Step;

  do {
    if (page < 0)
      page = 4;
    if (page > 4)
      page = 0;

    switch (page) {
    case 0:
      page_ok = true;
      break;

    case 1:
      if (selected_waypoint->Details.empty()) 
        page += Step;
      else
        page_ok = true;

      break;

    case 2:
      page_ok = true;
      break;

    case 3:
      if (!hasimage1)
        page += Step;
      else
        page_ok = true;

      break;

    case 4:
      if (!hasimage2)
        page += Step;
      else
        page_ok = true;

      break;

    default:
      page_ok = true;
      page = 0;
      break;
      // error!
    }
  } while (!page_ok);

  wInfo->set_visible(page == 0);
  wDetails->set_visible(page == 1);
  wCommand->set_visible(page == 2);
  wImage->set_visible(page >= 3);
}

static void
OnPaintDetailsListItem(Canvas &canvas, const RECT rc, unsigned DrawListIndex)
{
  assert(selected_waypoint);
  assert(DrawListIndex < nTextLines);

  const TCHAR* text = selected_waypoint->Details.c_str();
  int nstart = LineOffsets[DrawListIndex];
  int nlen;
  if (DrawListIndex < nTextLines - 1) {
    nlen = LineOffsets[DrawListIndex + 1] - LineOffsets[DrawListIndex];
    nlen--;
  } else {
    nlen = _tcslen(text + nstart) - 1;
  }

  if (nlen > 0)
    canvas.text(rc.left + Layout::FastScale(2), rc.top + Layout::FastScale(2),
                text + nstart, nlen);
}

static void
OnNextClicked(gcc_unused WndButton &button)
{
  NextPage(+1);
}

static void
OnPrevClicked(gcc_unused WndButton &button)
{
  NextPage(-1);
}

static void
OnCloseClicked(gcc_unused WndButton &button)
{
  wf->SetModalResult(mrOK);
}

static bool
FormKeyDown(WndForm &Sender, unsigned key_code)
{
  (void)Sender;
  switch (key_code) {
  case VK_LEFT:
#ifdef GNAV
  case '6':
#endif
    ((WndButton *)wf->FindByName(_T("cmdPrev")))->set_focus();
    NextPage(-1);
    return true;

  case VK_RIGHT:
#ifdef GNAV
  case '7':
#endif
    ((WndButton *)wf->FindByName(_T("cmdNext")))->set_focus();
    NextPage(+1);
    return true;

  default:
    return false;
  }
}

static void
OnGotoClicked(gcc_unused WndButton &button)
{
  if (protected_task_manager == NULL)
    return;

  assert(selected_waypoint != NULL);

  protected_task_manager->do_goto(*selected_waypoint);
  wf->SetModalResult(mrOK);

#ifdef ENABLE_OPENGL
  CommonInterface::main_window.map.invalidate();
#else
  draw_thread->trigger_redraw();
#endif
}

static task_edit_result
replace_in_task(const Waypoint &wp)
{
  assert(protected_task_manager != NULL);
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  TaskEvents task_events;
  GlidePolar glide_polar(task_manager->get_glide_polar());
  std::auto_ptr<OrderedTask> task(task_manager->clone(task_events,
                                                      XCSoarInterface::SettingsComputer(),
                                                      glide_polar));
  task->check_duplicate_waypoints(way_points);

  unsigned i = task->getActiveIndex();
  if (i >= task->task_size())
    return UNMODIFIED;

  task->relocate(i, wp);

  if (!task->check_task())
    return INVALID;

  task_manager->commit(*task);
  return SUCCESS;
}

static void
OnReplaceClicked(gcc_unused WndButton &button)
{
  if (protected_task_manager == NULL)
    return;

  switch (replace_in_task(*selected_waypoint)) {
  case SUCCESS:
    protected_task_manager->task_save_default();
    wf->SetModalResult(mrOK);
    break;

  case UNMODIFIED:
    MessageBoxX(_("No active task point."), _("Replace in task"),
                MB_OK | MB_ICONINFORMATION);
    break;

  case INVALID:
    MessageBoxX(_("Task would not be valid after the change."), _("Error"),
                MB_OK | MB_ICONEXCLAMATION);
    break;
  }
}

static void 
OnNewHomeClicked(gcc_unused WndButton &button)
{
  assert(selected_waypoint != NULL);

  XCSoarInterface::SetSettingsComputer().SetHome(*selected_waypoint);

  {
    ScopeSuspendAllThreads suspend;
    WayPointGlue::SetHome(way_points, terrain,
                          XCSoarInterface::SetSettingsComputer(),
                          false, false);
  }

  wf->SetModalResult(mrOK);
}

static void
OnTeamCodeClicked(gcc_unused WndButton &button)
{
  assert(selected_waypoint != NULL);

  XCSoarInterface::SetSettingsComputer().TeamCodeRefWaypoint =
      selected_waypoint->id;
  Profile::Set(szProfileTeamcodeRefWaypoint,
      XCSoarInterface::SettingsComputer().TeamCodeRefWaypoint);

  wf->SetModalResult(mrOK);
}

static task_edit_result
insert_in_task(const Waypoint &wp)
{
  assert(protected_task_manager != NULL);
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  TaskEvents task_events;
  GlidePolar glide_polar(task_manager->get_glide_polar());
  std::auto_ptr<OrderedTask> task(task_manager->clone(task_events,
                                                      XCSoarInterface::SettingsComputer(),
                                                      glide_polar));

  int i = task->getActiveIndex();
  /* skip all start points */
  while (true) {
    if (i >= (int)task->task_size())
      return UNMODIFIED;

    const TaskPoint *tp = task->get_tp(i);
    if (tp == NULL || tp->is_intermediate())
      break;

    ++i;
  }

  const AbstractTaskFactory &factory = task->get_factory();
  OrderedTaskPoint *tp = (OrderedTaskPoint *)factory.createIntermediate(wp);
  if (tp == NULL)
    return UNMODIFIED;

  bool success = task->insert(*tp, i);
  delete tp;
  if (!success)
    return UNMODIFIED;

  if (!task->check_task())
    return INVALID;

  task_manager->commit(*task);
  return SUCCESS;
}

static void
OnInsertInTaskClicked(gcc_unused WndButton &button)
{
  if (protected_task_manager == NULL)
    return;

  switch (insert_in_task(*selected_waypoint)) {
  case SUCCESS:
    protected_task_manager->task_save_default();
    wf->SetModalResult(mrOK);
    break;

  case UNMODIFIED:
  case INVALID:
    MessageBoxX(_("Task would not be valid after the change."), _("Error"),
                MB_OK | MB_ICONEXCLAMATION);
    break;
  }
}

static task_edit_result
append_to_task(const Waypoint &wp)
{
  assert(protected_task_manager != NULL);
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  TaskEvents task_events;
  GlidePolar glide_polar(task_manager->get_glide_polar());
  std::auto_ptr<OrderedTask> task(task_manager->clone(task_events,
                                                      XCSoarInterface::SettingsComputer(),
                                                      glide_polar));

  int i = task->task_size() - 1;
  /* skip all finish points */
  while (i >= 0) {
    const OrderedTaskPoint *tp = task->get_tp(i);
    if (tp == NULL)
      break;

    if (tp->successor_allowed()) {
      ++i;
      break;
    }

    --i;
  }

  const AbstractTaskFactory &factory = task->get_factory();
  OrderedTaskPoint *tp = (OrderedTaskPoint *)factory.createIntermediate(wp);
  if (tp == NULL)
    return UNMODIFIED;

  bool success = i >= 0 ? task->insert(*tp, i) : task->append(*tp);
  delete tp;

  if (!success)
    return UNMODIFIED;

  if (!task->check_task())
    return INVALID;

  task_manager->commit(*task);
  return SUCCESS;
}

static void
OnAppendInTaskClicked(gcc_unused WndButton &button)
{
  if (protected_task_manager == NULL)
    return;

  switch (append_to_task(*selected_waypoint)) {
  case SUCCESS:
    protected_task_manager->task_save_default();
    wf->SetModalResult(mrOK);
    break;

  case UNMODIFIED:
  case INVALID:
    MessageBoxX(_("Task would not be valid after the change."), _("Error"),
                MB_OK | MB_ICONEXCLAMATION);
    break;
  }
}

static task_edit_result
remove_from_task(const Waypoint &wp)
{
  assert(protected_task_manager != NULL);
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  TaskEvents task_events;
  GlidePolar glide_polar(task_manager->get_glide_polar());
  std::auto_ptr<OrderedTask> task(task_manager->clone(task_events,
                                                      XCSoarInterface::SettingsComputer(),
                                                      glide_polar));
  task->check_duplicate_waypoints(way_points);

  bool modified = false;
  for (unsigned i = task->task_size(); i--;) {
    const OrderedTaskPoint *tp = task->get_tp(i);
    assert(tp != NULL);

    if (tp->get_waypoint() == wp) {
      task->remove(i);
      modified = true;
    }
  }

  if (!modified)
    return UNMODIFIED;

  if (!task->check_task())
    return INVALID;

  task_manager->commit(*task);
  return SUCCESS;
}

static void
OnRemoveFromTaskClicked(gcc_unused WndButton &button)
{
  if (protected_task_manager == NULL)
    return;

  switch (remove_from_task(*selected_waypoint)) {
  case SUCCESS:
    protected_task_manager->task_save_default();
    wf->SetModalResult(mrOK);
    break;

  case UNMODIFIED:
    MessageBoxX(_("Waypoint not in task."), _("Remove from task"),
                MB_OK | MB_ICONINFORMATION);
    break;

  case INVALID:
    MessageBoxX(_("Task would not be valid after the change."), _("Error"),
                MB_OK | MB_ICONEXCLAMATION);
    break;
  }
}

static void
OnImagePaint(WndOwnerDrawFrame *Sender, Canvas &canvas)
{
  (void)Sender;

  canvas.clear_white();
  if (page == 3) {
    canvas.copy(jpgimage1);
  } else if (page == 4) {
    canvas.copy(jpgimage2);
  }
}

static CallBackTableEntry CallBackTable[] = {
    DeclareCallBackEntry(OnNextClicked),
    DeclareCallBackEntry(OnPrevClicked),
    DeclareCallBackEntry(NULL)
};

void 
dlgWayPointDetailsShowModal(SingleWindow &parent, const Waypoint& way_point)
{
  selected_waypoint = &way_point;

  wf = LoadDialog(CallBackTable, parent,
                  Layout::landscape ? _T("IDR_XML_WAYPOINTDETAILS_L") :
                                      _T("IDR_XML_WAYPOINTDETAILS"));
  assert(wf != NULL);

  nTextLines = 0;

  TCHAR buffer[MAX_PATH];
  const TCHAR *Directory = NULL;
  if (Profile::GetPath(szProfileWayPointFile, szWaypointFile))
    Directory = DirName(szWaypointFile, buffer);
  if (Directory == NULL)
    Directory = _T("");

  _stprintf(path_modis, _T("%s" DIR_SEPARATOR_S "modis-%03d.jpg"),
           Directory,
            selected_waypoint->original_id);
  _stprintf(path_google,_T("%s" DIR_SEPARATOR_S "google-%03d.jpg"),
           Directory,
            selected_waypoint->original_id);

  TCHAR sTmp[128];
  _stprintf(sTmp, _T("%s: '%s'"), wf->GetCaption(), selected_waypoint->Name.c_str());
  wf->SetCaption(sTmp);

  WndProperty *wp;
  wp = ((WndProperty *)wf->FindByName(_T("prpWpComment")));
  wp->SetText(selected_waypoint->Comment.c_str());

  Units::LongitudeToString(selected_waypoint->Location.Longitude, sTmp, sizeof(sTmp)-1);
  ((WndProperty *)wf->FindByName(_T("prpLongitude")))
    ->SetText(sTmp);

  Units::LatitudeToString(selected_waypoint->Location.Latitude, sTmp, sizeof(sTmp)-1);
  ((WndProperty *)wf->FindByName(_T("prpLatitude")))
    ->SetText(sTmp);

  Units::FormatUserAltitude(selected_waypoint->Altitude, sTmp, sizeof(sTmp)-1);
  ((WndProperty *)wf->FindByName(_T("prpAltitude")))
    ->SetText(sTmp);

  SunEphemeris sun;
  sun.CalcSunTimes(selected_waypoint->Location,
                   XCSoarInterface::Basic().DateTime,
                   fixed(GetUTCOffset()) / 3600);

  int sunsethours = (int)sun.TimeOfSunSet;
  int sunsetmins = (int)((sun.TimeOfSunSet - fixed(sunsethours)) * 60);

  _stprintf(sTmp, _T("%02d:%02d"), sunsethours, sunsetmins);
  ((WndProperty *)wf->FindByName(_T("prpSunset")))->SetText(sTmp);

  GeoVector gv = XCSoarInterface::Basic().Location.
                 distance_bearing(selected_waypoint->Location);

  TCHAR DistanceText[MAX_PATH];
  Units::FormatUserDistance(gv.Distance, DistanceText, 10);
  ((WndProperty *)wf->FindByName(_T("prpDistance"))) ->SetText(DistanceText);

  _stprintf(sTmp, _T("%d")_T(DEG), iround(gv.Bearing.value_degrees()));
  ((WndProperty *)wf->FindByName(_T("prpBearing"))) ->SetText(sTmp);

  if (protected_task_manager != NULL) {
    GlidePolar glide_polar = protected_task_manager->get_glide_polar();
    GlidePolar safety_polar = protected_task_manager->get_safety_polar();

    UnorderedTaskPoint t(way_point, XCSoarInterface::SettingsComputer());
    GlideResult r;

    // alt reqd at current mc

    const AIRCRAFT_STATE aircraft_state =
      ToAircraftState(XCSoarInterface::Basic());
    r = TaskSolution::glide_solution_remaining(t, aircraft_state, glide_polar);
    wp = (WndProperty *)wf->FindByName(_T("prpMc2"));
    if (wp) {
      _stprintf(sTmp, _T("%.0f %s"),
                (double)Units::ToUserAltitude(r.AltitudeDifference),
                Units::GetAltitudeName());
      wp->SetText(sTmp);
    }

    // alt reqd at mc 0

    glide_polar.set_mc(fixed_zero);
    r = TaskSolution::glide_solution_remaining(t, aircraft_state, glide_polar);
    wp = (WndProperty *)wf->FindByName(_T("prpMc0"));
    if (wp) {
      _stprintf(sTmp, _T("%.0f %s"),
                (double)Units::ToUserAltitude(r.AltitudeDifference),
                Units::GetAltitudeName());
      wp->SetText(sTmp);
    }

    // alt reqd at safety mc

    r = TaskSolution::glide_solution_remaining(t, aircraft_state, safety_polar);
    wp = (WndProperty *)wf->FindByName(_T("prpMc1"));
    if (wp) {
      _stprintf(sTmp, _T("%.0f %s"),
                (double)Units::ToUserAltitude(r.AltitudeDifference),
                Units::GetAltitudeName());
      wp->SetText(sTmp);
    }
  }

  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->FindByName(_T("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  wInfo = ((WndFrame *)wf->FindByName(_T("frmInfos")));
  wCommand = ((WndFrame *)wf->FindByName(_T("frmCommands")));
  wImage = ((WndOwnerDrawFrame *)wf->FindByName(_T("frmImage")));
  wDetails = (WndListFrame*)wf->FindByName(_T("frmDetails"));
  wDetails->SetPaintItemCallback(OnPaintDetailsListItem);

  assert(wInfo != NULL);
  assert(wCommand != NULL);
  assert(wImage != NULL);
  assert(wDetails != NULL);

  nTextLines = TextToLineOffsets(way_point.Details.c_str(), LineOffsets, MAXLINES);
  wDetails->SetLength(nTextLines);

  wCommand->hide();
  wImage->SetOnPaintNotify(OnImagePaint);

  WndButton *wb;

  wb = ((WndButton *)wf->FindByName(_T("cmdGoto")));
  if (wb)
    wb->SetOnClickNotify(OnGotoClicked);

  wb = ((WndButton *)wf->FindByName(_T("cmdReplace")));
  if (wb)
    wb->SetOnClickNotify(OnReplaceClicked);

  wb = ((WndButton *)wf->FindByName(_T("cmdNewHome")));
  if (wb)
    wb->SetOnClickNotify(OnNewHomeClicked);

  wb = ((WndButton *)wf->FindByName(_T("cmdTeamCode")));
  if (wb)
    wb->SetOnClickNotify(OnTeamCodeClicked);

  wb = ((WndButton *)wf->FindByName(_T("cmdInserInTask")));
  if (wb)
    wb->SetOnClickNotify(OnInsertInTaskClicked);

  wb = ((WndButton *)wf->FindByName(_T("cmdAppendInTask")));
  if (wb)
    wb->SetOnClickNotify(OnAppendInTaskClicked);

  wb = ((WndButton *)wf->FindByName(_T("cmdRemoveFromTask")));
  if (wb)
    wb->SetOnClickNotify(OnRemoveFromTaskClicked);

  hasimage1 = jpgimage1.load_file(path_modis);
  hasimage2 = jpgimage2.load_file(path_google);

  page = 0;

  NextPage(0); // JMW just to turn proper pages on/off

  wf->ShowModal();

  delete wf;
}
