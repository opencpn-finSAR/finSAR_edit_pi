/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  finSAR_edit Plugin
 * Author:   David Register, Mike Rossiter
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 */

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include <wx/glcanvas.h>
#endif  // precompiled headers

#include <wx/fileconf.h>
#include <wx/stdpaths.h>

#include "ocpn_plugin.h"

#include "finSAR_edit_pi.h"
#include "finSAR_editUIDialogBase.h"
#include "finSAR_editUIDialog.h"

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin *create_pi(void *ppimgr) {
  return new finSAR_edit_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin *p) { delete p; }

//---------------------------------------------------------------------------------------------------------
//
//    finSAR_edit PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

#include "icons.h"

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

finSAR_edit_pi::finSAR_edit_pi(void *ppimgr) : opencpn_plugin_117(ppimgr) {
  // Create the PlugIn icons
  initialize_images();

  wxFileName fn;

  auto path = GetPluginDataDir("finSAR_edit_pi");
  fn.SetPath(path);
  fn.AppendDir("data");
  fn.SetFullName("finSAR_edit_panel_icon.png");

  path = fn.GetFullPath();

  wxInitAllImageHandlers();

  wxLogDebug(wxString("Using icon path: ") + path);
  if (!wxImage::CanRead(path)) {
    wxLogDebug("Initiating image handlers.");
    wxInitAllImageHandlers();
  }
  wxImage panelIcon(path);
  if (panelIcon.IsOk())
    m_panelBitmap = wxBitmap(panelIcon);
  else
    wxLogWarning("finSAR_edit panel icon has NOT been loaded");

  m_bShowfinSAR_edit = false;
}

finSAR_edit_pi::~finSAR_edit_pi(void) {
  delete _img_finSAR_edit_pi;
  delete _img_finSAR_edit;
}

int finSAR_edit_pi::Init(void) {
  AddLocaleCatalog(_T("opencpn-finSAR_edit_pi"));

  // Set some default private member parameters
  m_finSAR_edit_dialog_x = 0;
  m_finSAR_edit_dialog_y = 0;
  m_finSAR_edit_dialog_sx = 200;
  m_finSAR_edit_dialog_sy = 400;
  m_pfinSAR_editDialog = NULL;
  m_pfinSAR_editOverlayFactory = NULL;
  m_bfinSAR_editShowIcon = true;

  ::wxDisplaySize(&m_display_width, &m_display_height);

  m_pconfig = GetOCPNConfigObject();

  //    And load the configuration items
  LoadConfig();

  // Get a pointer to the opencpn display canvas, to use as a parent for the
  // finSAR_edit dialog
  m_parent_window = GetOCPNCanvasWindow();

  wxMenu dummy_menu;
  m_position_menu_id = AddCanvasContextMenuItem(
      new wxMenuItem(&dummy_menu, -1, _("Delete Tidal Current Station")), this);
  SetCanvasContextMenuItemViz(m_position_menu_id, true);

  //    This PlugIn needs a toolbar icon, so request its insertion if enabled
  //    locally
  if (m_bfinSAR_editShowIcon)
#ifdef ocpnUSE_SVG
    m_leftclick_tool_id =
        InsertPlugInToolSVG(_T("finSAR_edit"), _svg_finSAR_edit, _svg_finSAR_edit,
                            _svg_finSAR_edit_toggled, wxITEM_CHECK, _("finSAR_edit"),
                            _T(""), NULL, finSAR_edit_TOOL_POSITION, 0, this);
#else
    m_leftclick_tool_id = InsertPlugInTool(
        _T(""), _img_finSAR_edit, _img_finSAR_edit, wxITEM_CHECK, _("finSAR_edit"), _T(""),
        NULL, finSAR_edit_TOOL_POSITION, 0, this);
#endif

  return (WANTS_OVERLAY_CALLBACK | WANTS_OPENGL_OVERLAY_CALLBACK |
          WANTS_NMEA_SENTENCES |
          WANTS_TOOLBAR_CALLBACK | WANTS_CURSOR_LATLON | INSTALLS_TOOLBAR_TOOL |
          WANTS_CONFIG | WANTS_ONPAINT_VIEWPORT | WANTS_PLUGIN_MESSAGING);
}

bool finSAR_edit_pi::DeInit(void) {
  if (m_pfinSAR_editDialog) {
    m_pfinSAR_editDialog->Close();
    delete m_pfinSAR_editDialog;
    m_pfinSAR_editDialog = NULL;
  }

  delete m_pfinSAR_editOverlayFactory;
  m_pfinSAR_editOverlayFactory = NULL;

  return true;
}

int finSAR_edit_pi::GetAPIVersionMajor() { return atoi(API_VERSION); }

int finSAR_edit_pi::GetAPIVersionMinor() {
  std::string v(API_VERSION);
  size_t dotpos = v.find('.');
  return atoi(v.substr(dotpos + 1).c_str());
}

int finSAR_edit_pi::GetPlugInVersionMajor() { return PLUGIN_VERSION_MAJOR; }

int finSAR_edit_pi::GetPlugInVersionMinor() { return PLUGIN_VERSION_MINOR; }

wxBitmap *finSAR_edit_pi::GetPlugInBitmap() { return &m_panelBitmap; }

wxString finSAR_edit_pi::GetCommonName() { return PLUGIN_API_NAME; }

wxString finSAR_edit_pi::GetShortDescription() { return PKG_SUMMARY; }

wxString finSAR_edit_pi::GetLongDescription() { return PKG_DESCRIPTION; }

void finSAR_edit_pi::SetDefaults(void) {}

int finSAR_edit_pi::GetToolbarToolCount(void) { return 1; }

void finSAR_edit_pi::OnToolbarToolCallback(int id) {
  if (!m_pfinSAR_editDialog) {
    m_pfinSAR_editDialog = new finSAR_editUIDialog(m_parent_window, this);
    wxPoint p = wxPoint(m_finSAR_edit_dialog_x, m_finSAR_edit_dialog_y);
    m_pfinSAR_editDialog->pPlugIn = this;
    m_pfinSAR_editDialog->Move(0,
                           0);  // workaround for gtk autocentre dialog behavior
    m_pfinSAR_editDialog->Move(p);

    // Create the drawing factory
    m_pfinSAR_editOverlayFactory = new finSAR_editOverlayFactory(*m_pfinSAR_editDialog);
    m_pfinSAR_editOverlayFactory->SetParentSize(m_display_width, m_display_height);
  }

  // Qualify the finSAR_edit dialog position
  bool b_reset_pos = false;

#ifdef __WXMSW__
  //  Support MultiMonitor setups which an allow negative window positions.
  //  If the requested window does not intersect any installed monitor,
  //  then default to simple primary monitor positioning.
  RECT frame_title_rect;
  frame_title_rect.left = m_finSAR_edit_dialog_x;
  frame_title_rect.top = m_finSAR_edit_dialog_y;
  frame_title_rect.right = m_finSAR_edit_dialog_x + m_finSAR_edit_dialog_sx;
  frame_title_rect.bottom = m_finSAR_edit_dialog_y + 30;

  if (NULL == MonitorFromRect(&frame_title_rect, MONITOR_DEFAULTTONULL))
    b_reset_pos = true;
#else
  //    Make sure drag bar (title bar) of window on Client Area of screen, with
  //    a little slop...
  wxRect window_title_rect;  // conservative estimate
  window_title_rect.x = m_finSAR_edit_dialog_x;
  window_title_rect.y = m_finSAR_edit_dialog_y;
  window_title_rect.width = m_finSAR_edit_dialog_sx;
  window_title_rect.height = 30;

  wxRect ClientRect = wxGetClientDisplayRect();
  ClientRect.Deflate(
      60, 60);  // Prevent the new window from being too close to the edge
  if (!ClientRect.Intersects(window_title_rect)) b_reset_pos = true;

#endif

  if (b_reset_pos) {
    m_finSAR_edit_dialog_x = 20;
    m_finSAR_edit_dialog_y = 170;
    m_finSAR_edit_dialog_sx = 300;
    m_finSAR_edit_dialog_sy = 540;
  }

  // Toggle finSAR_edit overlay display
  m_bShowfinSAR_edit = !m_bShowfinSAR_edit;

  //    Toggle dialog?
  if (m_bShowfinSAR_edit) {
    m_pfinSAR_editDialog->Show();
  } else {
    m_pfinSAR_editDialog->Hide();
  }

  // Toggle is handled by the toolbar but we must keep plugin manager b_toggle
  // updated to actual status to ensure correct status upon toolbar rebuild
  SetToolbarItemState(m_leftclick_tool_id, m_bShowfinSAR_edit);
  // SetCanvasContextMenuItemViz(m_position_menu_id, true);

  RequestRefresh(m_parent_window);  // refresh main window
}

void finSAR_edit_pi::OnfinSAR_editDialogClose() {
  m_bShowfinSAR_edit = false;
  SetToolbarItemState(m_leftclick_tool_id, m_bShowfinSAR_edit);
  SetCanvasContextMenuItemViz(m_position_menu_id, m_bShowfinSAR_edit);

  m_pfinSAR_editDialog->Hide();

  SaveConfig();

  RequestRefresh(m_parent_window);  // refresh main window
}

wxString finSAR_edit_pi::StandardPath() {
  wxString stdPath(*GetpPrivateApplicationDataLocation());
  wxString s = wxFileName::GetPathSeparator();

  stdPath += s + "SAR";
  if (!wxDirExists(stdPath)) wxMkdir(stdPath);
  stdPath = stdPath + s + "files";
  if (!wxDirExists(stdPath)) wxMkdir(stdPath);

  stdPath += s;
  return stdPath;
}

bool finSAR_edit_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp) {
  if (!m_pfinSAR_editDialog || !m_pfinSAR_editDialog->IsShown() ||
      !m_pfinSAR_editOverlayFactory)
    return false;

  if (m_pfinSAR_editDialog) {
    //m_pfinSAR_editDialog->OnCursor();
    m_pfinSAR_editDialog->SetViewPort(vp);
    m_pfinSAR_editDialog->MakeBoxPoints();
  }

  piDC pidc(dc);

  m_pfinSAR_editOverlayFactory->RenderOverlay(pidc, *vp);
  return true;
}

bool finSAR_edit_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp) {
  if (!m_pfinSAR_editDialog || !m_pfinSAR_editDialog->IsShown() ||
      !m_pfinSAR_editOverlayFactory)
    return false;

  if (m_pfinSAR_editDialog) {
    //m_pfinSAR_editDialog->OnCursor();
    m_pfinSAR_editDialog->SetViewPort(vp);
    m_pfinSAR_editDialog->MakeBoxPoints();
  }

  piDC piDC;
  glEnable(GL_BLEND);
  piDC.SetVP(vp);

  m_pfinSAR_editOverlayFactory->RenderOverlay(piDC, *vp);
  return true;
}

void finSAR_edit_pi::SetCursorLatLon(double lat, double lon) {
  //if (m_pfinSAR_editDialog) m_pfinSAR_editDialog->SetCursorLatLon(lat, lon);

  m_cursor_lat = lat;
  m_cursor_lon = lon;
}

void finSAR_edit_pi::SetPositionFix(PlugIn_Position_Fix &pfix) {
  m_ship_lon = pfix.Lon;
  m_ship_lat = pfix.Lat;
  // std::cout<<"Ship--> Lat: "<<m_ship_lat<<" Lon: "<<m_ship_lon<<std::endl;
  //}
}
bool finSAR_edit_pi::LoadConfig(void) {
  wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

  if (!pConf) return false;

  pConf->SetPath(_T( "/PlugIns/finSAR_edit" ));

  m_CopyFolderSelected = pConf->Read(_T( "finSAR_editFolder" ));

  if (m_CopyFolderSelected == wxEmptyString) {
    wxString g_SData_Locn = *GetpSharedDataLocation();
    // Establish location of Tide and Current data
    pTC_Dir = new wxString(_T("tcdata"));
    pTC_Dir->Prepend(g_SData_Locn);

    m_CopyFolderSelected = *pTC_Dir;
  }

  m_finSAR_edit_dialog_sx = pConf->Read(_T( "finSAR_editDialogSizeX" ), 300L);
  m_finSAR_edit_dialog_sy = pConf->Read(_T( "finSAR_editDialogSizeY" ), 540L);
  m_finSAR_edit_dialog_x = pConf->Read(_T( "finSAR_editDialogPosX" ), 20L);
  m_finSAR_edit_dialog_y = pConf->Read(_T( "finSAR_editDialogPosY" ), 170L);

  return true;
}

bool finSAR_edit_pi::SaveConfig(void) {
  wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

  if (!pConf) return false;

  pConf->SetPath(_T( "/PlugIns/finSAR_edit" ));

  pConf->Write(_T( "finSAR_editFolder" ), m_CopyFolderSelected);

  pConf->Write(_T( "finSAR_editDialogSizeX" ), m_finSAR_edit_dialog_sx);
  pConf->Write(_T( "finSAR_editDialogSizeY" ), m_finSAR_edit_dialog_sy);
  pConf->Write(_T( "finSAR_editDialogPosX" ), m_finSAR_edit_dialog_x);
  pConf->Write(_T( "finSAR_editDialogPosY" ), m_finSAR_edit_dialog_y);

  return true;
}

void finSAR_edit_pi::SetColorScheme(PI_ColorScheme cs) {
  DimeWindow(m_pfinSAR_editDialog);
}
/*
void finSAR_edit_pi::OnContextMenuItemCallback(int id)
{

        if (!m_pfinSAR_editDialog)
                return;

        if (id == m_position_menu_id) {

                m_cursor_lat = GetCursorLat();
                m_cursor_lon = GetCursorLon();

                m_pfinSAR_editDialog->OnContextMenu(m_cursor_lat, m_cursor_lon);
        }
}
*/
void finSAR_edit_pi::OnContextMenuItemCallback(int id) {
  if (!m_pfinSAR_editDialog) return;

  if (id == m_position_menu_id) {
    m_cursor_lat = GetCursorLat();
    m_cursor_lon = GetCursorLon();
  }
}

bool finSAR_edit_pi::MouseEventHook(wxMouseEvent &event) {
 // if (!m_pfinSAR_editDialog) return false;
/*
  if (event.LeftDown()) {
    //wxMessageBox("here");
    if (m_pfinSAR_editDialog) {
      m_cursor_lat = GetCursorLat();
      m_cursor_lon = GetCursorLon();
      wxString lat = wxString::Format("%f", m_cursor_lat);
      wxString lon = wxString::Format("%f", m_cursor_lon);

      m_pfinSAR_editDialog->m_Lat1->SetValue(lat);
      m_pfinSAR_editDialog->m_Lon1->SetValue(lon);
    }
    
  }
  */
  return true;
}

void finSAR_edit_pi::SetNMEASentence(wxString &sentence) {
  if (NULL != m_pfinSAR_editDialog) {
    m_pfinSAR_editDialog->SetNMEAMessage(sentence);
  }
}

void finSAR_edit_pi::SetPluginMessage(wxString &message_id,
                                      wxString &message_body) {
  
    if (message_id == "OCPN_WPT_ACTIVATED") {
      m_route_active = true;      
      Plugin_Active_Leg_Info myleg_info;
      SetActiveLegInfo(myleg_info);
      wp_Btw = wxString::Format("%f", myleg_info.Btw);
      wxMessageBox(wp_Btw);
    }
  
}

void finSAR_edit_pi::SetActiveLegInfo(Plugin_Active_Leg_Info &leg_info) {
  if (m_route_active) {
    wp_Btw = wxString::Format("%f", myleg_info.Btw);    
    //wxMessageBox(wp_Dtw);
  }
}