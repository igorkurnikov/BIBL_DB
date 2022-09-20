#include "boost/format.hpp"
#include "boost/algorithm/string/case_conv.hpp"
#include "DataObject.h"


DataObject::DataObject()
{
	obj_id = 0;
	is_loaded = false;
}


DataObject::~DataObject()
{
}

int DataObject::GetId() const
{
	return obj_id;
}

void DataObject::SetId(int id)
{
	if (obj_id != id) Clear();
	obj_id = id;
}

AuthorRef::AuthorRef()
{
	type = AUTHOR_OBJ;
	importance = 0;
}

AuthorRef::AuthorRef(const AuthorRef& ref)
{
	type = AUTHOR_OBJ;
	obj_id = ref.obj_id;
	last_name = ref.last_name;
	first_name = ref.first_name;
	initials = ref.initials;
	middle_name = ref.middle_name;
	suffix = ref.suffix;
	address = ref.address;
	importance = ref.importance;
	url = ref.url;
	note = ref.note;
}

AuthorRef::~AuthorRef()
{

}

void AuthorRef::Clear()
{
	type = AUTHOR_OBJ;
	obj_id = 0;
	importance = 0;
	last_name = "";
	first_name = "";
	initials = "";
	middle_name = "";
	suffix = "";
	address = "";
	url = "";
	note = "";
}

std::string AuthorRef::ToString( int format ) const
{
	return last_name + "," + initials;
}

std::string AuthorRef::AuthVecToString(const std::vector<AuthorRef>& auth_vec)
{
	int i;
	int n = auth_vec.size();
	std::string str;
	for (i = 0; i < n; i++)
	{
		const AuthorRef& aref = auth_vec[i];
		std::string lname = boost::to_lower_copy(aref.last_name);
		lname[0] = toupper(lname[0]);
		std::string ini = boost::to_upper_copy(aref.initials);

		str += lname;
		str += ",";
		str += ini;
		str += "; ";
	}
	return str;
}

JournalRef::JournalRef()
{
	type = JOURNAL_OBJ;
	Clear();
}

JournalRef::JournalRef(const JournalRef& ref)
{
	type = JOURNAL_OBJ;
	obj_id = ref.obj_id;
	full_name = ref.full_name;
	std_abbr = ref.std_abbr;
	short_abbr = ref.short_abbr;
	fname_abbr = ref.fname_abbr;
	publisher_id = ref.publisher_id;
	abbr_29 = ref.abbr_29;
	issn = ref.issn;
	essn = ref.essn;
	nlm_id = ref.nlm_id;
	publisher_str = ref.publisher_str;
}

JournalRef::~JournalRef()
{

}

void JournalRef::Clear()
{
	type = JOURNAL_OBJ;
	obj_id = 0;
	full_name = "";
	std_abbr = "";
	short_abbr = "";
	fname_abbr = "";
	publisher_id = "";
	abbr_29 = "";
	issn = "";
	essn = "";
	nlm_id = "";
	publisher_str = "";
}

std::string JournalRef::ToString( int format ) const
{
	return std_abbr;
}

BiblRef::BiblRef()
{
	type = REF_OBJ;
	Clear();
}

BiblRef::BiblRef(const BiblRef& bref_old)
{
	type = REF_OBJ;
	obj_id = bref_old.obj_id;
	ext_ref_id = bref_old.ext_ref_id;
	ref_type = bref_old.ref_type;
	auth_vec = bref_old.auth_vec;
	title = bref_old.title;
	book_title = bref_old.book_title;
	jrn = bref_old.jrn;
	vol = bref_old.vol;
	iss = bref_old.iss;
	pub_year = bref_old.pub_year;
	pub_month = bref_old.pub_month;
	first_page = bref_old.first_page;
	last_page = bref_old.last_page;
	isi_id = bref_old.isi_id;
	isi_id_int = bref_old.isi_id_int;
	ga_code = bref_old.ga_code;
	pubmed_id = bref_old.pubmed_id;
	reprint_status = bref_old.reprint_status;
	importance = bref_old.importance;
	doi = bref_old.doi;
	url = bref_old.url;
	pii_id = bref_old.pii_id;
	medline_id = bref_old.medline_id;
	incomplete_auth_flag = bref_old.incomplete_auth_flag;
	num_cited_in = bref_old.num_cited_in;
	num_citing = bref_old.num_citing;
	last_update_time = bref_old.last_update_time;

	authors_str = bref_old.authors_str;

	keywords_str = bref_old.keywords_str;
	abstract_str = bref_old.abstract_str;
	notes = bref_old.notes;
}

BiblRef::~BiblRef()
{

}

void BiblRef::Clear()
{
	type = REF_OBJ;
	obj_id = 0;
	ext_ref_id = "";
	ref_type = 0;
	auth_vec.clear();
	title = "";
	book_title = "";
	jrn.Clear();
	vol = "";
	iss = "";
	pub_year = "";
	pub_month = "";
	first_page = "";
	last_page = "";
	isi_id = "";
	isi_id_int = 0;
	ga_code = "";
	pubmed_id = "";
	reprint_status = "";
	importance = 0;
	doi = "";
	url = "";
	pii_id = "";
	medline_id = "";
	authors_str = "";
	incomplete_auth_flag = 0;
	num_cited_in = 0;
	num_citing = 0;
	last_update_time = 0;

	keywords_str = "";
	abstract_str = "";
	notes = "";

	is_loaded = false;
}

std::string BiblRef::ToString(int format) const
{
	int i, n;

	std::string str;

	if (format < 2)
	{
		str += "id= ";
		str += ext_ref_id;
		str += "int_id= ";
		str += boost::str(boost::format("%d") % obj_id);
		str += "\n";
	}

	if (format == 2)
	{
		if (!ext_ref_id.empty())
		{
			str += "(";
			str += ext_ref_id.c_str();
			str += ")";
		}
	}

	if (!authors_str.empty())
	{
		str += authors_str;
	}
	else
	{
		str += GetAuthorStr();
	}

	str += "\n";
	str += title;
	str += "\n";
	str += jrn.std_abbr;
	str += "(";
	str += pub_year;
	str += ") ";
	str += "v.";
	str += vol;
	str += "(";
	str += iss;
	str += ") ";
	str += boost::str(boost::format("pp.%s-%s") % first_page.c_str() % last_page.c_str());

	return str;
}

std::string BiblRef::GetAuthorStr() const
{
	return AuthorRef::AuthVecToString(auth_vec);
}

bool BiblRef::HasDOI() const
{
	if (!doi.empty()) return true;
	return false;
}





KeyWord::KeyWord()
{
	type = KEYWORD_OBJ;
}

KeyWord::~KeyWord()
{

}



void KeyWord::Clear()
{
	obj_id = 0;
	data_str.clear();
}

std::string KeyWord::ToString( int format ) const
{
	return data_str;
}

bool CompareObjSPtrTypeString(const ObjSPtr& sp_obj1, const ObjSPtr& sp_obj2)
{
	if (sp_obj1->type < sp_obj2->type) return true;
	if (sp_obj1->type > sp_obj2->type) return false;

	return sp_obj1->ToString() < sp_obj2->ToString();
}

bool CompareObjTypeString(const DataObject& obj1, const DataObject& obj2)
{
	if (obj1.type < obj2.type) return true;
	if (obj1.type > obj2.type) return false;

	std::string str1 = obj1.ToString();
	std::string str2 = obj2.ToString();
	return(str1 < str2);
}

