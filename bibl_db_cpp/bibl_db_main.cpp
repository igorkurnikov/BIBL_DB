#include <vector>
#include <set>
#include <iostream>
#include <fstream>

#include "wx/wx.h" 
#include "wx/dnd.h"
#include "wx/dataobj.h"

#include "bibldb.h"
#include "dialogs_bibl_db.h"

 
class BiblDBApp: public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();
};

IMPLEMENT_APP(BiblDBApp) 

class DebugFormatter : public wxLogFormatter
{
	virtual wxString Format(wxLogLevel level,
                            const wxString& msg,
                            const wxLogRecordInfo& info) const
    {
        return wxString::Format("%s : %s(%d) : %s",
            info.func, info.filename, info.line, msg);
    }
};

class PlainFormatter : public wxLogFormatter
{
	virtual wxString Format(wxLogLevel level,
                            const wxString& msg,
                            const wxLogRecordInfo& info) const
    {
        return wxString::Format("%s", msg);
    }
};

bool BiblDBApp::OnInit()
{
	int ires;
	
	BiblDlg* p_dlg = new BiblDlg( NULL );
	wxLogWindow* log_wnd = new wxLogWindow(NULL,"Bibliographic Database Log Window",true,false);
//	wxLogFormatter* prev_f = log_wnd->SetFormatter( new DebugFormatter() );
	wxLogFormatter* prev_f = log_wnd->SetFormatter( new PlainFormatter() );

	SetTopWindow(p_dlg);

	p_dlg->Show(TRUE);	

 	return TRUE;
} 

int BiblDBApp::OnExit()
{
#if defined(MYSQL_DB)
	if(BiblDB::db_mysql) mysql_close((MYSQL*)BiblDB::db_mysql);
#endif
	return TRUE;
}

void StartBiblDBApp()
{
	BiblDBApp* m_App = new BiblDBApp();
	m_App->OnInit();
}


void StartMainFrame()
{

}
