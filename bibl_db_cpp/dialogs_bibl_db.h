/*! \file dialogs_wx_5.h

    wxWindows Dialogs 5 in HARLEM 
 
    \author Igor Kurnikov  
    \date 2006-
*/

#if !defined(DIALOGS_WX_5_H)
#define DIALOGS_WX_5_H

class wxGrid;
class wxGridEvent;
class wxListEvent;
class wxListCtrl;

#include <wx/treebase.h>

#include "bibldb.h"

class BiblDlg : public wxFrame
{
public:
    BiblDlg(wxWindow *parent);

	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();
//	virtual bool IsTopLevel() const { return FALSE; }

	void OnInitDialog();

    void OnBtn1(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    void OnSetWOSIDVal( wxCommandEvent& event);
	
	void OnBrowseRefs(wxCommandEvent& event);
	void DeleteRefs(wxCommandEvent& event);
	void OnSelectAuthors(wxCommandEvent& event);
	void OnShowCiting(wxCommandEvent& event);
	void SetKeyWordsRefs(wxCommandEvent& event);
    void DelKeyWordsRefs(wxCommandEvent& event);
	void OnBrowseKW(wxCommandEvent& event);
	void OnBrowseTopics(wxCommandEvent& event);
	void OnBrowseJournalsCGI(wxCommandEvent& event);
	void OnBrowseJournals(wxCommandEvent& event);
	void OnNewRef(wxCommandEvent& event);
	void OnNewBrowseRefsWin(wxCommandEvent& event);
	void OnNewPythonWin(wxCommandEvent& event);
	void OnGridLabelLeftClick(wxGridEvent& event);
	void OnGridCellLeftClick(wxGridEvent& event);
	void OnGridCellRightClick(wxGridEvent& event);
	void OnEndLabelEdit(wxGridEvent& event);

	void OnDeleteRefs(wxCommandEvent& event);

	void OnInitWebDriver(wxCommandEvent& event);
	void OnUpdateWOSID(wxCommandEvent& event);
	void OnUpdateWOSIDRemote(wxCommandEvent& event);
	void OnImportWOSFile(wxCommandEvent& event);
	void GotoWOSMain(wxCommandEvent& event);
	void OnGotoWOSMarkedList(wxCommandEvent& event);
	void GotoWOSGenSearch(wxCommandEvent& event);
	void WOSSearchPars(wxCommandEvent& event);

	void LoadWWWAllRefs   (wxCommandEvent& event);
	void LoadWWWMarkedRefs(wxCommandEvent& event);
	void OnGSDialog(wxCommandEvent& event);

	void OnImportRefs(wxCommandEvent& event);
	void OnSortByAuthor(wxCommandEvent& event);
	void OnMarkAll(wxCommandEvent& event);
	void OnUnMarkAll(wxCommandEvent& event);

    BiblDB* bibl_db; 
	BibRefRequest breq;
	std::vector<BiblRef> refs_vec;

    wxGrid*  refs_grid;
	wxString bibl_cgi_url;

	wxTextCtrl* txt_ref_id_citing_this;
	wxTextCtrl* txt_ref_id_cited_in_this; 

	int n_col_choose;
	int n_col_ref;
	int n_col_importance;
	int n_col_pdf;
	int n_col_doi;
	int n_col_isi;
	int n_col_supp;
	int n_col_citing_this;
	int n_col_cited_in;

private:
    DECLARE_EVENT_TABLE()
};

class RefDropTarget : public wxTextDropTarget
{
public:
    RefDropTarget(BiblDB* bibl_db_new, wxTextCtrl *ref_id_txt_par) 
	{ 
		bibl_db = bibl_db_new;
		ref_id_txt = ref_id_txt_par; 
	}

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& text);

private:
    wxTextCtrl* ref_id_txt;
	BiblDB*     bibl_db; 
};

class RefsGridFileDropTarget : public wxFileDropTarget
{
public:
	RefsGridFileDropTarget(BiblDB* bibl_db_par, BiblDlg* bibl_dlg_par,  wxGrid* refs_grid_par)
	{
		bibl_db = bibl_db_par;
		bibl_dlg = bibl_dlg_par;
		refs_grid = refs_grid_par;
		last_cell_row = -1;
		last_cell_col = -1;
		last_cell_colour = *wxWHITE;
	}

	// virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& text);
	virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult);
	virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);

private:
	BiblDlg* bibl_dlg;
	wxGrid* refs_grid; 
	BiblDB* bibl_db;

	int last_cell_row;
	int last_cell_col;

	wxColour last_cell_colour;

};


class KWListDataObject : public wxTextDataObject
{
public: 
	KWListDataObject( wxString str_kw ): wxTextDataObject(str_kw) 
	{
		cat_str_old = "";
		action = "copy";
	}

	std::vector<std::string> kw_list;
	std::string cat_str_old;
	std::string action;
};

class KWListDropTarget : public wxTextDropTarget
{
public:
    KWListDropTarget(BiblDB* bibl_db_new, wxListCtrl* kw_id_list_par, wxComboBox* cat_combo_par ) 
	{ 
		bibl_db = bibl_db_new;
		kw_id_list = kw_id_list_par; 
		cat_combo = cat_combo_par;
	}

	virtual wxDragResult OnData	( wxCoord x, wxCoord y, wxDragResult defResult); 
	virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& text);

private:
    wxListCtrl*    kw_id_list;
	BiblDB*       bibl_db; 
	wxComboBox*   cat_combo;
};

class KWTextDropTarget : public wxTextDropTarget
{
public:
    KWTextDropTarget(BiblDB* bibl_db_new, wxTextCtrl *kw_id_text_ctrl_par, wxFrame* p_frame_par) 
	{ 
		bibl_db = bibl_db_new;
		kw_id_text_ctrl = kw_id_text_ctrl_par; 
		p_frame = p_frame_par;
	}

	virtual wxDragResult OnData	( wxCoord x, wxCoord y, wxDragResult defResult); 
	virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& text);

private:
    wxTextCtrl*  kw_id_text_ctrl;
	wxFrame*    p_frame;
	BiblDB*     bibl_db; 
};

class TreeItemObjectData : public wxTreeItemData
{
public:
	TreeItemObjectData()
	{
		obj_id = 0;
		obj_type = UNDEF_OBJ;
		obj_str = "";
	}

	TreeItemObjectData( int obj_id_par, OBJECT_TYPE obj_type_par, std::string obj_str_par )
	{
		obj_id = obj_id_par;
		obj_type = obj_type_par;
		obj_str = obj_str_par;
	}

	virtual ~TreeItemObjectData() {} 

	int obj_id;
	OBJECT_TYPE obj_type;
	std::string obj_str;
};

class ObjAssocDlg : public wxFrame
{
public:
    ObjAssocDlg(BiblDB* bibl_db_new, wxWindow *parent);

	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();

	void OnInitDialog();
	void OnClose(wxCloseEvent& event);
	void OnUpdateObjList(wxListEvent& event ); 
	void OnBeginDragObjListLeft( wxListEvent& event );
	void OnBeginDragObjListRight( wxListEvent& event );
	void OnBeginDragObjList(wxListEvent& event, bool right = false );
	void OnAddDelObjAssoc(wxCommandEvent& event);
	void OnAssocSelObj(wxCommandEvent& event);
    void OnRemoveAssocSelObj(wxCommandEvent& event);
	void OnFilterAssocObj(wxCommandEvent& event);

	void GetObjFilters(int i); //!< Get Arrays of object filters for col i 
	
	void OnEraseKW( wxCommandEvent& event);
	void OnMergeKW( wxCommandEvent& event);
	void OnSetKW12( wxCommandEvent& event);

	void OnComboKWTextChange( wxCommandEvent& event);

	void UpdateSelIDs();

    BiblDB* bibl_db; 

	wxTreeCtrl* obj_tree;
	wxTreeItemId tree_root_id;

	wxListCtrl* obj_list_ctrl[4];
	wxComboBox* obj_combo[4];
	wxTextCtrl* obj_top_txt[4];
	wxTextCtrl* obj_bot_txt[4];
	wxTextCtrl* text_kw1;
	wxTextCtrl* text_kw2;

	wxCheckBox* filter_chk[4];
	wxCheckBox* filter_right_chk[4];
	wxCheckBox* filter_left_chk[4];
	wxCheckBox* filter_ndir_right_chk[4];
	wxCheckBox* filter_ndir_left_chk[4];

	std::vector<int> sel_obj_id_filter_right[4];
	std::vector<int> sel_obj_id_filter_left[4];
	std::vector<int> sel_obj_id_filter_ndir[4];
	std::vector<int> cat_id_vec[4];
	std::vector<int> obj_id_vec_bot[4];

	// std::vector<ObjSPtr> obj_top_txt[4];
	// std::vector<ObjSPtr> obj_bot_txt[4];

	std::vector<ObjSPtr> obj_list[4];
	std::vector<int> sel_obj_ids[4];
	std::vector<ObjSPtr> sel_objs[4];
	long fst_visible[4]; 

private:
    DECLARE_EVENT_TABLE()
};

class TopicsDlg : public wxFrame
{
public:
    TopicsDlg(BiblDB* bibl_db_new, wxTextCtrl* kw_text_ctrl_new, wxWindow *parent);

	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();

	void OnInitDialog();
	void OnClose(wxCloseEvent& event);
	
	void OnUpdateTopics(wxCommandEvent& event);

    BiblDB* bibl_db; 
    wxTextCtrl* kw_text_ctrl;
	std::vector<int>  sel_kw_ids;

private:
    DECLARE_EVENT_TABLE()
};


class AuthDlg : public wxFrame
{
public:
    AuthDlg(BiblDB* bibl_db_new, wxTextCtrl* auth_text_ctrl_new, wxWindow *parent);

	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();

	void OnInitDialog();
	void OnClose(wxCloseEvent& event);
	void OnSetKWMarked(wxCommandEvent& event);
	void OnDelKWMarked(wxCommandEvent& event);
	void OnSearchAuth(wxCommandEvent& event);
	void OnBrowseKW(wxCommandEvent& event);
    void OnSetImportance(wxCommandEvent& event);
	void OnUpdateDisplayOptions(wxCommandEvent& event);
	void OnLeftClickGrid(wxGridEvent& event);
    void OnRightClickGrid(wxGridEvent& event);
	void OnEndLabelEdit(wxGridEvent& event);
	

	void SetColumns(); //!< Set columns of the author grid

	AuthorRequest areq;        //!< Author's request associated with the dialog
	std::vector<AuthorRef> auth_vec;          //!< Array of authors associated with the dialog
	std::vector<wxString> auth_kw_vec;  //!< Array of keyword strings

    BiblDB* bibl_db; 
    wxTextCtrl* auth_text_ctrl;
	wxGrid*  auth_grid;

//	int n_col_importance;
//	int n_col_url;
//	int n_col_note;

private:
    DECLARE_EVENT_TABLE()
};

class EditRefDlg : public wxFrame
{
public:
    EditRefDlg(BiblDB* bibl_db_new, int ref_id, wxWindow *parent);

	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();

	void OnInitDialog();
	void OnClose(wxCloseEvent& event);
	void OnSaveChanges(wxCommandEvent& event);
	void OnDiscardChanges(wxCommandEvent& event);
	void OnNewRef(wxCommandEvent& event);
	void OnSetCurrentUpdateTime(wxCommandEvent& event);

	BiblRef bref;        //!< Reference To Edit
	wxSizer* top_sizer;  //!< Top sizer of the dialog

    BiblDB* bibl_db; 

private:
    DECLARE_EVENT_TABLE()
};

class JournalsDlg : public wxFrame
{
public:
    JournalsDlg(BiblDB* bibl_db_new, const char* journal_flt, wxWindow *parent);

	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();

	void OnInitDialog();
	void OnClose(wxCloseEvent& event);
	void UpdateJournalList(wxCommandEvent& event);
	void OnMergeJournals(wxCommandEvent& event);
	void LoadJournalList1(wxCommandEvent& event);
	void LoadJournalList2(wxCommandEvent& event);
	void LoadJournalList3(wxCommandEvent& event);
	void OnRightClick(wxGridEvent& event);
	void OnNewJournal(wxCommandEvent& event);

	std::vector<int> sel_jrn_ids; //!< Journals ID selected
	
	JournalRequest jreq; //!< Journal Database request 

	wxSizer* top_sizer;  //!< Top sizer of the dialog
	wxGrid*  jrn_grid;   //!< Grid of Journals


    BiblDB* bibl_db; 

private:
    DECLARE_EVENT_TABLE()
};

class EditJournalDlg : public wxFrame
{
public:
    EditJournalDlg(BiblDB* bibl_db_new, int jrn_id , wxWindow *parent);

	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();

	void OnInitDialog();
	void OnClose(wxCloseEvent& event);
	void OnSaveChanges(wxCommandEvent& event);
	void OnDiscardChanges(wxCommandEvent& event);
	void OnNewJournal(wxCommandEvent& event);

	JournalRef jref;     //!< Journals To Edit
	wxArrayString syn_array; //!< Journal Synonyms Array 
	wxSizer* top_sizer;  //!< Top sizer of the dialog
	
    BiblDB* bibl_db; 

private:
    DECLARE_EVENT_TABLE()
};


class ImportRefsDlg : public wxFrame
{
public:
    ImportRefsDlg(BiblDB* bibl_db_new, wxWindow *parent);

	virtual bool TransferDataToWindow();
	virtual bool TransferDataFromWindow();

	void OnInitDialog();
	void OnClose(wxCloseEvent& event);
	void OnCleanRefsTextBox(wxCommandEvent& event);
	void OnLoadFromTextBox(wxCommandEvent& event);
	void OnLoadFromFile(wxCommandEvent& event);
	
    BiblDB* bibl_db; 
	BibRefInfo ref_info;
    wxTextCtrl* kw_text_ctrl;

private:
    DECLARE_EVENT_TABLE()
};

class StdStringValidator: public wxValidator
//! Validator for std::string
{
public: 
	StdStringValidator(std::string* pstr_new);
	virtual ~StdStringValidator();
    StdStringValidator(const StdStringValidator& val);  //mikola 09-13-2007 compatibility with gcc >=3.4
    bool Copy(const StdStringValidator& val);           //mikola 09-13-2007 compatibility with gcc >=3.4
	std::string*  pstr;

	virtual wxObject* Clone() const;
	virtual bool TransferToWindow();
	virtual bool TransferFromWindow();
	virtual bool Validate();
};

class wxBiblDataObject : public wxClientData
{
public:
	wxBiblDataObject(ObjSPtr& obj_sptr_ref)
	{
		obj_sptr = obj_sptr_ref;
	}
	ObjSPtr obj_sptr;
};


#endif // !defined(DIALOGS_WX_5_H)
