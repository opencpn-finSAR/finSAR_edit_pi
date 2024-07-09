/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  finSAR_edit Object
 * Author:   Mike Rossiter
 *
 ***************************************************************************
 *   Copyright (C) 2016 by Mike Rossiter  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 */
#include <wx/intl.h>
#include "wx/wx.h"

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/debug.h>
#include <wx/graphics.h>
#include <wx/stdpaths.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "finSAR_edit_pi.h"
#include "icons.h"
#include <wx/arrimpl.cpp>

#ifdef __WXMSW__
#include <windows.h>
#endif
#include <memory.h>

#include <wx/colordlg.h>
#include <wx/event.h>
#include "pugixml.hpp"

class ConfigurationDialog;
class RouteProp;
class Position;

using namespace std;
using namespace pugi;

#define FAIL(X)  \
  do {           \
    error = X;   \
    goto failed; \
  } while (0)

static double deg2rad(double degrees) { return M_PI * degrees / 180.0; }
static double rad2deg(double radians) { return 180.0 * radians / M_PI; }

GetRouteDialog::GetRouteDialog(wxWindow* parent, wxWindowID id,
                               const wxString& title, const wxPoint& position,
                               const wxSize& size, long style)
    : wxDialog(parent, id, title, position, size, style) {
  wxString dimensions = wxT(""), s;
  wxPoint p;
  wxSize sz;

  sz.SetWidth(size.GetWidth() - 20);
  sz.SetHeight(size.GetHeight() - 70);

  p.x = 6;
  p.y = 2;

  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxVERTICAL);

  dialogText =
      new wxListView(this, wxID_ANY, p, sz, wxLC_NO_HEADER | wxLC_REPORT,
                     wxDefaultValidator, wxT(""));
  wxFont* pVLFont = wxTheFontList->FindOrCreateFont(
      12, wxFONTFAMILY_SWISS, wxNORMAL, wxFONTWEIGHT_NORMAL, FALSE,
      wxString(_T("Arial")));
  dialogText->SetFont(*pVLFont);

  bSizer1->Add(dialogText, 0, wxALL, 5);

  m_sdbSizer1 = new wxStdDialogButtonSizer();
  m_sdbSizer1OK = new wxButton(this, wxID_OK);
  m_sdbSizer1->AddButton(m_sdbSizer1OK);
  m_sdbSizer1Cancel = new wxButton(this, wxID_CANCEL);
  m_sdbSizer1->AddButton(m_sdbSizer1Cancel);
  m_sdbSizer1->Realize();

  bSizer1->Add(m_sdbSizer1, 1, wxALIGN_CENTER, 5);

  m_sdbSizer1OK->SetDefault();

  this->SetSizer(bSizer1);
  this->Layout();
  bSizer1->Fit(this);
}

GetRouteDialog::~GetRouteDialog() {}

finSAR_editUIDialog::finSAR_editUIDialog(wxWindow* parent, finSAR_edit_pi* ppi)
    : finSAR_editUIDialogBase(parent),
      m_ConfigurationDialog(this, wxID_ANY, _("Tidal Routes"),
                            wxDefaultPosition, wxSize(-1, -1),
                            wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
  pParent = parent;
  pPlugIn = ppi;
  m_bBearingLine = false;

  wxFileConfig* pConf = GetOCPNConfigObject();

  if (pConf) {
    pConf->SetPath(_T ( "/PlugIns/finSAR_edit" ));

    pConf->Read(_T("finSAR_editFolder"), &m_FolderSelected);
  }

  // m_dirPicker1->GetTextCtrlValue();

  // wxMessageBox(m_FolderSelected);

  this->Connect(wxEVT_MOVE, wxMoveEventHandler(finSAR_editUIDialog::OnMove));

  m_dtNow = wxDateTime::Now();

  wxString initStartDate = m_dtNow.Format(_T("%Y-%m-%d  %H:%M"));

  Fit();
  SetMinSize(GetBestSize());

  for (int z = 0; z < 10; z++) {
    for (int fz = 0; fz < 10; fz++) {
      my_files[z][fz].Add("0");
    }
  }

  id_wpt = wxEmptyString;
}

finSAR_editUIDialog::~finSAR_editUIDialog() {
  wxFileConfig* pConf = GetOCPNConfigObject();
  ;

  if (pConf) {
    pConf->SetPath(_T ( "/PlugIns/finSAR_edit" ));
  }

  // SaveXML(m_default_configuration_path);
}

void finSAR_editUIDialog::SetCursorLatLon(double lat, double lon) {
  m_cursor_lon = lon;
  m_cursor_lat = lat;
}

void finSAR_editUIDialog::SetViewPort(PlugIn_ViewPort* vp) {
  if (m_vp == vp) return;

  m_vp = new PlugIn_ViewPort(*vp);
  m_bMoveUpDownLeftRight = false;
  centreLat = m_vp->clat;
  centreLon = m_vp->clon;
  // RequestRefresh(pParent);
}

void finSAR_editUIDialog::OnClose(wxCloseEvent& event) {
  pPlugIn->OnfinSAR_editDialogClose();
}

void finSAR_editUIDialog::OnMove(wxMoveEvent& event) {
  //    Record the dialog position
  wxPoint p = GetPosition();
  pPlugIn->SetfinSAR_editDialogX(p.x);
  pPlugIn->SetfinSAR_editDialogY(p.y);

  event.Skip();
}

void finSAR_editUIDialog::OnSize(wxSizeEvent& event) {
  //    Record the dialog size
  wxSize p = event.GetSize();
  pPlugIn->SetfinSAR_editDialogSizeX(p.x);
  pPlugIn->SetfinSAR_editDialogSizeY(p.y);

  event.Skip();
}

wxString finSAR_editUIDialog::MakeDateTimeLabel(wxDateTime myDateTime) {
  wxDateTime dt = myDateTime;

  wxString s2 = dt.Format(_T( "%Y-%m-%d"));
  wxString s = dt.Format(_T("%H:%M"));
  wxString dateLabel = s2 + _T(" ") + s;

  return dateLabel;
}

void finSAR_editUIDialog::OnInformation(wxCommandEvent& event) {}

void finSAR_editUIDialog::GetTable(wxString myRoute) {}

void finSAR_editUIDialog::AddChartRoute(wxString myRoute) {}

void finSAR_editUIDialog::OnReadRTZ(wxCommandEvent& event) { ReadRTZ(); }

int finSAR_editUIDialog::GetRandomNumber(int range_min, int range_max) {
  long u = (long)wxRound(
      ((double)rand() / ((double)(RAND_MAX) + 1) * (range_max - range_min)) +
      range_min);
  return (int)u;
}

bool finSAR_editUIDialog::OpenXML(wxString filename, bool reportfailure) {
  return false;
}

void finSAR_editUIDialog::SaveXML() {
  // Create Main level XML container
  xml_document xmlDoc;

  auto declarationNode = xmlDoc.append_child(node_declaration);

  declarationNode.append_attribute("version") = "1.0";

  declarationNode.append_attribute("encoding") = "UTF-8";

  // Create XML root node called animals
  xml_node pRoot = xmlDoc.append_child("targets");

  // ************* Add routeInfo to root node *******

  // xml_node routeInfo = pRoot.append_child("routeInfo");
  // routeInfo.append_attribute("routeName").set_value(route_name.mb_str());

  // Insert cat's name as first child of animal

  // ************* Add waypoints *******
  // xml_node targetpoints = pRoot.append_child("target_points");

  int idn = 0;
  wxString route;

  for (std::vector<IndexTarget>::iterator itOut = i_vector.begin();
       itOut != i_vector.end(); itOut++) {
    xml_node m_targetpoint = pRoot.append_child("index_target");
    route = (*itOut).route_name;

    // wxString myIdn = wxString::Format("%i", idn);
    m_targetpoint.append_attribute("route_name").set_value(route.mb_str());
    m_targetpoint.append_attribute("wp_id").set_value((*itOut).wpId.mb_str());

    xml_node b_position = m_targetpoint.append_child("begin");

    wxString bLat = wxString::Format("%f", (*itOut).beginLat);
    wxString bLon = wxString::Format("%f", (*itOut).beginLon);

    b_position.append_attribute("lat").set_value(bLat);
    b_position.append_attribute("lon").set_value(bLon);

    xml_node e_position = m_targetpoint.append_child("end");

    wxString eLat = wxString::Format("%f", (*itOut).endLat);
    wxString eLon = wxString::Format("%f", (*itOut).endLon);

    e_position.append_attribute("lat").set_value(eLat);
    e_position.append_attribute("lon").set_value(eLon);

    idn++;
  }
  // done adding waypoints
  // Write xmlDoc into a file

  wxFileDialog dlg(this, _("Save in XML format"), wxEmptyString, route,
                   " XML files(*.xml) | *.xml;*XML",
                   wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dlg.ShowModal() == wxID_OK) {
    if (dlg.GetPath() != wxEmptyString) {
      wxString file_name = route + ".xml";
      // dlg.GetFilename();
      wxString file_path = dlg.GetPath();

      // Route name must be the same as the file name, without file extension

      int fl = file_name.length();
      wxString xml_name = file_name.SubString(0, (fl - 5));

      wxMessageBox(xml_name);

      if (route != xml_name) {
        wxMessageBox(_("XML file name must be the same as route name"),
                     "Error");
        return;
      }

      xmlDoc.save_file(file_path.mb_str());
      return;
    } else
      return;
  }
}

void finSAR_editUIDialog::SelectRoutePoints(wxString routeName) {}

wxString finSAR_editUIDialog::SelectRoute(bool isDR) { return ""; }

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:: This function converts decimal degrees to radians :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double finSAR_editUIDialog::deg2rad(double deg) { return (deg * pi / 180); }
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:: This function converts radians to decimal degrees :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double finSAR_editUIDialog::rad2deg(double rad) { return (rad * 180 / pi); };

struct xml_string_writer : pugi::xml_writer {
  std::string result;

  virtual void write(const void* data, size_t size) {
    result.append(static_cast<const char*>(data), size);
  }
};

//////////////////////////////////////////
//////////////////////////////////////////
//////////////////////////////////////////

std::string InnerXML(pugi::xml_node target) {
  xml_string_writer writer;
  for (pugi::xml_node child = target.first_child(); child;
       child = child.next_sibling())
    child.print(writer, "");
  return writer.result;
}

void finSAR_editUIDialog::AddTestItems(wxCommandEvent& event) {
  wxString filename;
  wxFileDialog dlg(this, "Select file", wxEmptyString, wxEmptyString,
                   "RTZ files(*.rtz) | *.*;*.*", wxFD_OPEN);
  if (dlg.ShowModal() == wxID_OK) {
    if (dlg.GetPath() != wxEmptyString) {
      filename = dlg.GetPath();
      // wxMessageBox(filename);
    }

  } else
    wxMessageBox(_("No file entered"));

  ca = 0;
  cf = 0;

  pugi::xml_document xmlDoc;
  pugi::xml_parse_result result = xmlDoc.load_file(filename.mb_str());

  // pugi::xml_node pRoot = xmlDoc.child("Areas");
  // if (pRoot == nullptr) return;

  // pugi::xml_node pAreasElement = xmlDoc.child("Areas");
  string area_name;
  pugi::xml_node nodes = xmlDoc.child("Areas");
  for (pugi::xml_node node = nodes.first_child(); node;
       node = node.next_sibling()) {
    if (!strcmp(node.name(), "Area")) {
      pugi::xml_node name = node.first_child();
      // pugi::xml_node name = node.child("Name");

      if (!strcmp(name.name(), "Name")) {
        string area_name = InnerXML(node.child("Name"));
        // wxMessageBox(area_name);
        my_areas[ca].Add(area_name);
        ca++;
      }
      pugi::xml_node routes = node.first_child().next_sibling();

      if (!strcmp(routes.name(), "Routes")) {
        // wxMessageBox("here");

        pugi::xml_node rnode = routes.first_child();
        if (!strcmp(rnode.name(), "file")) {
          cf = 0;
          for (pugi::xml_node child = routes.first_child(); child;
               child = child.next_sibling()) {
            string f = InnerXML(child);
            my_files[ca][cf].Add(f, 1);
            // wxMessageBox(my_files[ca][cf].Item(1));

            cf++;
          }
        }
        /*
          pugi::xml_node files;

          for (pugi::xml_node files = routes.first_child(); routes;
               files = routes.next_sibling()) {
            string file_name = InnerXML(routes.child("file");
            wxMessageBox(file_name);
          }*/
      }
    }
  }
  wxString testca = wxString::Format("%i", ca);
  wxString testcf = wxString::Format("%i", cf);

  // wxMessageBox(my_areas[1].Item(0));
  // wxMessageBox(testca);
  // wxMessageBox(testcf);
}

/*
void finSAR_editUIDialog::ChartTheRoute(wxString myRoute) {
  PlugIn_Route_Ex* newRoute =
      new PlugIn_Route_Ex;  // for adding a route on OpenCPN chart display

  newRoute->m_NameString = myRoute;
  newRoute->m_isVisible = true;

  double lati, loni, value, value1;
  bool m_bNameVisible = true;

  for (std::vector<Position>::iterator itp = my_positions.begin();
       itp != my_positions.end(); itp++) {
    PlugIn_Waypoint_Ex* wayPoint = new PlugIn_Waypoint_Ex;

    wayPoint->m_MarkName = (*itp).wpName;

    //if (!(*itp).lat.ToDouble(&value)) { /* error! */
//}
// lati = value;
// if (!(*itp).lon.ToDouble(&value1)) { /* error! */
// }
// loni = value1;

/*
m_bNameVisible = (*itp).is_visible;
m_bNameVisible = true;

wayPoint->m_lat = lati;
wayPoint->m_lon = loni;
wayPoint->IsVisible = true;
//wayPoint->IsNameVisible = m_bNameVisible;

wayPoint->IconName = "diamond";

newRoute->pWaypointList->Append(wayPoint);
}

AddPlugInRouteEx(newRoute, true);

wxMessageBox("Route & Mark Manager will show the imported route",
           "Imported Route");

GetParent()->Refresh();
}
*/
int finSAR_editUIDialog::GetScale(double myChartScale) {
  // If myChartScale is not exactly as shown in OpenCPN get the best scale to
  // use.

  if (myChartScale < 1200) {
    myChartScale = 800;
  }

  else if (myChartScale >= 1200 && myChartScale < 2400) {
    myChartScale = 1600;
  }

  else if (myChartScale >= 2400 && myChartScale < 4800) {
    myChartScale = 3200;
  }

  else if (myChartScale >= 4800 && myChartScale < 9600) {
    myChartScale = 6400;
  }

  else if (myChartScale >= 9600 && myChartScale < 19200) {
    myChartScale = 12800;
  }

  else if (myChartScale >= 19200 && myChartScale < 38400) {
    myChartScale = 25600;
  }

  else if (myChartScale >= 38400 && myChartScale < 76800) {
    myChartScale = 51200;
  }

  else if (myChartScale >= 76800 && myChartScale < 153550) {
    myChartScale = 102400;
  }

  else if (myChartScale >= 153550 && myChartScale < 306850) {
    myChartScale = 204700;
  }

  else if (myChartScale >= 306850 && myChartScale < 613700) {
    myChartScale = 409000;
  }

  else if (myChartScale >= 613700 && myChartScale < 1227400) {
    myChartScale = 818000;
  }

  else if (myChartScale >= 1227400 && myChartScale < 2454800) {
    myChartScale = 1636000;
  }

  else if (myChartScale >= 2454800 && myChartScale < 4909600) {
    myChartScale = 3272000;
  }

  else
    myChartScale = 9999000;

  // Use the modified scale to find the correct zoom level
  // Would switch case be quicker???

  if (myChartScale == 800) {
    return 19;
  }

  else if (myChartScale == 1600) {
    return 18;
  }

  else if (myChartScale == 3200) {
    return 17;
  }

  else if (myChartScale == 6400) {
    return 16;
  }

  else if (myChartScale == 12800) {
    return 15;
  }

  else if (myChartScale == 25600) {
    return 14;
  }

  else if (myChartScale == 51200) {
    return 13;
  }

  else if (myChartScale == 102400) {
    return 12;
  }

  else if (myChartScale == 204700) {
    return 11;
  } else if (myChartScale == 409000) {
    return 10;
  }

  else if (myChartScale == 818000) {
    return 9;
  }

  else if (myChartScale == 1636000) {
    return 8;
  }

  else if (myChartScale == 3272000) {
    return 7;
  }

  else
    return 6;
}

void finSAR_editUIDialog::OnButtonEBL(wxCommandEvent& event) {
  m_bBearingLine = true;

  this->m_timer1.Start(200);
}

void finSAR_editUIDialog::OnButtonEBL_off(wxCommandEvent& event) {
  m_bBearingLine = false;

  this->m_timer1.Stop();

  RequestRefresh(pParent);
}

void finSAR_editUIDialog::key_shortcut(wxKeyEvent& event) {
  // wxMessageBox("here");
  //  of course, it doesn't have to be the control key. You can use others:
  //  http://docs.wxwidgets.org/stable/wx_wxkeyevent.html
  if (event.GetModifiers() == wxMOD_CONTROL) {
    switch (event.GetKeyCode()) {
      case 'S':  // can return the upper ASCII value of a key
        // do whatever you like for a Ctrl+G event here!
        // wxMessageBox(_("CTRL+G") );
        // OnCursor();
        break;  /*
      case WXK_LEFT: // we also have special keycodes for non-ascii values.
          // get a full list of special keycodes here:
          // http://docs.wxwidgets.org/stable/wx_keycodes.html
          wxMessageBox(_("CTRL+Left") );
          break;*/
      default:  // do nothing
        break;
    }
  }
  event.Skip();
}

/*
void finSAR_editUIDialog::SetBearingWaypoint() {
 // wxMessageBox("here");
  centreLat = m_vp->clat;
  centreLon = m_vp->clon;

  PlugIn_Waypoint_Ex* wayPoint = new PlugIn_Waypoint_Ex;
  wayPoint->m_MarkName = "bearing";
  wayPoint->m_GUID = "bearing";
  wayPoint->m_lat = centreLat;
  wayPoint->m_lon = centreLon;
  wayPoint->IsVisible = true;
  wayPoint->IsNameVisible = false;
  wayPoint->IconName = "circle";
  AddSingleWaypointEx(wayPoint);

  RequestRefresh(pParent);

  //this->m_Lat1->SetValue(wxString::Format("%f", centreLat));
  //this->m_Lon1->SetValue(wxString::Format("%f", centreLon));
}*/

void finSAR_editUIDialog::MakeBoxPoints() {
  myZoom = GetScale(m_vp->chart_scale);

  double boxlat = m_vp->clat;
  double boxlon = m_vp->clon;

  double mpp;
  mpp = 156543.03392 * cos(boxlat * PI / 180) / pow(2, myZoom);

  mpp *= 640;   // Reference box is 640x640
  mpp /= 1852;  // metres to NM

  double halfbox = mpp / 60 / 2;  // decimal degree

  double centreC = boxlat;
  double dlat1, dlat2;

  dlat1 = centreC + halfbox;
  dlat2 = centreC - halfbox;

  int pixheight;

  wxPoint p1;
  GetCanvasPixLL(m_vp, &p1, dlat1, boxlon);

  wxPoint p2;
  GetCanvasPixLL(m_vp, &p2, dlat2, boxlon);

  pixheight = (p2.y) - (p1.y);

  pixheight /= 2;

  myPixArray[0].x = 0;
  myPixArray[0].y = pixheight;
  myPixArray[0].y *= -1;

  myPixArray[1].x = 0;
  myPixArray[1].y = pixheight;

  myPixArray[2].x = 0;
  myPixArray[2].y = 0;

  myPixArray[3].x = pixheight;
  myPixArray[3].x *= -1;
  myPixArray[3].y = 0;

  myPixArray[4].x = pixheight;
  myPixArray[4].y = 0;

  myPixHeight = pixheight * 2;
}

void finSAR_editUIDialog::OnTimer(wxTimerEvent& event) { MakeEBLEvent(); }

void finSAR_editUIDialog::MakeEBLEvent() {
  if (m_bBearingLine) {
    this->m_Lat1->SetValue(wxString::Format("%.6f", pPlugIn->GetCursorLat()));
    this->m_Lon1->SetValue(wxString::Format("%.6f", pPlugIn->GetCursorLon()));

    ebl_lat = pPlugIn->GetCursorLat();
    ebl_lon = pPlugIn->GetCursorLon();
    m_ShipLat2 = pPlugIn->GetShipLat();
    m_ShipLon2 = pPlugIn->GetShipLon();
  } else {
    ebl_lat = 0;
    ebl_lon = 0;

    m_ShipLat2 = 0;
    m_ShipLat2 = 0;
  }

  RequestRefresh(pParent);
}

void finSAR_editUIDialog::OnNewRoute(wxCommandEvent& event) {
  // This sleep is needed to give the time for the currently pressed modifier
  // keys, if any, to be released. Notice that Control modifier could well be
  // pressed if this command was activated from the menu using accelerator
  // and keeping it pressed would totally derail the test below, e.g. "A" key
  // press would actually become "Ctrl+A" selecting the entire text and so on.

  wxMessageBox("Press CTRL+R to enter route name\nPress \"End Route\" on completion");

  pParent->SetFocus();
  wxUIActionSimulator sim;
  sim.KeyDown(82, wxMOD_CONTROL);

  // sim.KeyUp(82, wxMOD_CONTROL);
}

void finSAR_editUIDialog::OnEndRoute(wxCommandEvent& event) {
  // This sleep is needed to give the time for the currently pressed modifier
  // keys, if any, to be released. Notice that Control modifier could well be
  // pressed if this command was activated from the menu using accelerator
  // and keeping it pressed would totally derail the test below, e.g. "A" key
  // press would actually become "Ctrl+A" selecting the entire text and so on.

  pPlugIn->m_pfinSAR_editDialog->SetFocus();
  wxUIActionSimulator sim;
  sim.KeyUp(82, wxMOD_CONTROL);
  //
  //
  std::vector<std::unique_ptr<PlugIn_Route_Ex>> routes;
  auto uids = GetRouteGUIDArray();
  for (size_t i = 0; i < uids.size(); i++) {
    routes.push_back(std::move(GetRouteEx_Plugin(uids[i])));
  }

  GetRouteDialog RouteDialog(this, -1, _("Select the route to follow"),
                             wxPoint(200, 200), wxSize(300, 200),
                             wxCAPTION | wxRESIZE_BORDER);

  RouteDialog.dialogText->InsertColumn(0, "", 0, wxLIST_AUTOSIZE);
  RouteDialog.dialogText->SetColumnWidth(0, 290);
  RouteDialog.dialogText->InsertColumn(1, "", 0, wxLIST_AUTOSIZE);
  RouteDialog.dialogText->SetColumnWidth(1, 0);
  RouteDialog.dialogText->DeleteAllItems();

  int in = 0;
  std::vector<std::string> names;
  for (const auto& r : routes) names.push_back(r->m_NameString.ToStdString());

  for (size_t n = 0; n < names.size(); n++) {
    wxString routeName = names[in];
    if (routeName == "") {
      wxMessageBox(
          "Please add name of route in \"Route->Properties\"\nand try again",
          "Missing Name");
      return;
    }
    RouteDialog.dialogText->InsertItem(in, "", -1);
    RouteDialog.dialogText->SetItem(in, 0, routeName);
    in++;
  }

  // ReadNavobj();
  long si = -1;
  long itemIndex = -1;
  // int f = 0;

  wxListItem row_info;
  wxString cell_contents_string = wxEmptyString;
  bool foundRoute = false;

  if (RouteDialog.ShowModal() != wxID_OK) {
    wxMessageBox("Cancelled");
  } else {
    for (;;) {
      itemIndex = RouteDialog.dialogText->GetNextItem(
          itemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

      if (itemIndex == -1) break;

      // Got the selected item index
      if (RouteDialog.dialogText->IsSelected(itemIndex)) {
        si = itemIndex;
        foundRoute = true;
        break;
      }
    }

    if (foundRoute) {
      // Set what row it is (m_itemId is a member of the regular
      // wxListCtrl class)
      row_info.m_itemId = si;
      // Set what column of that row we want to query for information.
      row_info.m_col = 0;
      // Set text mask
      row_info.m_mask = wxLIST_MASK_TEXT;

      // Get the info and store it in row_info variable.
      RouteDialog.dialogText->GetItem(row_info);
      // Extract the text out that cell
      cell_contents_string = row_info.m_text;
      rtept initPoint;
      nextRoutePointIndex = 0;
      bool foundRoute = false;
      wxString myGuid;

      for (size_t i = 0; i < uids.size(); i++) {
        thisRoute = GetRouteEx_Plugin(uids[i]);
        if (thisRoute->m_NameString == cell_contents_string) {
          myGuid = uids[i];
          foundRoute = true;
          break;
        }
      }

      if (foundRoute) {
        countRoutePoints = thisRoute->pWaypointList->size();
        myList = thisRoute->pWaypointList;

        PlugIn_Waypoint_Ex* myWaypoint;
        theWaypoints.clear();
        // Trying to remove the route by deleting the waypoints
        // Plugin_WaypointExList* temp_list;

        wxPlugin_WaypointExListNode* pwpnode = myList->GetFirst();
        while (pwpnode) {
          myWaypoint = pwpnode->GetData();
          theWaypoints.push_back(myWaypoint);
          // temp_list->Append(myWaypoint);
          pwpnode = pwpnode->GetNext();
        }

        WriteRTZ(thisRoute->m_NameString);

        wxString rtz_file = thisRoute->m_NameString + ".rtz";
        //wxMessageBox(rtz_file);

        int id = pPlugIn->GetRoute_Id(rtz_file);
        //wxString sid = wxString::Format("%i", id);
        //wxMessageBox(sid);
        if (id != 0) pPlugIn->DeleteRTZ_Id(id);
        // Now add the modified route
        pPlugIn->Add_RTZ_db(rtz_file);

        /*
        temp_list->DeleteContents(true);
        temp_list->Clear();

        thisRoute->pWaypointList->Clear();  // no-longer-managed object
        thisRoute->pWaypointList->delete()(thisRoute->pWaypointList);
        */
        DeletePlugInRoute(myGuid);
      } else
        wxMessageBox("Route not found");
    }
  }
}

void finSAR_editUIDialog::WriteRTZ(wxString route_name) {
  // Select the route from the route table
  //
  // Create Main level XML container
  xml_document xmlDoc;

  auto declarationNode = xmlDoc.append_child(node_declaration);

  declarationNode.append_attribute("version") = "1.0";

  declarationNode.append_attribute("encoding") = "UTF-8";

  const char* value = "http://www.cirm.org/RTZ/1/2";

  // Create XML root node called animals
  xml_node pRoot = xmlDoc.append_child("route");

  pRoot.append_attribute("xmlns").set_value(value);
  pRoot.append_attribute("xmlns:xsi")
      .set_value("http://www.w3.org/2001/XMLSchema-instance");

  pRoot.append_attribute("version").set_value("1.2");

  // ************* Add routeInfo to root node *******

  xml_node routeInfo = pRoot.append_child("routeInfo");
  routeInfo.append_attribute("routeName").set_value(route_name.mb_str());

  // Insert cat's name as first child of animal

  // ************* Add waypoints *******
  xml_node waypoints = pRoot.append_child("waypoints");

  int idn = 0;

  for (std::vector<PlugIn_Waypoint_Ex*>::iterator itOut = theWaypoints.begin();
       itOut != theWaypoints.end(); itOut++) {
    xml_node m_waypoint = waypoints.append_child("waypoint");
    wxString myIdn = wxString::Format(wxT("%i"), idn);
    m_waypoint.append_attribute("id").set_value(myIdn.mb_str());
    m_waypoint.append_attribute("name").set_value(
        (*itOut)->m_MarkName.mb_str());
    m_waypoint.append_attribute("revision").set_value("0");

    xml_node position = m_waypoint.append_child("position");
    double dLat = (*itOut)->m_lat;
    double dLon = (*itOut)->m_lon;
    wxString sLat = wxString::Format("%f", dLat);
    wxString sLon = wxString::Format("%f", dLon);

    position.append_attribute("lat").set_value(sLat);
    position.append_attribute("lon").set_value(sLon);

    idn++;
  }
  // done adding waypoints
  // Write xmlDoc into a file

  wxString file_name = route_name + ".rtz";
  wxString file_path = pPlugIn->StandardPathRTZ() + file_name;

  //wxMessageBox(file_path);

  // Route name must be the same as the file name, without file extension

  xmlDoc.save_file(file_path.mb_str());
}

void finSAR_editUIDialog::OnLoadRTZ(wxCommandEvent& event) {
  ReadRTZ();
  // ChartTheRoute(mySelectedRoute);
  i_vector.clear();
}

void finSAR_editUIDialog::OnIndex(wxCommandEvent& event) {
  // ReadRTZ();
  std::string str = "finSAR_edit_pi_pi";
  SendPluginMessage(wxString("finSAR_edit_pi_pi"), str);
  return;

  active_waypoint = FindActiveWaypoint(id_wpt);
  if (active_waypoint->wpName == wxEmptyString) {
    wxMessageBox("Activate the route/waypoint");
    return;
  }
  // wxMessageBox(active_waypoint->wpName);
  // wxMessageBox(prev_waypoint->wpName);
  FindIndex(active_waypoint, prev_waypoint);
}

void finSAR_editUIDialog::FindIndex(Position* A, Position* B) {
  double value = 0.0;
  A->lat.ToDouble(&value);
  double lat1 = value;
  A->lon.ToDouble(&value);
  double lon1 = value;
  B->lat.ToDouble(&value);
  double lat2 = value;
  B->lon.ToDouble(&value);
  double lon2 = value;

  double lat3 = centreLat;
  double lon3 = centreLon;

  // Bearing A -> C
  double dist1, dist2, brg1, brg2;
  DistanceBearingMercator_Plugin(lat3, lon3, lat1, lon1, &brg1, &dist1);
  // Bearing A -> B
  DistanceBearingMercator_Plugin(lat2, lon2, lat1, lon1, &brg2, &dist2);

  wxString sbrg1 = wxString::Format("%f", brg1);
  // wxMessageBox(sbrg1);

  wxString sbrg2 = wxString::Format("%f", brg2);
  // wxMessageBox(sbrg2);
  /*
  Location a;
  Location b;
  Location x;
*/
  double ax = dist1;
  double alfa = (abs(brg2 - brg1)) / 180 * PI;
  double distance = sin(alfa) * ax;

  double beta = cos(alfa) * ax;

  double dlat, dlon;
  PositionBearingDistanceMercator_Plugin(lat1, lon1, brg2, beta, &dlat, &dlon);

  wxString sdlat = wxString::Format("%f", dlat);
  // wxMessageBox(sdlat);

  wxString sdlon = wxString::Format("%f", dlon);
  // wxMessageBox(sdlon);

  i_target = new IndexTarget;
  i_target->route_name = A->route_name;
  // wxMessageBox(i_target->route_name);
  i_target->wpId = A->wpId;
  // wxMessageBox(i_target->wpId);
  i_target->beginLat = dlat;
  i_target->beginLon = dlon;
  i_target->endLat = lat3;
  i_target->endLon = lon3;

  i_vector.push_back(*i_target);
}

void finSAR_editUIDialog::OnRange(wxCommandEvent& event) {
  ReadRTZ();
  active_waypoint = FindActiveWaypoint(id_wpt);
  if (active_waypoint->wpName == wxEmptyString) {
    wxMessageBox("Check NMEA");
    return;
  }
  // wxMessageBox(active_waypoint->wpName);
  // wxMessageBox(prev_waypoint->wpName);
  FindRange(active_waypoint, prev_waypoint);
}

void finSAR_editUIDialog::FindRange(Position* A, Position* B) {}

void finSAR_editUIDialog::OnDirection(wxCommandEvent& event) {
  ReadRTZ();
  active_waypoint = FindActiveWaypoint(id_wpt);
  if (active_waypoint->wpName == wxEmptyString) {
    wxMessageBox("Check NMEA");
    return;
  }
  // wxMessageBox(active_waypoint->wpName);
  // wxMessageBox(prev_waypoint->wpName);
  FindDirection(active_waypoint, prev_waypoint);
}

void finSAR_editUIDialog::FindDirection(Position* A, Position* B) {
  double value = 0.0;
  A->lat.ToDouble(&value);
  double lat1 = value;
  A->lon.ToDouble(&value);
  double lon1 = value;
  B->lat.ToDouble(&value);
  double lat2 = value;
  B->lon.ToDouble(&value);
  double lon2 = value;

  double lat3 = centreLat;
  double lon3 = centreLon;
  double brg, dist;

  // Bearing A -> B
  DistanceBearingMercator_Plugin(lat1, lon1, lat2, lon2, &brg, &dist);

  d_vector.clear();

  d_target = new DirectionTarget;
  d_target->m_lat = lat3;
  d_target->m_lon = lon3;
  d_target->m_dir = brg;

  d_vector.push_back(*d_target);
}

void finSAR_editUIDialog::OnSaveObjects(wxCommandEvent& event) {
  wxString save_folder = pPlugIn->StandardPath();
  SaveXML();
}

void finSAR_editUIDialog::SetNMEAMessage(wxString sentence) {
  // $GPAPB,A,A,0.10,R,N,V,V,011,M,DEST,011,M,011,M*3C
  wxString token[40];
  wxString s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
  token[0] = "";

  wxStringTokenizer tokenizer(sentence, ",");
  int i = 0;
  while (tokenizer.HasMoreTokens()) {
    token[i] = tokenizer.GetNextToken();
    i++;
  }
  if (token[0].Right(3) == "APB") {
    s10 = token[10];

    // m_bGotAPB = true;
    // wxMessageBox(s11);
    id_wpt = s10;
  }
}

void finSAR_editUIDialog::ReadRTZ() {
  rtz_version = "";
  Position my_position;
  my_positions.clear();

  wxString filename;
  wxFileDialog dlg(this, "Select file", wxEmptyString, wxEmptyString,
                   "RTZ files(*.rtz) | *.rtz;*.RTZ", wxFD_OPEN);
  if (dlg.ShowModal() == wxID_OK) {
    if (dlg.GetPath() != wxEmptyString) {
      filename = dlg.GetPath();
      // wxMessageBox(filename);
    }

  } else
    wxMessageBox(_("No file entered"));

  pugi::xml_document xmlDoc;
  pugi::xml_parse_result result =
      xmlDoc.load_file(filename.mb_str(), parse_default | parse_declaration);

  string rtz_version = xmlDoc.child("route").attribute("version").value();

  pugi::xml_node pRoot = xmlDoc.child("route").child("routeInfo");
  if (pRoot == nullptr) return;

  wxString error;
  wxProgressDialog* progressdialog = NULL;

  my_position.visible = "1";  // dummy value to make sure there is an entry
  my_position.planned_speed =
      "5";  // dummy value to make sure there is an entry
  my_position.route_name = "na";
  my_position.wpSym = "diamond";

  xml_node pRouteNameElement = xmlDoc.child("route").child("routeInfo");

  if (pRouteNameElement == nullptr) return;

  string route_name = pRouteNameElement.attribute("routeName").value();
  my_position.route_name = route_name;
  mySelectedRoute = route_name;

  bool exists = false;
  // wxMessageBox(route_name);

  xml_node pWaypointsElement = xmlDoc.child("route").child("waypoints");
  if (pWaypointsElement == nullptr) return;

  xml_node pListWaypointsElement = pWaypointsElement.child("waypoint");
  if (pListWaypointsElement == nullptr) return;

  while (pListWaypointsElement != nullptr) {
    string value = "nullptr";
    value = pListWaypointsElement.attribute("id").value();
    if (value == "nullptr") return;  // must have id
    my_position.wpId = value;
    // wxMessageBox(value);

    value = pListWaypointsElement.attribute("name").value();
    if (value != "nullptr") {
      my_position.wpName = value;
    } else {
      my_position.wpName = my_position.wpId;  // make name same as id if missing
    }

    xml_node pListPositionElement = pListWaypointsElement.child("position");
    if (pListPositionElement == nullptr) return;
    while (pListPositionElement != nullptr) {
      string stp = pListPositionElement.attribute("lat").value();
      my_position.lat = stp;
      // wxMessageBox(stp);

      string stpl = pListPositionElement.attribute("lon").value();
      my_position.lon = stpl;
      // wxMessageBox(stpl);

      pListPositionElement = pListPositionElement.next_sibling(
          "position");  // stop the loop when position empty
    }

    pListWaypointsElement = pListWaypointsElement.next_sibling(
        "waypoint");  // stop the loop when waypoints empty

    my_positions.push_back(my_position);
  }

  int count = my_positions.size();
  wxString mycount = wxString::Format("%i", count);
  // wxMessageBox(mycount);
  // SetRTZversion(rtz_version);
}

Position* finSAR_editUIDialog::FindActiveWaypoint(wxString wpt_name) {
  active_waypoint = new Position;
  prev_waypoint = new Position;
  std::vector<Position>::iterator prev;
  for (std::vector<Position>::iterator itp = my_positions.begin(),
                                       prev = my_positions.end();
       itp != my_positions.end(); prev = itp, itp++) {
    wxString wpn = (*itp).wpName;

    if (wpn == wpt_name) {
      active_waypoint->wpName = wpn;
      active_waypoint->lat = (*itp).lat;
      active_waypoint->lon = (*itp).lon;
      active_waypoint->route_name = (*itp).route_name;
      // wxMessageBox(active_waypoint->route_name);
      active_waypoint->wpId = (*itp).wpId;
      // wxMessageBox(active_waypoint->wpId);

      prev_waypoint->wpName = (*prev).wpName;
      prev_waypoint->lat = (*prev).lat;
      prev_waypoint->lon = (*prev).lon;

      return active_waypoint;
    }
  }
  active_waypoint->wpName = wxEmptyString;
  return active_waypoint;
}

///////////////////////////////////////////////////////////////////////////

ConfigurationDialog::ConfigurationDialog(wxWindow* parent, wxWindowID id,
                                         const wxString& title,
                                         const wxPoint& pos, const wxSize& size,
                                         long style)
    : wxDialog(parent, id, title, pos, size, style) {
  this->SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxFlexGridSizer* fgSizer95;
  fgSizer95 = new wxFlexGridSizer(3, 1, 0, 0);
  fgSizer95->AddGrowableCol(0);
  fgSizer95->AddGrowableCol(1);
  fgSizer95->SetFlexibleDirection(wxBOTH);
  fgSizer95->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  wxStaticBoxSizer* sbSizer29;
  sbSizer29 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Routes")),
                                   wxVERTICAL);

  m_lRoutes =
      new wxListBox(sbSizer29->GetStaticBox(), wxID_ANY, wxDefaultPosition,
                    wxDefaultSize, 0, NULL, 0 | wxLB_ALWAYS_SB);
  sbSizer29->Add(m_lRoutes, 1, wxALL | wxEXPAND, 5);

  fgSizer95->Add(sbSizer29, 1, wxEXPAND, 5);

  wxFlexGridSizer* fgSizer78;
  fgSizer78 = new wxFlexGridSizer(1, 0, 0, 0);
  fgSizer78->SetFlexibleDirection(wxBOTH);
  fgSizer78->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_bDelete = new wxButton(this, wxID_ANY, _("Delete"), wxDefaultPosition,
                           wxDefaultSize, 0);
  fgSizer78->Add(m_bDelete, 0, wxALL, 5);

  m_bSelect = new wxButton(this, wxID_ANY, _("Route Table"), wxDefaultPosition,
                           wxDefaultSize, 0);
  fgSizer78->Add(m_bSelect, 0, wxALL, 5);

  m_bGenerate = new wxButton(this, wxID_ANY, _("Chart Route"),
                             wxDefaultPosition, wxDefaultSize, 0);
  fgSizer78->Add(m_bGenerate, 0, wxALL, 5);

  m_bClose = new wxButton(this, wxID_ANY, _("Close"), wxDefaultPosition,
                          wxDefaultSize, 0);
  fgSizer78->Add(m_bClose, 0, wxALL, 5);

  fgSizer95->Add(fgSizer78, 1, wxEXPAND, 5);

  this->SetSizer(fgSizer95);
  this->Layout();
  fgSizer95->Fit(this);
  this->Centre(wxBOTH);

  // Connect Events
  m_bDelete->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(ConfigurationDialog::OnDelete), NULL,
                     this);
  m_bSelect->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(ConfigurationDialog::OnInformation),
                     NULL, this);
  m_bGenerate->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(ConfigurationDialog::OnGenerate),
                       NULL, this);
  m_bClose->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                    wxCommandEventHandler(ConfigurationDialog::OnClose), NULL,
                    this);
}

ConfigurationDialog::~ConfigurationDialog() {
  // Disconnect Events
  m_bDelete->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
                        wxCommandEventHandler(ConfigurationDialog::OnDelete),
                        NULL, this);
  m_bSelect->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ConfigurationDialog::OnInformation), NULL, this);
  m_bGenerate->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ConfigurationDialog::OnGenerate), NULL, this);
  m_bClose->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(ConfigurationDialog::OnClose),
                       NULL, this);
}

void ConfigurationDialog::OnDelete(wxCommandEvent& event) {
  wxString rn;
  int s;
  s = m_lRoutes->GetSelection();
  rn = m_lRoutes->GetString(s);
}
void ConfigurationDialog::OnInformation(wxCommandEvent& event) {
  wxString rn;
  int s;
  s = m_lRoutes->GetSelection();

  if (s == -1) {
    wxMessageBox(_("Please select a route"));
    return;
  }

  rn = m_lRoutes->GetString(s);
  /*

  if (m_lRoutes->IsEmpty()){
          wxMessageBox(_("Please select positions and generate a route"));
          return;
  }
  */
  pPlugIn->m_pfinSAR_editDialog->GetTable(rn);
  return;  //
}

void ConfigurationDialog::OnGenerate(wxCommandEvent& event) {
  wxString rn;
  int s;
  s = m_lRoutes->GetSelection();

  if (s == -1) {
    wxMessageBox(_("Please select a route"));
    return;
  }

  rn = m_lRoutes->GetString(s);

  pPlugIn->m_pfinSAR_editDialog->AddChartRoute(rn);
}

void ConfigurationDialog::OnClose(wxCommandEvent& event) { Hide(); }
