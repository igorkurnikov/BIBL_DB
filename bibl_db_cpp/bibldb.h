/*! \file bibldb.h

    classes to access bibliographic database

   \author Igor Kurnikov
   \date 2003-

*/
#ifndef BIBLDB_H
#define BIBLDB_H

#include "cwos.h"
#include "DataObject.h"

const int wxEVT_DELETE_REFS = 55001;
const std::vector<int> EMPTY_VEC_INT;


enum ASSOC_TYPE {KEYWORD_TAG = 1, AUTH_REF = 2 };


class RefWebInfo
//!< Class for WEB location of the reference
{
public:
	std::string html_url;
	std::string pdf_url;
	std::string doi;
	std::string support_info;
	
	std::string journal_url;
	std::string issue_list_url;
	std::string issue_url;
};

class wxHTTPBuilder;
class wxHTTPBuilderThread;
class wxInputStream;

class BibRefRequest
//!< Bibiographic references Database request
{
public:
  
  BibRefRequest();
  virtual ~BibRefRequest();

  int Clear(); //!< Clear content of the request , fill out with default values 

  std::string ref_id_from; 
  std::string ref_id_to;
  std::string pub_year_from; 
  std::string pub_year_to;
  std::string ref_id_citing_this;
  std::string ref_id_cited_in_this;
  std::string author_flt_str;
  std::string title_flt_str;
  std::string journal_flt;
  std::string isi_id_flt;
  std::string importance_flt;
  std::string reprint_flt;
  std::string keywords_refs_flt;
  std::string volume_flt;
  std::string issue_flt;
  std::string pages_flt;

  int n_ref_limit;
  int n_ref_offset;
};

class BibRefInfo
//!< Bibiographic Reference Info 
{
public:
	BibRefInfo();
	virtual ~BibRefInfo();

	int Clear();

	std::string keywords_str;
	std::string cited_ref;
	std::string cited_in_ref;
};

class AuthorRequest
//!< Author Database request
{
public:
	AuthorRequest();
	virtual ~AuthorRequest();

	std::string search_author_name_flt;
	std::string auth_importance_flt;
	std::string keywords_auth_flt;

	int Clear(); //!< Clear content of the request , fill out with default values 
};

class JournalRequest
//!< Journal Database request
{
public:
	JournalRequest();
	virtual ~JournalRequest();

	std::string journal_name_flt;
	std::string jrn_id_from;
	std::string jrn_id_to;

	int Clear(); //!< Clear content of the request , fill out with default values 

};

const int GET_REF_ABSTRACT   = 0x0001;
const int GET_REF_NOTES      = 0x0002;
const int GET_REF_KEY_WORDS  = 0x0004;
const int GET_REF_AUTH_VEC   = 0x0008;

const int GET_REF_FULL = GET_REF_ABSTRACT | GET_REF_NOTES | GET_REF_KEY_WORDS | GET_REF_AUTH_VEC;

#pragma pack(push, 8)
//#define WXHTTPENGINE 1

class BiblDB
//!< Class to access my Bibliographic Database from HARLEM
{
public:
  BiblDB();
  virtual ~BiblDB();

  int Init(); //!< Connect to DB, return FALSE on failure 
  int InitPython(); //!< Initialize Python Interface 

  static int RunPythonScriptInString(std::string script_string); //!< Execute Python script in string 

  std::vector<int> SearchRefs(const BibRefRequest& breq); //!< Search references parameters specified in breq
  std::vector<int> SearchAuths(const AuthorRequest& areq); //!< Search authors using parameters pecified in Authors request

  int CreateNewObj( int obj_type ); //!< Create New Object with type obj_type in the Database, return object id

  int DeleteRefs(const std::vector<int>& del_refs_id); //!< Delete References with given ids 

  int GetRefsByID(std::vector<int>& ref_id_vec, std::vector<BiblRef>& refs_vec );     //!< Get References by a list of IDs (to refs_vec)
  BiblRef GetRefByID(int ref_id_a, int option = GET_REF_FULL );           //!< Get Reference by ID 
  int GetRefID(const char* jrn_name, const char* pub_year, const char* vol, const char* issue, const char* fst_page); //!< Get Ref ID  return (-1) if not found and (-2) if not unique.. 
  int GetRefID( const BiblRef& bref );     //!< Get Reference ID from the information in bref
  int CreateNewRefDB();                     //!< Create New Empty reference in the database, return ref_id
  int UpdateReferenceDB( const BiblRef& bref, int force_update = 0); //!< Update reference on the database using info in bref if force_update = 1 update all fields if  force_update = 0 - add to fields(keywords) or not update non-empty fields
  int UpdateAuthorsForRef(int ref_id, const std::vector<AuthorRef>& auth_vec);    //!< Update Authors for reference ref_id
  int UpdateTextFieldForRef(int ref_id, const char* value, const char* field_name, const char* table_name = "REFS"); //!< Set Value of the text field of the reference ref_id
  std::string GetTextFieldForRef(int ref_id, const char* field_name, const char* table_name = "REFS");               //!< Get Value of the text field of the reference ref_id         
  int UpdateIntFieldForRef(int ref_id, int value, const char* field_name, const char* table_name = "REFS");  //!< Set Value of int field of the reference ref_id
  int GetIntFieldForRef(int ref_id, const char* field_name, const char* table_name = "REFS");  //!< Get Value of int field of the reference ref_id         
  int SetImportanceForRef(int ref_id,int value); //!< Set importance for reference

  std::string GetStdPrefix( const BiblRef& bref ) const;  //!< Get Standard Prefix for the reference
  bool IsSourceInfoComplete( const BiblRef& bref ) const; //!< Check if info identifing Reference Source ( Journal name, volume( or Issue or Archive id) year page number etc are complete ) 


  std::vector<AuthorRef> GetAuthForRef(int ref_id_a);    //!< Get a list of authors for reference ref_id_a
  std::vector<AuthorRef> GetAuthForRef_SH(int ref_id_a); //!< Get a list of authors for reference ref_id_a - short version
  AuthorRef GetAuthByID(int auth_id_a);   //!< Get Author reference by its ID
  int GetAuthsByID(std::vector<int>& auth_id_vec, std::vector<AuthorRef>& auth_vec ); //!< Get an array of Author's reference by their ids
  int CreateNewAuthor(const AuthorRef& aref); //!< Create New Author 
  int GetAuthorID(const AuthorRef& aref, int create_new_author = 0);     //!< Get Author ID possibly creating a new author 
  int UpdateAuthorStrForRef( int ref_id );   //!< Update Author String from Author Vector
 
  int SetAuthImportance(int auth_id, int importance ); //!< Set Importance factor for an author
  int SetAuthURL (int auth_id , const char* url_str ); //!< Set URL for an author
  int SetAuthNote(int auth_id , const char* note_str ); //!< Set Note for an author

  JournalRef GetJournalByID( int journal_id );      //!< Get Journal By ID
  int GetJournalID( const JournalRef& jrn_ref, int create_new_journal = 0);   //!< Get Journal ID for the journal reference (create new ID if a new journal and create_new_journal != 0 ) 
  int GetJournalID( const char* jrn_name);  //!< Get Journal ID for journal name, return 0 if journal doesn't exist 
  int GetJournalIDByFullName( const char* jrn_full_name);  //!< Get Journal ID from journal full name, return 0 if journal doesn't exist 
  std::vector<int> FilterJournalIDs(const JournalRequest& jreq); //!< Get IDs of journals satisfying the request
  int CreateNewJournal(const JournalRef& jrn_ref);  //!< Create New Journal in the Database
  int UpdateJournal(const JournalRef& jrn_ref, int force_update = 0); //!< Update Journal Information in DB 
  std::vector<std::string> GetSynForJournal(int journal_id); //!< Get a list of synonyms for journal
  int SetSynForJournal(int journal_id, const char* jrn_name);     //!< Set synonym for journal
  int RemoveSynForJournal( int journal_id, const char* jrn_name);  //!< Remove Synonym for journal
  int LoadJournalList1(FILE* fp); //!< Load List of Journals from HTML file in WOS format
  int LoadJournalList2(FILE* fp); //!< Load List of Std Journal Abbreviations from HTML file 
  int LoadJournalList3(const char* fname); //!< Load List of WOS Journal Abbreviations from HTML file 
  int MergeJournals(int jrn_id_from, int jrn_id_to); //!< Merge keyword 1( rename) to keyword 2
  bool IsPreprintJournal( int journal_id ) const; //!< Check if this Preprint Journal ( like ArXiv )
  bool IsPreprintJournal( const JournalRef& jrn_ref ) const; //!< Check if this Preprint Journal ( like ArXiv )

  int GetIncompleteFlagForRef(int ref_id); //!< Get incomplete status of the reference ( if true not all authors are set)
  int SetIncompleteFlagForRef(int ref_id, int value_new = 1); //!< Set incomplete status of the reference

  int SetCurrentUpdateTime(int ref_id); //!< Set Last update time for the reference to current time

  int SetCitation(int ref_id_citing, int ref_id_cited );    //!< Assign reference ref_id_cited to be cited by reference ref_id_citing
  int RemoveCitation(int ref_id_citing, int ref_id_cited ); //!< Remove citation relationship between ref_id_citing and ref_id_cited 
  std::vector<int> GetRefsCitedIn(int ref_id_citing);  //!< Get Reference IDs cited in ref_id_citing
  std::vector<int> GetRefsCiting(int ref_id_cited);    //!< Get Reference IDs citing ref_id_cited
 
  void PrintRef1(const BiblRef& bref);
  void PrintRefFullStr(const BiblRef& bref, std::string& str); //!< Print full content of the reference to the string

  int LoadRefFromWeb(const BiblRef& bref); //!< Try to Load PDF and HTML files of the Reference from WEB 
  int SaveRefPDF(const BiblRef& bref, std::string pdf_path, bool overwrite = false ); //!< Save Reference PDF given by a local path to the Database
  int MoveRefPDFToStdLocation(const BiblRef& bref); //!< Move reference PDFs to the standard path in the Database 

  int GetACSRefInfo(const BiblRef& bref, RefWebInfo& web_info); //!< Get Info on Web Location of reference from ACS journal 
  int GetAPSRefInfo(const BiblRef& bref, RefWebInfo& web_info); //!< Get Info on Web Location of reference from APS journal 
  int GetAIPRefInfo(const BiblRef& bref, RefWebInfo& web_info); //!< Get Info on Web Location of reference from AIP journal 
  int GetSpringerJRefInfo(const BiblRef& bref, RefWebInfo& web_info); //!< Get Info on Web Location of reference from Springer journal 
  int GetSpringerSRefInfo(const BiblRef& bref, RefWebInfo& web_info); //!< Get Info on Web Location of reference from Springer series 
  int GetSciDirRefInfo(const BiblRef& bref, RefWebInfo& web_info); //!< Get Info on Web Location of reference from Ssience Direct journal 
  int GetSTD1RefInfo(const BiblRef& bref, RefWebInfo& web_info); //!< Get Info on Web Location of reference from STD1 type journal 
  int GetAnnuRevRefInfo(const BiblRef& bref, RefWebInfo& web_info); //!< Get Info on Web Location of reference from AnnuaL Review type journal 

  std::string GetBiblDir(); //!< Get local bibl directory 
  std::string GetLocPrefix(const BiblRef& bref); //!< Get prefix of the files (PDF,HTML..) corresponding to the reference
  std::string GetLocDir_DOI(const BiblRef& bref); //!< Get Local DOI directory for the reference 
  std::string GetLocDir_default(const BiblRef& bref); //!< Get Local default directory for the reference 
  std::string GetLocPathPDF_default(const BiblRef& bref); //!< Get default path for PDF file of the reference
  std::string GetLocPathPDF_no_subdir(const BiblRef& bref); //!< Get path for PDF file of the reference stored without subdir ( old style before 2021 )
  std::string GetLocPathPDF(const BiblRef& bref); //!< Get Local Path of PDF file for the reference	
  std::string GetLocPathSupp_no_subdir(const BiblRef& bref); //!< Get Local Path of supplement for the reference without subdir ( old style before 2021 )
  bool HasSupp(const BiblRef& bref); //!< check if the reference has supplemental materials in the database 
  std::string GetPDFName(const BiblRef& bref);  //!< Get Name of the PDF file for the reference
  std::string GetPDFDir(const BiblRef& bref); //!< Get directory name containing PDF file of the reference 
  bool OpenRefDir(const BiblRef& bref); //!< Open Reference Directory with PDF files and supplemental info with a file manager 
  bool DeleteRefDir(const BiblRef& bref); //!< Delete Reference Directory 
  bool DeleteRefPDF_no_subdir(const BiblRef& bref); //!< Delete Reference PDF without subdir ( old style before 2021 )

  int GetKeyWordID(const char* keyw, int create_keyw = 0); //!< get ID of the keyword - create if create_keyw = TRUE

  std::string GetKeyWordByID(int keyw_id); //!< Get Keyword by Keyword ID
  int AssocObjIDs(const std::vector<int>& obj_ids_left, const std::vector<int>& obj_ids_right ); //!< Associate Object IDs  
  int DelAssocObjIDs(const std::vector<int>& obj_ids_left, const std::vector<int>& obj_ids_right); //!< Delete Association of Object IDs  

  int AddKWToCategory( const char* kw, std::string cat_str); //!< Add Keyword to the category 
  int DelKWFromCategory( const char* kw, std::string cat_str); //!< Delete Keyword from the category
  int GetKWForCategory(std::vector<std::string>& str_arr, std::string cat_str, std::vector<int>* sel_kw_ids_right_par = NULL, std::vector<int>* sel_kw_ids_left_par = NULL ); //!< Get Key Words for the particular category 
  ObjVec GetObjectsForCategory(std::string cat_str); //!< Get list of objects for a category
  std::vector<int> GetAssocKWRight(int obj_id ); //!< Get Associated Keywords on the Right

  ObjVec LoadDataObjects(const std::vector<int>& ids_vec); //!< Load Objects from DB for a list of ids 
  std::vector<int> GetAssocObjIDs(const std::vector<int>& obj_ids_left1, const std::vector<int>& obj_ids_ndir = EMPTY_VEC_INT, const std::vector<int>& obj_ids_left2 = EMPTY_VEC_INT, const std::vector<int>& obj_ids_right = EMPTY_VEC_INT );
  int GetObjType(int obj_id); //!< Get Object type from its DB id

  int SetObjKW(int obj_id, const std::string& key_words ); //!< Set Keywords (separated by ;)  for the object
  int SetObjKWID(int obj_id, int kw_id );    //!< Set Keyword ID for the object
  int DelObjKW(int obj_id, const std::string& keyw ); //!< Delete association of the object with keywords (separated by ; can contain %)  
  int DelObjKW_All(int obj_id);          //!< Delete all keywords of the object
  std::string GetObjKW(int obj_id);       //!< Get Keywords (separated by ';' ) for the object
  std::vector<int> GetObjKWID(int obj_id); //!< Get Keywords' IDs of the object;
  
  int EraseKeyWord(const char* kw1); //!< Completely erase Keyword from DB
  int MergeKeyWords(const char* kw1, const char* kw2); //!< Merge keyword 1( rename) to keyword 2
  int SetKW2RefKW1(const char* kw1, const char* kw2);  //!< Associate all references of keyword 1 with keyword 2
  int GetObjsByKW(std::vector<int>& obj_id_vec, int obj_type, const std::string& kw ); // !< get Objects of a given type by keyword
  int GetRefsByKW(std::vector<int>& ref_id_vec, const char* kw);  //!< Get Reference IDs for a key word
  int GetAuthsByKW(std::vector<int>& auth_id_vec, const char* kw);  //!< Get Authors IDs for a key word

//! Interaction with Web of Science   

  int InitWebDriver();
  int InitWOS();
  int InitWOSRemote();
  int GotoWOSSearchPage();
  int GotoWOSMainPage(); //!< Open Web of Science main page using Selenium webdriver
  int SearchWOSPars(const BibRefRequest& breq); //!< Search WOS with search parameters in breq 
  int GotoWOSMarkedList();             //!<  Goto WOS Marked List
  int GotoWOSRef(BiblRef& ref);        //!<  Goto WOS citation on the web
  int GotoWOSRef2(BiblRef& ref);       //!<  Goto WOS citation on the web new version
  int GotoWOSRefDriver(BiblRef& ref);  //!<  Goto WOS citation on the web  using webdriver
  int GotoWOSCitedRefDriver(BiblRef& ref);  //!<  Goto WOS cited references on the web using webdriver
  int GotoWOSCitingRefDriver(BiblRef& ref); //!<  Goto WOS citing references on the web using webdriver
  int GotoWOSCitedRef2(BiblRef& ref);  //!<  Goto WOS cited references on the web new version
  int GotoWOSCitingRef(BiblRef& ref);  //!<  Goto WOS list of citing references on the web of science

  int GotoGScholarRefDriver(BiblRef& ref); //!<  Goto Google Scholar citation on the web using webdriver
   
  int ImportRefsISI( wxInputStream& stream, const BibRefInfo* ref_info = NULL );  //!< Load references to the database from input stream
  int ImportRefsRIS(std::istream& stream, const BibRefInfo* ref_info = NULL);  //!< Load references to the database from input stream
  int ImportRefsPubMedXmlStr(const char* refs_str, const char* kw_str = NULL); //!< Import References from String in PubMed XML Format 
  int ImportRefsPubMedXmlFile(FILE* finp, const char* kw_str = NULL );          //!< Import References from File in PubMed XML Format 
  int ImportRefsBibTeXStr( std::string refs_str, const BibRefInfo* ref_info = NULL ); //!< References from String in BibTeX Format 
  int ImportRefsNBIB(std::istream& stream, const BibRefInfo* ref_info = NULL);  //!< Load references to the database from input stream in NBIB format

  int AxxFun();

  int SQLQuery(const std::string& query, std::vector<std::string>* p_resp = NULL); //!< SQL query, return result in p_resp
  int SQLIntFunc(const std::string& query); //!< SQL query that return integer (999999 if error)
  
  int GetLastSQLResNumRows() const;   //!< Return number of rows in the last SQL result
  int GetLastSQLResNumCols() const;   //!< Return number of columns in the last SQL result
  int GetLastSQLResError() const;     //!< Return Error Code of the last SQL result
  
public:
  int num_rows_sql;           //!< The number of rows in the last SQL request 
  int num_cols_sql;           //!< The number of columns in the last SQL request
  int sql_error_code;         //!< Error Code of the lat SQL request
  int junk;
  int show_cit_flag;          //!< Show citation info 

  int webdriver_flag;  //!< Flag to indicate that the webdriver is initialized  

  std::string wos_sid;
  std::string wos_start_url;
  std::string wos_cgi;
  std::string wos_cgi_2;
  std::string wos_main_url;

  static std::string acro_read_str; //!< Full path of acrobat reader executable

  static void* db_mysql;
  int ref_id_s;
  int init_flag; //!< flag to indicate that connection and tables were initiated
  CWos* pwos;    //!< object to handle interactions with Web of Science website

  static BiblDB* InitNewDB() 
  {
 	  BiblDB* p_bibl_db = new BiblDB; 
      if( p_bibl_db && p_bibl_db->Init() ) return p_bibl_db;
	  return NULL; 
  }
};

//! BiblDBApp starter for python runs
void StartBiblDBApp();

#pragma pack(pop)


#endif // end of !BIBLDB_H

