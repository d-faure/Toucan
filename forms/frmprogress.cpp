/////////////////////////////////////////////////////////////////////////////////
// Author:      Steven Lamerton
// Copyright:   Copyright (C) 2007-2009 Steven Lamerton
// License:     GNU GPL 2 (See readme for more info)
/////////////////////////////////////////////////////////////////////////////////

#include "frmmain.h"
#include "frmprogress.h"
#include "../variables.h"
#include "../script.h"
#include "../toucan.h"
#include "../controls/vdtc.h"

#include <wx/stdpaths.h>
#include <wx/listctrl.h>
#include <wx/textfile.h>

#include <shobjidl.h>
#undef Yield

//frmProgress event table
BEGIN_EVENT_TABLE(frmProgress, wxFrame)
	EVT_BUTTON(wxID_OK, frmProgress::OnOkClick)
	EVT_BUTTON(wxID_CANCEL, frmProgress::OnCancelClick)
	EVT_BUTTON(wxID_SAVE, frmProgress::OnSaveClick)
END_EVENT_TABLE()

//Constructor
frmProgress::frmProgress(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style){
	Init();
	wxFrame::Create(parent, id, caption, pos, size, style);
#ifdef __WXMSW__
	m_TaskBarId = RegisterWindowMessage(wxT("TaskbarButtonCreated"));
#endif
	CreateControls();
	Centre();
}

//frmProgress initialisation
void frmProgress::Init(){
	m_List = NULL;
	m_OK = NULL;
	m_Cancel = NULL;
	m_Save = NULL;
	m_Gauge = NULL;
	m_Taskbar = NULL;
}

//Create controls
void frmProgress::CreateControls(){
	frmProgress* Frame = this;

	wxPanel* Panel = new wxPanel(Frame, ID_PANEL_PROGRESS, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

	wxBoxSizer* TopSizer = new wxBoxSizer(wxVERTICAL);
	Panel->SetSizer(TopSizer);

	m_Gauge = new wxGauge(Panel, ID_PROGRESS_GAUGE, 100, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH|wxGA_HORIZONTAL);
	TopSizer->Add(m_Gauge, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 5);

	m_List = new wxListCtrl(Panel, ID_PROGRESS_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_VRULES);
	TopSizer->Add(m_List, 1, wxGROW|wxALL, 5);

	wxBoxSizer* ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(ButtonSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	m_OK = new wxButton(Panel, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	ButtonSizer->Add(m_OK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	m_Cancel = new wxButton(Panel, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	ButtonSizer->Add(m_Cancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	m_Save = new wxButton(Panel, wxID_SAVE, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
	ButtonSizer->Add(m_Save, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	
	//Add columns
	m_List->InsertColumn(0, _("Time"));
	m_List->InsertColumn(1, _("Status"));
	
	//Set the form icon
	wxString strPath = wxPathOnly(wxStandardPaths::Get().GetExecutablePath()) + wxFILE_SEP_PATH;
	this->SetIcon(wxIcon(strPath + wxT("Toucan.ico"), wxBITMAP_TYPE_ICO));
}

void frmProgress::OnOkClick(wxCommandEvent& WXUNUSED(event)){
	this->Close();
}

void frmProgress::OnCancelClick(wxCommandEvent& WXUNUSED(event)){
	wxGetApp().SetAbort(true);
}

void frmProgress::OnSaveClick(wxCommandEvent& WXUNUSED(event)){
	wxString strCaption = _("Save");
	wxString strWildcard;
	strWildcard = wxT("Text Files (*.txt)|*.txt");
	wxString defaultFilename = wxEmptyString;
	wxString defaultDir = wxT("/");
	wxFileDialog dialog(this, strCaption, defaultDir, defaultFilename, strWildcard, wxFD_SAVE);
	if(dialog.ShowModal() == wxID_OK){
		wxTextFile file(dialog.GetPath());
		if(wxFileExists(dialog.GetPath())){
			file.Open();
			file.Clear();
			file.Write();
		}
		else{
			file.Create();
		}
		for(int i = 0; i < m_List->GetItemCount() - 1; i++){
			wxListItem itemcol1, itemcol2;

			itemcol1.m_itemId = i;
			itemcol1.m_col = 0;
			itemcol1.m_mask = wxLIST_MASK_TEXT;
			m_List->GetItem(itemcol1);
			itemcol2.m_itemId = i;
			itemcol2.m_col = 1;
			itemcol2.m_mask = wxLIST_MASK_TEXT;
			m_List->GetItem(itemcol2);
			file.AddLine(itemcol1.m_text + wxT("\t") + itemcol2.m_text);
		}
	file.Write();
	}
}

#ifdef __WXMSW__
WXLRESULT frmProgress::MSWWindowProc(WXUINT message, WXWPARAM wparam, WXLPARAM lparam){
	if(message == m_TaskBarId){
		if(m_Taskbar){
			m_Taskbar->Release();
		}
		
		CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, (void**)&m_Taskbar);
		m_Taskbar->HrInit();
		m_Taskbar->SetProgressValue (static_cast<HWND>(this->GetHandle()), 50, 100);
	}
	return wxFrame::MSWWindowProc(message, wparam, lparam);
}
#endif