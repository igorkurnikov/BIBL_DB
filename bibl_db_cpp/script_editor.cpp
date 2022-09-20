// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all 'standard' wxWidgets headers)
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/filename.h>
#include "script_editor.h"
#include "Python.h"


//! language types
const CommonInfo g_CommonPrefs = {
    // editor functionality prefs
    true,  // syntaxEnable
    true,  // foldEnable
    true,  // indentEnable
    // display defaults prefs
    false, // overTypeInitial
    false, // readOnlyInitial
    false,  // wrapModeInitial
    false, // displayEOLEnable
    false, // IndentGuideEnable
    true,  // lineNumberEnable
    false, // longLineOnEnable
    false, // whiteSpaceEnable
};

//----------------------------------------------------------------------------
// keywordlists
// C++
const char* CppWordlist1 =
"asm auto bool break case catch char class const const_cast "
"continue default delete do double dynamic_cast else enum explicit "
"export extern false float for friend goto if inline int long "
"mutable namespace new operator private protected public register "
"reinterpret_cast return short signed sizeof static static_cast "
"struct switch template this throw true try typedef typeid "
"typename union unsigned using virtual void volatile wchar_t "
"while";
const char* CppWordlist2 =
"file";
const char* CppWordlist3 =
"a addindex addtogroup anchor arg attention author b brief bug c "
"class code date def defgroup deprecated dontinclude e em endcode "
"endhtmlonly endif endlatexonly endlink endverbatim enum example "
"exception f$ f[ f] file fn hideinitializer htmlinclude "
"htmlonly if image include ingroup internal invariant interface "
"latexonly li line link mainpage name namespace nosubgrouping note "
"overload p page par param post pre ref relates remarks return "
"retval sa section see showinitializer since skip skipline struct "
"subsection test throw todo typedef union until var verbatim "
"verbinclude version warning weakgroup $ @ \"\" & < > # { }";

// Python
const char* PythonWordlist1 =
"and assert break class continue def del elif else except exec "
"finally for from global if import in is lambda None not or pass "
"print raise return try while yield";
const char* PythonWordlist2 =
"ACCELERATORS ALT AUTO3STATE AUTOCHECKBOX AUTORADIOBUTTON BEGIN "
"BITMAP BLOCK BUTTON CAPTION CHARACTERISTICS CHECKBOX CLASS "
"COMBOBOX CONTROL CTEXT CURSOR DEFPUSHBUTTON DIALOG DIALOGEX "
"DISCARDABLE EDITTEXT END EXSTYLE FONT GROUPBOX ICON LANGUAGE "
"LISTBOX LTEXT MENU MENUEX MENUITEM MESSAGETABLE POPUP PUSHBUTTON "
"RADIOBUTTON RCDATA RTEXT SCROLLBAR SEPARATOR SHIFT STATE3 "
"STRINGTABLE STYLE TEXTINCLUDE VALUE VERSION VERSIONINFO VIRTKEY";


//----------------------------------------------------------------------------
//! languages
const LanguageInfo g_LanguagePrefs[] = {
    // C++
    {"C++",
     "*.c;*.cc;*.cpp;*.cxx;*.cs;*.h;*.hh;*.hpp;*.hxx;*.sma",
     wxSTC_LEX_CPP,
     {{mySTC_TYPE_DEFAULT, NULL},
      {mySTC_TYPE_COMMENT, NULL},
      {mySTC_TYPE_COMMENT_LINE, NULL},
      {mySTC_TYPE_COMMENT_DOC, NULL},
      {mySTC_TYPE_NUMBER, NULL},
      {mySTC_TYPE_WORD1, CppWordlist1}, // KEYWORDS
      {mySTC_TYPE_STRING, NULL},
      {mySTC_TYPE_CHARACTER, NULL},
      {mySTC_TYPE_UUID, NULL},
      {mySTC_TYPE_PREPROCESSOR, NULL},
      {mySTC_TYPE_OPERATOR, NULL},
      {mySTC_TYPE_IDENTIFIER, NULL},
      {mySTC_TYPE_STRING_EOL, NULL},
      {mySTC_TYPE_DEFAULT, NULL}, // VERBATIM
      {mySTC_TYPE_REGEX, NULL},
      {mySTC_TYPE_COMMENT_SPECIAL, NULL}, // DOXY
      {mySTC_TYPE_WORD2, CppWordlist2}, // EXTRA WORDS
      {mySTC_TYPE_WORD3, CppWordlist3}, // DOXY KEYWORDS
      {mySTC_TYPE_ERROR, NULL}, // KEYWORDS ERROR
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL},
      {-1, NULL}},
     mySTC_FOLD_COMMENT | mySTC_FOLD_COMPACT | mySTC_FOLD_PREPROC},
     // Python
     {"Python",
      "*.py;*.pyw",
      wxSTC_LEX_PYTHON,
      {{mySTC_TYPE_DEFAULT, NULL},
       {mySTC_TYPE_COMMENT_LINE, NULL},
       {mySTC_TYPE_NUMBER, NULL},
       {mySTC_TYPE_STRING, NULL},
       {mySTC_TYPE_CHARACTER, NULL},
       {mySTC_TYPE_WORD1, PythonWordlist1}, // KEYWORDS
       {mySTC_TYPE_DEFAULT, NULL}, // TRIPLE
       {mySTC_TYPE_DEFAULT, NULL}, // TRIPLEDOUBLE
       {mySTC_TYPE_DEFAULT, NULL}, // CLASSNAME
       {mySTC_TYPE_DEFAULT, PythonWordlist2}, // DEFNAME
       {mySTC_TYPE_OPERATOR, NULL},
       {mySTC_TYPE_IDENTIFIER, NULL},
       {mySTC_TYPE_DEFAULT, NULL}, // COMMENT_BLOCK
       {mySTC_TYPE_STRING_EOL, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL},
       {-1, NULL}},
      mySTC_FOLD_COMMENTPY | mySTC_FOLD_QUOTESPY},
      // * (any)
      {wxTRANSLATE(DEFAULT_LANGUAGE),
       "*.*",
       wxSTC_LEX_PROPERTIES,
       {{mySTC_TYPE_DEFAULT, NULL},
        {mySTC_TYPE_DEFAULT, NULL},
        {mySTC_TYPE_DEFAULT, NULL},
        {mySTC_TYPE_DEFAULT, NULL},
        {mySTC_TYPE_DEFAULT, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL},
        {-1, NULL}},
       0},
};

const int g_LanguagePrefsSize = WXSIZEOF(g_LanguagePrefs);

//----------------------------------------------------------------------------
//! style types
const StyleInfo g_StylePrefs[] = {
    // mySTC_TYPE_DEFAULT
    {"Default",
     "BLACK", "WHITE",
     "", 10, 0, 0},

     // mySTC_TYPE_WORD1
     {"Keyword1",
      "BLUE", "WHITE",
      "", 10, mySTC_STYLE_BOLD, 0},

     // mySTC_TYPE_WORD2
     {"Keyword2",
      "MIDNIGHT BLUE", "WHITE",
      "", 10, 0, 0},

     // mySTC_TYPE_WORD3
     {"Keyword3",
      "CORNFLOWER BLUE", "WHITE",
      "", 10, 0, 0},
     // mySTC_TYPE_WORD4
     {"Keyword4",
      "CYAN", "WHITE",
      "", 10, 0, 0},

     // mySTC_TYPE_WORD5
     {"Keyword5",
      "DARK GREY", "WHITE",
      "", 10, 0, 0},

     // mySTC_TYPE_WORD6
     {"Keyword6",
      "GREY", "WHITE",
      "", 10, 0, 0},

     // mySTC_TYPE_COMMENT
      {"Comment",
       "FOREST GREEN", "WHITE",
       "", 10, 0, 0},

     // mySTC_TYPE_COMMENT_DOC
     {"Comment (Doc)",
      "FOREST GREEN", "WHITE",
      "", 10, 0, 0},

      // mySTC_TYPE_COMMENT_LINE
      {"Comment line",
       "FOREST GREEN", "WHITE",
       "", 10, 0, 0},

       // mySTC_TYPE_COMMENT_SPECIAL
       {"Special comment",
        "FOREST GREEN", "WHITE",
        "", 10, mySTC_STYLE_ITALIC, 0},

        // mySTC_TYPE_CHARACTER
        {"Character",
         "KHAKI", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_CHARACTER_EOL
        {"Character (EOL)",
         "KHAKI", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_STRING
        {"String",
        "BROWN", "WHITE",
        "", 10, 0, 0},

        // mySTC_TYPE_STRING_EOL
        {"String (EOL)",
         "BROWN", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_DELIMITER
        {"Delimiter",
         "ORANGE", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_PUNCTUATION
        {"Punctuation",
         "ORANGE", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_OPERATOR
        {"Operator",
         "BLACK", "WHITE",
         "", 10, mySTC_STYLE_BOLD, 0},

        // mySTC_TYPE_BRACE
        {"Label",
         "VIOLET", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_COMMAND
        {"Command",
         "BLUE", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_IDENTIFIER
        {"Identifier",
         "BLACK", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_LABEL
        {"Label",
         "VIOLET", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_NUMBER
        {"Number",
         "SIENNA", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_PARAMETER
        {"Parameter",
         "VIOLET", "WHITE",
         "", 10, mySTC_STYLE_ITALIC, 0},

        // mySTC_TYPE_REGEX
        {"Regular expression",
         "ORCHID", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_UUID
        {"UUID",
         "ORCHID", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_VALUE
        {"Value",
         "ORCHID", "WHITE",
         "", 10, mySTC_STYLE_ITALIC, 0},

        // mySTC_TYPE_PREPROCESSOR
        {"Preprocessor",
         "GREY", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_SCRIPT
        {"Script",
         "DARK GREY", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_ERROR
        {"Error",
         "RED", "WHITE",
         "", 10, 0, 0},

        // mySTC_TYPE_UNDEFINED
        {"Undefined",
         "ORANGE", "WHITE",
         "", 10, 0, 0}

};

const int g_StylePrefsSize = WXSIZEOF(g_StylePrefs);


wxBEGIN_EVENT_TABLE(PythonEditorFrame, wxFrame)
	// common
	EVT_CLOSE(PythonEditorFrame::OnClose)
	// file
	EVT_MENU(wxID_OPEN, PythonEditorFrame::OnFileOpen)
	EVT_MENU(wxID_SAVE, PythonEditorFrame::OnFileSave)
	EVT_MENU(wxID_SAVEAS, PythonEditorFrame::OnFileSaveAs)
	EVT_MENU(wxID_CLOSE, PythonEditorFrame::OnFileClose)
	// properties
	EVT_MENU(myID_PROPERTIES, PythonEditorFrame::OnProperties)
	// print and exit
	//EVT_MENU(wxID_PRINT_SETUP, PythonEditorFrame::OnPrintSetup)
	//EVT_MENU(wxID_PREVIEW, PythonEditorFrame::OnPrintPreview)
	//EVT_MENU(wxID_PRINT, PythonEditorFrame::OnPrint)
	EVT_MENU(wxID_EXIT, PythonEditorFrame::OnExit)
	// Menu items with standard IDs forwarded to the editor.
	EVT_MENU(wxID_CLEAR, PythonEditorFrame::OnEdit)
	EVT_MENU(wxID_CUT, PythonEditorFrame::OnEdit)
	EVT_MENU(wxID_COPY, PythonEditorFrame::OnEdit)
	EVT_MENU(wxID_PASTE, PythonEditorFrame::OnEdit)
	EVT_MENU(wxID_SELECTALL, PythonEditorFrame::OnEdit)
	EVT_MENU(wxID_REDO, PythonEditorFrame::OnEdit)
	EVT_MENU(wxID_UNDO, PythonEditorFrame::OnEdit)
	EVT_MENU(wxID_FIND, PythonEditorFrame::OnEdit)
	// And all our edit-related menu commands.
	EVT_MENU_RANGE(myID_EDIT_FIRST, myID_EDIT_LAST, PythonEditorFrame::OnEdit)
	// help
	EVT_CONTEXT_MENU(PythonEditorFrame::OnContextMenu)
wxEND_EVENT_TABLE()

PythonEditorFrame::PythonEditorFrame() : wxFrame(NULL, wxID_ANY, _("Python Editor"))
{
	// create menu
	m_menuBar = new wxMenuBar;
	CreateMenu();

	m_edit = new PythonEditor(this, wxID_ANY);
	m_edit->SetFocus();
	//editor->SetFont(wxFontInfo().Family(wxFONTFAMILY_TELETYPE));
	//wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	//sizer->Add(editor, 1, wxEXPAND);
	//SetSizer(sizer);
	m_edit->SetText(
		"import biblpy\n"
		"a = 1 + 1\n"
		"print(a)\n"
	);
}

PythonEditorFrame::~PythonEditorFrame() {
}

void PythonEditorFrame::OnFileOpen(wxCommandEvent& WXUNUSED(event)) {
	if (!m_edit) return;
#if wxUSE_FILEDLG
	wxString fname;
	wxFileDialog dlg(this, "Open file", wxEmptyString, wxEmptyString, "Any file (*)|*",
		wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);
	if (dlg.ShowModal() != wxID_OK) return;
	fname = dlg.GetPath();
	FileOpen(fname);
#endif // wxUSE_FILEDLG
}

void PythonEditorFrame::OnClose(wxCloseEvent& event) {
    wxCommandEvent evt;
    OnFileClose(evt);
    if (m_edit && m_edit->Modified()) {
        if (event.CanVeto()) event.Veto(true);
        return;
    }
    Destroy();
}

void PythonEditorFrame::OnExit(wxCommandEvent& WXUNUSED(event)) {
    Close(true);
}


void PythonEditorFrame::OnFileSave(wxCommandEvent& WXUNUSED(event)) {
	if (!m_edit) return;
	if (!m_edit->Modified()) {
		wxMessageBox(_("There is nothing to save!"), _("Save file"),
			wxOK | wxICON_EXCLAMATION);
		return;
	}
	m_edit->SaveFile();
}

void PythonEditorFrame::OnFileSaveAs(wxCommandEvent& WXUNUSED(event)) {
	if (!m_edit) return;
#if wxUSE_FILEDLG
	wxString filename;
	wxFileDialog dlg(this, "Save file", wxEmptyString, wxEmptyString, "Any file (*)|*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() != wxID_OK) return;
	filename = dlg.GetPath();
	m_edit->SaveFile(filename);
#endif // wxUSE_FILEDLG
}

void PythonEditorFrame::OnFileClose(wxCommandEvent& WXUNUSED(event)) {
	if (!m_edit) return;
	if (m_edit->Modified()) {
		if (wxMessageBox(_("Text is not saved, save before closing?"), _("Close"),
			wxYES_NO | wxICON_QUESTION) == wxYES) {
			m_edit->SaveFile();
			if (m_edit->Modified()) {
				wxMessageBox(_("Text could not be saved!"), _("Close abort"),
					wxOK | wxICON_EXCLAMATION);
				return;
			}
		}
	}
	m_edit->SetFilename(wxEmptyString);
	m_edit->ClearAll();
	m_edit->SetSavePoint();
}

// properties event handlers
void PythonEditorFrame::OnProperties(wxCommandEvent& WXUNUSED(event)) {
    if (!m_edit) return;
    EditProperties dlg(m_edit, 0);
}

void PythonEditorFrame::OnContextMenu(wxContextMenuEvent& evt)
{
	wxPoint point = evt.GetPosition();
	// If from keyboard
	if (point.x == -1 && point.y == -1)
	{
		wxSize size = GetSize();
		point.x = size.x / 2;
		point.y = size.y / 2;
	}
	else
	{
		point = ScreenToClient(point);
	}

	wxMenu menu;
	menu.Append(wxID_ABOUT, "&About");
	menu.Append(wxID_EXIT, "E&xit");
	PopupMenu(&menu, point);
}


void PythonEditorFrame::CreateMenu()
{
	// File menu
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(wxID_OPEN, _("&Open ..\tCtrl+O"));
	menuFile->Append(wxID_SAVE, _("&Save\tCtrl+S"));
	menuFile->Append(wxID_SAVEAS, _("Save &as ..\tCtrl+Shift+S"));
	menuFile->Append(wxID_CLOSE, _("&Close\tCtrl+W"));
	menuFile->AppendSeparator();
    menuFile->Append(myID_RUN_IN_PYTHON, _("&Run in Python\tCtrl+W"));
	menuFile->Append(myID_PROPERTIES, _("Proper&ties ..\tCtrl+I"));
	menuFile->AppendSeparator();
	menuFile->Append(wxID_PRINT_SETUP, _("Print Set&up .."));
	menuFile->Append(wxID_PREVIEW, _("Print Pre&view\tCtrl+Shift+P"));
	menuFile->Append(wxID_PRINT, _("&Print ..\tCtrl+P"));
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT, _("&Quit\tCtrl+Q"));

	// Edit menu
	wxMenu* menuEdit = new wxMenu;
	menuEdit->Append(wxID_UNDO, _("&Undo\tCtrl+Z"));
	menuEdit->Append(wxID_REDO, _("&Redo\tCtrl+Shift+Z"));
	menuEdit->AppendSeparator();
	menuEdit->Append(wxID_CUT, _("Cu&t\tCtrl+X"));
	menuEdit->Append(wxID_COPY, _("&Copy\tCtrl+C"));
	menuEdit->Append(wxID_PASTE, _("&Paste\tCtrl+V"));
	menuEdit->Append(wxID_CLEAR, _("&Delete\tDel"));
	menuEdit->AppendSeparator();
	menuEdit->Append(wxID_FIND, _("&Find\tCtrl+F"));
	menuEdit->Enable(wxID_FIND, false);
	menuEdit->Append(myID_FINDNEXT, _("Find &next\tF3"));
	menuEdit->Enable(myID_FINDNEXT, false);
	menuEdit->Append(myID_REPLACE, _("&Replace\tCtrl+H"));
	menuEdit->Enable(myID_REPLACE, false);
	menuEdit->Append(myID_REPLACENEXT, _("Replace &again\tShift+F4"));
	menuEdit->Enable(myID_REPLACENEXT, false);
	menuEdit->AppendSeparator();
	menuEdit->Append(myID_BRACEMATCH, _("&Match brace\tCtrl+M"));
	menuEdit->Append(myID_GOTO, _("&Goto\tCtrl+G"));
	menuEdit->Enable(myID_GOTO, false);
	menuEdit->AppendSeparator();
	menuEdit->Append(myID_INDENTINC, _("&Indent increase\tTab"));
	menuEdit->Append(myID_INDENTRED, _("I&ndent reduce\tShift+Tab"));
	menuEdit->AppendSeparator();
	menuEdit->Append(wxID_SELECTALL, _("&Select all\tCtrl+A"));
	menuEdit->Append(myID_SELECTLINE, _("Select &line\tCtrl+L"));

	// highlight submenu
	wxMenu* menuHighlight = new wxMenu;
	int Nr;
	for (Nr = 0; Nr < g_LanguagePrefsSize; Nr++) {
		menuHighlight->Append(myID_HIGHLIGHTFIRST + Nr,
			g_LanguagePrefs[Nr].name);
	}

	// charset submenu
	wxMenu* menuCharset = new wxMenu;
	menuCharset->Append(myID_CHARSETANSI, _("&ANSI (Windows)"));
	menuCharset->Append(myID_CHARSETMAC, _("&MAC (Macintosh)"));

	// View menu
	wxMenu* menuView = new wxMenu;
	menuView->Append(myID_HIGHLIGHTLANG, _("&Highlight language .."), menuHighlight);
	menuView->AppendSeparator();
	menuView->AppendCheckItem(myID_FOLDTOGGLE, _("&Toggle current fold\tCtrl+T"));
	menuView->AppendCheckItem(myID_OVERTYPE, _("&Overwrite mode\tIns"));
	menuView->AppendCheckItem(myID_WRAPMODEON, _("&Wrap mode\tCtrl+U"));
	menuView->AppendSeparator();
	menuView->AppendCheckItem(myID_DISPLAYEOL, _("Show line &endings"));
	menuView->AppendCheckItem(myID_INDENTGUIDE, _("Show &indent guides"));
	menuView->AppendCheckItem(myID_LINENUMBER, _("Show line &numbers"));
	menuView->AppendCheckItem(myID_LONGLINEON, _("Show &long line marker"));
	menuView->AppendCheckItem(myID_WHITESPACE, _("Show white&space"));
	menuView->AppendSeparator();
	menuView->Append(myID_USECHARSET, _("Use &code page of .."), menuCharset);

	// Annotations menu
	wxMenu* menuAnnotations = new wxMenu;
	menuAnnotations->Append(myID_ANNOTATION_ADD, _("&Add or edit an annotation..."),
		_("Add an annotation for the current line"));
	menuAnnotations->Append(myID_ANNOTATION_REMOVE, _("&Remove annotation"),
		_("Remove the annotation for the current line"));
	menuAnnotations->Append(myID_ANNOTATION_CLEAR, _("&Clear all annotations"));

	wxMenu* menuAnnotationsStyle = new wxMenu;
	menuAnnotationsStyle->AppendRadioItem(myID_ANNOTATION_STYLE_HIDDEN, _("&Hidden"));
	menuAnnotationsStyle->AppendRadioItem(myID_ANNOTATION_STYLE_STANDARD, _("&Standard"));
	menuAnnotationsStyle->AppendRadioItem(myID_ANNOTATION_STYLE_BOXED, _("&Boxed"));
	menuAnnotations->AppendSubMenu(menuAnnotationsStyle, "&Style");

	// change case submenu
	wxMenu* menuChangeCase = new wxMenu;
	menuChangeCase->Append(myID_CHANGEUPPER, _("&Upper case"));
	menuChangeCase->Append(myID_CHANGELOWER, _("&Lower case"));

	// convert EOL submenu
	wxMenu* menuConvertEOL = new wxMenu;
	menuConvertEOL->Append(myID_CONVERTCR, _("CR (&Linux)"));
	menuConvertEOL->Append(myID_CONVERTCRLF, _("CR+LF (&Windows)"));
	menuConvertEOL->Append(myID_CONVERTLF, _("LF (&Macintosh)"));

	// Extra menu
	wxMenu* menuExtra = new wxMenu;
	menuExtra->AppendCheckItem(myID_READONLY, _("&Readonly mode"));
	menuExtra->AppendSeparator();
	menuExtra->Append(myID_CHANGECASE, _("Change &case to .."), menuChangeCase);
	menuExtra->AppendSeparator();
	menuExtra->Append(myID_CONVERTEOL, _("Convert line &endings to .."), menuConvertEOL);
	menuExtra->AppendCheckItem(myID_MULTIPLE_SELECTIONS, _("Toggle &multiple selections"));
	menuExtra->AppendCheckItem(myID_MULTI_PASTE, _("Toggle multi-&paste"));
	menuExtra->AppendCheckItem(myID_MULTIPLE_SELECTIONS_TYPING, _("Toggle t&yping on multiple selections"));
	menuExtra->AppendSeparator();
#if defined(__WXMSW__) && wxUSE_GRAPHICS_DIRECT2D
	wxMenu* menuTechnology = new wxMenu;
	menuTechnology->AppendRadioItem(myID_TECHNOLOGY_DEFAULT, _("&Default"));
	menuTechnology->AppendRadioItem(myID_TECHNOLOGY_DIRECTWRITE, _("Direct&Write"));
	menuExtra->AppendSubMenu(menuTechnology, _("&Technology"));
	menuExtra->AppendSeparator();
#endif
	menuExtra->AppendCheckItem(myID_CUSTOM_POPUP, _("C&ustom context menu"));

	// Window menu
	wxMenu* menuWindow = new wxMenu;
	menuWindow->Append(myID_WINDOW_MINIMAL, _("&Minimal editor"));

	// Help menu
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT, _("&About ..\tCtrl+D"));

	// construct menu
	m_menuBar->Append(menuFile, _("&File"));
	m_menuBar->Append(menuEdit, _("&Edit"));
	m_menuBar->Append(menuView, _("&View"));
	m_menuBar->Append(menuAnnotations, _("&Annotations"));
	m_menuBar->Append(menuExtra, _("E&xtra"));
	m_menuBar->Append(menuWindow, _("&Window"));
	m_menuBar->Append(menuHelp, _("&Help"));
	SetMenuBar(m_menuBar);

	m_menuBar->Check(myID_ANNOTATION_STYLE_BOXED, true);
}

void PythonEditorFrame::FileOpen(wxString fname)
{
	wxFileName w(fname); w.Normalize(); fname = w.GetFullPath();
	m_edit->LoadFile(fname);
	m_edit->SelectNone();
}

// edit events
void PythonEditorFrame::OnEdit(wxCommandEvent& event) {
	if (m_edit) m_edit->GetEventHandler()->ProcessEvent(event);
}

// The (uniform) style used for the annotations.
const int ANNOTATION_STYLE = wxSTC_STYLE_LASTPREDEFINED + 1;

// A small image of a hashtag symbol used in the autocompletion window.
const char* hashtag_xpm[] = {
"10 10 2 1",
" 	c None",
".	c #BD08F9",
"  ..  ..  ",
"  ..  ..  ",
"..........",
"..........",
"  ..  ..  ",
"  ..  ..  ",
"..........",
"..........",
"  ..  ..  ",
"  ..  ..  " };


PythonEditor::PythonEditor(wxWindow* parent, wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style) : wxStyledTextCtrl(parent, id, pos, size, style)
{
	//SetLexerPython();

	//SetProperty("fold", "1");
	//SetProperty("fold.comment", "1");
	//SetProperty("fold.compact", "1");
	//SetProperty("fold.preprocessor", "1");
	//SetProperty("fold.html", "1");
	//SetProperty("fold.html.preprocessor", "1");

	//SetMarginType(margin_id_lineno, wxSTC_MARGIN_NUMBER);
	//SetMarginWidth(margin_id_lineno, 32);

	//// set visibility
	//SetVisiblePolicy(wxSTC_VISIBLE_STRICT | wxSTC_VISIBLE_SLOP, 1);
	//SetXCaretPolicy(wxSTC_CARET_EVEN | wxSTC_VISIBLE_STRICT | wxSTC_CARET_SLOP, 1);
	//SetYCaretPolicy(wxSTC_CARET_EVEN | wxSTC_VISIBLE_STRICT | wxSTC_CARET_SLOP, 1);

	//MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS, "WHITE", "BLACK");
	//MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS, "WHITE", "BLACK");
	//MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_VLINE, "WHITE", "BLACK");
	//MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUSCONNECTED, "WHITE", "BLACK");
	//MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUSCONNECTED, "WHITE", "BLACK");
	//MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNER, "WHITE", "BLACK");
	//MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_LCORNER, "WHITE", "BLACK");

	//SetMarginMask(margin_id_fold, wxSTC_MASK_FOLDERS);
	//SetMarginWidth(margin_id_fold, 32);
	//SetMarginSensitive(margin_id_fold, true);

	//SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);

	//SetTabWidth(4);
	//SetUseTabs(false);
	//SetWrapMode(wxSTC_WRAP_WORD);
	//SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_END);

	m_LineNrID = 0;
	m_DividerID = 1;
	m_FoldingID = 2;

	// initialize language
	m_language = NULL;

	// default font for all styles
	SetViewEOL(g_CommonPrefs.displayEOLEnable);
	SetIndentationGuides(g_CommonPrefs.indentGuideEnable);
	SetEdgeMode(g_CommonPrefs.longLineOnEnable ?
		wxSTC_EDGE_LINE : wxSTC_EDGE_NONE);
	SetViewWhiteSpace(g_CommonPrefs.whiteSpaceEnable ?
		wxSTC_WS_VISIBLEALWAYS : wxSTC_WS_INVISIBLE);
	SetOvertype(g_CommonPrefs.overTypeInitial);
	SetReadOnly(g_CommonPrefs.readOnlyInitial);
	SetWrapMode(g_CommonPrefs.wrapModeInitial ?
		wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);
	wxFont font(wxFontInfo(10).Family(wxFONTFAMILY_MODERN));
	StyleSetFont(wxSTC_STYLE_DEFAULT, font);
	StyleSetForeground(wxSTC_STYLE_DEFAULT, *wxBLACK);
	StyleSetBackground(wxSTC_STYLE_DEFAULT, *wxWHITE);
	StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour("DARK GREY"));
	StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
	StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour("DARK GREY"));
	InitializePrefs(DEFAULT_LANGUAGE);

	// set visibility
	SetVisiblePolicy(wxSTC_VISIBLE_STRICT | wxSTC_VISIBLE_SLOP, 1);
	SetXCaretPolicy(wxSTC_CARET_EVEN | wxSTC_VISIBLE_STRICT | wxSTC_CARET_SLOP, 1);
	SetYCaretPolicy(wxSTC_CARET_EVEN | wxSTC_VISIBLE_STRICT | wxSTC_CARET_SLOP, 1);

	// markers
	MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_DOTDOTDOT, "BLACK", "BLACK");
	MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_ARROWDOWN, "BLACK", "BLACK");
	MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY, "BLACK", "BLACK");
	MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_DOTDOTDOT, "BLACK", "WHITE");
	MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_ARROWDOWN, "BLACK", "WHITE");
	MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY, "BLACK", "BLACK");
	MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY, "BLACK", "BLACK");

	// annotations
	AnnotationSetVisible(wxSTC_ANNOTATION_BOXED);

	// autocompletion
	wxBitmap bmp(hashtag_xpm);
	RegisterImage(0, bmp);

	// call tips
	CallTipSetBackground(*wxYELLOW);
	m_calltipNo = 1;

	// miscellaneous
	m_LineNrMargin = TextWidth(wxSTC_STYLE_LINENUMBER, "_999999");
	m_FoldingMargin = FromDIP(16);
	CmdKeyClear(wxSTC_KEY_TAB, 0); // this is done by the menu accelerator key
	SetLayoutCache(wxSTC_CACHE_PAGE);
	UsePopUp(wxSTC_POPUP_ALL);
}

PythonEditor::~PythonEditor() {}

wxBEGIN_EVENT_TABLE(PythonEditor, wxStyledTextCtrl)
	// common
	EVT_SIZE(PythonEditor::OnSize)
	// edit
	EVT_MENU(wxID_CLEAR, PythonEditor::OnEditClear)
	EVT_MENU(wxID_CUT, PythonEditor::OnEditCut)
	EVT_MENU(wxID_COPY, PythonEditor::OnEditCopy)
	EVT_MENU(wxID_PASTE, PythonEditor::OnEditPaste)
    EVT_MENU(myID_RUN_IN_PYTHON, PythonEditor::OnRunInPython)
	EVT_MENU(myID_INDENTINC, PythonEditor::OnEditIndentInc)
	EVT_MENU(myID_INDENTRED, PythonEditor::OnEditIndentRed)
	EVT_MENU(wxID_SELECTALL, PythonEditor::OnEditSelectAll)
	EVT_MENU(myID_SELECTLINE, PythonEditor::OnEditSelectLine)
	EVT_MENU(wxID_REDO, PythonEditor::OnEditRedo)
	EVT_MENU(wxID_UNDO, PythonEditor::OnEditUndo)
	// find
	EVT_MENU(wxID_FIND, PythonEditor::OnFind)
	EVT_MENU(myID_FINDNEXT, PythonEditor::OnFindNext)
	EVT_MENU(myID_REPLACE, PythonEditor::OnReplace)
	EVT_MENU(myID_REPLACENEXT, PythonEditor::OnReplaceNext)
	EVT_MENU(myID_BRACEMATCH, PythonEditor::OnBraceMatch)
	EVT_MENU(myID_GOTO, PythonEditor::OnGoto)
	// view
	EVT_MENU_RANGE(myID_HIGHLIGHTFIRST, myID_HIGHLIGHTLAST, PythonEditor::OnHighlightLang)
	EVT_MENU(myID_DISPLAYEOL, PythonEditor::OnDisplayEOL)
	EVT_MENU(myID_INDENTGUIDE, PythonEditor::OnIndentGuide)
	EVT_MENU(myID_LINENUMBER, PythonEditor::OnLineNumber)
	EVT_MENU(myID_LONGLINEON, PythonEditor::OnLongLineOn)
	EVT_MENU(myID_WHITESPACE, PythonEditor::OnWhiteSpace)
	EVT_MENU(myID_FOLDTOGGLE, PythonEditor::OnFoldToggle)
	EVT_MENU(myID_OVERTYPE, PythonEditor::OnSetOverType)
	EVT_MENU(myID_READONLY, PythonEditor::OnSetReadOnly)
	EVT_MENU(myID_WRAPMODEON, PythonEditor::OnWrapmodeOn)
	EVT_MENU(myID_CHARSETANSI, PythonEditor::OnUseCharset)
	EVT_MENU(myID_CHARSETMAC, PythonEditor::OnUseCharset)
	// annotations
	EVT_MENU(myID_ANNOTATION_ADD, PythonEditor::OnAnnotationAdd)
	EVT_MENU(myID_ANNOTATION_REMOVE, PythonEditor::OnAnnotationRemove)
	EVT_MENU(myID_ANNOTATION_CLEAR, PythonEditor::OnAnnotationClear)
	EVT_MENU(myID_ANNOTATION_STYLE_HIDDEN, PythonEditor::OnAnnotationStyle)
	EVT_MENU(myID_ANNOTATION_STYLE_STANDARD, PythonEditor::OnAnnotationStyle)
	EVT_MENU(myID_ANNOTATION_STYLE_BOXED, PythonEditor::OnAnnotationStyle)
	// extra
	EVT_MENU(myID_CHANGELOWER, PythonEditor::OnChangeCase)
	EVT_MENU(myID_CHANGEUPPER, PythonEditor::OnChangeCase)
	EVT_MENU(myID_CONVERTCR, PythonEditor::OnConvertEOL)
	EVT_MENU(myID_CONVERTCRLF, PythonEditor::OnConvertEOL)
	EVT_MENU(myID_CONVERTLF, PythonEditor::OnConvertEOL)
	EVT_MENU(myID_MULTIPLE_SELECTIONS, PythonEditor::OnMultipleSelections)
	EVT_MENU(myID_MULTI_PASTE, PythonEditor::OnMultiPaste)
	EVT_MENU(myID_MULTIPLE_SELECTIONS_TYPING, PythonEditor::OnMultipleSelectionsTyping)
	EVT_MENU(myID_CUSTOM_POPUP, PythonEditor::OnCustomPopup)
	EVT_MENU(myID_TECHNOLOGY_DEFAULT, PythonEditor::OnTechnology)
	EVT_MENU(myID_TECHNOLOGY_DIRECTWRITE, PythonEditor::OnTechnology)
	// stc
	EVT_STC_MARGINCLICK(wxID_ANY, PythonEditor::OnMarginClick)
	EVT_STC_CHARADDED(wxID_ANY, PythonEditor::OnCharAdded)
	EVT_STC_CALLTIP_CLICK(wxID_ANY, PythonEditor::OnCallTipClick)

	EVT_KEY_DOWN(PythonEditor::OnKeyDown)
wxEND_EVENT_TABLE()

//bool PythonEditor::SetFont(const wxFont& font)
//{
//	StyleSetFont(wxSTC_STYLE_DEFAULT, font);
//	return wxStyledTextCtrl::SetFont(font);
//}

void PythonEditor::SetLexerPython()
{
	SetLexer(wxSTC_LEX_PYTHON);
	StyleSetForeground(wxSTC_P_DEFAULT, wxColor(128, 128, 128));
	StyleSetForeground(wxSTC_P_COMMENTLINE, wxColor(0, 127, 0));
	StyleSetForeground(wxSTC_P_NUMBER, wxColor(0, 127, 127));
	StyleSetForeground(wxSTC_P_STRING, wxColor(127, 0, 127));
	StyleSetForeground(wxSTC_P_WORD, wxColor(0, 0, 127));
	StyleSetBold(wxSTC_P_WORD, true);

	StyleSetForeground(wxSTC_P_CHARACTER, wxColor(0, 0, 255));
	StyleSetForeground(wxSTC_P_TRIPLE, wxColor(127, 0, 0));
	StyleSetForeground(wxSTC_P_TRIPLEDOUBLE, wxColor(127, 0, 0));
	StyleSetForeground(wxSTC_P_CLASSNAME, wxColor(0, 0, 255));
	StyleSetBold(wxSTC_P_CLASSNAME, true);
	StyleSetForeground(wxSTC_P_DEFNAME, wxColor(0, 127, 127));
	StyleSetBold(wxSTC_P_DEFNAME, true);
	StyleSetForeground(wxSTC_P_OPERATOR, wxColor(0, 0, 255));
	StyleSetBold(wxSTC_P_OPERATOR, true);
	StyleSetForeground(wxSTC_P_IDENTIFIER, wxColor(0, 0, 255));
	StyleSetForeground(wxSTC_P_COMMENTBLOCK, wxColor(127, 127, 127));

	StyleSetForeground(wxSTC_P_STRINGEOL, wxColor(0, 0, 0));

	StyleSetForeground(wxSTC_P_WORD2, wxColor(64, 112, 144));
	StyleSetForeground(wxSTC_P_DECORATOR, wxColor(128, 80, 0));



	/*StyleSetForeground(wxSTC_H_DEFAULT, *wxBLACK);
	StyleSetForeground(wxSTC_H_TAG, *wxBLUE);
	StyleSetForeground(wxSTC_H_TAGUNKNOWN, *wxBLUE);
	StyleSetForeground(wxSTC_H_ATTRIBUTE, *wxRED);
	StyleSetForeground(wxSTC_H_ATTRIBUTEUNKNOWN, *wxRED);
	StyleSetBold(wxSTC_H_ATTRIBUTEUNKNOWN, true);
	StyleSetForeground(wxSTC_H_NUMBER, *wxBLACK);
	StyleSetForeground(wxSTC_H_DOUBLESTRING, *wxBLACK);
	StyleSetForeground(wxSTC_H_SINGLESTRING, *wxBLACK);
	StyleSetForeground(wxSTC_H_OTHER, *wxBLUE);
	StyleSetForeground(wxSTC_H_COMMENT, wxColour("GREY"));
	StyleSetForeground(wxSTC_H_ENTITY, *wxRED);
	StyleSetBold(wxSTC_H_ENTITY, true);
	StyleSetForeground(wxSTC_H_TAGEND, *wxBLUE);
	StyleSetForeground(wxSTC_H_XMLSTART, *wxBLUE);
	StyleSetForeground(wxSTC_H_XMLEND, *wxBLUE);
	StyleSetForeground(wxSTC_H_CDATA, *wxRED);
*/
}

//void PythonEditor::OnMarginClick(wxStyledTextEvent& event)
//{
//	if (event.GetMargin() == margin_id_fold)
//	{
//		int lineClick = LineFromPosition(event.GetPosition());
//		int levelClick = GetFoldLevel(lineClick);
//		if ((levelClick & wxSTC_FOLDLEVELHEADERFLAG) > 0)
//		{
//			ToggleFold(lineClick);
//		}
//	}
//}

//void PythonEditor::OnText(wxStyledTextEvent& event)
//{
//	wxLogDebug("Modified");
//	event.Skip();
//}

// common event handlers
void PythonEditor::OnSize(wxSizeEvent& event) {
    int x = GetClientSize().x +
        (g_CommonPrefs.lineNumberEnable ? m_LineNrMargin : 0) +
        (g_CommonPrefs.foldEnable ? m_FoldingMargin : 0);
    if (x > 0) SetScrollWidth(x);
    event.Skip();
}

// edit event handlers
void PythonEditor::OnEditRedo(wxCommandEvent& WXUNUSED(event)) {
    if (!CanRedo()) return;
    Redo();
}

void PythonEditor::OnEditUndo(wxCommandEvent& WXUNUSED(event)) {
    if (!CanUndo()) return;
    Undo();
}

void PythonEditor::OnEditClear(wxCommandEvent& WXUNUSED(event)) {
    if (GetReadOnly()) return;
    Clear();
}

void PythonEditor::OnKeyDown(wxKeyEvent& event)
{
    if (CallTipActive())
        CallTipCancel();
    if (event.GetKeyCode() == WXK_SPACE && event.ControlDown() && event.ShiftDown())
    {
        // Show our first call tip at the current position of the caret.
        m_calltipNo = 1;
        ShowCallTipAt(GetCurrentPos());
        return;
    }
    event.Skip();
}

void PythonEditor::OnEditCut(wxCommandEvent& WXUNUSED(event)) {
    if (GetReadOnly() || (GetSelectionEnd() - GetSelectionStart() <= 0)) return;
    Cut();
}

void PythonEditor::OnEditCopy(wxCommandEvent& WXUNUSED(event)) {
    if (GetSelectionEnd() - GetSelectionStart() <= 0) return;
    Copy();
}

void PythonEditor::OnEditPaste(wxCommandEvent& WXUNUSED(event)) {
    if (!CanPaste()) return;
    Paste();
}

void PythonEditor::OnRunInPython(wxCommandEvent& event)
{
    wxLogDebug("Run Python script");
    wxString txt = this->GetText();

    PyGILState_STATE gstate = PyGILState_Ensure();
    if (PyErr_Occurred()) {  // PyErr_Print();  
        PyErr_Clear();
    }
    int ires = PyRun_SimpleString(txt.mb_str());
    if (PyErr_Occurred()) {  // PyErr_Print(); 
        PyErr_Clear();
    }
    PyGILState_Release(gstate);
}

void PythonEditor::OnFind(wxCommandEvent& WXUNUSED(event)) {
}

void PythonEditor::OnFindNext(wxCommandEvent& WXUNUSED(event)) {
}

void PythonEditor::OnReplace(wxCommandEvent& WXUNUSED(event)) {
}

void PythonEditor::OnReplaceNext(wxCommandEvent& WXUNUSED(event)) {
}

void PythonEditor::OnBraceMatch(wxCommandEvent& WXUNUSED(event)) {
    int min = GetCurrentPos();
    int max = BraceMatch(min);
    if (max > (min + 1)) {
        BraceHighlight(min + 1, max);
        SetSelection(min + 1, max);
    }
    else {
        BraceBadLight(min);
    }
}

void PythonEditor::OnGoto(wxCommandEvent& WXUNUSED(event)) {
}

void PythonEditor::OnEditIndentInc(wxCommandEvent& WXUNUSED(event)) {
    CmdKeyExecute(wxSTC_CMD_TAB);
}

void PythonEditor::OnEditIndentRed(wxCommandEvent& WXUNUSED(event)) {
    CmdKeyExecute(wxSTC_CMD_DELETEBACK);
}

void PythonEditor::OnEditSelectAll(wxCommandEvent& WXUNUSED(event)) {
    SetSelection(0, GetTextLength());
}

void PythonEditor::OnEditSelectLine(wxCommandEvent& WXUNUSED(event)) {
    int lineStart = PositionFromLine(GetCurrentLine());
    int lineEnd = PositionFromLine(GetCurrentLine() + 1);
    SetSelection(lineStart, lineEnd);
}

void PythonEditor::OnHighlightLang(wxCommandEvent& event) {
    InitializePrefs(g_LanguagePrefs[event.GetId() - myID_HIGHLIGHTFIRST].name);
}

void PythonEditor::OnDisplayEOL(wxCommandEvent& WXUNUSED(event)) {
    SetViewEOL(!GetViewEOL());
}

void PythonEditor::OnIndentGuide(wxCommandEvent& WXUNUSED(event)) {
    SetIndentationGuides(!GetIndentationGuides());
}

void PythonEditor::OnLineNumber(wxCommandEvent& WXUNUSED(event)) {
    SetMarginWidth(m_LineNrID,
        GetMarginWidth(m_LineNrID) == 0 ? m_LineNrMargin : 0);
}

void PythonEditor::OnLongLineOn(wxCommandEvent& WXUNUSED(event)) {
    SetEdgeMode(GetEdgeMode() == 0 ? wxSTC_EDGE_LINE : wxSTC_EDGE_NONE);
}

void PythonEditor::OnWhiteSpace(wxCommandEvent& WXUNUSED(event)) {
    SetViewWhiteSpace(GetViewWhiteSpace() == 0 ?
        wxSTC_WS_VISIBLEALWAYS : wxSTC_WS_INVISIBLE);
}

void PythonEditor::OnFoldToggle(wxCommandEvent& WXUNUSED(event)) {
    ToggleFold(GetFoldParent(GetCurrentLine()));
}

void PythonEditor::OnSetOverType(wxCommandEvent& WXUNUSED(event)) {
    SetOvertype(!GetOvertype());
}

void PythonEditor::OnSetReadOnly(wxCommandEvent& WXUNUSED(event)) {
    SetReadOnly(!GetReadOnly());
}

void PythonEditor::OnWrapmodeOn(wxCommandEvent& WXUNUSED(event)) {
    SetWrapMode(GetWrapMode() == 0 ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);
}

void PythonEditor::OnUseCharset(wxCommandEvent& event) {
    int Nr;
    int charset = GetCodePage();
    switch (event.GetId()) {
    case myID_CHARSETANSI: {charset = wxSTC_CHARSET_ANSI; break; }
    case myID_CHARSETMAC: {charset = wxSTC_CHARSET_ANSI; break; }
    }
    for (Nr = 0; Nr < wxSTC_STYLE_LASTPREDEFINED; Nr++) {
        StyleSetCharacterSet(Nr, charset);
    }
    SetCodePage(charset);
}

void PythonEditor::OnAnnotationAdd(wxCommandEvent& WXUNUSED(event))
{
    const int line = GetCurrentLine();

    wxString ann = AnnotationGetText(line);
    ann = wxGetTextFromUser
    (
        wxString::Format("Enter annotation for the line %d", line),
        "Edit annotation",
        ann,
        this
    );
    if (ann.empty())
        return;

    AnnotationSetText(line, ann);
    AnnotationSetStyle(line, ANNOTATION_STYLE);

    // Scintilla doesn't update the scroll width for annotations, even with
    // scroll width tracking on, so do it manually.
    const int width = GetScrollWidth();

    // NB: The following adjustments are only needed when using
    //     wxSTC_ANNOTATION_BOXED annotations style, but we apply them always
    //     in order to make things simpler and not have to redo the width
    //     calculations when the annotations visibility changes. In a real
    //     program you'd either just stick to a fixed annotations visibility or
    //     update the width when it changes.

    // Take into account the fact that the annotation is shown indented, with
    // the same indent as the line it's attached to.
    int indent = GetLineIndentation(line);

    // This is just a hack to account for the width of the box, there doesn't
    // seem to be any way to get it directly from Scintilla.
    indent += 3;

    const int widthAnn = TextWidth(ANNOTATION_STYLE, ann + wxString(indent, ' '));

    if (widthAnn > width)
        SetScrollWidth(widthAnn);
}

void PythonEditor::OnAnnotationRemove(wxCommandEvent& WXUNUSED(event))
{
    AnnotationSetText(GetCurrentLine(), wxString());
}

void PythonEditor::OnAnnotationClear(wxCommandEvent& WXUNUSED(event))
{
    AnnotationClearAll();
}

void PythonEditor::OnAnnotationStyle(wxCommandEvent& event)
{
    int style = 0;
    switch (event.GetId()) {
    case myID_ANNOTATION_STYLE_HIDDEN:
        style = wxSTC_ANNOTATION_HIDDEN;
        break;

    case myID_ANNOTATION_STYLE_STANDARD:
        style = wxSTC_ANNOTATION_STANDARD;
        break;

    case myID_ANNOTATION_STYLE_BOXED:
        style = wxSTC_ANNOTATION_BOXED;
        break;
    }

    AnnotationSetVisible(style);
}

void PythonEditor::OnChangeCase(wxCommandEvent& event) {
    switch (event.GetId()) {
    case myID_CHANGELOWER: {
        CmdKeyExecute(wxSTC_CMD_LOWERCASE);
        break;
    }
    case myID_CHANGEUPPER: {
        CmdKeyExecute(wxSTC_CMD_UPPERCASE);
        break;
    }
    }
}

void PythonEditor::OnConvertEOL(wxCommandEvent& event) {
    int eolMode = GetEOLMode();
    switch (event.GetId()) {
    case myID_CONVERTCR: { eolMode = wxSTC_EOL_CR; break; }
    case myID_CONVERTCRLF: { eolMode = wxSTC_EOL_CRLF; break; }
    case myID_CONVERTLF: { eolMode = wxSTC_EOL_LF; break; }
    }
    ConvertEOLs(eolMode);
    SetEOLMode(eolMode);
}

void PythonEditor::OnMultipleSelections(wxCommandEvent& WXUNUSED(event)) {
    bool isSet = GetMultipleSelection();
    SetMultipleSelection(!isSet);
}

void PythonEditor::OnMultiPaste(wxCommandEvent& WXUNUSED(event)) {
    int pasteMode = GetMultiPaste();
    if (wxSTC_MULTIPASTE_EACH == pasteMode) {
        SetMultiPaste(wxSTC_MULTIPASTE_ONCE);
    }
    else {
        SetMultiPaste(wxSTC_MULTIPASTE_EACH);
    }
}

void PythonEditor::OnMultipleSelectionsTyping(wxCommandEvent& WXUNUSED(event)) {
    bool isSet = GetAdditionalSelectionTyping();
    SetAdditionalSelectionTyping(!isSet);
}

void PythonEditor::OnCustomPopup(wxCommandEvent& evt)
{
    UsePopUp(evt.IsChecked() ? wxSTC_POPUP_NEVER : wxSTC_POPUP_ALL);
}

void PythonEditor::OnTechnology(wxCommandEvent& event)
{
    SetTechnology(event.GetId() == myID_TECHNOLOGY_DIRECTWRITE ? wxSTC_TECHNOLOGY_DIRECTWRITE : wxSTC_TECHNOLOGY_DEFAULT);
}

//! misc
void PythonEditor::OnMarginClick(wxStyledTextEvent& event) {
    if (event.GetMargin() == 2) {
        int lineClick = LineFromPosition(event.GetPosition());
        int levelClick = GetFoldLevel(lineClick);
        if ((levelClick & wxSTC_FOLDLEVELHEADERFLAG) > 0) {
            ToggleFold(lineClick);
        }
    }
}

void PythonEditor::OnCharAdded(wxStyledTextEvent& event) {
    char chr = (char)event.GetKey();
    int currentLine = GetCurrentLine();
    // Change this if support for mac files with \r is needed
    if (chr == '\n') {
        int lineInd = 0;
        if (currentLine > 0) {
            lineInd = GetLineIndentation(currentLine - 1);
        }
        if (lineInd == 0) return;
        SetLineIndentation(currentLine, lineInd);
        GotoPos(PositionFromLine(currentLine) + lineInd);
    }
    else if (chr == '#') {
        wxString s = "define?0 elif?0 else?0 endif?0 error?0 if?0 ifdef?0 "
            "ifndef?0 include?0 line?0 pragma?0 undef?0";
        AutoCompShow(0, s);
    }
}

void PythonEditor::OnCallTipClick(wxStyledTextEvent& event)
{
    if (event.GetPosition() == 1) {
        // If position=1, the up arrow has been clicked. Show the next tip.
        m_calltipNo = m_calltipNo == 3 ? 1 : (m_calltipNo + 1);
        ShowCallTipAt(CallTipPosAtStart());
    }
    else if (event.GetPosition() == 2) {
        // If position=2, the down arrow has been clicked. Show previous tip.
        m_calltipNo = m_calltipNo == 1 ? 3 : (m_calltipNo - 1);
        ShowCallTipAt(CallTipPosAtStart());
    }
}


//----------------------------------------------------------------------------
// private functions
void PythonEditor::ShowCallTipAt(int position)
{
    // In a call tip string, the character '\001' will become a clickable
    // up arrow and '\002' will become a clickable down arrow.
    wxString ctString = wxString::Format("\001 %d of 3 \002 ", m_calltipNo);
    if (m_calltipNo == 1)
        ctString += "This is a call tip. Try clicking the up or down buttons.";
    else if (m_calltipNo == 2)
        ctString += "It is meant to be a context sensitive popup helper for "
        "the user.";
    else
        ctString += "This is a call tip with multiple lines.\n"
        "You can provide slightly longer help with "
        "call tips like these.";

    if (CallTipActive())
        CallTipCancel();
    CallTipShow(position, ctString);
}

wxString PythonEditor::DeterminePrefs(const wxString& filename) {

    LanguageInfo const* curInfo;

    // determine language from filepatterns
    int languageNr;
    for (languageNr = 0; languageNr < g_LanguagePrefsSize; languageNr++) {
        curInfo = &g_LanguagePrefs[languageNr];
        wxString filepattern = curInfo->filepattern;
        filepattern.Lower();
        while (!filepattern.empty()) {
            wxString cur = filepattern.BeforeFirst(';');
            if ((cur == filename) ||
                (cur == (filename.BeforeLast('.') + ".*")) ||
                (cur == ("*." + filename.AfterLast('.')))) {
                return curInfo->name;
            }
            filepattern = filepattern.AfterFirst(';');
        }
    }
    return wxEmptyString;

}

bool PythonEditor::InitializePrefs(const wxString& name) {

    // initialize styles
    StyleClearAll();
    LanguageInfo const* curInfo = NULL;

    // determine language
    bool found = false;
    int languageNr;
    for (languageNr = 0; languageNr < g_LanguagePrefsSize; languageNr++) {
        curInfo = &g_LanguagePrefs[languageNr];
        if (curInfo->name == name) {
            found = true;
            break;
        }
    }
    if (!found) return false;

    // set lexer and language
    SetLexer(curInfo->lexer);
    m_language = curInfo;

    // set margin for line numbers
    SetMarginType(m_LineNrID, wxSTC_MARGIN_NUMBER);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour("DARK GREY"));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
    SetMarginWidth(m_LineNrID, 0); // start out not visible

    // annotations style
    StyleSetBackground(ANNOTATION_STYLE, wxColour(244, 220, 220));
    StyleSetForeground(ANNOTATION_STYLE, *wxBLACK);
    StyleSetSizeFractional(ANNOTATION_STYLE,
        (StyleGetSizeFractional(wxSTC_STYLE_DEFAULT) * 4) / 5);

    // default fonts for all styles!
    int Nr;
    for (Nr = 0; Nr < wxSTC_STYLE_LASTPREDEFINED; Nr++) {
        wxFont font(wxFontInfo(10).Family(wxFONTFAMILY_MODERN));
        StyleSetFont(Nr, font);
    }

    // set common styles
    StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColour("DARK GREY"));
    StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour("DARK GREY"));

    // initialize settings
    if (g_CommonPrefs.syntaxEnable) {
        int keywordnr = 0;
        for (Nr = 0; Nr < STYLE_TYPES_COUNT; Nr++) {
            if (curInfo->styles[Nr].type == -1) continue;
            const StyleInfo& curType = g_StylePrefs[curInfo->styles[Nr].type];
            wxFont font(wxFontInfo(curType.fontsize)
                .Family(wxFONTFAMILY_MODERN)
                .FaceName(curType.fontname));
            StyleSetFont(Nr, font);
            if (curType.foreground.length()) {
                StyleSetForeground(Nr, wxColour(curType.foreground));
            }
            if (curType.background.length()) {
                StyleSetBackground(Nr, wxColour(curType.background));
            }
            StyleSetBold(Nr, (curType.fontstyle & mySTC_STYLE_BOLD) > 0);
            StyleSetItalic(Nr, (curType.fontstyle & mySTC_STYLE_ITALIC) > 0);
            StyleSetUnderline(Nr, (curType.fontstyle & mySTC_STYLE_UNDERL) > 0);
            StyleSetVisible(Nr, (curType.fontstyle & mySTC_STYLE_HIDDEN) == 0);
            StyleSetCase(Nr, curType.lettercase);
            const char* pwords = curInfo->styles[Nr].words;
            if (pwords) {
                SetKeyWords(keywordnr, pwords);
                keywordnr += 1;
            }
        }
    }

    // set margin as unused
    SetMarginType(m_DividerID, wxSTC_MARGIN_SYMBOL);
    SetMarginWidth(m_DividerID, 0);
    SetMarginSensitive(m_DividerID, false);

    // folding
    SetMarginType(m_FoldingID, wxSTC_MARGIN_SYMBOL);
    SetMarginMask(m_FoldingID, wxSTC_MASK_FOLDERS);
    StyleSetBackground(m_FoldingID, *wxWHITE);
    SetMarginWidth(m_FoldingID, 0);
    SetMarginSensitive(m_FoldingID, false);
    if (g_CommonPrefs.foldEnable) {
        SetMarginWidth(m_FoldingID, curInfo->folds != 0 ? m_FoldingMargin : 0);
        SetMarginSensitive(m_FoldingID, curInfo->folds != 0);
        SetProperty("fold", curInfo->folds != 0 ? "1" : "0");
        SetProperty("fold.comment",
            (curInfo->folds & mySTC_FOLD_COMMENT) > 0 ? "1" : "0");
        SetProperty("fold.compact",
            (curInfo->folds & mySTC_FOLD_COMPACT) > 0 ? "1" : "0");
        SetProperty("fold.preprocessor",
            (curInfo->folds & mySTC_FOLD_PREPROC) > 0 ? "1" : "0");
        SetProperty("fold.html",
            (curInfo->folds & mySTC_FOLD_HTML) > 0 ? "1" : "0");
        SetProperty("fold.html.preprocessor",
            (curInfo->folds & mySTC_FOLD_HTMLPREP) > 0 ? "1" : "0");
        SetProperty("fold.comment.python",
            (curInfo->folds & mySTC_FOLD_COMMENTPY) > 0 ? "1" : "0");
        SetProperty("fold.quotes.python",
            (curInfo->folds & mySTC_FOLD_QUOTESPY) > 0 ? "1" : "0");
    }
    SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED |
        wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);

    // set spaces and indentation
    SetTabWidth(4);
    SetUseTabs(false);
    SetTabIndents(true);
    SetBackSpaceUnIndents(true);
    SetIndent(g_CommonPrefs.indentEnable ? 4 : 0);

    // others
    SetViewEOL(g_CommonPrefs.displayEOLEnable);
    SetIndentationGuides(g_CommonPrefs.indentGuideEnable);
    SetEdgeColumn(80);
    SetEdgeMode(g_CommonPrefs.longLineOnEnable ? wxSTC_EDGE_LINE : wxSTC_EDGE_NONE);
    SetViewWhiteSpace(g_CommonPrefs.whiteSpaceEnable ?
        wxSTC_WS_VISIBLEALWAYS : wxSTC_WS_INVISIBLE);
    SetOvertype(g_CommonPrefs.overTypeInitial);
    SetReadOnly(g_CommonPrefs.readOnlyInitial);
    SetWrapMode(g_CommonPrefs.wrapModeInitial ?
        wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);

    return true;
}

bool PythonEditor::LoadFile()
{
#if wxUSE_FILEDLG
    // get filename
    if (!m_filename) {
        wxFileDialog dlg(this, "Open file", wxEmptyString, wxEmptyString,
            "Any file (*)|*", wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);
        if (dlg.ShowModal() != wxID_OK) return false;
        m_filename = dlg.GetPath();
    }

    // load file
    return LoadFile(m_filename);
#else
    return false;
#endif // wxUSE_FILEDLG
}

bool PythonEditor::LoadFile(const wxString& filename) {

    // load file in edit and clear undo
    if (!filename.empty()) m_filename = filename;

    wxStyledTextCtrl::LoadFile(m_filename);

    EmptyUndoBuffer();

    // determine lexer language
    wxFileName fname(m_filename);
    InitializePrefs(DeterminePrefs(fname.GetFullName()));

    return true;
}

bool PythonEditor::SaveFile()
{
#if wxUSE_FILEDLG
    // return if no change
    if (!Modified()) return true;

    // get filename
    if (!m_filename) {
        wxFileDialog dlg(this, "Save file", wxEmptyString, wxEmptyString, "Any file (*)|*",
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dlg.ShowModal() != wxID_OK) return false;
        m_filename = dlg.GetPath();
    }

    // save file
    return SaveFile(m_filename);
#else
    return false;
#endif // wxUSE_FILEDLG
}

bool PythonEditor::SaveFile(const wxString& filename) {

    // return if no change
    if (!Modified()) return true;

    //     // save edit in file and clear undo
    //     if (!filename.empty()) m_filename = filename;
    //     wxFile file (m_filename, wxFile::write);
    //     if (!file.IsOpened()) return false;
    //     wxString buf = GetText();
    //     bool okay = file.Write (buf);
    //     file.Close();
    //     if (!okay) return false;
    //     EmptyUndoBuffer();
    //     SetSavePoint();

    //     return true;

    return wxStyledTextCtrl::SaveFile(filename);

}

bool PythonEditor::Modified() {

    // return modified state
    return (GetModify() && !GetReadOnly());
}

//----------------------------------------------------------------------------
// EditProperties
//----------------------------------------------------------------------------

EditProperties::EditProperties(PythonEditor* edit,
    long style)
    : wxDialog(edit, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize,
        style | wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {

    // sets the application title
    SetTitle(_("Properties"));
    wxString text;

    // full name
    wxBoxSizer* fullname = new wxBoxSizer(wxHORIZONTAL);
    fullname->Add(10, 0);
    fullname->Add(new wxStaticText(this, wxID_ANY, _("Full filename"),
        wxDefaultPosition, wxSize(80, wxDefaultCoord)),
        0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    fullname->Add(new wxStaticText(this, wxID_ANY, edit->GetFilename()),
        0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

    // text info
    wxGridSizer* textinfo = new wxGridSizer(4, 0, 2);
    textinfo->Add(new wxStaticText(this, wxID_ANY, _("Language"),
        wxDefaultPosition, wxSize(80, wxDefaultCoord)),
        0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    textinfo->Add(new wxStaticText(this, wxID_ANY, edit->m_language->name),
        0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    textinfo->Add(new wxStaticText(this, wxID_ANY, _("Lexer-ID: "),
        wxDefaultPosition, wxSize(80, wxDefaultCoord)),
        0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    text = wxString::Format("%d", edit->GetLexer());
    textinfo->Add(new wxStaticText(this, wxID_ANY, text),
        0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    wxString EOLtype;
    switch (edit->GetEOLMode()) {
    case wxSTC_EOL_CR: {EOLtype = "CR (Unix)"; break; }
    case wxSTC_EOL_CRLF: {EOLtype = "CRLF (Windows)"; break; }
    case wxSTC_EOL_LF: {EOLtype = "CR (Macintosh)"; break; }
    }
    textinfo->Add(new wxStaticText(this, wxID_ANY, _("Line endings"),
        wxDefaultPosition, wxSize(80, wxDefaultCoord)),
        0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    textinfo->Add(new wxStaticText(this, wxID_ANY, EOLtype),
        0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

    // text info box
    wxStaticBoxSizer* textinfos = new wxStaticBoxSizer(
        new wxStaticBox(this, wxID_ANY, _("Information")),
        wxVERTICAL);
    textinfos->Add(textinfo, 0, wxEXPAND);
    textinfos->Add(0, 6);

    // statistic
    wxGridSizer* statistic = new wxGridSizer(4, 0, 2);
    statistic->Add(new wxStaticText(this, wxID_ANY, _("Total lines"),
        wxDefaultPosition, wxSize(80, wxDefaultCoord)),
        0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    text = wxString::Format("%d", edit->GetLineCount());
    statistic->Add(new wxStaticText(this, wxID_ANY, text),
        0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    statistic->Add(new wxStaticText(this, wxID_ANY, _("Total chars"),
        wxDefaultPosition, wxSize(80, wxDefaultCoord)),
        0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    text = wxString::Format("%d", edit->GetTextLength());
    statistic->Add(new wxStaticText(this, wxID_ANY, text),
        0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    statistic->Add(new wxStaticText(this, wxID_ANY, _("Current line"),
        wxDefaultPosition, wxSize(80, wxDefaultCoord)),
        0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    text = wxString::Format("%d", edit->GetCurrentLine());
    statistic->Add(new wxStaticText(this, wxID_ANY, text),
        0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    statistic->Add(new wxStaticText(this, wxID_ANY, _("Current pos"),
        wxDefaultPosition, wxSize(80, wxDefaultCoord)),
        0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
    text = wxString::Format("%d", edit->GetCurrentPos());
    statistic->Add(new wxStaticText(this, wxID_ANY, text),
        0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

    // char/line statistics
    wxStaticBoxSizer* statistics = new wxStaticBoxSizer(
        new wxStaticBox(this, wxID_ANY, _("Statistics")),
        wxVERTICAL);
    statistics->Add(statistic, 0, wxEXPAND);
    statistics->Add(0, 6);

    // total pane
    wxBoxSizer* totalpane = new wxBoxSizer(wxVERTICAL);
    totalpane->Add(fullname, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
    totalpane->Add(0, 6);
    totalpane->Add(textinfos, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    totalpane->Add(0, 10);
    totalpane->Add(statistics, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    totalpane->Add(0, 6);
    wxButton* okButton = new wxButton(this, wxID_OK, _("OK"));
    okButton->SetDefault();
    totalpane->Add(okButton, 0, wxALIGN_CENTER | wxALL, 10);

    SetSizerAndFit(totalpane);

    ShowModal();
}

