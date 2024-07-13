///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.0.0-0-g0efcecf)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/panel.h>
#include <wx/statline.h>
#include <wx/notebook.h>
#include <wx/timer.h>
#include <wx/menu.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class finSAR_editUIDialogBase
///////////////////////////////////////////////////////////////////////////////
class finSAR_editUIDialogBase : public wxFrame
{
	private:

	protected:
		wxNotebook* m_notebook1;
		wxPanel* m_panelRoutes;
		wxTextCtrl* m_textCtrl9;
		wxButton* m_button11;
		wxStaticText* m_staticText2;
		wxChoice* m_choice2;
		wxStaticText* m_staticText21;
		wxChoice* m_choice21;
		wxButton* m_button112;
		wxButton* m_bLoadRoute;
		wxTextCtrl* m_textCtrlRouteInUse;
		wxStaticText* m_staticText3;
		wxTextCtrl* m_textCtrl6;
		wxButton* m_bDeleteRoute;
		wxPanel* m_panelExtensions;
		wxTextCtrl* m_textCtrl8;
		wxButton* m_button15;
		wxStaticText* m_staticText1;
		wxTextCtrl* m_textRouteName;
		wxStaticText* m_staticText11;
		wxTextCtrl* m_textExtensionsFileName;
		wxButton* m_buttonAttach;
		wxButton* m_buttonAttach1;
		wxButton* m_buttonAttach2;
		wxStaticLine* m_staticline20;
		wxStaticLine* m_staticline21;
		wxStaticLine* m_staticline22;
		wxStaticLine* m_staticline23;
		wxStaticLine* m_staticline19;
		wxStaticLine* m_staticline24;
		wxButton* m_bButtonBearing1;
		wxStaticLine* m_staticline25;
		wxButton* m_button8;
		wxStaticLine* m_staticline26;
		wxStaticLine* m_staticline27;
		wxStaticLine* m_staticline28;
		wxStaticLine* m_staticline29;
		wxStaticLine* m_staticline30;
		wxStaticLine* m_staticline31;
		wxStaticLine* m_staticline32;
		wxButton* m_buttonSave;
		wxTimer m_timer1;
		wxMenu* m_mHelp;
		wxMenu* m_mRoutes;
		wxMenu* m_mExtensions;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void key_shortcut( wxKeyEvent& event ) { event.Skip(); }
		virtual void OnSize( wxSizeEvent& event ) { event.Skip(); }
		virtual void OnNewRoute( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveRoute( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLoadRoute( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDeleteRoute( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLoadExtensionsFile( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnIndex( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDirection( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnButtonEBL( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnButtonEBL_off( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveObjects( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTimer( wxTimerEvent& event ) { event.Skip(); }
		virtual void OnInformation( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAbout( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveExtensionsFile( wxCommandEvent& event ) { event.Skip(); }


	public:
		wxChoice* m_choiceRoutes;
		wxTextCtrl* m_Lat1;
		wxTextCtrl* m_Lon1;
		wxMenuBar* m_menubar3;

		finSAR_editUIDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("finSAR_edit"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 377,839 ), long style = wxCAPTION|wxCLOSE_BOX|wxFRAME_FLOAT_ON_PARENT|wxRESIZE_BORDER|wxSYSTEM_MENU|wxTAB_TRAVERSAL );

		~finSAR_editUIDialogBase();

};

