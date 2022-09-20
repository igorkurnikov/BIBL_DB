#pragma once
#include <string>
#include <vector>
#include <set>

enum OBJECT_TYPE {
	UNDEF_OBJ = 0, REF_OBJ = 1, AUTHOR_OBJ = 2, JOURNAL_OBJ = 3,
	KEYWORD_OBJ = 4, NOTE_OBJ = 5
};

class DataObject
{
public:
	DataObject();
	virtual ~DataObject();

	virtual std::string ToString(int format = 0) const = 0; //!< Get String Representation of the object  
	virtual bool IsLoaded() noexcept { return is_loaded; }  //!< Check if the object data loaded from DB
	virtual void Clear() = 0; //!< Clear content of the object
	int GetId() const;   //!< Get DB id of the object
    void SetId( int id); //!< Set DB id of the object

	int obj_id;

public:
	bool is_loaded;   //!< indicator Data of the DataObject are loaded from the DB
	OBJECT_TYPE type; //!< Object type
};

typedef std::vector<std::shared_ptr<DataObject>> ObjVec;
typedef std::shared_ptr<DataObject>  ObjSPtr;

bool CompareObjTypeString(const DataObject& obj1, const DataObject& obj2);
bool CompareObjSPtrTypeString(const ObjSPtr& sp_obj1, const ObjSPtr& sp_obj2);

class AuthorRef : public DataObject
{
public:
	AuthorRef();
	AuthorRef(const AuthorRef& ref);
	~AuthorRef();

	virtual void Clear() override; //!< clear reference content
	virtual std::string ToString(int format = 0) const override; //!< Get String Representation of the object   

	static std::string AuthVecToString(const std::vector<AuthorRef>& auth_vec); //!< Format an array of Authors to string

	std::string last_name;
	std::string first_name;
	std::string initials;
	std::string middle_name;
	std::string suffix;
	std::string address;
	std::string url;
	std::string note;
	int importance;
};

class JournalRef : public DataObject
	//!< reference of the journal
{
public:
	JournalRef();
	JournalRef(const JournalRef& ref);
	~JournalRef();

	virtual void Clear() override;
	virtual std::string ToString(int format = 0) const override; //!< Get String Representation of the object   

	std::string full_name;    //!< Standard Full Name of the journal
	std::string std_abbr;     //!< Standard Abbreviation
	std::string short_abbr;
	std::string fname_abbr;    //!< Prefix to use in article file names
	std::string publisher_id;  //!< Publisher id 
	std::string abbr_29;       //!< 29 character abbreviation
	std::string issn;          //!< ISSN number of the journal
	std::string essn;
	std::string nlm_id;
	std::string publisher_str;  //!< Name of the publisher

	std::set<std::string> synonyms; //!< Journal Synonyms 
};

const int BIB_REF_TYPE_JOURNAL = 0;
const int BIB_REF_TYPE_IN_SERIES = 1;
const int BIB_REF_TYPE_BOOK = 2;
const int BIB_REF_TYPE_BOOK_CHAPTER = 3;

class BiblRef : public DataObject
{
public:
	BiblRef();
	BiblRef(const BiblRef& bref_old);
	~BiblRef();

	virtual void Clear() override;
	virtual std::string ToString( int format = 0 ) const override; //!< Get String Representation of the object 

	std::string GetAuthorStr() const; //!< Format string of Authors

	std::string ext_ref_id;
	int ref_type;  //!< Reference Type (BIB_REF_... const)

	std::vector<AuthorRef> auth_vec;
	std::string authors_str; //!< String with Authors Names in Std format
	std::string title;
	std::string book_title;
	
	JournalRef jrn;
	std::string vol;
	std::string iss;
	std::string pub_year;
	std::string pub_month;
	std::string first_page;
	std::string last_page;
	std::string isi_id;
	int isi_id_int;
	std::string ga_code;
	std::string pubmed_id;
	std::string reprint_status;
	int importance;
	std::string doi;
	std::string url;
	std::string pii_id;
	std::string medline_id;
	int incomplete_auth_flag; //!< flag to indicate that the author list of the reference is incomplete 
	int num_cited_in; //!< number of refs cited in the paper
	int num_citing;   //!< number of references citing given paper
	time_t last_update_time; //!< time (in seconds ? ) from January 1 1970 

	std::string keywords_str;  //!< Keywords associated with the reference (separated by ';')
	std::string abstract_str;
	std::string notes;

	bool HasDOI() const;  //!< Has a DOI identifier 
};

class KeyWord : public DataObject
{
public:
	KeyWord();
	~KeyWord();

	virtual void Clear() override; //!< Clear the memmory content of the object 
	virtual std::string ToString( int format = 0 ) const override; //!< Get String Representation of the object

	std::string data_str;
protected:

};