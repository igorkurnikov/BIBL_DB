/*! \file dialogs_bibl_db.cpp

    wxWidgets Dialogs for bibl db
 
    \author Igor Kurnikov  
    \date 2006-
*/

#define DIALOGS_BIBL_DB_CPP

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/dnd.h"
#include "wx/dataobj.h"
#include "wx/valgen.h"
#include "wx/grid.h"
#include "wx/dir.h"
#include "wx/filename.h"
#include "wx/wfstream.h"
#include "wx/listctrl.h"
#include "wx/combo.h"
#include "wx/tokenzr.h"
#include "wx/notebook.h"
#include "wx/sstream.h"
#include <wx/treebase.h>
#include <wx/busyinfo.h>

#include <vector>
#include <set>
#include <algorithm>
#include <string>
#include <fstream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace fs = boost::filesystem;
using namespace boost::algorithm;

#include "dialogs_bibl_db.h"
#include "ha_wx_res_5_wdr.h"
#include "script_editor.h"

#include "bibldb.h"

#include "mysql.h"

BiblDlg::BiblDlg(wxWindow *parent) : 
wxFrame(parent,  wxID_ANY, wxString("Bibliographic Database") )
{
	this->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    bibl_db = NULL;
	refs_grid = NULL;

	bibl_db = BiblDB::InitNewDB();
	 
	bibl_db->InitPython();

	wxColour back_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
 	SetBackgroundColour(back_colour);

	bibl_refs_1_page(this,true);

    wxMenuBar* bibl_db_menu_bar = bibl_db_menu();
    SetMenuBar(bibl_db_menu_bar); 
	
	bibl_cgi_url = "http://dumbo/cgi-bin/bibl1.cgi";

	OnInitDialog();
}


void BiblDlg::OnInitDialog()
{
	n_col_choose = 0;
	n_col_ref = 1;
	n_col_importance = 2;
	n_col_pdf = 3;
	n_col_doi = 4;
	n_col_isi = 5;
	n_col_supp = 6;
	n_col_citing_this = 7;
	n_col_cited_in = 8;

	wxTextCtrl* txt_ctrl;
 
    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_REF_OFFSET);
	txt_ctrl->SetValidator( wxGenericValidator(&breq.n_ref_offset));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_REF_LIMIT);
	txt_ctrl->SetValidator( wxGenericValidator(&breq.n_ref_limit));

    txt_ctrl = (wxTextCtrl*) FindWindow(REF_ID_FROM);
	txt_ctrl->SetValidator( StdStringValidator(&breq.ref_id_from));

    txt_ctrl = (wxTextCtrl*) FindWindow(REF_ID_TO);
	txt_ctrl->SetValidator( StdStringValidator(&breq.ref_id_to));

    txt_ctrl = (wxTextCtrl*) FindWindow(PUB_YEAR_FROM);
	txt_ctrl->SetValidator( StdStringValidator(&breq.pub_year_from));

    txt_ctrl = (wxTextCtrl*) FindWindow(PUB_YEAR_TO);
	txt_ctrl->SetValidator( StdStringValidator(&breq.pub_year_to));

    txt_ref_id_citing_this   = (wxTextCtrl*) FindWindow(IDC_CITED_ID);
	txt_ref_id_citing_this->SetDropTarget(new RefDropTarget(bibl_db, txt_ref_id_citing_this));

    txt_ref_id_cited_in_this = (wxTextCtrl*) FindWindow(IDC_CITED_IN_ID);
	txt_ref_id_cited_in_this->SetDropTarget(new RefDropTarget(bibl_db, txt_ref_id_cited_in_this));

    txt_ctrl = (wxTextCtrl*) FindWindow(AUTHOR_FLT_TXT);
	txt_ctrl->SetValidator( StdStringValidator(&breq.author_flt_str));
	
    txt_ctrl = (wxTextCtrl*) FindWindow(TITLE_WORDS_TXT);
	txt_ctrl->SetValidator( StdStringValidator(&breq.title_flt_str));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_JOURNAL_FLT);
    txt_ctrl->SetValidator( StdStringValidator(&breq.journal_flt));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_IMPORTANCE_FLT);
	txt_ctrl->SetValidator( StdStringValidator(&breq.importance_flt));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_REPRINT_FLT);
	txt_ctrl->SetValidator( StdStringValidator(&breq.reprint_flt));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_KEYWORDS_REFS_FLT);
	txt_ctrl->SetValidator( StdStringValidator(&breq.keywords_refs_flt));
	txt_ctrl->SetDropTarget(new KWTextDropTarget(bibl_db, txt_ctrl, this));

    refs_grid = (wxGrid*) FindWindow(IDC_REFS_GRID);

 //   refs_grid->CreateGrid( 0, 0 );
    refs_grid->AutoSizeRows();
	refs_grid->AutoSizeColumns(false);
	refs_grid->EnableDragGridSize();
	refs_grid->EnableDragColSize();
    refs_grid->SetRowMinimalAcceptableHeight(35);
//    refs_grid->AppendRows(10);
//    refs_grid->AppendCols(2);
    refs_grid->SetColLabelValue(n_col_choose,"CH");
	refs_grid->SetColFormatBool(n_col_choose);
    refs_grid->SetColLabelValue(n_col_ref,"REFERENCES");
	refs_grid->SetColLabelValue(n_col_importance,"IMP");
	refs_grid->SetColLabelValue(n_col_pdf,"PDF");
	refs_grid->SetColLabelValue(n_col_doi,"DOI");
	refs_grid->SetColLabelValue(n_col_isi,"ISI");
	refs_grid->SetColLabelValue(n_col_supp,"SUP");
	refs_grid->SetColLabelValue(n_col_citing_this,"citing\nthis");
	refs_grid->SetColLabelValue(n_col_cited_in,"cited\nin");

    refs_grid->SetColSize(n_col_choose,25);
    refs_grid->SetColSize(n_col_ref,500);
	refs_grid->SetColSize(n_col_importance,25);
	refs_grid->SetColSize(n_col_pdf,25);
	refs_grid->SetColSize(n_col_doi,25);
	refs_grid->SetColSize(n_col_isi,25);
	refs_grid->SetColSize(n_col_supp,25);
	refs_grid->SetColSize(n_col_citing_this,35);
	refs_grid->SetColSize(n_col_cited_in,35);

	refs_grid->SetRowLabelSize(60);
//	refs_grid->DisableCellEditControl();

	refs_grid->SetDropTarget(new RefsGridFileDropTarget(bibl_db, this, refs_grid));
	
	//refs_grid->DragAcceptFiles(true);
	//refs_grid->Connect(wxEVT_DROP_FILES, wxDropFilesEventHandler(BiblDlg::OnDropFiles), NULL, this);

	TransferDataToWindow();
}

bool BiblDlg::TransferDataToWindow()
{
	wxCheckBox* check_show_citing = (wxCheckBox*) FindWindow(IDC_SHOW_CITING);
	if( bibl_db->show_cit_flag )  check_show_citing->SetValue(true);
	else check_show_citing->SetValue(false);

    wxTextCtrl*  txt_ctrl;
	int id;
	std::string str, str2;

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_CITED_ID);
	id = atoi( breq.ref_id_citing_this.c_str());
	if( id != 0 )
	{
		BiblRef bref = bibl_db->GetRefByID(id);
		str = bref.ToString(2);
		str2 = boost::str(boost::format("(%d)%s") % bref.obj_id % str.c_str() );
		txt_ctrl->SetValue(str2.c_str());
	}

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_CITED_IN_ID);
	id = atoi( breq.ref_id_cited_in_this.c_str());
	if( id != 0 )
	{
		BiblRef bref = bibl_db->GetRefByID(id);
		str = bref.ToString(2);
		str2 = boost::str(boost::format("(%d)%s") % bref.obj_id % str.c_str() );
		txt_ctrl->SetValue(str2.c_str());
	}

    int i,n;
	n = refs_vec.size();

    int ir = refs_grid->GetNumberRows();

	wxTextCtrl* text_ctrl =  (wxTextCtrl*) FindWindow(IDC_NUM_SHOWN_REFS);
	text_ctrl->SetValue( wxString::Format("%d",n) );

    if(ir != n )
	{
		if( ir > 0) refs_grid->DeleteRows(0, ir);
		refs_grid->AppendRows(n);
	}
	
	for( i= 0; i < n; i++)
    {
		BiblRef& bref = refs_vec[i];
		if(bref.obj_id <= 0) continue;

		std::string str = bref.ToString(2);
		wxString str_wx(str.c_str(), wxConvUTF8);
//      refs_list->Append(str);
		std::string ref_id_str;
		ref_id_str = boost::str(boost::format("%d") % bref.obj_id);
//        refs_grid->SetRowSize(i,50);
        refs_grid->SetRowLabelValue(i,ref_id_str.c_str());
        
		wxString pdf_fname = bibl_db->GetLocPathPDF(bref);
		if( wxFile::Exists(pdf_fname.c_str()))
		  refs_grid->SetCellBackgroundColour(i, n_col_pdf, *wxGREEN);

		if(bibl_db->HasSupp(bref) )
		  refs_grid->SetCellBackgroundColour(i, n_col_supp, *wxGREEN);
        
		refs_grid->SetCellValue( i, n_col_ref,str_wx);
		str = boost::str(boost::format("%d") % bref.importance);
		if( bref.importance > 5) 
		{
			refs_grid->SetCellBackgroundColour(i, n_col_importance, *wxRED);
		}
		else if( bref.importance > 2)  
		{
			refs_grid->SetCellBackgroundColour(i, n_col_importance, *wxYELLOW);
		}
		else if( bref.importance > 0 )
		{
			refs_grid->SetCellBackgroundColour(i, n_col_importance, *wxLIGHT_GREY);
		}
		refs_grid->SetCellValue( i, n_col_importance, str);
		if( !bref.doi.empty() ) refs_grid->SetCellBackgroundColour(i, n_col_doi, *wxCYAN);
		if( !bref.isi_id.empty() ) refs_grid->SetCellBackgroundColour(i, n_col_isi, *wxBLUE);

		if( bibl_db->show_cit_flag ) 
		{
			std::vector<int> refs_citin = bibl_db->GetRefsCitedIn(bref.obj_id);
			str = boost::str(boost::format("%d\n(%d)") % refs_citin.size() % bref.num_cited_in);
			refs_grid->SetCellValue( i, n_col_cited_in, str);

			std::vector<int> refs_citing = bibl_db->GetRefsCiting(bref.obj_id);
			str = boost::str(boost::format("%d\n(%d)") % refs_citing.size() % bref.num_citing);
			refs_grid->SetCellValue( i, n_col_citing_this, str);
		}
		else
		{
			refs_grid->SetCellValue( i, n_col_cited_in, "");
			refs_grid->SetCellValue( i, n_col_citing_this, "");
		}
    }

//	refs_grid->AutoSizeColumns();
	refs_grid->AutoSizeRows();

	return wxFrame::TransferDataToWindow();
}

bool BiblDlg::TransferDataFromWindow()
{
	wxTextCtrl* txt_ctrl;
	std::string str, str2;
	int id;

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_CITED_ID);
	if( !txt_ctrl->IsEmpty() )
	{
		wxString str_wx = txt_ctrl->GetValue();
		str = str_wx.c_str();
		boost::trim(str);
	
		id = 0;
		if( !str.empty() )
		{
			if( str[0] == '(')
			{
				int iend = str.find(')');
				if( iend != std::string::npos )
				{
					str2 = str.substr(1,iend-1);
					str = str2;
				}
			}
			id = atoi( str.c_str());
		}
		wxLogMessage(" BiblDlg::TransferDataFromWindow() \n");
		wxLogMessage(" Citing this ID: %s ",str.c_str());
		if( id != 0 ) 
		{
			breq.ref_id_citing_this = str;
		}
		else
		{
			breq.ref_id_citing_this.clear();
		}
	}
	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_CITED_IN_ID);
	str = txt_ctrl->GetValue().c_str();
	boost::trim(str);
	
	id = 0;
	if( !str.empty() )
	{
		if( str[0] == '(')
		{
			int iend = str.find(')');
			if( iend != std::string::npos )
			{
				str2 = str.substr(1,iend-1);
				str = str2;
			}
		}	
		id = atoi( str.c_str());
		wxLogMessage(" Cited in this ID: %s",str.c_str());
	}
	if( id != 0 ) 
	{
		breq.ref_id_cited_in_this = str;
	}
	else
	{
		breq.ref_id_cited_in_this.clear();
	}

	return wxFrame::TransferDataFromWindow();
}

void BiblDlg::OnGridLabelLeftClick(wxGridEvent& event)
{
	int col = event.GetCol();
	int row = event.GetRow();

	wxLogMessage(" BiblDlg::OnGridLabelLeftClick() col =%d  row=%d \n",col,row);

	wxString str_val = refs_grid->GetRowLabelValue(row);

	wxTextDataObject textData(str_val);
	wxDropSource source(textData, this);

	int flags = 0;
	wxDragResult result = source.DoDragDrop(flags);
	switch ( result )
	{
		case wxDragError:  wxLogMessage(" Drop Error\n");    break;
		case wxDragNone:   wxLogMessage(" Drop Nothing\n");   break;
		case wxDragCopy:   wxLogMessage(" Drop Copied \n");    break;
		case wxDragMove:   wxLogMessage(" Drop Moved \n");    break;
		case wxDragCancel: wxLogMessage(" Drop Canceled \n");  break;
		default:           wxLogMessage(" Drop Unknown \n");      break;
	}
}

void BiblDlg::OnGridCellLeftClick(wxGridEvent& event)
{
	int col = event.GetCol();
	int row = event.GetRow();

	wxLogMessage(" BiblDlg::OnGridCellLeftClick() col =%d  row=%d \n",col,row);

	wxString cmd_url;

	if( col == 0)
	{
		wxString str_val = refs_grid->GetCellValue(row,col);
		if(str_val.IsEmpty())
		{
			str_val = "1";
		}
		else
		{
			str_val.Clear();
		}
		refs_grid->SetCellValue(row,col,str_val);
	}
//	else if( col == 1 && event.ControlDown() )
//	{
//		BiblRef& ref = refs_vec[row];
//
//		cmd_url =  bibl_cgi_url + "?do=get_paper";
//		cmd_url += "&j=";
//		cmd_url += ref.jrn.std_abbr.c_str();
//		cmd_url += "&y=";
//		cmd_url += ref.pub_year.c_str();
//		cmd_url += "&v=";
//		cmd_url += ref.vol.c_str();
//		cmd_url += "&is=";
//		cmd_url += ref.iss.c_str();
//		cmd_url += "&fp=";
//		cmd_url += ref.first_page.c_str();
//
//		cmd_url.Replace(" ","+");
//
//		wxLogMessage(" cmd_url: %s \n", cmd_url.c_str());
//
//		wxString cmd = pApp->html_browser.c_str();
//		cmd += " ";
//		cmd += cmd_url;

//		wxLaunchDefaultBrowser(cmd_url);

//		wxExecute(cmd);
//	}
//	else
//	{
		event.Skip();
//	}
}

void BiblDlg::OnGridCellRightClick(wxGridEvent& event)
{
	int col = event.GetCol();
	int row = event.GetRow();

	BiblRef& ref = refs_vec[row];

	wxString cmd_url;

	wxArrayString actions;

	actions.Add("Open PDF (Acrobat Reader)");
//	actions.Add("Open PDF (Acrobat)");
	actions.Add("Open Supplement (Acrobat Reader)");
	actions.Add("Open Reference Directory");
//	actions.Add("Edit Reference WEB");
	actions.Add("Edit Reference");
	actions.Add("Ref on Web of Science");
	actions.Add("Ref on Web of Science driver");
	actions.Add("Ref on PubMed");
	actions.Add("Ref on Google Scholar");
	actions.Add("Load Full Text from WEB");
	actions.Add("Find Paper on WEB by DOI");
	actions.Add("Find Paper draft on arXiv");
	actions.Add("Cited Refs on Web of Sci");
	actions.Add("Citing Refs on Web of Sci");
	actions.Add("Move Ref PDFs to the Ref Directory");
	actions.Add("Delete Ref Directory");
	actions.Add("Delete Ref PDF no subdir (old style)");

	wxString sres = wxGetSingleChoice("", "Action for the reference:", actions, NULL, -1, -1 , true, 200, 200,0);

    if( sres == "Edit Reference WEB")
	{
		wxString ref_id_str;
		ref_id_str.Printf("%d",ref.obj_id);
		ref_id_str.Trim(FALSE);
		ref_id_str.Trim(TRUE);

		cmd_url =  bibl_cgi_url + "?do=edit_ref";
		cmd_url += "&cur_ref_id=";
		cmd_url += ref_id_str;
		
		cmd_url.Replace(" ","+");

		wxLogMessage(" cmd_url: %s \n", cmd_url.c_str());
		
		wxLaunchDefaultBrowser(cmd_url);
	}
	if (sres == "Edit Reference")
	{
		EditRefDlg* edit_ref_dlg = new EditRefDlg(bibl_db, ref.obj_id, this);
		edit_ref_dlg->Show(TRUE);
	}
	else if (sres == "Ref on Web of Science")
	{
		bibl_db->GotoWOSRef(ref);
	}
	else if (sres == "Ref on Web of Science driver")
	{
		bibl_db->GotoWOSRefDriver(ref);
	}
	else if (sres == "Open Reference Directory")
	{
			bibl_db->OpenRefDir(ref); 
	}
	else if(sres == "Open PDF (Acrobat Reader)" )
	{
		if( !bibl_db->acro_read_str.empty() )
		{
//			wxString pdf_dir = bibl_db->GetPDFDir(ref).c_str();
//			::wxSetWorkingDirectory(pdf_dir);		
//			wxString cur_dir = ::wxGetCwd();
			
			fs::path loc_path = bibl_db->GetLocPathPDF(ref);
			fs::path pdf_dir_path = loc_path.parent_path();
			wxLogMessage(" Set Working Directory: %s \n", pdf_dir_path.c_str() );
			fs::current_path(pdf_dir_path);
			wxLogMessage(" Current Directory: %s \n", fs::current_path().c_str());

			wxString cmd;
			cmd = bibl_db->acro_read_str;
			cmd += " \"";
        	cmd += loc_path.c_str();
			cmd += "\"";
			wxLogMessage("cmd = %s",cmd.c_str());
			wxExecute(cmd);
		}
		else
		{
			wxLogMessage(" Acrobat Reader executable is not found \n");
		}
	}
//	else if(sres == "Open Supplement (Acrobat Reader)" )
//	{
//		if( !bibl_db->acro_read_str.empty() )
//		{
//		wxString loc_path = bibl_db->GetLocPathSupp(ref);
	//		wxString cmd = bibl_db->acro_read_str;
	//		cmd += " \"";
	//		cmd += loc_path;
	//		cmd += "\"";
	//		wxLogMessage("cmd = %s",cmd.c_str());
	//		wxExecute(cmd);
	//	}
	//	else
	//	{
	//		wxLogMessage(" Acrobat Reader executable is not found \n");
	//	}
	//}
	else if(sres == "Open PDF (Acrobat)" )
	{
		wxString adobe_path = "\"C:/Program Files/Adobe/Acrobat 7.0/";
		wxString acrobat_str = adobe_path;
		acrobat_str += "Acrobat/";
	    acrobat_str += "acrobat.exe";
		acrobat_str += "\"";
		wxString loc_path = bibl_db->GetLocPathPDF(ref);
		wxString cmd = acrobat_str;
		cmd += " ";
		cmd += loc_path;
		wxExecute(cmd);
	}
	else if(sres == "Load Full Text from WEB")
	{
		int ires = bibl_db->LoadRefFromWeb(ref);
		TransferDataToWindow();		
	}
	else if(sres == "Cited Refs on Web of Sci")
	{
//	     bibl_db->GotoWOSCitedRef2(ref);
		 bibl_db->GotoWOSCitedRefDriver(ref);
	}
	else if(sres == "Citing Refs on Web of Sci")
	{
//	     bibl_db->GotoWOSCitingRef(ref);
		 bibl_db->GotoWOSCitingRefDriver(ref);
	}
    else if(sres == "Find Paper on WEB by DOI")
	{
		if( ref.doi.empty() )
		{
			wxLogMessage("doi is not set for the reference \n");
			return ;
		}
		else
		{
			cmd_url =  "http://dx.doi.org/";
			cmd_url += ref.doi;

			wxLogMessage(" cmd_url: %s \n", cmd_url.c_str());

			wxLaunchDefaultBrowser(cmd_url);
		}
	}
    else if(sres == "Find Paper draft on arXiv")
	{
		if( ref.jrn.fname_abbr != "arxiv")
		{
			wxLogMessage("Reference doesn't have arXiv entry \n");
			return ;
		}
		else
		{
			if (ref.vol.empty())
			{
				wxLogMessage("Reference arXiv id (vol) is empty \n");
				return;
			}
			cmd_url =  "http://arxiv.org/abs/";
			cmd_url += ref.vol;

			wxLogMessage(" cmd_url: %s \n", cmd_url.c_str());

			wxLaunchDefaultBrowser(cmd_url);
		}
	}
	else if(sres == "Ref on PubMed")
	{
		if( ref.pubmed_id.empty() )
		{
			wxLogMessage("pubmed_id is not set for the reference \n");
			return ;
		}
		else
		{
			cmd_url =  "http://www.ncbi.nlm.nih.gov/sites/entrez?Db=pubmed&Cmd=ShowDetailView&TermToSearch=";
			cmd_url += ref.pubmed_id;

			wxLogMessage(" cmd_url: %s \n", cmd_url.c_str());

		    wxLaunchDefaultBrowser(cmd_url);

//			wxString cmd = pApp->html_browser.c_str();
//			cmd += " ";
//			cmd += cmd_url;
//			wxExecute(cmd);
		}
	}
	else if (sres == "Ref on Google Scholar")
	{
		bibl_db->GotoGScholarRefDriver(ref);
	}
	else if (sres == "Move Ref PDFs to the Ref Directory")
	{
		bibl_db->MoveRefPDFToStdLocation(ref);
	}
	else if (sres == "Delete Ref Directory")
	{
		std::string ref_dir = bibl_db->GetLocDir_default(ref);
		if (fs::is_directory(ref_dir))
		{
			wxString ref_dir_wx(ref_dir.c_str(), wxConvUTF8);
			wxString msg_txt = _("Do you want to delete directory: ") + ref_dir_wx;
			wxMessageDialog* dial = new wxMessageDialog(NULL, msg_txt, _("Quit"), wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
			int ires = dial->ShowModal();
			switch (ires)
			{
			case wxYES:
				wxLogMessage("Remove directory " + ref_dir_wx);
				fs::remove_all(ref_dir);
				break;
			case wxNO:
				break;
			default:
				break;
			};
		}
		std::string ref_doi_dir = bibl_db->GetLocDir_DOI(ref);
		if (fs::is_directory(ref_doi_dir))
		{
			wxString ref_doi_dir_wx(ref_doi_dir.c_str(), wxConvUTF8);
			wxString msg_txt = _("Do you want to delete Reference DOI directory: ") + ref_doi_dir_wx;
			wxMessageDialog* dial = new wxMessageDialog(NULL, msg_txt, _("Quit"), wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
			int ires = dial->ShowModal();
			switch (ires)
			{
			case wxYES:
				wxLogMessage("Remove Reference DOI directory " + ref_doi_dir_wx);
				fs::remove_all(ref_doi_dir);
				break;
			case wxNO:
				break;
			default:
				break;
			}
		}
	}
	else if (sres == "Delete Ref PDF no subdir (old style)")
	{
		std::string pdf_path_old = bibl_db->GetLocPathPDF_no_subdir(ref);
		if (fs::exists(pdf_path_old))
		{
			wxString pdf_path_old_wx(pdf_path_old.c_str(), wxConvUTF8);
			wxString msg_txt = _("Do you want to delete Reference PDF (No subdir old style) ") + pdf_path_old_wx;
			wxMessageDialog* dial = new wxMessageDialog(NULL, msg_txt, _("Quit"), wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
			int ires = dial->ShowModal();
			switch (ires)
			{
			case wxYES:
				wxLogMessage("Deleted Reference PDF (No subdir old style) " + pdf_path_old_wx);
				fs::remove_all(pdf_path_old);
				break;
			case wxNO:
				break;
			default:
				break;
			}
		}
	}
	else
	{
		event.Skip();
	}
	this->TransferDataToWindow();
}

void BiblDlg::OnEndLabelEdit(wxGridEvent& event)
{
	wxLogMessage(" Enter BiblDlg::OnEndLabelEdit() \n");
	
	int icol = event.GetCol();
	int irow = event.GetRow();
	
	wxString str_val = refs_grid->GetCellValue(irow,icol);

	long ival;
	bool bres;
//	double dval;
		
	if( irow > refs_vec.size()) return;
	BiblRef& bref = refs_vec[irow];

	if(icol == n_col_importance )
	{
		bres = str_val.ToLong(&ival);
		if(bres && ival != bref.importance)
		{
			bibl_db->SetImportanceForRef(bref.obj_id,ival);
			bref.importance = ival;
		}
		else
		{
			wxString str = wxString::Format("%d",bref.importance);
			refs_grid->SetCellValue(irow,icol,str);
		}
	}	
}


void BiblDlg::OnBrowseRefs(wxCommandEvent& WXUNUSED(event))
{
    bool bres = TransferDataFromWindow();
    if(!bres)
    {
        wxLogMessage("Invalid entries in the filter controls \n");
        return;
    }
	wxBusyCursor wait;
    
	wxStopWatch  sw;

    std::vector<int> ref_id_vec = bibl_db->SearchRefs(breq);
 	
	long time1 = sw.Time();

	wxLogMessage("\n Time to search references: %d ms \n", time1 );

//	bibl_db->GetRefsByID(ref_id_vec,refs_vec );

	int nref = ref_id_vec.size();
	refs_vec.resize(nref);
	
	int ir;
	for( ir = 0; ir < nref; ir++)
	{
//		bibl_db->UpdateAuthorStrForRef(ref_id_vec[ir]);
		refs_vec[ir] = bibl_db->GetRefByID(ref_id_vec[ir], 0);
//	    long time2 = ::wxGetElapsedTime();
//	    wxLogMessage("\n Time to get reference id= %d info: %d ms \n",ref_id_vec[ir], time2);
	}

	long time3 = sw.Time();
	wxLogMessage("\n Time to get reference info: %d ms \n", time3);

	TransferDataToWindow();

	long time4 = sw.Time();

	wxLogMessage("\n Time to display references: %d ms \n", time4);
}

void BiblDlg::OnBrowseKW(wxCommandEvent& WXUNUSED(event))
{
	wxTextCtrl* kw_text_ctrl =  (wxTextCtrl*) FindWindow(IDC_KEYWORDS_REFS_FLT);

	ObjAssocDlg* kw_dlg = new ObjAssocDlg(bibl_db, this);
	kw_dlg->Show(TRUE);
}

void BiblDlg::OnBrowseTopics(wxCommandEvent& WXUNUSED(event))
{
	wxTextCtrl* kw_text_ctrl =  (wxTextCtrl*) FindWindow(IDC_KEYWORDS_REFS_FLT);

	TopicsDlg* topics_dlg = new TopicsDlg(bibl_db, kw_text_ctrl, this);
	topics_dlg->Show(TRUE);
}

void BiblDlg::OnImportRefs(wxCommandEvent& event)
{
	ImportRefsDlg* import_refs_dlg = new ImportRefsDlg(bibl_db, this);
	import_refs_dlg->Show(TRUE);
}

bool comp_by_auth(const BiblRef& ref1, const BiblRef& ref2)
{
		return (ref1.authors_str < ref2.authors_str);
}

void 
BiblDlg::OnSortByAuthor(wxCommandEvent& event)
{
	std::sort(refs_vec.begin(),refs_vec.end(),comp_by_auth);
	TransferDataToWindow();
}

void BiblDlg::OnMarkAll(wxCommandEvent& event)
{
	int nr = refs_grid->GetNumberRows();
	int i;
	for(i = 0; i < nr; i++)
	{
		 refs_grid->SetCellValue(i,0,"1");
	}
	TransferDataToWindow();
}

void BiblDlg::OnUnMarkAll(wxCommandEvent& event)
{
	int nr = refs_grid->GetNumberRows();
	int i;
	for(i = 0; i < nr; i++)
	{
		 refs_grid->SetCellValue(i,0,"0");
	}
	TransferDataToWindow();
}


void BiblDlg::OnBrowseJournalsCGI(wxCommandEvent& WXUNUSED(event))
{
	wxString cmd_url =  bibl_cgi_url + "?do=browse_journals";		
	cmd_url.Replace(" ","+");

	wxLaunchDefaultBrowser(cmd_url);
}

void BiblDlg::OnBrowseJournals(wxCommandEvent& WXUNUSED(event))
{
	JournalsDlg* jrn_dlg = new JournalsDlg(bibl_db, "",this);
	jrn_dlg->Show(TRUE);
}

void BiblDlg::OnNewRef(wxCommandEvent& WXUNUSED(event))
{
	wxString cmd_url =  bibl_cgi_url + "?do=edit_ref&do_in_edit_ref=new_ref";		
	cmd_url.Replace(" ","+");
		
	wxLaunchDefaultBrowser(cmd_url);
}

void BiblDlg::OnNewBrowseRefsWin(wxCommandEvent& event)
{
	BiblDlg* p_dlg = new BiblDlg( NULL );
    p_dlg->Show(TRUE);
//	SetTopWindow(p_dlg);
}


void  BiblDlg::OnNewPythonWin(wxCommandEvent& event)
{
	PythonEditorFrame* frame = new PythonEditorFrame;
	frame->Layout();
	frame->Show();
}

void BiblDlg::OnInitWebDriver(wxCommandEvent& event)
{
	bibl_db->InitWebDriver();
}

void BiblDlg::OnUpdateWOSID(wxCommandEvent& event)
{
	bibl_db->InitWOS();
    TransferDataToWindow();
}

void BiblDlg::OnImportWOSFile(wxCommandEvent& event)
{
		wxString cmd_url =  bibl_cgi_url + "?do=import_bibl_file";		
		cmd_url.Replace(" ","+");

//		wxLogMessage(" cmd_url: %s \n", cmd_url.c_str());
		
		wxLaunchDefaultBrowser(cmd_url);
}

void BiblDlg::OnGotoWOSMarkedList(wxCommandEvent& event)
{
	bibl_db->GotoWOSMarkedList();
}	

void BiblDlg::OnUpdateWOSIDRemote(wxCommandEvent& event)
{
	bibl_db->InitWOSRemote();
    TransferDataToWindow();
}

void BiblDlg::GotoWOSGenSearch(wxCommandEvent& event)
{
	bibl_db->GotoWOSSearchPage();
}

void BiblDlg::WOSSearchPars(wxCommandEvent& event)
{
	TransferDataFromWindow();
	bibl_db->SearchWOSPars(breq);
}

void BiblDlg::GotoWOSMain(wxCommandEvent& event)
{
	bibl_db->GotoWOSMainPage();
//	if(bibl_db->wos_start_url.empty())
//	{
//		bibl_db->InitWOS();
//	}	
//	wxLaunchDefaultBrowser(bibl_db->wos_start_url);
}


void BiblDlg::OnClose(wxCloseEvent& event)
{
	if( bibl_db )
	{
		delete bibl_db;
		bibl_db = NULL;
	}
    event.Skip();
}

void BiblDlg::DeleteRefs(wxCommandEvent& event)
{
    TransferDataFromWindow();
	
	int nr = refs_grid->GetNumberRows();
	if( nr != refs_vec.size())
	{
		wxLogMessage(" Error in BiblDlg::DeleteRefs() \n");
		wxLogMessage(" The Number of rows does not equal to the number of references \n");
		return;
	}
	int i;

	wxArrayInt del_ids;

	for(i = 0; i < nr; i++)
	{
		 wxString str_val = refs_grid->GetCellValue(i,0);
		 if(!str_val.IsEmpty())
		 {
			int refid = refs_vec[i].obj_id;
			del_ids.Add(refid);
		 }
	}

	wxString msg = "Do you want to delete selected references with IDs: \n";
	
	int nd = del_ids.size();
	
	std::vector<int> del_ids_2(nd);
	
	for(i= 0; i < nd ; i++)
	{
		del_ids_2[i] = del_ids[i];
		wxString str;
		str.Printf("%d\n",del_ids[i]);
		msg += str;
	}

	int ires = wxMessageBox(msg,"Delete References",wxYES_NO);
	if(ires == wxYES)
	{
		wxLogMessage("Delete References \n");
		bibl_db->DeleteRefs(del_ids_2);
		
		std::vector<BiblRef>::iterator itr = refs_vec.begin();

		while(itr != refs_vec.end())
		{
			BiblRef& ref = *itr;
			int j;
			bool to_del = false;
			for( j = 0; j < nd; j++)
			{
				if( ref.obj_id == del_ids_2[j])
				{
					to_del = true;
				}
			}
			if( to_del )
			{
				itr     = refs_vec.erase(itr);
			}
			else
			{
				itr++;
			}
		}		
		TransferDataToWindow();
	}
	if(ires == wxNO)
	{
		wxLogMessage("Don't delete References \n");
	}
	
}

void BiblDlg::OnSelectAuthors(wxCommandEvent& event)
{
 	wxTextCtrl* auth_text_ctrl =  (wxTextCtrl*) FindWindow(AUTHOR_FLT_TXT);

	AuthDlg* auth_dlg = new AuthDlg(bibl_db, auth_text_ctrl,this);
	auth_dlg->Show(TRUE);   
}

void BiblDlg::OnShowCiting(wxCommandEvent& event)
{	
	wxCheckBox* p_chk_box = (wxCheckBox*) FindWindow(IDC_SHOW_CITING);
	bibl_db->show_cit_flag = p_chk_box->IsChecked() ? TRUE : FALSE;
	
	TransferDataToWindow();
}

void BiblDlg::SetKeyWordsRefs(wxCommandEvent& event)
{
	TransferDataFromWindow();
	
	std::string refs_flt = trim_copy(breq.keywords_refs_flt);

	if( refs_flt.empty() ) return;

	int nr = refs_grid->GetNumberRows();
	if( nr != refs_vec.size())
	{
		wxLogMessage(" Error in BiblDlg::SetKeyWordsRefs() \n");
		wxLogMessage(" Number of rows does not equal to the number of selected references \n");
		return;
	}
	int i,j;

	for(i = 0; i < nr; i++)
	{
		 wxString str_val = refs_grid->GetCellValue(i,0);
		 if(!str_val.IsEmpty())
		 {
			int ref_id = refs_vec[i].obj_id;
			bibl_db->SetObjKW( ref_id, refs_flt.c_str() );
		 }
	}
}

void BiblDlg::LoadWWWAllRefs(wxCommandEvent& event)
{	
	TransferDataFromWindow();
	int nr = refs_vec.size();
	int i;
	for(i = 0; i < nr; i++)
	{
        bibl_db->PrintRef1(refs_vec[i]);
		bibl_db->LoadRefFromWeb(refs_vec[i]);
	}
}

void BiblDlg::LoadWWWMarkedRefs(wxCommandEvent& event)
{	
	TransferDataFromWindow();
	int nr = refs_vec.size();
	int i;
	for(i = 0; i < nr; i++)
	{
		wxString str_val = refs_grid->GetCellValue(i,0);
		if( !str_val.IsEmpty())
		{
		   bibl_db->PrintRef1(refs_vec[i]);
		   bibl_db->LoadRefFromWeb(refs_vec[i]);
		}
	}
}

void BiblDlg::DelKeyWordsRefs(wxCommandEvent& event)
{
	TransferDataFromWindow();
	
	if(breq.keywords_refs_flt.empty()) return;
	
	int nr = refs_grid->GetNumberRows();
	if( nr != refs_vec.size())
	{
		wxLogMessage(" Error in BiblDlg::SetKeyWordsRefs() \n");
		wxLogMessage(" Number of rows does not equalt to the number of selected references \n");
		return;
	}
	int i,j;

	for(i = 0; i < nr; i++)
	{
		 wxString str_val = refs_grid->GetCellValue(i,0);
		
		 if(!str_val.IsEmpty())
		 {
			int refid = refs_vec[i].obj_id;
			bibl_db->DelObjKW( refid, breq.keywords_refs_flt );
		 }
	}
}

void BiblDlg::OnSetWOSIDVal(wxCommandEvent& event)
{
//	wxTextCtrl* wos_id_txt = (wxTextCtrl*) FindWindow(IDC_WOS_ID_VAL);
//	wxString wos_id = wos_id_txt->GetValue();
//	wos_id = wos_id.Strip(wxString::both);
//	bibl_db->wos_sid = wos_id;
}

void BiblDlg::OnDeleteRefs(wxCommandEvent& event)
{
	wxLogMessage("BiblDlg::OnDeleteRefs() \n");
	wxString str_sent = event.GetString();
	wxLogMessage("Sent string %s \n", str_sent.c_str());
}

//DEFINE_EVENT_TYPE(wxEVT_DELETE_REFS)

BEGIN_EVENT_TABLE(BiblDlg, wxFrame)
	EVT_COMMAND(wxID_ANY, wxEVT_DELETE_REFS, BiblDlg::OnDeleteRefs)
//	EVT_BUTTON(IDC_WOS_ID_VAL,     BiblDlg::OnSetWOSIDVal)
	EVT_MENU(BROWSE_REFS,        BiblDlg::OnBrowseRefs)
	EVT_MENU(DELETE_REFS,        BiblDlg::DeleteRefs)
	EVT_MENU(SET_KEYW_REFS,      BiblDlg::SetKeyWordsRefs)
	EVT_MENU(DELETE_KEYW_REFS,   BiblDlg::DelKeyWordsRefs)
	EVT_MENU(BROWSE_KEYW,        BiblDlg::OnBrowseKW)
	EVT_MENU(BROWSE_TOPICS,      BiblDlg::OnBrowseTopics)
	EVT_MENU(BROWSE_AUTH,        BiblDlg::OnSelectAuthors)
	EVT_MENU(IDM_BROWSE_JOURNALS,  BiblDlg::OnBrowseJournals)
	EVT_MENU(IDM_NEW_REF,          BiblDlg::OnNewRef)
	EVT_MENU(IDM_NEW_BROWSE_REFS_WIN, BiblDlg::OnNewBrowseRefsWin)
	EVT_MENU(IDM_PYTHON_WIN,       BiblDlg::OnNewPythonWin)
	EVT_BUTTON(IDC_SELECT_AUTH,    BiblDlg::OnSelectAuthors)
	EVT_CHECKBOX(IDC_SHOW_CITING,  BiblDlg::OnShowCiting)
	EVT_MENU(IDM_INIT_WEBDRIVER,   BiblDlg::OnInitWebDriver)
	EVT_MENU(UPDATE_WOS_IDS,       BiblDlg::OnUpdateWOSID)
	EVT_MENU(UPDATE_WOS_ID_REMOTE, BiblDlg::OnUpdateWOSIDRemote)
	EVT_MENU(IMPORT_WOS_FILES,     BiblDlg::OnImportWOSFile)
	EVT_MENU(IDM_GOTO_MAIN_WOS,    BiblDlg::GotoWOSMain)
	EVT_MENU(WOS_GO_MARKED_LIST,   BiblDlg::OnGotoWOSMarkedList)
	EVT_MENU(WOS_GEN_SEARCH,       BiblDlg::GotoWOSGenSearch)
	EVT_MENU(IDM_WOS_SEARCH_PARS,  BiblDlg::WOSSearchPars)
	EVT_MENU(IDC_LOAD_ALL_WWW,     BiblDlg::LoadWWWAllRefs)
	EVT_MENU(IDC_LOAD_MARKED_WWW,  BiblDlg::LoadWWWMarkedRefs)
	EVT_MENU(IDM_IMPORT_REFS,  BiblDlg::OnImportRefs)
	EVT_MENU(IDM_SORT_BY_AUTHOR,        BiblDlg::OnSortByAuthor)
	EVT_MENU(IDM_MARK_ALL,        BiblDlg::OnMarkAll)
	EVT_MENU(IDM_UNMARK_ALL,        BiblDlg::OnUnMarkAll)
	EVT_GRID_LABEL_LEFT_CLICK( BiblDlg::OnGridLabelLeftClick)
	EVT_GRID_LABEL_RIGHT_CLICK( BiblDlg::OnGridCellRightClick)
	EVT_GRID_CELL_LEFT_CLICK( BiblDlg::OnGridCellLeftClick)
	EVT_GRID_CELL_RIGHT_CLICK( BiblDlg::OnGridCellRightClick)
	EVT_GRID_CELL_CHANGED( BiblDlg::OnEndLabelEdit )
	EVT_CLOSE(BiblDlg::OnClose)
END_EVENT_TABLE()

bool RefDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& text)
{
	if( text.IsNumber() )
	{
		int id = atoi(text.c_str());
		
		if( id != 0 )
		{
			std::string str,str2;

			BiblRef bref = bibl_db->GetRefByID(id);
			str = bref.ToString(2);
			str2 = boost::str(boost::format("(%d)%s") % bref.obj_id % str.c_str() );
			ref_id_txt->SetValue(str2.c_str());
		}
		return true;
	}
    return false;
}

wxDragResult RefsGridFileDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult)
{
	wxPoint pt(x, y);

	wxGridCellCoords grid_crd = refs_grid->XYToCell(pt);

	int row_drag = grid_crd.GetRow();
	int col_drag = grid_crd.GetCol();

	if (last_cell_row != row_drag || last_cell_row != col_drag)
	{
		if (last_cell_row >= 0 && last_cell_row < refs_grid->GetNumberRows() &&
			last_cell_col >= 0 && last_cell_col < refs_grid->GetNumberCols())
		{
			refs_grid->SetCellBackgroundColour(last_cell_row, last_cell_col, last_cell_colour);
		}
		last_cell_row = row_drag;
		last_cell_col = col_drag;

		if (last_cell_row >= 0 && last_cell_row < refs_grid->GetNumberRows() &&
			last_cell_col >= 0 && last_cell_col < refs_grid->GetNumberCols())
		{
			last_cell_colour = refs_grid->GetCellBackgroundColour(last_cell_row, last_cell_col);
		}
		else
		{
			last_cell_colour = *wxWHITE;
		}
	}

	wxLogMessage("Dragging at row = %5d  col = %5d ", row_drag, col_drag);
	
	if (row_drag < 0 || row_drag >= refs_grid->GetNumberRows()) return defResult;
	if (col_drag < 0 || col_drag >= refs_grid->GetNumberCols()) return defResult;

	refs_grid->SetCellBackgroundColour(row_drag, col_drag, *wxYELLOW);
	bibl_dlg->TransferDataToWindow();

	return defResult;
}

bool RefsGridFileDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
	bool res = false;
	if (last_cell_row >= 0 && last_cell_row < refs_grid->GetNumberRows() &&
		last_cell_col >= 0 && last_cell_col < refs_grid->GetNumberCols())
	{
		refs_grid->SetCellBackgroundColour(last_cell_row, last_cell_col, last_cell_colour);
	}	
	
	if (filenames.GetCount()  > 0) {

		wxLogMessage("Dropping files");

		wxPoint pt(x,y);

		wxGridCellCoords grid_crd = refs_grid->XYToCell(pt);

		int row_drop = grid_crd.GetRow();
		int col_drop = grid_crd.GetCol();

		wxLogMessage("Dropped at row = %5d  col = %5d ", row_drop, col_drop);

		if (row_drop < 0 || col_drop < 0)
		{
			wxLogMessage(" Invalid drop grid row and/or column ");
			return false;
		}

		wxString str_val = refs_grid->GetRowLabelValue(row_drop);
		int ref_id = std::stoi(str_val.ToStdString());

		wxLogMessage(" ref_id = %d ", ref_id);
		BiblRef bref = bibl_db->GetRefByID(ref_id);

		wxBusyCursor busyCursor;
		wxWindowDisabler disabler;
		wxBusyInfo busyInfo(_("Adding files, wait please..."));

		wxString name;
		//		wxArrayString files;

		for (int i = 0; i < filenames.GetCount(); i++) {
			
			std::string fname = filenames[i].ToStdString();
			wxLogMessage(" Dropping file %s", name);

			if (col_drop == bibl_dlg->n_col_ref || col_drop == bibl_dlg->n_col_pdf)
			{
				wxLogMessage("Save PDF file ");
				bibl_db->SaveRefPDF(bref, fname);
			}

			//			if (wxFileExists(name))
			//				files.push_back(name);
			//			else if (wxDirExists(name))
			//				wxDir::GetAllFiles(name, &files);

		}
		res = true;
	}
	bibl_dlg->TransferDataToWindow();
	return res;
}

bool KWListDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& text_wx)
{ 
	return true;
}

wxDragResult KWListDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
    if ( !GetData() ) return wxDragNone;

    wxTextDataObject *dobj = (wxTextDataObject *)m_dataObject;
	if(!dobj) return wxDragNone;

 //   KWListDataObject* dobj_kw_list = (KWListDataObject*) dobj;

	std::string cat_str = cat_combo->GetStringSelection().ToStdString();

	wxString text_wx = dobj->GetText();

	int kw_id = 0;
	std::vector<std::string> kw_list;
	std::string cat_str_old = "";
	std::string kw;
	std::string text = text_wx.ToStdString();
	boost::split(kw_list,text,boost::is_any_of(";"));

	for( int i=0; i < kw_list.size(); i++)
	{
		boost::to_lower( kw_list[i]);
		boost::trim( kw_list[i]);
		if( kw_list[i].find("###") == 0 ) 
		{
			cat_str_old = kw_list[i].substr(3);
			kw_list[i] = "";
		}
	}

	if( cat_str_old == cat_str ) return wxDragNone;

//	if( !dobj_kw_list )
//	{
//		try
//		{
//			kw_id = boost::lexical_cast<int>(text);
//			kw = bibl_db->GetKeyWordByID(kw_id);
//		}
//		catch(boost::bad_lexical_cast &)
//		{
//			kw = text;
//			kw_id = bibl_db->GetKeyWordID(kw.c_str(),FALSE);
//		}
//	}

//	wxLogMessage(" KWListDropTarget::OnDropText kw = %s  kw_id= %d  cat_str_old= %s cat_str_new = %s ", 
//		           (wxString)kw, kw_id, (wxString) cat_str_old, (wxString) cat_str ); 

	wxString msg;
	if( def == wxDragCopy ) msg = "Are you sure you want to copy Keywords:  \n";
	else if( def == wxDragMove ) msg = "Are you sure you want to move Keywords:  \n";
	for( int i = 0; i < kw_list.size(); i++ )
	{
		std::string kw = kw_list[i];
		if( kw.empty() ) continue;
		msg += kw + " \n";
	}
	msg += wxString::Format("\n From category %s  to category  %s \n", (wxString)cat_str_old, (wxString) cat_str );

	int ires = wxNO;
	
	if( def == wxDragCopy ) ires = ::wxMessageBox(msg, "Confirm Copy KeyWord ", wxYES_NO);
	else if( def == wxDragMove ) ires = ::wxMessageBox(msg, "Confirm Move KeyWord ", wxYES_NO);

	if( ires == wxYES)
	{
//		if( dobj_kw_list ) 
//		{
			for( int i = 0; i < kw_list.size(); i++ )
			{
				std::string kw = kw_list[i];
				if( kw.empty() ) continue;
				bibl_db->AddKWToCategory(kw.c_str(),cat_str);
				if( !cat_str.empty() && !cat_str_old.empty() && def ==  wxDragMove ) bibl_db->DelKWFromCategory(kw.c_str(),cat_str_old);
			}
//		}
//		else
//		{
//			bibl_db->AddKWToCategory(kw.c_str(),cat_str);
//			bibl_db->DelKWFromCategory(kw.c_str(),cat_str_old);
//		}
		return wxDragMove;
	}
	return wxDragNone;
}


bool KWTextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& text_wx)
{ 
	return true;
}

wxDragResult KWTextDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
    if ( !GetData() ) return wxDragNone;

    wxTextDataObject *dobj = (wxTextDataObject *)m_dataObject;
	if(!dobj) return wxDragNone;

	std::string str_new = dobj->GetText().ToStdString();
	p_frame->TransferDataToWindow();
	wxString str_wx_old = kw_id_text_ctrl->GetValue();
	std::string str_old = str_wx_old.ToStdString();

	std::vector<std::string> kw_arr_old;
	std::vector<std::string> kw_arr_new;

	boost::split( kw_arr_old,str_old,boost::is_any_of(";"));
	boost::split( kw_arr_new,str_new,boost::is_any_of(";"));

	int i;
	for( i = 0; i < kw_arr_new.size(); i++)
		kw_arr_old.push_back( kw_arr_new[i] );

	str_new.clear();
	for( i = 0; i < kw_arr_old.size(); i++ )
	{
		std::string kw = kw_arr_old[i];
		boost::to_lower(kw);
		boost::trim(kw);
		int ipos = kw.find("###");
		if( ipos != std::string::npos) kw = kw.substr(0,ipos);
		if( kw.empty() ) continue;
		str_new += kw;
		if( i < (kw_arr_old.size() - 1) ) str_new += ";"; 
	}
	kw_id_text_ctrl->SetValue( (wxString) str_new ); 
	p_frame->TransferDataFromWindow();
	wxString str_check = kw_id_text_ctrl->GetValue();
	return wxDragCopy;
}

AuthDlg::AuthDlg(BiblDB* bibl_db_new, wxTextCtrl* auth_text_ctrl_new, wxWindow *parent) : 
wxFrame(parent, -1, wxString("Browse Authors") )
{
	this->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    bibl_db = bibl_db_new;
	auth_text_ctrl = auth_text_ctrl_new;

	auth_grid = NULL;

	wxColour back_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
 	SetBackgroundColour(back_colour);

	authors_dlg(this,TRUE);

	OnInitDialog();
}

BEGIN_EVENT_TABLE(AuthDlg, wxFrame)
    EVT_BUTTON(IDC_AUTH_SET_KW,         AuthDlg::OnSetKWMarked)
    EVT_BUTTON(IDC_AUTH_DEL_KW,         AuthDlg::OnDelKWMarked)
	EVT_BUTTON(IDC_AUTH_BROWSE_KW,      AuthDlg::OnBrowseKW)
    EVT_BUTTON(IDC_AUTH_SET_IMPORTANCE, AuthDlg::OnSetImportance)
	EVT_BUTTON(IDC_SEARCH_AUTH,         AuthDlg::OnSearchAuth)
	EVT_BUTTON(IDC_UPDATE_DISPLAY_OPT,  AuthDlg::OnUpdateDisplayOptions)
	EVT_GRID_CELL_LEFT_CLICK( AuthDlg::OnLeftClickGrid)
	EVT_GRID_CELL_RIGHT_CLICK( AuthDlg::OnRightClickGrid)
//	EVT_GRID_CELL_CHANGE( AuthDlg::OnEndLabelEdit )
    EVT_CLOSE(AuthDlg::OnClose)
END_EVENT_TABLE()


void AuthDlg::OnInitDialog()
{
	wxTextCtrl* txt_ctrl;

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_AUTH_NAME_FLT);
	txt_ctrl->SetValidator( StdStringValidator(&areq.search_author_name_flt));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_AUTH_IMPORTANCE);
	txt_ctrl->SetValidator( StdStringValidator(&areq.auth_importance_flt));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_KW_AUTH_FLT);
	txt_ctrl->SetValidator( StdStringValidator(&areq.keywords_auth_flt));

    auth_grid = (wxGrid*) FindWindow(IDC_AUTH_GRID);

	SetColumns();

    auth_grid->AutoSizeRows(TRUE);
	TransferDataToWindow();
}

void AuthDlg::SetColumns()
{
	wxCheckBox* check_box;

	std::vector<std::string> col_names;
	col_names.push_back("CH");
	col_names.push_back("NAME");

	check_box = (wxCheckBox*) FindWindow(IDC_SHOW_IMPORTANCE);
	if(check_box->GetValue()) col_names.push_back("IMP");
	
	check_box = (wxCheckBox*) FindWindow(IDC_SHOW_KW);
	if(check_box->GetValue()) col_names.push_back("KW");

	check_box = (wxCheckBox*) FindWindow(IDC_SHOW_URL);
	if(check_box->GetValue()) col_names.push_back("URL");

	check_box = (wxCheckBox*) FindWindow(IDC_SHOW_NOTES);
	if(check_box->GetValue()) col_names.push_back("NOTE");

	int num_cols = col_names.size();

	int ncol_act = auth_grid->GetNumberCols();
	if(ncol_act != num_cols)
	{
		auth_grid->DeleteCols(0,ncol_act);
		auth_grid->InsertCols(0,num_cols);
	}

	int i;
	for(i = 0; i < num_cols; i++)
	{
		auth_grid->SetColLabelValue(i,col_names[i].c_str());
		
		if( col_names[i] == "CH")
		{
			auth_grid->SetColFormatBool(i);
			auth_grid->SetColSize(i,20);
		}
		if( col_names[i] == "NAME")
		{
			auth_grid->SetColSize(i,100);
		}
		if( col_names[i] == "IMP")
		{
			auth_grid->SetColSize(i,30);
		}
		if( col_names[i] == "URL")
		{
			auth_grid->SetColSize(i,80);
		}
		if( col_names[i] == "NOTE")
		{
			auth_grid->SetColSize(i,100);
		}
		if( col_names[i] == "KW")
		{
			auth_grid->SetColSize(i,100);
		}
	}
	
//	auth_grid->AutoSizeColumns();
//	auth_grid->SetColLabelSize(21);
}


void AuthDlg::OnClose(wxCloseEvent& event)
{
    event.Skip();
}

void AuthDlg::OnEndLabelEdit(wxGridEvent& event)
{
	wxLogMessage(" Enter AuthDlg::OnEndLabelEdit() \n");

	int icol = event.GetCol();
	int irow = event.GetRow();
	
	wxString str_val = auth_grid->GetCellValue(irow,icol);

	long ival;
	bool bres;
		
	if( irow > auth_vec.size()) return;
	AuthorRef& auth_ref = auth_vec[irow];

	wxString col_label = auth_grid->GetColLabelValue(icol);

	if(col_label == "IMP" )
	{
		bres = str_val.ToLong(&ival);
		if(bres && ival != auth_ref.importance)
		{
			bibl_db->SetAuthImportance(auth_ref.obj_id, ival);
			auth_ref.importance = ival;
		}
	}
	if( col_label == "URL" )
	{		
		if(str_val != auth_ref.url)
		{
			bibl_db->SetAuthURL(auth_ref.obj_id, str_val.c_str() );
			auth_ref.url = str_val;
		}
	}
	if( col_label == "NOTE"  )
	{
		if(str_val != auth_ref.note)
		{
			bibl_db->SetAuthNote(auth_ref.obj_id,str_val.c_str() );
			auth_ref.note = str_val;
		}
	}

	std::string str_std = str_val.ToStdString();

	if( col_label == "KW"  )
	{
		if(str_val != auth_kw_vec[irow])
		{
			bibl_db->DelObjKW_All( auth_ref.obj_id );
			bibl_db->SetObjKW( auth_ref.obj_id, str_std );
			auth_kw_vec[irow] = bibl_db->GetObjKW(auth_ref.obj_id);
			auth_grid->SetCellValue( irow, icol, auth_kw_vec[irow] );
		}
	}
}

bool AuthDlg::TransferDataToWindow()
{
    int n = auth_vec.size();
    int nrow = auth_grid->GetNumberRows();

    if(nrow != n )
	{
		if( nrow > 0) auth_grid->DeleteRows(0, nrow);
		auth_grid->AppendRows(n);
	}
	nrow = n;

    int irow,icol;

	int ncol = auth_grid->GetNumberCols();

    for( irow= 0; irow < nrow; irow++)
    {
		AuthorRef auth_ref = auth_vec[irow];
		wxString auth_id_str;
		auth_id_str.Printf("%d",auth_ref.obj_id);
		auth_grid->SetRowLabelValue(irow,auth_id_str);
		
		wxString str;
		for( icol = 0; icol < ncol; icol++ )
		{
			wxString col_label = auth_grid->GetColLabelValue(icol);	
			if( col_label == "CH")
			{
				
			}
			if( col_label == "NAME")
			{
				wxString str = auth_ref.last_name + " " + auth_ref.initials;
				auth_grid->SetCellValue( irow, icol, str);
			}
			if( col_label == "IMP")
			{
				wxString str = wxString::Format("%d",auth_ref.importance);
				auth_grid->SetCellValue( irow, icol, str);
			}
			if( col_label == "URL")
			{
				auth_grid->SetCellValue( irow, icol, auth_ref.url);
			}
			if( col_label == "NOTE")
			{
				auth_grid->SetCellValue( irow, icol, auth_ref.note);
			}
			if( col_label == "KW")
			{
				auth_grid->SetCellValue( irow, icol, auth_kw_vec[irow]);
			}   
		}
    }

//	auth_grid->AutoSizeColumns();
//	auth_grid->AutoSizeRows();
	
	return wxFrame::TransferDataToWindow();
}

bool AuthDlg::TransferDataFromWindow()
{
	return wxFrame::TransferDataFromWindow();
}

void AuthDlg::OnSetKWMarked(wxCommandEvent& event)
{
	TransferDataFromWindow();

	if( areq.keywords_auth_flt.empty()) return;

	int nr = auth_grid->GetNumberRows();
	int i,j;

	for(i = 0; i < nr; i++)
	{
		 std::string mark_val = auth_grid->GetCellValue(i,0).ToStdString();
		 std::string auth_id_str = auth_grid->GetRowLabelValue(i).ToStdString();

		 int auth_id = atoi(auth_id_str.c_str());
		 
		 if(!mark_val.empty() && auth_id > 0)
		 {
			 bibl_db->SetObjKW( auth_id, areq.keywords_auth_flt );
		 }
	}
	OnSearchAuth(event);
	TransferDataToWindow();
}

void AuthDlg::OnDelKWMarked(wxCommandEvent& event)
{
	TransferDataFromWindow();

	if(areq.keywords_auth_flt.empty()) return;

	int nr = auth_grid->GetNumberRows();
	int i,j;

	for(i = 0; i < nr; i++)
	{
		 wxString mark_val = auth_grid->GetCellValue(i,0);
		 if(!mark_val.IsEmpty())
		 {
			bibl_db->DelObjKW( auth_vec[i].obj_id, areq.keywords_auth_flt );
		 }
	}
	
	OnSearchAuth(event);
	TransferDataToWindow();
}

void AuthDlg::OnBrowseKW(wxCommandEvent& event)
{
	wxTextCtrl* kw_text_ctrl =  (wxTextCtrl*) FindWindow(IDC_KW_AUTH_FLT);

	ObjAssocDlg* kw_dlg = new ObjAssocDlg(bibl_db,this);
	kw_dlg->Show(TRUE);	
}

void AuthDlg::OnUpdateDisplayOptions(wxCommandEvent& event)
{
	SetColumns();
	TransferDataToWindow();
}

void AuthDlg::OnSetImportance(wxCommandEvent& event)
{
	TransferDataFromWindow();

	int importance = 0;

	if(!areq.auth_importance_flt.empty())
	{
		importance = atoi(areq.auth_importance_flt.c_str());
	}

	int nr = auth_grid->GetNumberRows();
	int i;

	if( auth_vec.size() != nr)
	{
		wxLogMessage(" Error in AuthDlg::OnSetImportance() \n");
		wxLogMessage(" Mismatch of auth_vec size and the grid size \n");
		return;
	}

	for(i = 0; i < nr; i++)
	{
		 wxString mark_val = auth_grid->GetCellValue(i,0);
		 wxString auth_id_str = auth_grid->GetRowLabelValue(i);

		 int auth_id = atoi(auth_id_str.c_str());
		 
		 if(!mark_val.IsEmpty() && auth_id > 0)
		 {
			 bibl_db->SetAuthImportance( auth_id, importance );
			 auth_vec[i].importance = importance;	
		 }
	}
	OnSearchAuth(event);
	TransferDataToWindow();
}

void AuthDlg::OnSearchAuth(wxCommandEvent& event)
{
    bool bres = TransferDataFromWindow();
    if(!bres)
    {
        wxLogMessage("Invalid entries in the filter controls \n");
        return;
    }
	wxBusyCursor wait;
    std::vector<int> auth_id_vec = bibl_db->SearchAuths(areq);
	bibl_db->GetAuthsByID(auth_id_vec,auth_vec);
	
	int n = auth_vec.size();
	auth_kw_vec.resize(n);
	int i;
	for(i = 0; i < n; i++)
	{
		auth_kw_vec[i] = bibl_db->GetObjKW( auth_vec[i].obj_id);
	}
	TransferDataToWindow();
}


void AuthDlg::OnLeftClickGrid(wxGridEvent& event)
{
	int col = event.GetCol();
	int row = event.GetRow();

	if( col == 0)
	{
		wxString str_val = auth_grid->GetCellValue(row,col);
		if(str_val.IsEmpty())
		{
			str_val = "1";
		}
		else
		{
			str_val.Clear();
		}
		auth_grid->SetCellValue(row,col,str_val);
	}
	else if( col == 1 && event.ControlDown() )
	{

	}
	else
	{
		event.Skip();
	}
}

void AuthDlg::OnRightClickGrid(wxGridEvent& event)
{

}



EditRefDlg::EditRefDlg(BiblDB* bibl_db_new, int ref_id, wxWindow *parent) : 
wxFrame(parent, -1, wxString("Edit Reference") )
{
	this->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    bibl_db = bibl_db_new;

	if( ref_id > 0) bref = bibl_db->GetRefByID(ref_id);

	wxColour back_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
 	SetBackgroundColour(back_colour);

	wxMenuBar* edit_ref_menu_bar = edit_ref_menu();
    SetMenuBar(edit_ref_menu_bar); 

	top_sizer = edit_ref_dlg(this,TRUE);

	OnInitDialog();
}

BEGIN_EVENT_TABLE(EditRefDlg, wxFrame)
    EVT_BUTTON(IDC_SAVE_CHANGES,     EditRefDlg::OnSaveChanges)
	EVT_BUTTON(IDC_DISCARD_CHANGES,  EditRefDlg::OnDiscardChanges)
    EVT_BUTTON(IDC_NEW_REF,          EditRefDlg::OnNewRef)
	EVT_MENU(IDM_CURRENT_UPDATE_TIME, EditRefDlg::OnSetCurrentUpdateTime)
    EVT_CLOSE(EditRefDlg::OnClose)
END_EVENT_TABLE()


void EditRefDlg::OnInitDialog()
{
	wxTextCtrl* txt_ctrl;
	
    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_INT_ID);
    txt_ctrl->SetValidator( wxGenericValidator(&bref.obj_id));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_EXT_ID);
    txt_ctrl->SetValidator( StdStringValidator(&bref.ext_ref_id));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_AUTH);
    txt_ctrl->SetValidator( StdStringValidator(&bref.authors_str));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_BOOK_TITLE);
    txt_ctrl->SetValidator( StdStringValidator(&bref.book_title));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_TITLE);
    txt_ctrl->SetValidator( StdStringValidator(&bref.title));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_YEAR);
    txt_ctrl->SetValidator( StdStringValidator(&bref.pub_year));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_MONTH);
    txt_ctrl->SetValidator( StdStringValidator(&bref.pub_month));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_VOL);
    txt_ctrl->SetValidator( StdStringValidator(&bref.vol));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_ISS);
    txt_ctrl->SetValidator( StdStringValidator(&bref.iss));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_FIRST_PAGE);
    txt_ctrl->SetValidator( StdStringValidator(&bref.first_page));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_LAST_PAGE);
    txt_ctrl->SetValidator( StdStringValidator(&bref.last_page));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_INT_ISI_ID);
    txt_ctrl->SetValidator( wxGenericValidator(&bref.isi_id_int));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_EXT_ISI_ID);
    txt_ctrl->SetValidator( StdStringValidator(&bref.isi_id));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_PUBMED_ID);
    txt_ctrl->SetValidator( StdStringValidator(&bref.pubmed_id));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_PII_ID);
    txt_ctrl->SetValidator( StdStringValidator(&bref.pii_id));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_DOI);
    txt_ctrl->SetValidator( StdStringValidator(&bref.doi));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_IMPORTANCE);
    txt_ctrl->SetValidator( wxGenericValidator(&bref.importance));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_NUM_CITED_IN);
    txt_ctrl->SetValidator( wxGenericValidator(&bref.num_cited_in));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_NUM_CITING);
    txt_ctrl->SetValidator( wxGenericValidator(&bref.num_citing));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_URL);
    txt_ctrl->SetValidator( StdStringValidator(&bref.url));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_ABSTRACT);
    txt_ctrl->SetValidator( StdStringValidator(&bref.abstract_str));

	TransferDataToWindow();
}

bool EditRefDlg::TransferDataToWindow()
{	
	int ref_id = bref.obj_id;
	if( ref_id > 0)
	{
		bref = bibl_db->GetRefByID(ref_id);
	}

	wxComboBox* ref_type_combo = (wxComboBox*) FindWindow( IDC_REF_TYPE_COMBO );
	if(bref.ref_type < 4) 
	{
		ref_type_combo->SetSelection(bref.ref_type);
	}
	else
	{
		ref_type_combo->SetSelection(0);
	}

	if( bref.ref_type == 0 )
	{
		if( top_sizer->IsShown(3) )
		{
			top_sizer->Hide(3);
//			top_sizer->Layout();
			this->Layout();
//			this->SendSizeEvent();
		}
	}
	if( bref.ref_type == 1 || bref.ref_type == 2 || bref.ref_type == 3 )
	{
		if( !top_sizer->IsShown(3) )
		{
			top_sizer->Show((size_t)3);
//			top_sizer->Layout();
			this->Layout();
//			this->SendSizeEvent();
		}		
	}

	wxTextCtrl* txt_ctrl;
	int i,n;

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_JOURNAL_ID);
	txt_ctrl->SetValue(wxString::Format("%d",bref.jrn.obj_id));

	wxCheckBox* check_box_ctrl;
	check_box_ctrl = (wxCheckBox*) FindWindow(IDC_INCOMPLETE_AUTH);
	if(bref.incomplete_auth_flag)
	{
		check_box_ctrl->SetValue(true);
	}

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_KEYWORDS);
	txt_ctrl->SetValue(bref.keywords_str);
	
	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_JOURNAL);
	txt_ctrl->Clear();
	txt_ctrl->AppendText(bref.jrn.std_abbr);

	wxDateTime date((time_t)bref.last_update_time);

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_LAST_UPDATE);
	txt_ctrl->SetValue(date.FormatDate());
	
	return wxFrame::TransferDataToWindow();
}

bool EditRefDlg::TransferDataFromWindow()
{	
	wxTextCtrl* txt_ctrl;
	wxString str;
	wxString msg;
	wxCheckBox* check_box_ctrl;

	wxComboBox* ref_type_combo = (wxComboBox*) FindWindow( IDC_REF_TYPE_COMBO );
	bref.ref_type = ref_type_combo->GetSelection();

	check_box_ctrl = (wxCheckBox*) FindWindow(IDC_INCOMPLETE_AUTH);

	if(check_box_ctrl->IsChecked())
	{
		bref.incomplete_auth_flag = 1;
	}
	else
	{
		bref.incomplete_auth_flag = 0;
	}

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_KEYWORDS);
	str = txt_ctrl->GetValue();

	wxString kw_str_new;
	wxStringTokenizer tkz( str, ";");
	while( tkz.HasMoreTokens())
	{
		wxString keyw = tkz.GetNextToken();
		keyw = keyw.Strip(wxString::both);
		keyw.MakeLower();

		if(keyw.IsEmpty()) continue;

		int kw_id = bibl_db->GetKeyWordID(keyw.c_str());
		if( kw_id == 0)
		{
			msg = "Are you sure you want to create new keyword word: ";
			msg += keyw;
			int ires = ::wxMessageBox(msg,"New KeyWord Warning:",wxYES_NO);
			if(ires == wxNO)
			{
				return false;
			}
		}
		if( !kw_str_new.IsEmpty() )  kw_str_new += ";";
		kw_str_new += keyw;
	}
	bref.keywords_str = kw_str_new;


	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_AUTH);
	str = txt_ctrl->GetValue();
	bref.auth_vec.clear();
	tkz.SetString( str, ";");
	while( tkz.HasMoreTokens())
	{
		wxString token = tkz.GetNextToken();
		token = token.Strip(wxString::both);
		if( !token.IsEmpty())
		{
			token.MakeUpper();
			AuthorRef auth_ref;
			wxString ini;
			ini  = token.AfterFirst(',');
			if( !ini.IsEmpty()) // ; - separator
			{
				auth_ref.last_name = token.BeforeFirst(',');
				auth_ref.initials  = token.AfterFirst(',');
			}
			else
			{
				auth_ref.last_name = token.BeforeFirst(' ');
				auth_ref.initials  = token.AfterFirst(' ');
			}

			boost::trim(auth_ref.initials);
			
			int auth_id = bibl_db->GetAuthorID(auth_ref);
			if( auth_id == 0)
			{
				msg = "Are you sure you want to create new Author: ";
				msg += auth_ref.last_name;
				msg += ",";
				msg += auth_ref.initials;
				int ires = ::wxMessageBox(msg,"New Author Warning:",wxYES_NO);
				if(ires == wxNO)
				{
					return false;
				}		
			}
			bref.auth_vec.push_back(auth_ref);
		}
	}

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_TEXT_JOURNAL);
	str = txt_ctrl->GetValue();
	int jrn_id = bibl_db->GetJournalID(str.c_str());
	bref.jrn.Clear();
	if( jrn_id == 0)
	{
		msg = "Are you sure you want to create new Journal: ";
		msg += str;
		int ires = ::wxMessageBox(msg,"New Journal Warning:",wxYES_NO);
		if(ires == wxNO)
		{
			return false;
		}		
	}
	bref.jrn.std_abbr = str;

	bool bres = wxFrame::TransferDataFromWindow();
	if(bres) 
	{
		bool bres2 = bibl_db->UpdateReferenceDB(bref,TRUE);
		return bres2;
	}
	return false;
}

void 
EditRefDlg::OnClose(wxCloseEvent& event)
{
    event.Skip();
}

void 
EditRefDlg::OnSaveChanges(wxCommandEvent& event)
{
	TransferDataFromWindow();
	TransferDataToWindow();
}

void 
EditRefDlg::OnDiscardChanges(wxCommandEvent& event)
{
	TransferDataToWindow();
}

void 
EditRefDlg::OnNewRef(wxCommandEvent& event)
{

}

void EditRefDlg::OnSetCurrentUpdateTime(wxCommandEvent& event)
{
	bibl_db->SetCurrentUpdateTime(bref.obj_id);
	TransferDataToWindow();
}



JournalsDlg::JournalsDlg(BiblDB* bibl_db_new, const char* journal_filter, wxWindow *parent) : 
wxFrame(parent, -1, wxString("List Journals") )
{
	this->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    bibl_db = bibl_db_new;

	jreq.journal_name_flt = journal_filter;
	boost::trim(jreq.journal_name_flt);

	wxColour back_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
 	SetBackgroundColour(back_colour);

	wxMenuBar* journals_menu_bar = journals_menu();
    SetMenuBar(journals_menu_bar); 

	top_sizer = journals_dlg(this,TRUE);

	OnInitDialog();
}

BEGIN_EVENT_TABLE(JournalsDlg, wxFrame)
    EVT_BUTTON(IDC_NEW_JOURNAL,      JournalsDlg::OnNewJournal)
	EVT_BUTTON(IDC_UPDATE_JRN_LIST,  JournalsDlg::UpdateJournalList)
	EVT_MENU(IDM_MERGE_JOURNALS,  JournalsDlg::OnMergeJournals)
	EVT_MENU(IDC_LOAD_JRN_LIST_1, JournalsDlg::LoadJournalList1)
	EVT_MENU(IDC_LOAD_JRN_LIST_2, JournalsDlg::LoadJournalList2)
	EVT_MENU(IDC_LOAD_JRN_LIST_3, JournalsDlg::LoadJournalList3)
	EVT_GRID_CELL_RIGHT_CLICK( JournalsDlg::OnRightClick)
    EVT_CLOSE(JournalsDlg::OnClose)
END_EVENT_TABLE()


void
JournalsDlg::OnInitDialog()
{
	wxTextCtrl* txt_ctrl;

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_JRN_NAME_FILTER);
    txt_ctrl->SetValidator( StdStringValidator(&jreq.journal_name_flt));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_JID_FROM);
    txt_ctrl->SetValidator( StdStringValidator(&jreq.jrn_id_from));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_JID_TO);
    txt_ctrl->SetValidator( StdStringValidator(&jreq.jrn_id_to));

	jrn_grid = (wxGrid*) FindWindow(IDC_JRN_GRID);
	
//	jrn_grid->AutoSizeRows();

	jrn_grid->SetColLabelValue(0,"Journal Std Names:");
	jrn_grid->SetColSize(0,350);
	jrn_grid->SetColLabelValue(1,"Publisher ID");
	jrn_grid->SetColSize(1,150);

	TransferDataToWindow();
}

bool JournalsDlg::TransferDataToWindow()
{	
	sel_jrn_ids = bibl_db->FilterJournalIDs(jreq);

	int nj = sel_jrn_ids.size();
	int j;
	wxString str;

	int nrows = jrn_grid->GetNumberRows();
	if( nrows != nj)
	{
		if( nrows > 0 ) jrn_grid->DeleteRows(0,nrows);
		jrn_grid->AppendRows(nj);
	}
	
	for(j = 0; j < nj; j++)
	{
		JournalRef jref = bibl_db->GetJournalByID(sel_jrn_ids[j]);
		str.Printf("%d",jref.obj_id);
		jrn_grid->SetRowLabelValue(j,str);
		str = wxString::FromUTF8( jref.full_name.c_str() );
		str += "\n";
		str += wxString::FromUTF8( jref.std_abbr.c_str() );
		str += "\n";
		str += wxString::FromUTF8( jref.fname_abbr.c_str() ) + "    ";
		str += wxString::FromUTF8(jref.short_abbr.c_str()) + "   ";
		str += wxString::FromUTF8(jref.abbr_29.c_str()) ;

		jrn_grid->SetCellValue(j,0,str);
		jrn_grid->SetCellValue(j,1,jref.publisher_id);
	}

	jrn_grid->AutoSizeRows();
//	jrn_grid->AutoSizeColumn(0);

	return wxFrame::TransferDataToWindow();
}

bool
JournalsDlg::TransferDataFromWindow()
{	
	wxTextCtrl* txt_ctrl;
	wxString str;
	wxString msg;
	wxCheckBox* check_box_ctrl;

	return wxFrame::TransferDataFromWindow();
}

void 
JournalsDlg::OnClose(wxCloseEvent& event)
{
    event.Skip();
}

void 
JournalsDlg::OnNewJournal(wxCommandEvent& event)
{

}

void JournalsDlg::UpdateJournalList(wxCommandEvent& event)
{
	TransferDataFromWindow();
	TransferDataToWindow();
}

void JournalsDlg::OnMergeJournals(wxCommandEvent& event)
{
	TransferDataFromWindow();
	if( jreq.jrn_id_from.empty() ||  jreq.jrn_id_to.empty())
	{
		wxLogMessage("Target or Source journal IDs are invalid ");
		return;
	}
	int jid_from;
	int jid_to;

	try
	{
		jid_from = boost::lexical_cast<int>(jreq.jrn_id_from);
		jid_to   = boost::lexical_cast<int>(jreq.jrn_id_to);
	}
	catch(boost::bad_lexical_cast &)
	{
        wxLogMessage("Target or Source journal IDs are invalid ");
		return;
    }

	JournalRef jref_from = bibl_db->GetJournalByID(jid_from);
	JournalRef jref_to = bibl_db->GetJournalByID(jid_to);

	wxString str_from;
	str_from += wxString::Format("Journal ID = %d \n",jref_from.obj_id);
	str_from += wxString::Format("Full  Name = %s \n",jref_from.full_name.c_str());
	str_from += wxString::Format("Std   Abbreviation = %s \n",jref_from.std_abbr.c_str());
	str_from += wxString::Format("Short Abbreviation = %s \n",jref_from.short_abbr.c_str());
	
	wxString str_to;
	str_to += wxString::Format("Journal ID = %d \n",jref_to.obj_id);
	str_to += wxString::Format("Full  Name = %s \n",jref_to.full_name.c_str());
	str_to += wxString::Format("Std   Abbreviation = %s \n",jref_to.std_abbr.c_str());
	str_to += wxString::Format("Short Abbreviation = %s \n",jref_to.short_abbr.c_str());

	wxString msg;
	msg = "Are you sure you want to merge Journal: \n";
	msg += str_from;
	msg += "To Journal \n";
	msg += str_to;

	int ires = ::wxMessageBox(msg,"Merge Journals Warning:",wxYES_NO);
	if(ires == wxNO)
	{
		return;
	}
	bibl_db->MergeJournals(jid_from,jid_to);
}


void 
JournalsDlg::LoadJournalList1(wxCommandEvent& event)
{
	wxString import_file_name = ::wxFileSelector("Choose file with a list of Journals",
		"c:/bibl/ref/journals","jres_500.htm","htm","HTML files (*.htm)|*.htm|HTML files (*.html)|*.html");

    if( !import_file_name.IsEmpty())
	{
		wxFFile file_inp(import_file_name);
		if( !file_inp.IsOpened() ) 
		{
			wxLogMessage("\nFailed to open file %s \n", import_file_name.c_str());
			return;
		}
		bibl_db->LoadJournalList1(file_inp.fp());
	}
}

void JournalsDlg::LoadJournalList2(wxCommandEvent& event)
{
	wxString import_file_name = ::wxFileSelector("Choose file with a list of Journals",
		"c:/bibl/ref/journals","sci_eng_j_abbr.html","html","HTML files (*.htm)|*.htm|HTML files (*.html)|*.html");

    if( !import_file_name.IsEmpty())
	{
		wxFFile file_inp(import_file_name);
		if( !file_inp.IsOpened() ) 
		{
			wxLogMessage("\nFailed to open file %s \n", import_file_name.c_str());
			return;
		}
		bibl_db->LoadJournalList2(file_inp.fp());
	}
}

void 
JournalsDlg::LoadJournalList3(wxCommandEvent& event)
{
	wxString import_file_name = ::wxFileSelector("Choose file with a list of Journals",
		"c:/bibl/ref/journals","wos_journals_a.html","html","HTML files (*.htm)|*.htm|HTML files (*.html)|*.html");

    if( !import_file_name.IsEmpty())
	{
		bibl_db->LoadJournalList3(import_file_name.c_str());
	}
}


void JournalsDlg::OnRightClick(wxGridEvent& event)
{
	int col = event.GetCol();
	int row = event.GetRow();

	int jid = this->sel_jrn_ids[row];

	wxString cmd_url;

	wxArrayString actions;

	actions.Add("Edit Journal");

	wxString sres = wxGetSingleChoice("Action for the journal", "Choose one ", actions);

	if( sres == "Edit Journal")
	{
		EditJournalDlg* edit_jrn_dlg = new EditJournalDlg(bibl_db, jid ,this);
		edit_jrn_dlg->Show(TRUE);   
	}
	else
	{
		event.Skip();
	}
}


EditJournalDlg::EditJournalDlg(BiblDB* bibl_db_new, int jrn_id, wxWindow *parent) : 
wxFrame(parent, -1, wxString("Edit Journal") )
{
	this->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    bibl_db = bibl_db_new;

	jref = bibl_db->GetJournalByID(jrn_id);

	wxColour back_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
 	SetBackgroundColour(back_colour);

	top_sizer = jrn_edit_dlg(this,TRUE);

	OnInitDialog();
}

BEGIN_EVENT_TABLE(EditJournalDlg, wxFrame)
    EVT_BUTTON(IDC_SAVE_CHANGES,     EditJournalDlg::OnSaveChanges)
	EVT_BUTTON(IDC_DISCARD_CHANGES,  EditJournalDlg::OnDiscardChanges)
    EVT_BUTTON(IDC_NEW_JOURNAL,      EditJournalDlg::OnNewJournal)
    EVT_CLOSE(EditJournalDlg::OnClose)
END_EVENT_TABLE()


void EditJournalDlg::OnInitDialog()
{
	wxTextCtrl* txt_ctrl;

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_JOURNAL_ID);
    txt_ctrl->SetValidator( wxGenericValidator(&jref.obj_id));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_FULL_NAME);
    txt_ctrl->SetValidator( StdStringValidator(&jref.full_name));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_STD_ABBR);
    txt_ctrl->SetValidator( StdStringValidator(&jref.std_abbr));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_SHORT_ABBR);
    txt_ctrl->SetValidator( StdStringValidator(&jref.short_abbr));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_FNAME_ABBR);
    txt_ctrl->SetValidator( StdStringValidator(&jref.fname_abbr));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_ABBR_29);
    txt_ctrl->SetValidator( StdStringValidator(&jref.abbr_29));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_PUBLISHER_STR);
    txt_ctrl->SetValidator( StdStringValidator(&jref.publisher_str));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_PUBLISHER_ID);
    txt_ctrl->SetValidator( StdStringValidator(&jref.publisher_id));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_ISSN);
    txt_ctrl->SetValidator( StdStringValidator(&jref.issn));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_ESSN);
    txt_ctrl->SetValidator( StdStringValidator(&jref.essn));

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_NLM_ID);
    txt_ctrl->SetValidator( StdStringValidator(&jref.nlm_id));

	TransferDataToWindow();
}

bool EditJournalDlg::TransferDataToWindow()
{	
	wxTextCtrl* txt_ctrl;
	std::vector<std::string> syn_array = bibl_db->GetSynForJournal(jref.obj_id);
	
	int ns = syn_array.size();
	int i;
	wxString str;
	for(i=0; i < ns; i++)
	{
		str += syn_array[i];
		str +=";";
		str +="\n";
	}
	
	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_SYN_LIST);
	txt_ctrl->SetValue(str);
	
	return wxFrame::TransferDataToWindow();
}

bool EditJournalDlg::TransferDataFromWindow()
{	
	wxTextCtrl* txt_ctrl;

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_SYN_LIST);
	std::string str_syn =  txt_ctrl->GetValue().ToStdString();
	
	jref.synonyms.clear();
	std::vector<std::string> str_arr;
	boost::split(str_arr, str_syn, is_any_of(";") );

	int i;
	for(i = 0; i < str_arr.size(); i++)
	{
		boost::trim(str_arr[i]);
		if(!str_arr[i].empty()) jref.synonyms.insert(str_arr[i]);
	}

	return wxFrame::TransferDataFromWindow();
}

void EditJournalDlg::OnClose(wxCloseEvent& event)
{
    event.Skip();
}

void EditJournalDlg::OnSaveChanges(wxCommandEvent& event)
{
	TransferDataFromWindow();
	bibl_db->UpdateJournal(jref,TRUE);
	TransferDataToWindow();
}

void EditJournalDlg::OnDiscardChanges(wxCommandEvent& event)
{
	TransferDataToWindow();
}

void EditJournalDlg::OnNewJournal(wxCommandEvent& event)
{

}

ObjAssocDlg::ObjAssocDlg(BiblDB* bibl_db_new, wxWindow *parent) : 
wxFrame(parent, -1, wxString("Key Words Select") )
{
	this->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    bibl_db = bibl_db_new;

	wxColour back_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
 	SetBackgroundColour(back_colour);

//    wxMenuBar* bibl_db_menu_bar = bibl_db_menu();
//    SetMenuBar(bibl_db_menu_bar);    

	keywords_dlg(this,TRUE);

	OnInitDialog();
}

void ObjAssocDlg::OnInitDialog()
{
	obj_combo[0] = dynamic_cast<wxComboBox*>(FindWindow(IDC_COMBO_KW_0));
	obj_combo[1] = dynamic_cast<wxComboBox*>(FindWindow(IDC_COMBO_KW_1));
	obj_combo[2] = dynamic_cast<wxComboBox*>(FindWindow(IDC_COMBO_KW_2));
	obj_combo[3] = dynamic_cast<wxComboBox*>(FindWindow(IDC_COMBO_KW_3));

	filter_chk[0] = dynamic_cast<wxCheckBox*>(FindWindow(IDC_FLT_0));
	filter_chk[1] = dynamic_cast<wxCheckBox*>(FindWindow(IDC_FLT_1));
	filter_chk[2] = dynamic_cast<wxCheckBox*>(FindWindow(IDC_FLT_2));
	filter_chk[3] = dynamic_cast<wxCheckBox*>(FindWindow(IDC_FLT_3));

	filter_right_chk[0] = dynamic_cast<wxCheckBox*>(FindWindow(IDC_FILTER_RIGHT_0));
	filter_right_chk[1] = dynamic_cast<wxCheckBox*>(FindWindow(IDC_FILTER_RIGHT_1));
	filter_right_chk[2] = dynamic_cast<wxCheckBox*>(FindWindow(IDC_FILTER_RIGHT_2));
	filter_right_chk[3] = dynamic_cast<wxCheckBox*>(FindWindow(IDC_FILTER_RIGHT_3));

	filter_left_chk[0] = dynamic_cast<wxCheckBox*>(FindWindow(IDC_FILTER_LEFT_0));
	filter_left_chk[1] = dynamic_cast<wxCheckBox*>(FindWindow(IDC_FILTER_LEFT_1));
	filter_left_chk[2] = dynamic_cast<wxCheckBox*>(FindWindow(IDC_FILTER_LEFT_2));
	filter_left_chk[3] = dynamic_cast<wxCheckBox*>(FindWindow(IDC_FILTER_LEFT_3));

	ObjVec cat_arr;
	cat_arr = bibl_db->GetObjectsForCategory("category");
		
	std::sort(cat_arr.begin(), cat_arr.end(), CompareObjSPtrTypeString);

	int i,j;
	std::vector<std::string> topics_arr;
	std::vector<int> obj_ids_cur;
	std::vector<wxTreeItemId> tree_items_next;
	std::vector<wxTreeItemId> tree_items_cur;
	std::vector<int> obj_ids_next;

	obj_tree = (wxTreeCtrl*) FindWindow(ID_KWTREE_0);

	int id_root = bibl_db->GetKeyWordID("topic");
	TreeItemObjectData* p_root_obj_data = new TreeItemObjectData(id_root, KEYWORD_OBJ,"topic");

	tree_root_id = obj_tree->AddRoot("ROOT",-1,-1,p_root_obj_data);
	tree_items_next.push_back(tree_root_id);
	
	for(;;)
	{
		if( tree_items_next.empty() ) break;
		tree_items_cur = tree_items_next;
		tree_items_next.clear();
		for( i=0; i < tree_items_cur.size(); i++ )
		{
			wxTreeItemId item_id = tree_items_cur[i];
			TreeItemObjectData* p_obj_data = (TreeItemObjectData*) obj_tree->GetItemData( item_id );
		
			std::vector<int> assoc_obj_ids = bibl_db->GetAssocKWRight(p_obj_data->obj_id );
			for( j= 0; j < assoc_obj_ids.size(); j ++ )
			{
				int obj_id = assoc_obj_ids[j];
				std::string obj_str = bibl_db->GetKeyWordByID(obj_id);
				TreeItemObjectData* p_obj_data = new TreeItemObjectData(obj_id, KEYWORD_OBJ,obj_str);
				wxTreeItemId leaf_item_id = obj_tree->AppendItem(item_id, (wxString) obj_str,-1,-1, p_obj_data);
				tree_items_next.push_back(leaf_item_id);
			}
		}
	}
	for( i=0; i < 4; i++ )
	{
		for( j = 0; j < cat_arr.size(); j++ )
		{
			obj_combo[i]->Append( (wxString) cat_arr[j]->ToString() );
			wxBiblDataObject* p_obj_wx = new wxBiblDataObject(cat_arr[j]);
			obj_combo[i]->SetClientObject(j, p_obj_wx );
		}
	}
	int idx;
	idx = obj_combo[0]->FindString( (wxString)"all");
	if( idx != wxNOT_FOUND ) obj_combo[0]->SetSelection(idx);
	idx = obj_combo[1]->FindString( (wxString)"category");
	if( idx != wxNOT_FOUND ) obj_combo[1]->SetSelection(idx);
	idx = obj_combo[2]->FindString( (wxString)"all");
	if( idx != wxNOT_FOUND ) obj_combo[2]->SetSelection(idx);
	idx = obj_combo[3]->FindString( (wxString)"all");
	if( idx != wxNOT_FOUND ) obj_combo[3]->SetSelection(idx);

	obj_list_ctrl[0] = dynamic_cast<wxListCtrl*>(FindWindow(IDC_KWLIST_0));
	obj_list_ctrl[0]->InsertColumn(0,"OBJECTS");
	obj_list_ctrl[0]->SetDropTarget(new KWListDropTarget(bibl_db, obj_list_ctrl[0],obj_combo[0]));

	obj_list_ctrl[1] = dynamic_cast<wxListCtrl*>(FindWindow(IDC_KWLIST_1));
	obj_list_ctrl[1]->InsertColumn(0,"OBJECTS");
	obj_list_ctrl[1]->SetDropTarget(new KWListDropTarget(bibl_db, obj_list_ctrl[1],obj_combo[1]));

	obj_list_ctrl[2] = dynamic_cast<wxListCtrl*>(FindWindow(IDC_KWLIST_2));
	obj_list_ctrl[2]->InsertColumn(0,"OBJECTS");
	obj_list_ctrl[2]->SetDropTarget(new KWListDropTarget(bibl_db, obj_list_ctrl[2],obj_combo[2]));

	obj_list_ctrl[3] = dynamic_cast<wxListCtrl*>(FindWindow(IDC_KWLIST_3));
	obj_list_ctrl[3]->InsertColumn(0,"OBJECTS");
	obj_list_ctrl[3]->SetDropTarget(new KWListDropTarget(bibl_db, obj_list_ctrl[3],obj_combo[3]));

	obj_top_txt[0] = dynamic_cast<wxTextCtrl*>(FindWindow(IDC_OBJ_TOP_0));
	obj_top_txt[0]->SetDropTarget(new KWTextDropTarget(bibl_db, obj_top_txt[0], this));
	obj_top_txt[1] = dynamic_cast<wxTextCtrl*>(FindWindow(IDC_OBJ_TOP_1));
	obj_top_txt[1]->SetDropTarget(new KWTextDropTarget(bibl_db, obj_top_txt[1], this));
	obj_top_txt[2] = dynamic_cast<wxTextCtrl*>(FindWindow(IDC_OBJ_TOP_2));
	obj_top_txt[2]->SetDropTarget(new KWTextDropTarget(bibl_db, obj_top_txt[2], this));
	obj_top_txt[3] = dynamic_cast<wxTextCtrl*>(FindWindow(IDC_OBJ_TOP_3));
	obj_top_txt[3]->SetDropTarget(new KWTextDropTarget(bibl_db, obj_top_txt[3], this));

	obj_bot_txt[0] = dynamic_cast<wxTextCtrl*>(FindWindow( IDC_KW_TXT_0));
	obj_bot_txt[0]->SetDropTarget(new KWTextDropTarget(bibl_db, obj_bot_txt[0], this));
    obj_bot_txt[1] = dynamic_cast<wxTextCtrl*>(FindWindow( IDC_KW_TXT_1));
	obj_bot_txt[1]->SetDropTarget(new KWTextDropTarget(bibl_db, obj_bot_txt[1], this));
    obj_bot_txt[2] = dynamic_cast<wxTextCtrl*>(FindWindow( IDC_KW_TXT_2));
	obj_bot_txt[2]->SetDropTarget(new KWTextDropTarget(bibl_db, obj_bot_txt[2], this));
    obj_bot_txt[3] = dynamic_cast<wxTextCtrl*>(FindWindow( IDC_KW_TXT_3));
	obj_bot_txt[3]->SetDropTarget(new KWTextDropTarget(bibl_db, obj_bot_txt[3], this));

	text_kw1 = dynamic_cast<wxTextCtrl*>(FindWindow(IDC_TEXT_KW1));
	text_kw1->SetDropTarget(new KWTextDropTarget(bibl_db, text_kw1,this));
	text_kw2 = dynamic_cast<wxTextCtrl*>(FindWindow(IDC_TEXT_KW2));
	text_kw2->SetDropTarget(new KWTextDropTarget(bibl_db, text_kw2,this));

	fst_visible[0] = 0;
	fst_visible[1] = 0;
	fst_visible[2] = 0;
	fst_visible[3] = 0;

	TransferDataToWindow();
}

void ObjAssocDlg::OnClose(wxCloseEvent& event)
{
    event.Skip();
}

void ObjAssocDlg::GetObjFilters(int i)
{
	cat_id_vec[i].clear();
	obj_id_vec_bot[i].clear();
	sel_obj_id_filter_right[i].clear();
	sel_obj_id_filter_left[i].clear();
	sel_obj_id_filter_ndir[i].clear();

	int isel = obj_combo[i]->GetSelection();

	wxBiblDataObject* p_data = dynamic_cast<wxBiblDataObject*>(obj_combo[i]->GetClientObject(isel));
	
	int cat_id = 0;
	int obj_id = 0;
	std::string cat_str = p_data->obj_sptr->ToString();
	boost::to_lower(cat_str);

	if (cat_str != "all")
	{
		cat_id = p_data->obj_sptr->GetId();
		if (cat_id > 0) cat_id_vec[i].push_back(cat_id);
		sel_obj_id_filter_right[i].push_back(cat_id);
	}

	std::string top_obj_str = obj_top_txt[i]->GetValue().ToStdString();
	std::vector<std::string> tokens;
	boost::split(tokens, top_obj_str, boost::is_any_of(";"));
	for (std::string token : tokens)
	{
		boost::to_lower(token);
		boost::trim(token);
		obj_id = bibl_db->GetKeyWordID(token.c_str());
		if (obj_id > 0)
		{
			if (filter_chk[i]->IsChecked())
			{
				sel_obj_id_filter_ndir[i].push_back(obj_id);
			}
			else if (filter_right_chk[i]->IsChecked())
			{
				sel_obj_id_filter_right[i].push_back(obj_id);
			}
			else if (filter_left_chk[i]->IsChecked())
			{
				sel_obj_id_filter_left[i].push_back(obj_id);
			}
		}
	}

	std::string bot_obj_str = obj_bot_txt[i]->GetValue().ToStdString();
	boost::split(tokens, bot_obj_str, boost::is_any_of(";"));
	for (std::string token : tokens)
	{
		boost::to_lower(token);
		boost::trim(token);
		obj_id = bibl_db->GetKeyWordID(token.c_str(),1);
		if (obj_id > 0)
		{
			obj_id_vec_bot[i].push_back(obj_id);
		}
	}
}

bool ObjAssocDlg::TransferDataToWindow()
{
	std::vector<int> id_vec;

//	wxLogMessage(" KeyWordsDlg::TransferDataToWindow() \n");
//	wxLogMessage(" Number of selected KW %d \n", sel_kw_ids.GetCount());

	int i,j;
	for( i = 0; i < 4; i++)
	{
		GetObjFilters(i);
		
		if(  filter_chk[i]->IsChecked() || filter_right_chk[i]->IsChecked() || filter_left_chk[i]->IsChecked() )
		{
			id_vec = bibl_db->GetAssocObjIDs(cat_id_vec[i], sel_obj_id_filter_ndir[i], sel_obj_id_filter_right[i], sel_obj_id_filter_left[i]);
			this->obj_list[i] = bibl_db->LoadDataObjects(id_vec);
		}
		else
		{
			id_vec = bibl_db->GetAssocObjIDs(cat_id_vec[i]);
			this->obj_list[i] = bibl_db->LoadDataObjects(id_vec);
		}
		std::sort(this->obj_list[i].begin(), this->obj_list[i].end(), CompareObjSPtrTypeString );
		int nkw = this->obj_list[i].size();

		obj_list_ctrl[i]->DeleteAllItems();
		for( j = 0; j < nkw; j++)
		{
			wxListItem item;
			std::string obj_str = obj_list[i][j]->ToString();
			item.SetId(j);
			item.SetText( obj_str );
			// item.SetData(&obj_list[j]);
			long lres = obj_list_ctrl[i]->InsertItem(item);
		}

		for(j = 0; j < sel_obj_ids[i].size(); j++)
		{
  		    std::string kw_sel  = bibl_db->GetKeyWordByID(sel_obj_ids[i][j]);
			long ipos = obj_list_ctrl[i]->FindItem(-1, (wxString) kw_sel );
			if( ipos > -1 ) obj_list_ctrl[i]->SetItemState(ipos, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		}

		if( fst_visible[i] < nkw ) obj_list_ctrl[i]->EnsureVisible( fst_visible[i] );
	}

	return wxFrame::TransferDataToWindow();
}

void ObjAssocDlg::UpdateSelIDs()
{
	int i,j;

	for(i = 0; i < 4; i++)
	{
		long ires = obj_list_ctrl[i]->GetTopItem();
		if( ires < 0 )  fst_visible[i] = 0;
		else fst_visible[i] = ires; 

		sel_obj_ids[i].clear();
		sel_objs[i].clear();
		long item = -1;
		for(;;)
		{
			item = obj_list_ctrl[i]->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if( item == -1 ) break;

			// wxString kw  = obj_list_ctrl[i]->GetItemText(item);
			// ObjSPtr obj_sptr =  *((ObjSPtr*)obj_list_ctrl[i]->GetItemData(item));
			ObjSPtr obj_sptr = this->obj_list[i][item];

			//int kw_id = bibl_db->GetKeyWordID(kw.c_str(),FALSE);
			int obj_id = obj_sptr->GetId();
			if (obj_id > 0)
			{
				sel_obj_ids[i].push_back(obj_id);
				sel_objs[i].push_back(obj_sptr);
			}
		}
	}
}

bool ObjAssocDlg::TransferDataFromWindow()
{
//	wxLogMessage(" KeyWordsDlg::TransferDataFromWindow() \n");

	UpdateSelIDs();

	return wxFrame::TransferDataFromWindow();
}

void ObjAssocDlg::OnUpdateObjList(wxListEvent& event )
{
	TransferDataFromWindow();
	TransferDataToWindow();
}

void ObjAssocDlg::OnBeginDragObjListLeft(wxListEvent& event )
{
	OnBeginDragObjList( event, false );
}

void ObjAssocDlg::OnBeginDragObjListRight(wxListEvent& event )
{
	OnBeginDragObjList( event, true );
}

void ObjAssocDlg::OnBeginDragObjList(wxListEvent& event, bool right)
{
	TransferDataFromWindow();

	int id = event.GetId();
	wxString kw;
//	if( event.IsSelection()) kw = event.GetString();

	int key_code = event.GetKeyCode();

	enum DRAG_ACTION{COPY_OBJ = 0, MOVE_OBJ = 1} action;

	action = COPY_OBJ;

	if( right ) action = MOVE_OBJ;

	std::string cat_str = "";
	int idx_combo = -1;
	if( id == IDC_KWLIST_0 )  idx_combo = 0;
	if( id == IDC_KWLIST_1 )  idx_combo = 1;
	if( id == IDC_KWLIST_2 )  idx_combo = 2;
	if( id == IDC_KWLIST_3 )  idx_combo = 3;

	cat_str = obj_combo[idx_combo]->GetStringSelection().ToStdString();
	
	if( sel_obj_ids[idx_combo].size() == 0 ) return;

	for(int i = 0; i < sel_obj_ids[idx_combo].size(); i++ )
	{
		if( i > 0 ) kw += ";";
		kw += bibl_db->GetKeyWordByID( sel_obj_ids[idx_combo][i] );
	}
	kw += ";###" + wxString(cat_str);

	wxTextDataObject kwData(kw);

	wxDropSource source(kwData, this);
	int flags = wxDrag_CopyOnly;
	if( action == MOVE_OBJ ) flags = wxDrag_DefaultMove;
//	wxDragResult result = wxDragCancel;
	wxDragResult result = source.DoDragDrop(flags);
	switch ( result )
	{
		case wxDragError:  wxLogMessage(" Drop Error\n");    break;
		case wxDragNone:   wxLogMessage(" Drop Nothing\n");   break;
		case wxDragCopy:   wxLogMessage(" Drop Copied \n");    break;
		case wxDragMove:   wxLogMessage(" Drop Moved \n");    break;
		case wxDragCancel: wxLogMessage(" Drop Canceled \n");  break;
		default:           wxLogMessage(" Drop Unknown \n");      break;
	}
//	if( result == wxDragCopy || result == wxDragMove ) TransferDataToWindow();
	TransferDataToWindow();
}


void ObjAssocDlg::OnComboKWTextChange( wxCommandEvent& event)
{
	TransferDataFromWindow();
	TransferDataToWindow();
}


void ObjAssocDlg::OnFilterAssocObj(wxCommandEvent& event)
{
	UpdateSelIDs(); 
    TransferDataToWindow();
}

void ObjAssocDlg::OnAssocSelObj(wxCommandEvent& event)
{
	UpdateSelIDs();
	bibl_db->AssocObjIDs(sel_obj_ids[0], sel_obj_ids[1] );
	bibl_db->AssocObjIDs(sel_obj_ids[0], sel_obj_ids[2] );
	bibl_db->AssocObjIDs(sel_obj_ids[0], sel_obj_ids[3] );
	bibl_db->AssocObjIDs(sel_obj_ids[1], sel_obj_ids[2] );
	bibl_db->AssocObjIDs(sel_obj_ids[1], sel_obj_ids[3] );
	bibl_db->AssocObjIDs(sel_obj_ids[2], sel_obj_ids[3] );
	TransferDataToWindow();
}

void ObjAssocDlg::OnRemoveAssocSelObj(wxCommandEvent& event)
{
    UpdateSelIDs();
	bibl_db->DelAssocObjIDs(sel_obj_ids[0], sel_obj_ids[1] );
	bibl_db->DelAssocObjIDs(sel_obj_ids[0], sel_obj_ids[2] );
	bibl_db->DelAssocObjIDs(sel_obj_ids[0], sel_obj_ids[3] );
	bibl_db->DelAssocObjIDs(sel_obj_ids[1], sel_obj_ids[2] );
	bibl_db->DelAssocObjIDs(sel_obj_ids[1], sel_obj_ids[3] );
	bibl_db->DelAssocObjIDs(sel_obj_ids[2], sel_obj_ids[3] );
	TransferDataToWindow();
}

void ObjAssocDlg::OnAddDelObjAssoc(wxCommandEvent& event)
{
	TransferDataFromWindow();
	int id = event.GetId();

	int del_kw = FALSE;
	int add_kw = FALSE;

	int level = -1;
	
	if( id == IDC_KW_ADD_0) { level = 0; add_kw = TRUE; }  
	if( id == IDC_KW_DEL_0) { level = 0; del_kw = TRUE; }  
	if( id == IDC_KW_ADD_1) { level = 1; add_kw = TRUE; }  
	if( id == IDC_KW_DEL_1) { level = 1; del_kw = TRUE; }  
	if( id == IDC_KW_ADD_2) { level = 2; add_kw = TRUE; }  
	if( id == IDC_KW_DEL_2) { level = 2; del_kw = TRUE; }  
	if( id == IDC_KW_ADD_3) { level = 3; add_kw = TRUE; }  
	if( id == IDC_KW_DEL_3) { level = 3; del_kw = TRUE; }  

	GetObjFilters(level);

	if (add_kw)
	{
		bibl_db->AssocObjIDs(sel_obj_id_filter_right[level], sel_obj_ids[level]);
		bibl_db->AssocObjIDs(sel_obj_id_filter_right[level], obj_id_vec_bot[level]);
		bibl_db->AssocObjIDs(sel_obj_id_filter_ndir[level], sel_obj_ids[level]);
		bibl_db->AssocObjIDs(sel_obj_id_filter_ndir[level], obj_id_vec_bot[level]);
		bibl_db->AssocObjIDs(sel_obj_ids[level], sel_obj_id_filter_left[level]);
		bibl_db->AssocObjIDs(obj_id_vec_bot[level], sel_obj_id_filter_left[level]);
		bibl_db->AssocObjIDs(sel_obj_ids[level], sel_obj_id_filter_ndir[level]);
		bibl_db->AssocObjIDs(obj_id_vec_bot[level], sel_obj_id_filter_ndir[level]);
	}
	if( del_kw ) 
	{
		bibl_db->DelAssocObjIDs(sel_obj_id_filter_right[level], sel_obj_ids[level]);
		bibl_db->DelAssocObjIDs(sel_obj_id_filter_right[level], obj_id_vec_bot[level]);
		bibl_db->DelAssocObjIDs(sel_obj_id_filter_ndir[level], sel_obj_ids[level]);
		bibl_db->DelAssocObjIDs(sel_obj_id_filter_ndir[level], obj_id_vec_bot[level]);
		bibl_db->DelAssocObjIDs(sel_obj_ids[level], sel_obj_id_filter_left[level]);
		bibl_db->DelAssocObjIDs(obj_id_vec_bot[level], sel_obj_id_filter_left[level]);
		bibl_db->DelAssocObjIDs(sel_obj_ids[level], sel_obj_id_filter_ndir[level]);
		bibl_db->DelAssocObjIDs(obj_id_vec_bot[level], sel_obj_id_filter_ndir[level]);
	}
	TransferDataToWindow();
}

void ObjAssocDlg::OnEraseKW( wxCommandEvent& event)
{
    wxTextCtrl* text_kw1 = (wxTextCtrl*) FindWindow(IDC_TEXT_KW1);
	wxString kw1 = text_kw1->GetValue();

	wxString msg;
	msg = "Are you sure you want to delete Keyword \n\n" + kw1;
	int ires = ::wxMessageBox(msg, "Confirm Delete KeyWord ", wxYES_NO);

	if( ires == wxYES)
	{
		bibl_db->EraseKeyWord(kw1.c_str());
		TransferDataToWindow();
	}
}

void ObjAssocDlg::OnMergeKW( wxCommandEvent& event)
{
    wxTextCtrl* text_kw1 = (wxTextCtrl*) FindWindow(IDC_TEXT_KW1);
	wxTextCtrl* text_kw2 = (wxTextCtrl*) FindWindow(IDC_TEXT_KW2);
	wxString kw1 = text_kw1->GetValue();
	wxString kw2 = text_kw2->GetValue();

	wxString msg;
	msg = "Are you sure you want to Merge Keyword \n\n" + kw1;
	msg += "\nTo\n";
	msg += kw2;
	
	int ires = ::wxMessageBox(msg, "Confirm Merge KeyWords ", wxYES_NO);

	if( ires == wxYES)
	{
		bibl_db->MergeKeyWords(kw1.c_str(), kw2.c_str());
		TransferDataToWindow();
	}
}

void ObjAssocDlg::OnSetKW12( wxCommandEvent& event)
{
    wxTextCtrl* text_kw1 = (wxTextCtrl*) FindWindow(IDC_TEXT_KW1);
	wxTextCtrl* text_kw2 = (wxTextCtrl*) FindWindow(IDC_TEXT_KW2);
	wxString kw1 = text_kw1->GetValue();
	wxString kw2 = text_kw2->GetValue();

    bibl_db->SetKW2RefKW1(kw1.c_str(), kw2.c_str());
	TransferDataToWindow();
}

BEGIN_EVENT_TABLE(ObjAssocDlg, wxFrame)
//    EVT_BUTTON(BROWSE_REFS, KeyWordsDlg::OnBrowseRefs)
  EVT_LIST_BEGIN_DRAG( IDC_KWLIST_0, ObjAssocDlg::OnBeginDragObjListLeft )
  EVT_LIST_BEGIN_DRAG( IDC_KWLIST_1, ObjAssocDlg::OnBeginDragObjListLeft )
  EVT_LIST_BEGIN_DRAG( IDC_KWLIST_2, ObjAssocDlg::OnBeginDragObjListLeft )
  EVT_LIST_BEGIN_DRAG( IDC_KWLIST_3, ObjAssocDlg::OnBeginDragObjListLeft )
  EVT_LIST_BEGIN_RDRAG( IDC_KWLIST_0, ObjAssocDlg::OnBeginDragObjListRight )
  EVT_LIST_BEGIN_RDRAG( IDC_KWLIST_1, ObjAssocDlg::OnBeginDragObjListRight )
  EVT_LIST_BEGIN_RDRAG( IDC_KWLIST_2, ObjAssocDlg::OnBeginDragObjListRight )
  EVT_LIST_BEGIN_RDRAG( IDC_KWLIST_3, ObjAssocDlg::OnBeginDragObjListRight )
//  EVT_LIST_ITEM_SELECTED( IDC_KWLIST_0, KeyWordsDlg::OnUpdateKWList )
//   EVT_LIST_ITEM_DESELECTED( IDC_KWLIST_0, KeyWordsDlg::OnUpdateKWList )
//  EVT_LIST_ITEM_SELECTED( IDC_KWLIST_1, KeyWordsDlg::OnUpdateKWList )
//  EVT_LIST_ITEM_DESELECTED( IDC_KWLIST_1, KeyWordsDlg::OnUpdateKWList )
//  EVT_LIST_ITEM_SELECTED( IDC_KWLIST_2, KeyWordsDlg::OnUpdateKWList )
//  EVT_LIST_ITEM_DESELECTED( IDC_KWLIST_2, KeyWordsDlg::OnUpdateKWList )
//  EVT_LIST_ITEM_SELECTED( IDC_KWLIST_3, KeyWordsDlg::OnUpdateKWList )
//  EVT_LIST_ITEM_DESELECTED( IDC_KWLIST_3, KeyWordsDlg::OnUpdateKWList )
  EVT_BUTTON( IDC_KW_ASSOCIATE, ObjAssocDlg::OnAssocSelObj )
  EVT_BUTTON( IDC_KW_REMOVE_ASSOC, ObjAssocDlg::OnRemoveAssocSelObj )
  EVT_BUTTON( IDC_KW_ADD_0, ObjAssocDlg::OnAddDelObjAssoc)
  EVT_BUTTON( IDC_KW_ADD_1, ObjAssocDlg::OnAddDelObjAssoc)
  EVT_BUTTON( IDC_KW_ADD_2, ObjAssocDlg::OnAddDelObjAssoc)
  EVT_BUTTON( IDC_KW_ADD_3, ObjAssocDlg::OnAddDelObjAssoc)
  EVT_BUTTON( IDC_KW_DEL_0, ObjAssocDlg::OnAddDelObjAssoc)
  EVT_BUTTON( IDC_KW_DEL_1, ObjAssocDlg::OnAddDelObjAssoc)
  EVT_BUTTON( IDC_KW_DEL_2, ObjAssocDlg::OnAddDelObjAssoc)
  EVT_BUTTON( IDC_KW_DEL_3, ObjAssocDlg::OnAddDelObjAssoc)

  EVT_BUTTON( IDC_ERASE_KW1,           ObjAssocDlg::OnEraseKW) 
  EVT_BUTTON( IDC_MERGE_KW1_KW2,       ObjAssocDlg::OnMergeKW) 
  EVT_BUTTON( IDC_SET_KW2_TO_REFS_KW1, ObjAssocDlg::OnSetKW12)

  EVT_TEXT( IDC_COMBO_KW_0, ObjAssocDlg::OnComboKWTextChange)
  EVT_TEXT( IDC_COMBO_KW_1, ObjAssocDlg::OnComboKWTextChange)
  EVT_TEXT( IDC_COMBO_KW_2, ObjAssocDlg::OnComboKWTextChange)
  EVT_TEXT( IDC_COMBO_KW_3, ObjAssocDlg::OnComboKWTextChange)

  EVT_CHECKBOX(  IDC_FILTER_RIGHT_0, ObjAssocDlg::OnFilterAssocObj )
  EVT_CHECKBOX(  IDC_FILTER_RIGHT_1, ObjAssocDlg::OnFilterAssocObj )
  EVT_CHECKBOX(  IDC_FILTER_RIGHT_2, ObjAssocDlg::OnFilterAssocObj )
  EVT_CHECKBOX(  IDC_FILTER_RIGHT_3, ObjAssocDlg::OnFilterAssocObj )
  EVT_CHECKBOX(  IDC_FILTER_LEFT_0, ObjAssocDlg::OnFilterAssocObj )
  EVT_CHECKBOX(  IDC_FILTER_LEFT_1, ObjAssocDlg::OnFilterAssocObj )
  EVT_CHECKBOX(  IDC_FILTER_LEFT_2, ObjAssocDlg::OnFilterAssocObj )
  EVT_CHECKBOX(  IDC_FILTER_LEFT_3, ObjAssocDlg::OnFilterAssocObj )
  EVT_CHECKBOX(  IDC_FLT_0, ObjAssocDlg::OnFilterAssocObj )
  EVT_CHECKBOX(  IDC_FLT_1, ObjAssocDlg::OnFilterAssocObj )
  EVT_CHECKBOX(  IDC_FLT_2, ObjAssocDlg::OnFilterAssocObj )
  EVT_CHECKBOX(  IDC_FLT_3, ObjAssocDlg::OnFilterAssocObj )

  EVT_CLOSE(ObjAssocDlg::OnClose)
END_EVENT_TABLE()


BEGIN_EVENT_TABLE(TopicsDlg, wxFrame)
  EVT_CLOSE( TopicsDlg::OnClose)
  EVT_BUTTON( IDC_UPDATE_TOPICS, TopicsDlg::OnUpdateTopics)
END_EVENT_TABLE()

TopicsDlg::TopicsDlg(BiblDB* bibl_db_new, wxTextCtrl* kw_text_ctrl_new, wxWindow *parent) : 
wxFrame(parent, -1, wxString("Topics") )
{
	this->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    bibl_db = bibl_db_new;
	kw_text_ctrl = kw_text_ctrl_new;

	wxColour back_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
 	SetBackgroundColour(back_colour);

	topics_dlg(this,TRUE);

	OnInitDialog();
}

void TopicsDlg::OnInitDialog()
{
	TransferDataToWindow();
}

void TopicsDlg::OnClose(wxCloseEvent& event)
{
    event.Skip();
}

bool TopicsDlg::TransferDataToWindow()
{
	return wxFrame::TransferDataToWindow();
}


bool TopicsDlg::TransferDataFromWindow()
{
	return wxFrame::TransferDataFromWindow();
}

void TopicsDlg::OnUpdateTopics( wxCommandEvent& event)
{
	TransferDataToWindow();
}

ImportRefsDlg::ImportRefsDlg(BiblDB* bibl_db_new, wxWindow *parent) : 
wxFrame(parent, -1, wxString("Load References") )
{
	this->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    bibl_db = bibl_db_new;

	wxColour back_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
 	SetBackgroundColour(back_colour);

//    wxMenuBar* bibl_db_menu_bar = bibl_db_menu();
//    SetMenuBar(bibl_db_menu_bar);    

	import_refs_dlg(this,TRUE);

	OnInitDialog();
}

void ImportRefsDlg::OnInitDialog()
{
	wxTextCtrl* txt_ctrl;

    txt_ctrl = (wxTextCtrl*) FindWindow(IDC_KW);
    txt_ctrl->SetValidator( StdStringValidator(&ref_info.keywords_str));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_CITED_REF);
    txt_ctrl->SetValidator( StdStringValidator(&ref_info.cited_ref));

	txt_ctrl = (wxTextCtrl*) FindWindow(IDC_CITED_IN_REF);
    txt_ctrl->SetValidator( StdStringValidator(&ref_info.cited_in_ref));

	TransferDataToWindow();
}

void ImportRefsDlg::OnClose(wxCloseEvent& event)
{
    event.Skip();
}

bool ImportRefsDlg::TransferDataToWindow()
{
	return wxFrame::TransferDataToWindow();
}


bool ImportRefsDlg::TransferDataFromWindow()
{
	return wxFrame::TransferDataFromWindow();
}

void ImportRefsDlg::OnCleanRefsTextBox(wxCommandEvent& event)
{
    wxTextCtrl* refs_txt_ctrl = (wxTextCtrl*) FindWindow(IDC_REFS_TEXT);
	refs_txt_ctrl->Clear();
}

void ImportRefsDlg::OnLoadFromTextBox(wxCommandEvent& event)
{
	TransferDataFromWindow();

    wxTextCtrl* refs_txt_ctrl = (wxTextCtrl*) FindWindow(IDC_REFS_TEXT);
	wxString refs_text = refs_txt_ctrl->GetValue();

	wxRadioBox* format_radio_box = (wxRadioBox*) FindWindow(IDC_REF_FORMAT);

	wxStringInputStream stream(refs_text);

	wxString format = format_radio_box->GetStringSelection();

	if( format == "PubMed XML" )
	{
		bibl_db->ImportRefsPubMedXmlStr(refs_text.c_str());
	}
	else if( format == "ISI" )
	{
		bibl_db->ImportRefsISI(stream,&ref_info);
	}
	else if ( format == "BibTeX" )
	{
		bibl_db->ImportRefsBibTeXStr( refs_text.ToStdString(), &ref_info );
	}
	else if (format == "RIS")
	{
		std::stringstream is(refs_text.ToStdString());
		bibl_db->ImportRefsRIS(is, &ref_info);
	}
	else if (format == "NBIB")
	{
		std::stringstream is(refs_text.ToStdString());
		bibl_db->ImportRefsNBIB(is, &ref_info);
	}
}

void ImportRefsDlg::OnLoadFromFile(wxCommandEvent& event)
{
	TransferDataFromWindow();
	
	wxRadioBox* format_radio_box = (wxRadioBox*) FindWindow(IDC_REF_FORMAT);
	wxString format = format_radio_box->GetStringSelection();

	wxString frame_title;
	wxString fname_default;
	wxString ext_default;
	wxString wildcard;

    if(format == "PubMed XML")
	{
        frame_title = "Choose PubMed XML file";
		fname_default = "pubmed.xml";
		ext_default   = "xml";
		wildcard      = "*.xml|*.xml";
	}
	else if( format == "ISI")
	{
		frame_title   = "Choose ISI import reference file";
		fname_default = "savedrecs.ciw";
		ext_default = "ciw";
		wildcard      = "*.ciw|*.ciw|*.txt|*.txt";
	}
	else if (format == "RIS")
	{
		frame_title = "Choose RIS import reference file";
		fname_default = "citations.ris";
		ext_default = "ris";
		wildcard = "*.ris|*.ris|*.txt|*.txt";
	}
	else if (format == "BibTeX")
	{
		frame_title = "Choose BibTeX import reference file";
		fname_default = "citations.bib";
		ext_default = "bib";
		wildcard = "*.bib|*.bib|*.txt|*.txt";
	}
	else if (format == "NBIB")
	{
		frame_title = "Choose NBIB import reference file";
		fname_default = "citations.nbib";
		ext_default = "nbib";
		wildcard = "*.nbib|*.nbib|*.txt|*.txt";
	}

    wxString import_file_name = ::wxFileSelector(frame_title,"c:/Users/igor/Downloads",fname_default,
		ext_default,wildcard);

    if( !import_file_name.IsEmpty())
	{
		wxFFile file_inp(import_file_name);
		if( !file_inp.IsOpened() ) 
		{
			wxLogMessage("\nFailed to open file %s \n", import_file_name.c_str());
			return;
		}

		if(format == "PubMed XML")
		{
			bibl_db->ImportRefsPubMedXmlFile(file_inp.fp(),ref_info.keywords_str.c_str());
		}
		if(format == "ISI")
		{
			wxFFileInputStream inp_file_stream(file_inp);
			bibl_db->ImportRefsISI(inp_file_stream, &ref_info);
		}
		if (format == "RIS")
		{
			std::string fname = import_file_name.ToStdString();
			std::ifstream inp_file_std_stream(fname);
			bibl_db->ImportRefsRIS(inp_file_std_stream, &ref_info);
		}
		if (format == "NBIB")
		{
			std::string fname = import_file_name.ToStdString();
			std::ifstream inp_file_std_stream(fname);
			bibl_db->ImportRefsNBIB(inp_file_std_stream, &ref_info);
		}
	}
}

BEGIN_EVENT_TABLE(ImportRefsDlg, wxFrame)
  EVT_BUTTON( IDC_CLEAN_REF_TEXT_BOX,     ImportRefsDlg::OnCleanRefsTextBox) 
  EVT_BUTTON( IDC_LOAD_FROM_TEXT_BOX,     ImportRefsDlg::OnLoadFromTextBox) 
  EVT_BUTTON( IDC_LOAD_FROM_FILE,         ImportRefsDlg::OnLoadFromFile)
  EVT_CLOSE ( ImportRefsDlg::OnClose)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////////
// StdStringValidator
//
StdStringValidator::StdStringValidator(std::string* pstr_new)
{
	if(pstr_new == NULL)
	{
		printf("Error in StdStringValidator::StdStringValidator() \n"); 
		printf("Invalid String pointer \n");
	}
	pstr = pstr_new;
//	PrintLog(" pstr point to %s \n", (*pstr).c_str());
}

StdStringValidator::~StdStringValidator()
{

}
StdStringValidator::StdStringValidator(const StdStringValidator& val)
    : wxValidator()
{
    Copy(val);
}

bool StdStringValidator::Copy(const StdStringValidator& val)
{
    wxValidator::Copy(val);

    pstr = val.pstr;

    return true;
}

wxObject* StdStringValidator::Clone() const
{
	StdStringValidator* ptr = new StdStringValidator(pstr);
	return ptr;
}

bool StdStringValidator::TransferToWindow()
{
	wxTextCtrl* ctrl = (wxTextCtrl*) GetWindow();
	wxString str_wx((*pstr).c_str(), wxConvUTF8);
	ctrl->SetValue(str_wx);
	return true;
}

bool StdStringValidator::TransferFromWindow()
{
	wxTextCtrl* ctrl = (wxTextCtrl*) GetWindow();
	wxString str = ctrl->GetValue();
	*pstr = str.ToStdString();
	return true;
}

bool StdStringValidator::Validate()
{	
//	PrintLog("StdStringValidator::Validate() \n");
//	PrintLog(" pstr point to %s \n", (*pstr).c_str());
	return true;
}

