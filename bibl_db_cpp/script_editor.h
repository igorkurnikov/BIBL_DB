#pragma once

#include <wx/stc/stc.h>  // styled text control

// ----------------------------------------------------------------------------
// standard IDs
// ----------------------------------------------------------------------------

enum {
	// menu IDs
	myID_PROPERTIES = wxID_HIGHEST,
	myID_EDIT_FIRST,
	myID_INDENTINC = myID_EDIT_FIRST,
	myID_RUN_IN_PYTHON,
	myID_INDENTRED,
	myID_FINDNEXT,
	myID_REPLACE,
	myID_REPLACENEXT,
	myID_BRACEMATCH,
	myID_GOTO,
	myID_DISPLAYEOL,
	myID_INDENTGUIDE,
	myID_LINENUMBER,
	myID_LONGLINEON,
	myID_WHITESPACE,
	myID_FOLDTOGGLE,
	myID_OVERTYPE,
	myID_READONLY,
	myID_WRAPMODEON,
	myID_ANNOTATION_ADD,
	myID_ANNOTATION_REMOVE,
	myID_ANNOTATION_CLEAR,
	myID_ANNOTATION_STYLE_HIDDEN,
	myID_ANNOTATION_STYLE_STANDARD,
	myID_ANNOTATION_STYLE_BOXED,
	myID_CHANGECASE,
	myID_CHANGELOWER,
	myID_CHANGEUPPER,
	myID_HIGHLIGHTLANG,
	myID_HIGHLIGHTFIRST,
	myID_HIGHLIGHTLAST = myID_HIGHLIGHTFIRST + 99,
	myID_CONVERTEOL,
	myID_CONVERTCR,
	myID_CONVERTCRLF,
	myID_CONVERTLF,
	myID_MULTIPLE_SELECTIONS,
	myID_MULTI_PASTE,
	myID_MULTIPLE_SELECTIONS_TYPING,
	myID_TECHNOLOGY_DEFAULT,
	myID_TECHNOLOGY_DIRECTWRITE,
	myID_CUSTOM_POPUP,
	myID_USECHARSET,
	myID_CHARSETANSI,
	myID_CHARSETMAC,
	myID_SELECTLINE,
	myID_EDIT_LAST = myID_SELECTLINE,
	myID_WINDOW_MINIMAL,

	// other IDs
	myID_ABOUTTIMER,
};

#define DEFAULT_LANGUAGE "Python"
#define STYLE_TYPES_COUNT 32

//! general style types
#define mySTC_TYPE_DEFAULT 0

#define mySTC_TYPE_WORD1 1
#define mySTC_TYPE_WORD2 2
#define mySTC_TYPE_WORD3 3
#define mySTC_TYPE_WORD4 4
#define mySTC_TYPE_WORD5 5
#define mySTC_TYPE_WORD6 6

#define mySTC_TYPE_COMMENT 7
#define mySTC_TYPE_COMMENT_DOC 8
#define mySTC_TYPE_COMMENT_LINE 9
#define mySTC_TYPE_COMMENT_SPECIAL 10

#define mySTC_TYPE_CHARACTER 11
#define mySTC_TYPE_CHARACTER_EOL 12
#define mySTC_TYPE_STRING 13
#define mySTC_TYPE_STRING_EOL 14

#define mySTC_TYPE_DELIMITER 15

#define mySTC_TYPE_PUNCTUATION 16

#define mySTC_TYPE_OPERATOR 17

#define mySTC_TYPE_BRACE 18

#define mySTC_TYPE_COMMAND 19
#define mySTC_TYPE_IDENTIFIER 20
#define mySTC_TYPE_LABEL 21
#define mySTC_TYPE_NUMBER 22
#define mySTC_TYPE_PARAMETER 23
#define mySTC_TYPE_REGEX 24
#define mySTC_TYPE_UUID 25
#define mySTC_TYPE_VALUE 26

#define mySTC_TYPE_PREPROCESSOR 27
#define mySTC_TYPE_SCRIPT 28

#define mySTC_TYPE_ERROR 29

//----------------------------------------------------------------------------
//! style bits types
#define mySTC_STYLE_BOLD 1
#define mySTC_STYLE_ITALIC 2
#define mySTC_STYLE_UNDERL 4
#define mySTC_STYLE_HIDDEN 8

//----------------------------------------------------------------------------
//! general folding types
#define mySTC_FOLD_COMMENT 1
#define mySTC_FOLD_COMPACT 2
#define mySTC_FOLD_PREPROC 4

#define mySTC_FOLD_HTML 16
#define mySTC_FOLD_HTMLPREP 32

#define mySTC_FOLD_COMMENTPY 64
#define mySTC_FOLD_QUOTESPY 128

//----------------------------------------------------------------------------
//! flags
#define mySTC_FLAG_WRAPMODE 16

struct CommonInfo {
	// editor functionality prefs
	bool syntaxEnable;
	bool foldEnable;
	bool indentEnable;
	// display defaults prefs
	bool readOnlyInitial;
	bool overTypeInitial;
	bool wrapModeInitial;
	bool displayEOLEnable;
	bool indentGuideEnable;
	bool lineNumberEnable;
	bool longLineOnEnable;
	bool whiteSpaceEnable;
};
extern const CommonInfo g_CommonPrefs;

//----------------------------------------------------------------------------
// LanguageInfo

struct LanguageInfo {
	const char* name;
	const char* filepattern;
	int lexer;
	struct {
		int type;
		const char* words;
	} styles[STYLE_TYPES_COUNT];
	int folds;
};

extern const LanguageInfo g_LanguagePrefs[];
extern const int g_LanguagePrefsSize;

//----------------------------------------------------------------------------
// StyleInfo
struct StyleInfo {
	const wxString name;
	const wxString foreground;
	const wxString background;
	const wxString fontname;
	int fontsize;
	int fontstyle;
	int lettercase;
};

extern const StyleInfo g_StylePrefs[];
extern const int g_StylePrefsSize;

class PythonEditor : public wxStyledTextCtrl
{
	friend class EditProperties;
	enum
	{
		margin_id_lineno,
		margin_id_fold,
	};

public:
	PythonEditor(wxWindow* parent, wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxVSCROLL);

	~PythonEditor();

	// event handlers
// common
	void OnSize(wxSizeEvent& event);
	// edit
	void OnEditRedo(wxCommandEvent& event);
	void OnEditUndo(wxCommandEvent& event);
	void OnEditClear(wxCommandEvent& event);
	void OnEditCut(wxCommandEvent& event);
	void OnEditCopy(wxCommandEvent& event);
	void OnEditPaste(wxCommandEvent& event);
	void OnRunInPython(wxCommandEvent& event);
	// find
	void OnFind(wxCommandEvent& event);
	void OnFindNext(wxCommandEvent& event);
	void OnReplace(wxCommandEvent& event);
	void OnReplaceNext(wxCommandEvent& event);
	void OnBraceMatch(wxCommandEvent& event);
	void OnGoto(wxCommandEvent& event);
	void OnEditIndentInc(wxCommandEvent& event);
	void OnEditIndentRed(wxCommandEvent& event);
	void OnEditSelectAll(wxCommandEvent& event);
	void OnEditSelectLine(wxCommandEvent& event);
	//! view
	void OnHighlightLang(wxCommandEvent& event);
	void OnDisplayEOL(wxCommandEvent& event);
	void OnIndentGuide(wxCommandEvent& event);
	void OnLineNumber(wxCommandEvent& event);
	void OnLongLineOn(wxCommandEvent& event);
	void OnWhiteSpace(wxCommandEvent& event);
	void OnFoldToggle(wxCommandEvent& event);
	void OnSetOverType(wxCommandEvent& event);
	void OnSetReadOnly(wxCommandEvent& event);
	void OnWrapmodeOn(wxCommandEvent& event);
	void OnUseCharset(wxCommandEvent& event);
	// annotations
	void OnAnnotationAdd(wxCommandEvent& event);
	void OnAnnotationRemove(wxCommandEvent& event);
	void OnAnnotationClear(wxCommandEvent& event);
	void OnAnnotationStyle(wxCommandEvent& event);
	//! extra
	void OnChangeCase(wxCommandEvent& event);
	void OnConvertEOL(wxCommandEvent& event);
	void OnMultipleSelections(wxCommandEvent& event);
	void OnMultiPaste(wxCommandEvent& event);
	void OnMultipleSelectionsTyping(wxCommandEvent& event);
	void OnCustomPopup(wxCommandEvent& evt);
	void OnTechnology(wxCommandEvent& event);
	// stc
	void OnMarginClick(wxStyledTextEvent& event);
	void OnCharAdded(wxStyledTextEvent& event);
	void OnCallTipClick(wxStyledTextEvent& event);

	void OnKeyDown(wxKeyEvent& event);

	// call tips
	void ShowCallTipAt(int position);

	//! language/lexer
	wxString DeterminePrefs(const wxString& filename);
	bool InitializePrefs(const wxString& filename);
	LanguageInfo const* GetLanguageInfo() { return m_language; }

	//! load/save file
	bool LoadFile();
	bool LoadFile(const wxString& filename);
	bool SaveFile();
	bool SaveFile(const wxString& filename);
	bool Modified();
	wxString GetFilename() { return m_filename; }
	void SetFilename(const wxString& filename) { m_filename = filename; }

	void SetLexerPython();

protected:

	wxString m_filename;

	// language properties
	LanguageInfo const* m_language;

	// margin variables
	int m_LineNrID;
	int m_LineNrMargin;
	int m_FoldingID;
	int m_FoldingMargin;
	int m_DividerID;

	// call tip data
	int m_calltipNo;

	wxDECLARE_EVENT_TABLE();
};

class PythonEditorFrame : public wxFrame
{
public:
	PythonEditorFrame();
	~PythonEditorFrame();

	//! event handlers
	//! common
	void OnClose(wxCloseEvent& event);
	void OnExit(wxCommandEvent& event);
	//! file
	void OnFileOpen(wxCommandEvent& event);
	void OnFileSave(wxCommandEvent& event);
	void OnFileSaveAs(wxCommandEvent& event);
	void OnFileClose(wxCommandEvent& event);
	//! properties
	void OnProperties(wxCommandEvent& event);
	//! print
	//void OnPrintSetup(wxCommandEvent& event);
	//void OnPrintPreview(wxCommandEvent& event);
	//void OnPrint(wxCommandEvent& event);
	//! edit events
	void OnEdit(wxCommandEvent& event);
	void OnContextMenu(wxContextMenuEvent& evt);

private:
	// edit object
	PythonEditor* m_edit;
	void FileOpen(wxString fname);

	//! creates the application menu bar
	wxMenuBar* m_menuBar;
	void CreateMenu();

	wxDECLARE_EVENT_TABLE();
};

//----------------------------------------------------------------------------
//! EditProperties
class EditProperties : public wxDialog {

public:

	//! constructor
	EditProperties(PythonEditor* edit, long style = 0);

private:

};