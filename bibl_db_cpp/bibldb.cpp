/*! \file bibldb.cpp

    classes to access bibliographic database through ODBC driver using WX library

   \author Igor Kurnikov
   \date 2003-

*/

#include "Python.h"

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/url.h"
#include "wx/wfstream.h"
#include "wx/sstream.h"
#include "wx/txtstrm.h"
#include <wx/filename.h>
#include <wx/log.h>

#include "tinyxml.h"

#include "wx/string.h"
#include <wx/arrstr.h>
// #include <wx/protocol/http.h>

#include <vector>  
#include <map>
#include <set>
#include <strstream>
#include <string>
#include <algorithm>
#include <locale>
#include <codecvt>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/exception/all.hpp>
#include <boost/locale.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>


namespace fs = boost::filesystem;

#include "winsock.h"
#define MYSQL_DB

#pragma warning(disable:4786)

#include "bibldb.h"

#if defined(MYSQL_DB)
#  include "mysql.h"
#endif

void* BiblDB::db_mysql = NULL;
std::string BiblDB::acro_read_str = "";


int BiblDB::RunPythonScriptInString(std::string script_string)
{
	PyGILState_STATE gstate = PyGILState_Ensure();
	if (PyErr_Occurred()) {  // PyErr_Print();  
		PyErr_Clear();
	}

	int ires = PyRun_SimpleString(script_string.c_str());

	if (PyErr_Occurred()) {  // PyErr_Print(); 
		PyErr_Clear();
	}
	PyGILState_Release(gstate);
	return TRUE;
}

BibRefRequest::BibRefRequest()
{
	Clear();
}

BibRefRequest::~BibRefRequest()
{

}

int BibRefRequest::Clear()
{
  ref_id_from    = ""; 
  ref_id_to      = "";
  pub_year_from  = ""; 
  pub_year_to    = ""; 
  ref_id_citing_this       = "";  
  ref_id_cited_in_this    = ""; 
  author_flt_str = "";
  title_flt_str  = "";
  journal_flt    = "";
  isi_id_flt     = "";
  importance_flt = "";
  reprint_flt    = "";
  keywords_refs_flt = "";
  volume_flt        = "";
  issue_flt         = "";
  pages_flt         = "";
  
  n_ref_limit  = 1000;
  n_ref_offset = 0;

  return TRUE;
}

BibRefInfo::BibRefInfo()
{
	Clear();
}

BibRefInfo::~BibRefInfo()
{

}

int BibRefInfo::Clear()
{
  cited_ref       = "";  
  cited_in_ref    = ""; 
  keywords_str   = "";

  return TRUE;
}


AuthorRequest::AuthorRequest()
{
	Clear();
}

AuthorRequest::~AuthorRequest()
{

}

int AuthorRequest::Clear()
{
   search_author_name_flt = "";
   auth_importance_flt    = "";
   keywords_auth_flt      = "";

   return TRUE;
}

JournalRequest::JournalRequest()
{
	Clear();
}

JournalRequest::~JournalRequest()
{

}

int JournalRequest::Clear()
{
	journal_name_flt.clear();
	jrn_id_from.clear();
	jrn_id_to.clear();

	return TRUE;
}

BiblDB::BiblDB()
{
	init_flag = FALSE;
	ref_id_s = 0;

//	wos_main_url = "http://www.isiknowledge.com";
	wos_main_url = "http://www.webofknowledge.com/?DestApp=WOS";
	wos_cgi      = "http://apps.webofknowledge.com/InboundService.do";
	wos_cgi_2    = "http://apps.isiknowledge.com/";

	pwos = new CWos();
	
	webdriver_flag = FALSE;
}

BiblDB::~BiblDB()
{
	delete(pwos);
}

int BiblDB::Init()
{   
	if( db_mysql == NULL )
	{
		wxString user = "";
		wxString pass = "";
		
		std::string db_name = "igor_bibl_db";
//		std::string db_name = "savvy-scion-211220:us-east1:krotsql";

//		std::string host = "35.229.73.252";
		std::string host = "localhost";

//		std::string password = "shapka12";
		std::string password = "lisa&volk";
	
#if defined(MYSQL_DB)
		db_mysql = (void*) mysql_init( (MYSQL*)db_mysql);
		//mysql_options((MYSQL*)db_mysql, MYSQL_SET_CHARSET_NAME, "utf8");
		//mysql_options((MYSQL*)db_mysql, MYSQL_INIT_COMMAND, "SET NAMES utf8");

		mysql_options((MYSQL*)db_mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");
		mysql_options((MYSQL*)db_mysql, MYSQL_INIT_COMMAND, "SET NAMES utf8mb4");

		if(!mysql_real_connect((MYSQL*)db_mysql,host.c_str(),"root",password.c_str(),db_name.c_str(),3306,NULL,0))
		{
			wxLogMessage("Unable to connect to Bibliographic Database \n");
			return FALSE;
		}
#endif

		std::vector<wxString> adobe_paths;
		adobe_paths.push_back("C:\\Program Files\\Adobe\\Acrobat 7.0\\");
		adobe_paths.push_back("C:\\Program Files\\Adobe\\Reader 9.0\\");
		adobe_paths.push_back("C:\\Program Files (x86)\\Adobe\\Reader 9.0\\");
		adobe_paths.push_back("C:\\Program Files (x86)\\Adobe\\Reader 10.0\\");
		adobe_paths.push_back("C:\\Program Files (x86)\\Adobe\\Reader 11.0\\");
		adobe_paths.push_back("C:\\Program Files (x86)\\Adobe\\Reader 12.0\\");
		adobe_paths.push_back("C:\\Program Files (x86)\\Adobe\\Acrobat Reader DC\\");
		adobe_paths.push_back("C:\\Program Files\\Adobe\\Acrobat DC\\");

		acro_read_str.clear();
		int i;
		wxString fname;
		for(i = 0; i < adobe_paths.size(); i++)
		{
			fname = adobe_paths[i];
			fname += "Reader\\";
			fname += "AcroRd32.exe";
			if( wxFile::Exists(fname) )
			{
				acro_read_str = fname;
				//wxLogMessage(" acro_read_str = %s \n",acro_read_str.c_str());
				break;
			}
			fname = adobe_paths[i];
			fname += "Acrobat\\";
			fname += "Acrobat.exe";
			if (wxFile::Exists(fname))
			{
				acro_read_str = fname;
				//wxLogMessage(" acro_read_str = %s \n", acro_read_str.c_str());
				break;
			}
		}
	}

    init_flag = TRUE;
	num_rows_sql = 0;
	num_cols_sql = 0;
	sql_error_code = 0;
	show_cit_flag = TRUE;

	// Get the default locale  // Should we do it just once in the beginning of the program??
	std::locale loc = boost::locale::generator().generate("");
	// Set the global locale to loc
	std::locale::global(loc);    // this is necessary for proper conversion of filename from UTF8??
	// Make boost.filesystem use it by default
	boost::filesystem::path::imbue(std::locale());

	return TRUE;
}


std::vector<int> BiblDB::SearchRefs(const BibRefRequest& breq)
{
	std::vector<int> ref_id_vec;

#if defined(MYSQL_DB)

    MYSQL_RES* res;
	MYSQL_RES* res1;
	MYSQL_ROW row;
	MYSQL_ROW row1;
	std::string query;

	int ires;

	std::string where_clause;
	int first_condition = TRUE;

	if(!breq.ref_id_from.empty())
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		where_clause += " ( REFS.REF_ID >=" + breq.ref_id_from + ") ";
	}

	if(!breq.ref_id_to.empty())
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		where_clause += " ( REFS.REF_ID <=" + breq.ref_id_to + ") ";
	}

	if(!breq.pub_year_from.empty())
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		where_clause += " ( REFS.PUB_YEAR >=" + breq.pub_year_from + ") ";
	}

	
	if(!breq.pub_year_to.empty())
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		where_clause += " ( REFS.PUB_YEAR <=" + breq.pub_year_to + ") ";
		
	}

	int add_cit_table = FALSE;
	if(!breq.ref_id_cited_in_this.empty())
	{
		try
		{
			int cit_in_id = boost::lexical_cast<int>(breq.ref_id_cited_in_this);
			add_cit_table = TRUE;
			if(first_condition) 
			{
				first_condition = FALSE;
			}
			else 
			{
				where_clause += " AND ";
			}

			std::vector<int> ref_id_vec = GetRefsCitedIn(atoi(breq.ref_id_cited_in_this.c_str()));
			int nr = ref_id_vec.size();
			int j;
			where_clause += "REF_ID IN( ";
			for( j=0; j < nr; j++)
			{
				std::string str_id;
				if( j != 0) where_clause += ",";
				str_id = boost::str(boost::format("%d") % ref_id_vec[j]);
				where_clause += str_id;
			}
			where_clause += " ) ";
		}
		catch(boost::bad_lexical_cast&){}
	}

	if(!breq.ref_id_citing_this.empty())
	{
		add_cit_table = TRUE;
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		
		std::vector<int> ref_id_vec = GetRefsCiting(atoi(breq.ref_id_citing_this.c_str()));
		int nr = ref_id_vec.size();
		int j;
		where_clause += "REF_ID IN( ";
		for( j=0; j < nr; j++)
		{
			std::string str_id;
			if( j != 0) where_clause += ",";
			str_id = boost::str(boost::format("%d") % ref_id_vec[j]);
			where_clause += str_id;
		}
		where_clause += " ) ";
	}

	if(!breq.isi_id_flt.empty())
	{
		add_cit_table = TRUE;
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		where_clause += " ( REFS.ISI_ID = " + breq.isi_id_flt + " )";
	}

	if(!breq.importance_flt.empty())
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		where_clause += " ( REFS.IMPORTANCE >= " + breq.importance_flt + " )";
	}

	if(!breq.reprint_flt.empty())
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		where_clause += " ( REFS.REPRINT_STATUS LIKE \'%" + breq.reprint_flt + "%\' )";
//		where_clause += " ( REFS.REPRINT_STATUS LIKE " + reprint_flt + " )";
	}

	std::vector<std::string> key_words;
	if(!breq.keywords_refs_flt.empty())
	{	
		boost::split(key_words,breq.keywords_refs_flt,boost::is_any_of(";"));
		for(std::string& keyw : key_words)
		{
			boost::trim(keyw);
			boost::to_lower(keyw);
		}
		std::vector<std::string>::iterator kitr = key_words.begin();
		for (; kitr != key_words.end();)
		{
			if ((*kitr).empty())
			{
				kitr = key_words.erase(kitr);
			}
			else
			{
				kitr++;
			}
		}
	}

	std::string limit_str;
	limit_str = boost::str(boost::format(" LIMIT %d, %d ") % breq.n_ref_offset % breq.n_ref_limit); 

	query = "SELECT REFS.REF_ID FROM REFS";

	if(!breq.journal_flt.empty())
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}

		std::vector<int> jids;		
		int is_list_int = FALSE;
		
		std::vector<std::string> jrn_str_arr;
		
		std::string journal_flt_str = breq.journal_flt;

		boost::trim(journal_flt_str);
		boost::split(jrn_str_arr, journal_flt_str, boost::is_any_of(";, "));

		for(std::string jid_str : jrn_str_arr)
		{
			int jid;
			boost::trim(jid_str); 
			if(jid_str.empty()) continue;
			try 
			{
				jid = boost::lexical_cast<int>(jid_str);
				is_list_int = TRUE;
				jids.push_back(jid);
			}
			catch(boost::bad_lexical_cast&)
			{
				is_list_int = FALSE;
				break;
			}
		}
		if( is_list_int == FALSE )
		{
			JournalRequest jreq;
			jreq.journal_name_flt = breq.journal_flt;
			jids = FilterJournalIDs(jreq);
		}
		int nid = jids.size();

		if( nid == 0 )
		{
			where_clause += " (JOURNAL_ID = 99999) ";
		}
		else
		{
			where_clause += " (JOURNAL_ID IN (";
			int i;
			for(i = 0; i < nid; i++)
			{
				if(i > 0) where_clause += ",";
				where_clause += boost::str(boost::format("%d") % jids[i]);
			}	
			where_clause += ") )";
		}
	}

	if(!breq.title_flt_str.empty())
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		where_clause += " (TITLE LIKE \'%" + breq.title_flt_str + "%\') ";
	}

	if(!breq.author_flt_str.empty())
	{
		std::string author_flt_str_copy = breq.author_flt_str;

		boost::trim(author_flt_str_copy);
		boost::to_upper(author_flt_str_copy);

		std::replace(author_flt_str_copy.begin(), author_flt_str_copy.end(), '*', '%');
		std::replace(author_flt_str_copy.begin(), author_flt_str_copy.end(), '?', '_');
		
		std::string last_name = "";
		std::string initials  = "";

		std::vector<std::string> name_part_arr;

		boost::split(name_part_arr, author_flt_str_copy, boost::is_any_of(","));

		int i = 0;
		for(std::string name_part : name_part_arr)
		{
			if(name_part.empty()) continue;
			if(i == 0) last_name = name_part;
			if(i == 1) initials  = name_part;
			i++;
		}

		std::string query_a  = "SELECT AUTH_ID FROM AUTHORS ";
		            query_a += "WHERE (LAST_NAME LIKE \'" + last_name + "\'";
		if(!initials.empty())
		{
			query_a += " AND INITIALS LIKE ";
			query_a += "\'";
			query_a += initials;
			query_a += "\'";
		}
		query_a += " )";

		wxLogMessage(" Authors Select Query: \n%s \n", (wxString) query_a);
		ires = mysql_real_query( (MYSQL*)db_mysql, query_a.c_str(), query_a.length());

		if(!ires)
		{
			res = mysql_store_result((MYSQL*)db_mysql);
			int nrow = mysql_num_rows(res);

			if( nrow > 0)
			{
				if(first_condition) 
				{
					first_condition = FALSE;
				}
				else 
				{
					where_clause += " AND ";
				}

				where_clause += "REFS.REF_ID IN (1000000";
				while( row = mysql_fetch_row(res) )
				{
					std::string auth_id_str = row[0];
					std::string query_ra = "SELECT REF_ID FROM AUTH_ORDER WHERE ";
					query_ra += "AUTH_ID = " + auth_id_str;

					wxLogMessage(" Refs By Author Query: \n%s \n", (wxString) query_ra );

					int ires1 = mysql_real_query( (MYSQL*)db_mysql, query_ra.c_str(), query_ra.length());
					if( !ires1 )
					{
						res1 = mysql_store_result((MYSQL*)db_mysql);
						int nrow1 = mysql_num_rows(res1);
						while( row1 = mysql_fetch_row(res1) )
						{
							where_clause += ",";
							where_clause += row1[0];
						}
						mysql_free_result(res1);
					}
				}
				where_clause += ") ";
			}
			mysql_free_result(res);
		}
	}

	if(key_words.size() > 0)
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		
		int nkw = key_words.size();
		int i,j;
		int first_kw = TRUE;
		for( i = 0; i < nkw; i++)
		{
			int keyw_id = GetKeyWordID( key_words[i].c_str(), FALSE);
			std::string kstr; 
			kstr = boost::str(boost::format("%d") % keyw_id);
			if( keyw_id > 0)
			{
				if( !first_kw)
				{
					where_clause += " AND ";
				}
				
				std::vector<int> ref_id_vec;
				int nr = GetRefsByKW(ref_id_vec, key_words[i].c_str());
				wxLogMessage("kw: %s assigned to %d references \n",key_words[i].c_str(), nr); 

				where_clause += "REF_ID IN ( ";
				for( j=0; j < nr; j++)
				{
					std::string str_id;
					if( j != 0) where_clause += ",";
					str_id = boost::str(boost::format("%d") % ref_id_vec[j]);
					where_clause += str_id;
				}
				where_clause += " ) ";
				first_kw = FALSE;
			}
		}
	}

	if(!where_clause.empty())
	{        
		query += " WHERE ";
		query += where_clause.c_str();
	}
	else
	{
		return ref_id_vec;
	}

	query += " ORDER BY PUB_YEAR DESC,JOURNAL_ID";
	query += limit_str;

	wxLogMessage(" Query: \n");
	wxLogMessage("%s \n", (wxString) query);

	if(db_mysql)
	{
	   if( !mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length()) )
	   {
          res = mysql_store_result((MYSQL*)db_mysql);
		  int nrow = mysql_num_rows(res);

		  wxLogMessage(" The number of found rows %d \n",nrow);
 	      ref_id_vec.resize(nrow);
		  for(int i=0; i < nrow; i++)
			ref_id_vec[i] = 0;
          
		  int ic = 0;
		  if(res) 
		  {
		    while( row = mysql_fetch_row(res) )
			{
//			    wxLogMessage("Matching reference ref_id = %s \n",row[0]);
				ref_id_vec[ic] = atoi( row[0]);
				ic++;
			}
		  }
          mysql_free_result(res);
	   }
	}
#endif

	return ref_id_vec;
}

std::vector<int> BiblDB::SearchAuths(const AuthorRequest& areq)
{
	std::vector<int> auth_id_vec;

#if defined(MYSQL_DB)

    MYSQL_RES *res;
	MYSQL_ROW row;

	std::string where_clause;
	int first_condition = TRUE;

	if(!areq.search_author_name_flt.empty())
	{
		std::string search_author_name_flt_copy = areq.search_author_name_flt;
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}

		boost::replace_all(search_author_name_flt_copy,"*","%");
		boost::trim(search_author_name_flt_copy);
		boost::to_upper(search_author_name_flt_copy);
		
		std::string last_name = "";
		std::string initials  = "";

		std::vector<std::string> name_part_arr;
		
		boost::split(name_part_arr,search_author_name_flt_copy,boost::is_any_of(", "));

		int i = 0;
		for(std::string name_part : name_part_arr)
		{
			if(name_part.empty()) continue;
			if( i == 0) last_name = name_part;
			if( i == 1) initials  = name_part;
			i++;
		}

		where_clause += " ( LAST_NAME LIKE \'" + last_name + "\'";
		if(!initials.empty())
		{
			where_clause += " AND INITIALS LIKE ";
			where_clause += "\'";
			where_clause += initials;
			where_clause += "\'";
		}
		where_clause += " )";
	}
		
	if(!areq.auth_importance_flt.empty())
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		where_clause += " ( AUTHORS.IMPORTANCE >=" + areq.auth_importance_flt + ") ";
	}

	std::vector<std::string> key_words;
	if(!areq.keywords_auth_flt.empty())
	{	
		boost::split(key_words,areq.keywords_auth_flt,boost::is_any_of(";"));
		for(std::string keyw : key_words)
		{
			boost::trim(keyw);
			boost::to_lower(keyw);
		}
	}

	if(key_words.size() > 0)
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		
		int nkw = key_words.size();
		int i,j;
		int first_kw = TRUE;
		for( i = 0; i < nkw; i++)
		{
			int keyw_id = GetKeyWordID( key_words[i].c_str(), FALSE);
			std::string kstr = boost::str(boost::format("%d") % keyw_id);
			if( keyw_id > 0)
			{
				if( !first_kw)
				{
					where_clause += " AND ";
				}
				
				std::vector<int> ref_id_vec;
				int nr = GetAuthsByKW(ref_id_vec, key_words[i].c_str());
				wxLogMessage("kw: %s assigned to %d authors \n",key_words[i].c_str(), nr); 

				where_clause += "AUTH_ID IN ( ";
				for( j=0; j < nr; j++)
				{
					std::string str_id;
					if( j != 0) where_clause += ",";
					str_id = boost::str(boost::format("%d") % ref_id_vec[j]);
					where_clause += str_id;
				}
				where_clause += " ) ";
				first_kw = FALSE;
			}
		}
	}

	std::string query = "SELECT AUTHORS.AUTH_ID, LAST_NAME, FIRST_NAME, INITIALS,";
	query += " MIDDLE_NAME, SUFFIX, ADDRESS, IMPORTANCE, URL, NOTE FROM AUTHORS";
	
	if(!where_clause.empty())
	{        
		query += " WHERE ";
		query += where_clause.c_str();
	}
	else
	{
		return auth_id_vec;
	}

	wxLogMessage(" Search Authors Query: \n");
	wxLogMessage("%s \n", query.c_str());

	if(db_mysql)
	{
	   if( !mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length()) )
	   {
          res = mysql_store_result((MYSQL*)db_mysql);
		  int nrow = mysql_num_rows(res);

		  wxLogMessage(" The number of found rows %d \n",nrow);
 	      auth_id_vec.resize(nrow);
		  for(int i=0; i< nrow; i++)
			auth_id_vec[i] = 0;
          
		  int ic = 0;
		  if(res) 
		  {
		    while( row = mysql_fetch_row(res) )
			{
				auth_id_vec[ic] = atoi( row[0] );
				ic++;
			}
		  }
          mysql_free_result(res);
	   }
	}

#endif

	return auth_id_vec;
}


int BiblDB::DeleteRefs(const std::vector<int>& del_refs_id)
{
	int nd = del_refs_id.size();
	if( nd == 0) return FALSE;

	int i,j, ires;
	
	std::string where_clause = "WHERE REF_ID IN(";
	std::string id_list_str;
	for(i = 0; i < nd; i++)
	{
		std::string str;
		str = boost::str( boost::format("%d") % del_refs_id[i]);
		if(i > 0) str = "," + str;
		where_clause += str;
		id_list_str  += str;

	}
	where_clause += ")";
	
	std::string query;

	query = "DELETE FROM REFS ";
	query += where_clause;

	wxLogMessage( (wxString) query);
	wxLogMessage("\n");

	ires = SQLQuery( query );

	query = "DELETE FROM AUTH_ORDER ";
	query += where_clause;

	wxLogMessage( (wxString) query);
	wxLogMessage("\n");

	ires = SQLQuery( query );

	query = "DELETE FROM ABSTRACTS ";
	query += where_clause;

	wxLogMessage((wxString)query);
	wxLogMessage("\n");

	ires = SQLQuery( query );

	query = "DELETE FROM OBJ_ASSOC ";
	query += "WHERE ID1 IN(" + id_list_str + ") OR ID2 IN(" + id_list_str + ")";

	wxLogMessage((wxString)query);
	wxLogMessage("\n");

	ires = SQLQuery(query);

	query = "DELETE FROM CITATIONS WHERE CITING IN(";
	for(i = 0; i < nd; i++)
	{
		if(i > 0) query += ",";
		query += boost::str( boost::format("%d") % del_refs_id[i] ) ;
	}
	query += ")";
	ires = SQLQuery(query);
	wxLogMessage( (wxString) query);

	query = "DELETE FROM CITATIONS WHERE CITED IN(";
	for(i = 0; i < nd; i++)
	{
		if(i > 0) query += ",";
		query += boost::str( boost::format("%d") % del_refs_id[i] ) ;
	}
	query += ")";
	ires = SQLQuery(query);
	wxLogMessage( (wxString) query);

	query = "DELETE FROM OBJECTS WHERE ID IN(";
	for(i = 0; i < nd; i++)
	{
		if(i > 0) query += ",";
		query += boost::str( boost::format("%d") % del_refs_id[i]) ;
	}
	query += ")";
	ires = SQLQuery(query);
	wxLogMessage( (wxString) query);

//	std::vector<int> del_refs_id_copy(del_refs_id);
//	wxString del_ids_str = "Deleted IDs string";

//	wxCommandEvent del_refs_event( wxEVT_DELETE_REFS, wxID_ANY);
//	del_refs_event.SetString( del_ids_str );

//	wxApp* wx_app = (wxApp*) pApp;
//	wx_app->ProcessEvent( del_refs_event );

	return TRUE;
}

std::string BiblDB::GetKeyWordByID(int keyw_id)
{
	std::string keyw;

#if defined(MYSQL_DB)

    MYSQL_RES *res;
	MYSQL_ROW row;

	int ires;

	std::string query;
	query = boost::str( boost::format("SELECT KEYWORD FROM KEYWORDS_LIST WHERE KEYWORD_ID = %d ") % keyw_id);

	if(db_mysql)
	{
	   ires = mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length());
	   if(ires) return ""; 
	   
       res = mysql_store_result((MYSQL*)db_mysql);
	   int nrow = mysql_num_rows(res);
	   
	   if( nrow > 0 )
	   {
		   row = mysql_fetch_row(res);
		   keyw = row[0]; 
	   }
	   mysql_free_result(res);	   
	}          
#endif
    return keyw;
}

int BiblDB::AxxFun()
{
#if defined(MYSQL_DB)

	MYSQL_RES *res;
	MYSQL_ROW row;
	wxString query;
	
    query = "SELECT * ";
	
	if(db_mysql)
	{
		if( !mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length()) )
		{
			res = mysql_store_result((MYSQL*)db_mysql);
			if(res) 
			{
				while( row = mysql_fetch_row(res) )
				{
					wxString kw = row[0];
					wxString level_str = row[1];
					int kw_id = GetKeyWordID(kw.c_str());
					if( kw_id > 0)
					{
						wxString query1 = "UPDATE  ";
						wxLogMessage("%s \n",query1.c_str());
						mysql_real_query((MYSQL*)db_mysql,query1.c_str(),query1.length());
					}
				}
				mysql_free_result(res);
			}
		}
	}
#endif

	return TRUE;
}

int BiblDB::AssocObjIDs(const std::vector<int>& kw_ids_left, const std::vector<int>& kw_ids_right)
{
	int i,j;
	int ires;
#if defined(MYSQL_DB)

    MYSQL_RES *res;

//	wxLogMessage(" BiblDB::AssocKeyWordsIDs() Number of selected KeyWords % d\n", nkw);

	for(i = 0; i < kw_ids_left.size(); i++)
	{
		for(j = 0; j < kw_ids_right.size(); j++)
		{
			std::string kw_left  = this->GetKeyWordByID(kw_ids_left[i]);
			std::string kw_right = this->GetKeyWordByID(kw_ids_right[j]);
			wxLogMessage("Associate:   %s  -->  %s ", kw_left, kw_right);

			std::string query;
			query = boost::str(boost::format("SELECT ID1 FROM OBJ_ASSOC WHERE ID1 = %d AND ID2 = %d ") 
				                              % kw_ids_left[i] % kw_ids_right[j]);
	        if(db_mysql)
			{
	           ires = mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length());
	           if(ires) continue;
	           res = mysql_store_result((MYSQL*)db_mysql);
	           int nrow = mysql_num_rows(res);
			   if(nrow > 0) 
			   {
				   mysql_free_result(res);
				   continue;
			   }
			   mysql_free_result(res);

//			   wxLogMessage("Q: %s \n", query.c_str());
			   query = boost::str( boost::format("INSERT INTO OBJ_ASSOC (ID1,ID2) VALUES (%d,%d)")
				                   % kw_ids_left[i] % kw_ids_right[j]);
			   ires = mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length());
			}
		}
	}
#endif
	return TRUE;
}

int BiblDB::DelAssocObjIDs(const std::vector<int>& kw_ids_left, const std::vector<int>& kw_ids_right )
{
	int i,j;

	for(i = 0; i < kw_ids_left.size(); i++)
	{
		for(j = 0; j < kw_ids_right.size(); j++)
		{
			std::string kw_left = this->GetKeyWordByID(kw_ids_left[i]);
			std::string kw_right = this->GetKeyWordByID(kw_ids_right[j]);
			wxLogMessage("Remove Association:   %s  -->  %s ", kw_left, kw_right);

			std::string query;
			query = boost::str(boost::format("DELETE FROM OBJ_ASSOC WHERE ID1 = %d AND ID2 = %d ") 
				               % kw_ids_left[i] % kw_ids_right[j]);
			SQLQuery(query.c_str());
		}
	}
	return TRUE;
}

int  BiblDB::SetCitation(int ref_id_citing, int ref_id_cited )
{
	if( ref_id_citing < 1 || ref_id_cited < 1) return FALSE;

	RemoveCitation(ref_id_citing, ref_id_cited);
	std::string query;
	query = boost::str(boost::format("REPLACE INTO CITATIONS (CITING,CITED) VALUES (%d,%d)") % ref_id_citing % ref_id_cited);
	int ires = SQLQuery(query.c_str());
	return ires;
}

int BiblDB::RemoveCitation(int ref_id_citing, int ref_id_cited )
{
	std::string query;

	query = boost::str(boost::format("delete from CITATIONS where citing = %d and cited = %d") % ref_id_citing % ref_id_cited);
	int ires = SQLQuery(query.c_str());
	return ires;
}
	

int BiblDB::GetKeyWordID(const char* keyw, int create_keyw)
{
	int keyw_id = 0;
#if defined(MYSQL_DB)

	if(!db_mysql) return 0;

	std::string key_word = keyw;
	boost::trim(key_word);
	boost::to_lower(key_word);

    MYSQL_RES *res;
	MYSQL_ROW row;

	int ires;

	std::string query = boost::str( boost::format("SELECT KEYWORD_ID FROM KEYWORDS_LIST WHERE KEYWORD = \'%s\' ") % key_word );

	ires = mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.size());
	if(ires) return 0; 
	   

	int nrow = 0;
    res = mysql_store_result((MYSQL*)db_mysql);
	if(res)
	{
		nrow = mysql_num_rows(res);
		   
		if( nrow > 0 )
		{
			row = mysql_fetch_row(res);
			keyw_id = atoi( row[0]); 
		}
		   
		mysql_free_result(res);
		   
		if( nrow == 0 && create_keyw )
		{
			  keyw_id = CreateNewObj( KEYWORD_OBJ );
			  if( keyw_id == 0 ) return 0;

			  query = boost::str( boost::format( "INSERT INTO KEYWORDS_LIST (KEYWORD_ID,KEYWORD) VALUES (%d,") % (keyw_id) );
			  query += "\'" + key_word + "\')";
//					query = boost::str( boost::format( "INSERT INTO KEYWORDS_LIST (KEYWORD_ID,KEYWORD) VALUES (%d,\'%s\')") % (keyw_id, key_word) );
			  ires = mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.size());
			  if(ires) return 0;
		}
	}
#endif
	   
    return keyw_id;
}

BiblRef BiblDB::GetRefByID(int ref_id_a, int option )
{
	BiblRef ref;
 
	std::vector<std::string> str_arr; 

#if defined(MYSQL_DB) 

    MYSQL_RES *res;
	MYSQL_ROW row;
	std::string query;

	std::string where_str;
	where_str = str(boost::format("REF_ID = %d") % ref_id_a);

    query = "SELECT ";
//    query += " * ";
	query += " REF_ID, EXT_REF_ID, REF_TYPE, TITLE, BOOK_TITLE,";
	query += " JOURNAL_ID, VOLUME, ISSUE, ";
	query += " PUB_YEAR, PUB_MONTH, FIRST_PAGE, LAST_PAGE, ";
	query += " ISI_ID, ISI_ID_INT, GA_CODE, PUBMED_ID, ";
	query += " REPRINT_STATUS, IMPORTANCE, DOI, URL, PII_ID, MEDLINE_ID, AUTHORS_STR, INCOMPLETE_FLAG, ";
	query += " NUM_CITED_IN, NUM_CITING, LAST_UPDATE_TIME ";
	query += "FROM REFS WHERE ";
	query += where_str;

	if(db_mysql)
	{
	   if( !mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length()) )
	   {
//		  long time = ::wxGetElapsedTime();
//		  wxLogMessage("\n In GetRefByID: Time to pt1  %d ms \n", time);

          res = mysql_store_result((MYSQL*)db_mysql);
		  int nrow = mysql_num_rows(res);

		  row = mysql_fetch_row(res);
		  if(row)
		  {
	                     ref.obj_id     = ref_id_a;
	         if(row[1])  ref.ext_ref_id = row[1];
	         if(row[2])  ref.ref_type   = atoi(row[2]);	                     
	         if(row[3])  ref.title      = row[3];  
             if(row[4])  ref.book_title = row[4];
	         if(row[5])  ref.jrn        = GetJournalByID(atoi(row[5]));
	         if(row[6])  ref.vol        = row[6];
	         if(row[7])  ref.iss        = row[7];
	         if(row[8])  ref.pub_year   = row[8];
	         if(row[9])  ref.pub_month  = row[9];
	         if(row[10]) ref.first_page = row[10];
	         if(row[11]) ref.last_page  = row[11];
             if(row[12]) ref.isi_id     = row[12];
	         if(row[13]) ref.isi_id_int = atoi(row[13]);
	         if(row[14]) ref.ga_code    = row[14];
	         if(row[15]) ref.pubmed_id = row[15];
             if(row[16]) ref.reprint_status = row[16];
	         if(row[17]) ref.importance     = atoi(row[17]);
	         if(row[18]) ref.doi            = row[18];
			 if(row[19]) ref.url            = row[19];
			 if(row[20]) ref.pii_id         = row[20];
			 if(row[21]) ref.medline_id     = row[21];
			 if(row[22]) ref.authors_str         = row[22];
			 if(row[23]) ref.incomplete_auth_flag     = atoi(row[23]);
			 if(row[24]) ref.num_cited_in             = atoi(row[24]);
			 if(row[25]) ref.num_citing               = atoi(row[25]);
			 if(row[26]) ref.last_update_time         = atol(row[26]);

//			 time = ::wxGetElapsedTime();
//		     wxLogMessage("\n In GetRefByID: Time to pt2  %d ms \n", time);

			 if( ref.authors_str.empty() || option & GET_REF_AUTH_VEC )
			 {
				 ref.auth_vec   = GetAuthForRef(ref_id_a);
			 }
		  }
          mysql_free_result(res);

//	      time = ::wxGetElapsedTime();
//		  wxLogMessage("\n In GetRefByID: Time to pt3  %d ms \n", time);


		  if( !ref.title.empty() )
		  {
			  boost::replace_all(ref.title,"&quot;","\"");
			  boost::replace_all(ref.title,"&apos;","\'");
		  }
		  if( !ref.book_title.empty() )
		  {
			  boost::replace_all(ref.book_title,"&quot;","\"");
			  boost::replace_all(ref.book_title,"&apos;","\'");
		  }

	   }
	   if( option & GET_REF_ABSTRACT || option & GET_REF_NOTES )
	   {
		  int first_field = TRUE;

		  query = "SELECT ";
		  if( option & GET_REF_ABSTRACT )
		  {
			  query += "ABSTRACT";
			  first_field = FALSE;
		  }

		  if( option & GET_REF_NOTES )
		  {
			  if(!first_field) query += ",";
			  query += "NOTE";
			
			  first_field = FALSE;
		  }
			
          query += " FROM ABSTRACTS WHERE " ;
		  query += str( boost::format("REF_ID = %d") % ref_id_a );

		  SQLQuery(query.c_str(), &str_arr);

		  if( option & GET_REF_ABSTRACT && str_arr.size() >= 1 )
		  {
			  for (int idx = 0; idx < str_arr.size(); idx = idx + 2)
			  {
				  if (str_arr[idx].size() > ref.abstract_str.size()) ref.abstract_str = str_arr[idx];
			  }
			  boost::replace_all(ref.abstract_str,"&quot;","\"");
			  boost::replace_all(ref.abstract_str,"&apos;","\'");
		  }

		  if( option & GET_REF_NOTES && str_arr.size() >= 2)
		  {
			 for (int idx = 1; idx < str_arr.size(); idx = idx + 2)
			 {
				  ref.notes += str_arr[idx];
			 }
			 boost::replace_all(ref.notes,"&quot;","\"");
		 	 boost::replace_all(ref.notes,"&apos;","\'");
		  }
	   }
	   if( option & GET_REF_KEY_WORDS )
	   {
		   ref.keywords_str = GetObjKW(ref_id_a);
	   }
	}

#endif

	return ref;
}

std::vector<int> BiblDB::GetRefsCitedIn(int ref_id_citing)
{
	std::vector<int> ref_id_vec;

	std::vector<std::string> str_arr;
	std::string query;
	query = str(boost::format("SELECT CITED FROM CITATIONS WHERE CITING = %d") % ref_id_citing);
	int ires = SQLQuery(query.c_str(),&str_arr);

	int nref = str_arr.size();
	
	ref_id_vec.resize(nref);
	
	int ic;
	try 
	{
		for(ic = 0; ic < nref; ic++)
		{
			ref_id_vec[ic] =  boost::lexical_cast<int>(str_arr[ic]);
		}
	}
	catch(boost::bad_lexical_cast&) {}

	return ref_id_vec;
}

std::vector<int> BiblDB::GetRefsCiting(int ref_id_cited)
{
	std::vector<int> ref_id_vec;
	std::vector<std::string> str_arr;
	std::string query;
	query = str(boost::format("SELECT CITING FROM CITATIONS WHERE CITED = %d") % ref_id_cited);
	int ires = SQLQuery(query.c_str(),&str_arr);

	int nref = str_arr.size();
	
	ref_id_vec.resize(nref);
	
	int ic;
	try 
	{
		for(ic = 0; ic < nref; ic++)
		{
			ref_id_vec[ic] = boost::lexical_cast<int>(str_arr[ic]);
		}
	}
	catch(boost::bad_lexical_cast&) {}

	return ref_id_vec;
}

int BiblDB::GetObjsByKW(std::vector<int>& obj_id_vec, int obj_type, const std::string& kw )
{
	int nrow = 0;
	obj_id_vec.clear();
	
	int kw_id = GetKeyWordID( kw.c_str() );
	if( kw_id == 0) return FALSE;

	std::string kw_id_str;
	kw_id_str = boost::str(boost::format("%d") % kw_id);

	std::string query = "SELECT ID1 FROM OBJ_ASSOC INNER JOIN OBJECTS ";
	query += boost::str(boost::format(" ON OBJ_ASSOC.ID1 = OBJECTS.ID AND OBJ_ASSOC.TYPE = %d ") % KEYWORD_TAG ); 
	query += boost::str(boost::format(" WHERE ID2 = %d AND OBJECTS.TYPE = %d ") % kw_id_str % obj_type ) ;

	wxLogMessage("GetObjsByKW Q: %s\n", query.c_str());

	std::vector<std::string> sres;
	nrow = SQLQuery( query, &sres );

	if(!nrow) return FALSE;

	for( int i = 0; i < nrow; i++ )
	{
		obj_id_vec.push_back( atoi(sres[i].c_str()) );
	}

	return nrow;
}

int BiblDB::GetRefsByKW(std::vector<int>& ref_id_vec, const char* kw)
{
	return GetObjsByKW( ref_id_vec, REF_OBJ, kw );
}

int BiblDB::GetAuthsByKW(std::vector<int>& auth_id_vec, const char* kw)
{
	return GetObjsByKW( auth_id_vec, AUTHOR_OBJ, kw );
}

int BiblDB::GetRefsByID(std::vector<int>& ref_id_vec, std::vector<BiblRef>& refs_vec )
{
#if defined(MYSQL_DB) 

    MYSQL_RES *res;
	MYSQL_ROW row;
	std::string query;

	std::string where_str;
	std::string str;

	int i,n;

	n = ref_id_vec.size();
	refs_vec.clear();
	refs_vec.resize(n);

	for(i = 0; i < n; i++)
	{
		refs_vec[i].obj_id = 0;
	}
	if(!db_mysql || n == 0) 
	{
		return FALSE;
	}

	where_str = "REF_ID IN (";
	for(i = 0; i < n; i++)
	{
		if( i != (n-1) )
		{
			str = boost::str(boost::format("%d,") % ref_id_vec[i]);
		}
		else
		{
			str = boost::str(boost::format("%d)") % ref_id_vec[i]);
		}
		where_str += str;
	}

    query = "SELECT ";
	query += " REF_ID, EXT_REF_ID, REF_TYPE, TITLE, BOOK_TITLE,";
	query += " JOURNAL_ID, VOLUME, ISSUE, ";
	query += " PUB_YEAR, PUB_MONTH, FIRST_PAGE, LAST_PAGE, ";
	query += " ISI_ID, ISI_ID_INT, GA_CODE, PUBMED_ID, ";
	query += " REPRINT_STATUS, IMPORTANCE, DOI, URL, PII_ID, MEDLINE_ID, ";
	query += " INCOMPLETE_FLAG, NUM_CITED_IN, NUM_CITING, LAST_UPDATE_TIME ";
	query += "FROM REFS WHERE ";
	query += where_str;
	query += " ORDER BY PUB_YEAR DESC,JOURNAL_ID";

	std::map<int, int> ref_id_map;

	if( !mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length()) )
	{
		res = mysql_store_result((MYSQL*)db_mysql);
		int nrow = mysql_num_rows(res);
		
		i = 0;
		
		while(row = mysql_fetch_row(res))
		{
			BiblRef& ref = refs_vec[i];
			if(row[0])  ref.obj_id     = atoi(row[0]);
			if(row[1])  ref.ext_ref_id = row[1];
			if(row[2])  ref.ref_type   = atoi(row[2]);
			//	                     ref.auth_vec   = GetAuthForRef(ref_id_a);
			if(row[3])  ref.title      = row[3];
			if(row[4])  ref.title      = row[4];

			if(row[5])  ref.jrn        = GetJournalByID(atoi(row[5]));
			if(row[6])  ref.vol        = row[6];
			if(row[7])  ref.iss        = row[7];
			if(row[8])  ref.pub_year   = row[8];
			if(row[9])  ref.pub_month  = row[9];
			if(row[10])  ref.first_page = row[10];
			if(row[11]) ref.last_page  = row[11];
			if(row[12]) ref.isi_id     = row[12];
			if(row[13]) ref.isi_id_int = atoi(row[13]);
			if(row[14]) ref.ga_code    = row[14];
			if(row[15]) ref.pubmed_id = row[15];
			if(row[16]) ref.reprint_status = row[16];
			if(row[17]) ref.importance     = atoi(row[17]);
			if(row[18]) ref.doi            = row[18];
			if(row[19]) ref.url            = row[19];
			if(row[20]) ref.pii_id         = row[20];
			if(row[21]) ref.medline_id     = row[21];
			if(row[22]) ref.authors_str         = row[22];
			if(row[23]) ref.incomplete_auth_flag     = atoi(row[23]);
			if(row[24]) ref.num_cited_in             = atoi(row[24]);
			if(row[25]) ref.num_citing               = atoi(row[25]);
			if(row[26]) ref.last_update_time         = atol(row[26]);
			
			ref_id_map[ref.obj_id] = i;
			i++;
		}
		mysql_free_result(res);
	}

    query =  " SELECT REF_ID, AUTHORS.AUTH_ID, LAST_NAME, INITIALS ";
	query += " FROM AUTHORS, AUTH_ORDER ";
	query += " WHERE AUTH_ORDER.AUTH_ID = AUTHORS.AUTH_ID AND ";

	query += where_str.c_str();
	
	query += " ORDER BY REF_ID,AUTH_POS ";
	
	int ref_id_a;

	if( !mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length()) )
	{
		res = mysql_store_result((MYSQL*)db_mysql);
		int nrow = mysql_num_rows(res);
		
		if(res) 
		{
			while( row = mysql_fetch_row(res) )
			{
				int auth_id = atoi(row[0]);
				AuthorRef auth_ref;
				if(row[0]) ref_id_a = atoi(row[0]);
                if(row[1]) auth_ref.obj_id     = atoi(row[1]);
				if(row[2]) auth_ref.last_name   = row[2];
				if(row[3]) auth_ref.initials    = row[3];
				if(ref_id_a > 0)
				{
				   int idx = ref_id_map[ref_id_a];
                   refs_vec[idx].auth_vec.push_back(auth_ref);
				}
//				wxLogMessage("%d %s \n", ref_id_a,  auth_ref.last_name.c_str());
			}
		}
		mysql_free_result(res);
	}
#endif

	return TRUE;
}


void BiblDB::PrintRef1(const BiblRef& bref)
{
	int i,n;
	wxLogMessage(" Reference:\n");
	wxLogMessage(" id = %d ", bref.obj_id);
	wxLogMessage(" ext_id = %s ", bref.ext_ref_id.c_str());
	wxLogMessage(" ref_type = %d \n", bref.ref_type);
	
    n = bref.auth_vec.size();
	for(i = 0; i < n; i++)
	{
		const AuthorRef& aref = bref.auth_vec[i];
		std::string lname = boost::to_lower_copy(aref.last_name);
		lname[0] = toupper(lname[0]);
		std::string ini   = boost::to_upper_copy(aref.initials);

		wxLogMessage("%s,%s; ",lname.c_str(),ini.c_str());
	}
	wxLogMessage("\n");

    wxLogMessage(" title = %s\n", bref.title.c_str());
	wxLogMessage(" book_title = %s\n", bref.book_title.c_str());
	wxLogMessage(" journal = %s\n", bref.jrn.std_abbr.c_str());
	wxLogMessage(" pub_year = %s\n",bref.pub_year.c_str());
	wxLogMessage(" volume = %s\n",bref.vol.c_str());
	wxLogMessage(" issue = %s\n",bref.iss.c_str());
	wxLogMessage(" first_page = %s\n",bref.first_page.c_str());
    wxLogMessage("\n");	
}



void BiblDB::PrintRefFullStr(const BiblRef& bref, std::string& str)
{
	int i,n;
	
	std::string tmp;
	wxDateTime update_time;

	str.clear();

	str+= "ref_id= ";
	str+= boost::str(boost::format("%d") % bref.obj_id);  
	str+= "  ext_ref_id= ";
	str+= bref.ext_ref_id.c_str();
	str+= "\n";

	if( !bref.authors_str.empty() )
	{
		str +=  bref.authors_str;
	}
	else
	{
		str += bref.GetAuthorStr();
	}
	
	str += "\n";
    str +=  bref.title.c_str();
	str += "\n";
	
	if( !bref.book_title.empty() )
	{
        str += "Book: ";
		str +=  bref.book_title.c_str();
		str += "\n";
	}

	if( !bref.jrn.std_abbr.empty())
	{
		str += bref.jrn.std_abbr.c_str();
	}
	else if( !bref.jrn.short_abbr.empty())
	{
		str += bref.jrn.short_abbr.c_str();
	}
	else
	{
		str += bref.jrn.full_name.c_str();
	}

	str += "(";
	str += bref.pub_year.c_str();
	str += ") ";
	str += "v.";
	str += bref.vol.c_str();
	str += "(";
	str += bref.iss.c_str();
	str += ") ";
	str += boost::str(boost::format("pp.%s-%s") % bref.first_page.c_str() % bref.last_page.c_str());

	str += "\n";
	str +=  boost::str(boost::format("isi_id = %s ") % bref.isi_id.c_str());  
	str +=  boost::str(boost::format("isi_id_int = %d ") % bref.isi_id_int); 
	str +=  boost::str(boost::format("ga_code = %s \n") % bref.ga_code.c_str()); 
	str +=  boost::str(boost::format("pubmed id = %s ") % bref.pubmed_id.c_str());  
	str +=  boost::str(boost::format("medline_id = %s \n") % bref.medline_id.c_str() );
	str +=  boost::str(boost::format("doi = %s \n") % bref.doi.c_str() ); 
	str +=  boost::str(boost::format("url = %s \n") % bref.url.c_str() ); 
	str +=  boost::str(boost::format("pii_id = %s \n") % bref.pii_id.c_str() ); 
	str +=  boost::str(boost::format("reprint_status = %s ") % bref.reprint_status.c_str()); 
	str +=  boost::str(boost::format("importance = %d \n") % bref.importance );
	str +=  boost::str(boost::format("incomplete_auth_flag = %d \n\n") % bref.incomplete_auth_flag ); 
	str +=  boost::str(boost::format("num_cited_in = %d   num_citing = %d \n") % bref.num_cited_in % bref.num_citing ); 
	update_time.Set(bref.last_update_time);
	str +=  boost::str(boost::format("last_update_time = %s \n") % update_time.FormatDate() ); 

	str += "KeyWords: \n";

	std::vector<std::string> keywords_arr;
	
	boost::split(keywords_arr, bref.keywords_str, boost::is_any_of(";"));
	std::string line;
	for( std::string keyw : keywords_arr )
	{
		line += keyw;
		line += " ; ";
		if( line.size() > 60) 
		{ 
			str += line; 
			str += "\n"; 
			line.clear();
		}
	}
	str += line; 

	str += "\n\n";
	str += "Abstract: \n";
	str += bref.abstract_str;
	str += "\n\n";

	if( !bref.notes.empty())
	{
		str += "Notes: \n";
		str += bref.notes;
		str += "\n";
	}
}

int BiblDB::SetObjKWID(int refid, int kw_id )
{
	std::string query;
	query = str( boost::format("INSERT INTO OBJ_ASSOC (ID1,ID2,TYPE) VALUES (%d, %d, %d) ") % refid % kw_id % KEYWORD_TAG );
    SQLQuery(query.c_str());

	return TRUE;
}

int BiblDB::UpdateAuthorStrForRef( int ref_id)
{ 
	std::vector<AuthorRef> auth_vec = GetAuthForRef(ref_id);
	std::string auth_str = AuthorRef::AuthVecToString(auth_vec);
	std::string query;
	query = str(boost::format("UPDATE REFS SET AUTHORS_STR = \"%s\" WHERE REF_ID = %d ") % auth_str.c_str() % ref_id);
	SQLQuery(query.c_str());

	return TRUE;
}

int BiblDB::SetObjKW(int obj_id, const std::string& key_words )
{	
	int ires;
	std::string query;

	std::vector<std::string> keyw_arr;

	boost::split(keyw_arr, key_words, boost::is_any_of(";"));

	for( std::string keyw : keyw_arr)
	{
		boost::trim(keyw);
		boost::to_lower(keyw);
		if(keyw.empty()) continue;
		int kw_id = GetKeyWordID( keyw.c_str(), TRUE );

		query = boost::str(boost::format("select count(*) from OBJ_ASSOC where id1 = %d and id2 = %d and type = %d ") % obj_id % kw_id % KEYWORD_TAG );
		int nfound = SQLIntFunc(query.c_str());
        
		if( nfound == 1 ) continue;
		if( nfound > 1) 
		{
			query = boost::str(boost::format("delete from OBJ_ASSOC where id1 =  %d and id2 = %d and type = %d ") % obj_id % kw_id % KEYWORD_TAG );
			SQLQuery(query.c_str());
		}

		query = "INSERT INTO OBJ_ASSOC (ID1,ID2,TYPE) ";
		query += boost::str(boost::format("VALUES (%d, %d, %d)") % obj_id % kw_id % KEYWORD_TAG );

		wxLogMessage("SetKWRef() Q: %s \n", (wxString) query );
		ires = SQLQuery(query);
	}

	return FALSE;
}

std::string BiblDB::GetObjKW(int obj_id)
{
	if(obj_id < 1) return "";
	
	int ires;
	std::string query;
	std::vector<std::string> res_arr;
	
	query = "SELECT KEYWORD FROM KEYWORDS_LIST INNER JOIN OBJ_ASSOC ";
	query += " ON KEYWORDS_LIST.KEYWORD_ID = OBJ_ASSOC.ID2 ";
	query += boost::str(boost::format(" WHERE ID1 = %d AND OBJ_ASSOC.TYPE = %d") % obj_id % KEYWORD_TAG );
	query += " ORDER BY KEYWORD";
	ires = SQLQuery(query.c_str(),&res_arr);
	
	std::string keyw_str;

	int i;
	int n = res_arr.size();
	for( i = 0; i < n; i++)
	{
		std::string keyw  = res_arr[i];
		boost::trim(keyw);
		boost::to_lower(keyw);
		keyw_str += keyw.c_str();
		keyw_str += ";";
	}
	if(boost::ends_with(keyw_str, ";")) boost::erase_tail(keyw_str,1);   
	return (keyw_str);
}

std::vector<int> BiblDB::GetObjKWID(int obj_id)
{
	std::string query;
	std::vector<std::string> keyw_ids_str;
	std::vector<int> keyw_ids;

	query = "SELECT ID2 FROM OBJ_ASSOC ";
	query += boost::str(boost::format(" WHERE ID1 = %d AND TYPE = %d ") % obj_id % KEYWORD_TAG );

	SQLQuery(query.c_str(), &keyw_ids_str);

	int nkw = keyw_ids_str.size();
	keyw_ids.resize(nkw);

	int ik;
	for( ik = 0; ik < nkw; ik++)
	{
		keyw_ids[ik] = atoi(keyw_ids_str[ik].c_str());
	}
	return keyw_ids;
}

int BiblDB::DelObjKW_All(int obj_id)
{
	std::string query;
	query = str(boost::format("DELETE FROM OBJ_ASSOC WHERE ID1 = %d AND TYPE = %d ") % obj_id % KEYWORD_TAG );
	SQLQuery(query.c_str());

	return TRUE;
}

int BiblDB::SetAuthImportance(int auth_id , int importance )
{
	int ires;
	std::string query;
	query = "UPDATE AUTHORS ";
	query += boost::str(boost::format("SET IMPORTANCE = (%d) WHERE AUTH_ID = (%d)") % importance % auth_id);

	wxLogMessage("SetAuthImportance() Q: %s \n", query.c_str());

	SQLQuery(query.c_str());
	return FALSE;
}

int BiblDB::SetAuthURL(int auth_id, const char* url_str )
{
	int ires;
	std::string query;
	query = "UPDATE AUTHORS ";
	query += boost::str(boost::format("SET URL = \'%s\' WHERE AUTH_ID = (%d)") % url_str % auth_id);

	SQLQuery(query.c_str());
	return FALSE;
}

int BiblDB::SetAuthNote(int auth_id, const char* note_str )
{
	int ires;
	std::string query;
	query = "UPDATE AUTHORS ";
	query += boost::str(boost::format("SET NOTE = \'%s\' WHERE AUTH_ID = (%d)") % note_str % auth_id);

	SQLQuery(query.c_str());
	return FALSE;
}

int BiblDB::DelObjKW(int obj_id, const std::string& kw_str )
{
	int ires;
	std::string query;
	
	std::vector<std::string> kw_id_str_arr;
	
	query = "SELECT KEYWORD_ID FROM KEYWORDS_LIST INNER JOIN OBJ_ASSOC ";
	query += boost::str( boost::format( " ON KEYWORDS_LIST.KEYWORD_ID = OBJ_ASSOC.ID2 AND OBJ_ASSOC.TYPE = %d ") % KEYWORD_TAG );
	query += boost::str( boost::format(" WHERE OBJ_ASSOC.ID1 = %d AND (") % obj_id);

	std::vector<std::string> keyw_arr;
	boost::split(keyw_arr, kw_str, boost::is_any_of(";"));
	
	int first_kw = TRUE;
	for( std::string keyw : keyw_arr)
	{
		boost::trim(keyw);
		boost::to_lower(keyw);
		boost::replace_all(keyw,"*","%");
		
		if(keyw.empty()) continue;
		
		if( !first_kw) query += " OR ";
		query += boost::str(boost::format(" KEYWORD LIKE \'%s\'") % keyw);
		first_kw = FALSE;
	}
	if ( first_kw ) return FALSE;
	query += ")";

	ires = SQLQuery(query.c_str(),&kw_id_str_arr);
	
	if( kw_id_str_arr.size() == 0) return FALSE;

	int n = kw_id_str_arr.size();
	int i;

	query = "DELETE FROM OBJ_ASSOC ";
	query += boost::str(boost::format( "WHERE ID1 = %d AND TYPE = %d AND ") % obj_id % KEYWORD_TAG );
	query += " ID2 IN (";
	
	for( i=0; i < n; i++)
	{
		if( i > 0) query += ",";
		query += kw_id_str_arr[i];
	}
	query += ")";

	wxLogMessage("DelObjKW() Q: %s \n", query.c_str());
	ires = SQLQuery(query.c_str());
	   
	return (!ires);
}


int BiblDB::AddKWToCategory( const char* kw, std::string cat_str )
{
	std::string keyw = kw;
	boost::trim(keyw);
	boost::to_lower(keyw);

	if(keyw.empty()) return FALSE;

	int kw_id = GetKeyWordID( keyw.c_str(), TRUE );
	int cat_id = GetKeyWordID( cat_str.c_str(), TRUE );

	if( cat_id < 1 ) return FALSE;

	wxLogMessage(" BiblDB::AddKWToCategory() kw_id = %d \n", kw_id); 

	std::string query;
	std::string cat_id_str = str(boost::format("%d") % (cat_id));
	std::string kw_id_str = str(boost::format("%d") % (kw_id));

	query = "SELECT count(*) ";
	query += "FROM OBJ_ASSOC ";
	query += "WHERE id1 = " + cat_id_str;
	query += " AND ";
	query += " id2 = " + kw_id_str;

	int nfound = SQLIntFunc( query );

	if( nfound > 0) return TRUE;

	query = "INSERT INTO OBJ_ASSOC (id1, id2) "; 
	query += " VALUES ( ";
	query += cat_id_str + ",";
	query += kw_id_str;
	query += " )";

	SQLQuery(query);

	return TRUE;
}

int BiblDB::DelKWFromCategory( const char* kw, std::string cat_str )
{
	std::string keyw = kw;
	boost::trim(keyw);
	boost::to_lower(keyw);

	if(keyw.empty()) return FALSE;

	int kw_id = -1;
	try { kw_id = boost::lexical_cast<int>( keyw ); }  
	catch( boost::bad_lexical_cast&) { kw_id = GetKeyWordID( keyw.c_str(), FALSE ); }

	if( kw_id < 1 ) return FALSE;

	int cat_id = -1;
	try { cat_id = boost::lexical_cast<int>( cat_str ); }  
	catch( boost::bad_lexical_cast&) { cat_id = GetKeyWordID( cat_str.c_str(), FALSE ); }

	if( cat_id < 1 ) return FALSE;

	std::string query;
	std::string cat_id_str = str(boost::format("%d") % (cat_id));
	std::string kw_id_str = str(boost::format("%d") % (kw_id));

	query = "DELETE FROM OBJ_ASSOC ";
	query += "WHERE id1 = " + cat_id_str;
	query += " AND ";
	query += " id2 = " + kw_id_str;

	SQLQuery(query);

	return TRUE;
}

std::vector<int> BiblDB::GetAssocKWRight(int obj_id )
{
	std::vector<int> res_vec;

	std::string query;

	query = "SELECT id2 ";
	query += " FROM OBJ_ASSOC INNER JOIN OBJECTS ON OBJ_ASSOC.id2 = OBJECTS.id AND OBJECTS.TYPE =  " + boost::str( boost::format("%d") % KEYWORD_OBJ );
	query += " WHERE id1 = ";
	query += boost::str( boost::format("%d") % obj_id );

	std::vector<std::string> str_arr_tmp;

	SQLQuery(query.c_str(), &str_arr_tmp);
    
	int nrow = GetLastSQLResNumRows();
    res_vec.reserve(nrow);
	
	int i;

	for(i=0; i < nrow; i++)
	{
		int kw_id = atoi(str_arr_tmp[i].c_str());
		res_vec.push_back( kw_id);
	}
	return res_vec;
}

std::vector<std::shared_ptr<DataObject> > BiblDB::LoadDataObjects(const std::vector<int>& ids_vec)
{
	std::vector<std::shared_ptr<DataObject> > obj_vec;
	for (int id : ids_vec)
	{
		const int type = this->GetObjType(id);
		if (type == REF_OBJ)
		{
			auto sp = std::make_shared<BiblRef>();
			sp->SetId(id);
			*sp = GetRefByID(id);
			obj_vec.push_back(sp);
		}
		else if (type == AUTHOR_OBJ)
		{
			auto sp = std::make_shared<AuthorRef>();
			sp->SetId(id);
			*sp = GetAuthByID(id);
			obj_vec.push_back(sp);
		}
		else if (type == JOURNAL_OBJ)
		{
			auto sp = std::make_shared<JournalRef>();
			sp->SetId(id);
			*sp = GetJournalByID(id);
			obj_vec.push_back(sp);
		}
		else if (type == KEYWORD_OBJ)
		{
			auto sp = std::make_shared<KeyWord>();
			sp->SetId(id);
			sp->data_str = GetKeyWordByID(id);
			obj_vec.push_back(sp);
		}
	}
	return obj_vec;
}

std::vector<int> BiblDB::GetAssocObjIDs(const std::vector<int>& obj_ids_left1, const std::vector<int>& obj_ids_ndir, const std::vector<int>& obj_ids_left2, const std::vector<int>& obj_ids_right)
{
	std::vector<int> f_ids_vec;
	std::set<int> f_ids_set;
	std::string query;

	std::string ids_left_str_1;
	std::string ids_left_str_2;
	std::string ids_ndir_str;
	std::string ids_right_str;

	std::string obj_type_list = std::to_string(KEYWORD_OBJ) + "," + std::to_string(AUTHOR_OBJ);

	for (const int id : obj_ids_left1)
	{
		if (!ids_left_str_1.empty()) ids_left_str_1 += " , ";
		ids_left_str_1 += std::to_string(id);
	}
	for (const int id : obj_ids_left2)
	{
		if (!ids_left_str_2.empty()) ids_left_str_2 += " , ";
		ids_left_str_2 += std::to_string(id);
	}
	for (const int id : obj_ids_right)
	{
		if (!ids_right_str.empty()) ids_right_str += " , ";
		ids_right_str += std::to_string(id);
	}
	for (const int id : obj_ids_ndir)
	{
		if (!ids_ndir_str.empty()) ids_ndir_str += " , ";
		ids_ndir_str += std::to_string(id);
	}

	// Select Object IDs Associated on the left

	query = "SELECT distinct id from OBJECTS ";
	// query += " FROM OBJ_ASSOC INNER JOIN OBJECTS ON OBJ_ASSOC.id2 = OBJECTS.id ";
	// query += " FROM OBJ_ASSOC INNER JOIN OBJECTS ON OBJ_ASSOC.id2 = OBJECTS.id AND OBJECTS.type in (" + obj_type_list + ") ";

	if ( obj_ids_left1.empty() && obj_ids_left2.empty() && obj_ids_right.empty() && obj_ids_ndir.empty()) return f_ids_vec;

	std::string where_str = " OBJECTS.type in(" + obj_type_list + ") ";
	if (obj_ids_left1.size() > 0)
	{
		if (!where_str.empty()) where_str += " AND ";
		where_str += " id in ( SELECT id2 FROM OBJ_ASSOC WHERE id1 in(" + ids_left_str_1 + ") )";
	}

	if (obj_ids_left2.size() > 0)
	{
		if (!where_str.empty()) where_str += " AND ";
		where_str += " id in ( SELECT id2 FROM OBJ_ASSOC WHERE id1 in(" + ids_left_str_2 + ") )";
	}
	if (obj_ids_ndir.size() > 0)
	{
		if (!where_str.empty()) where_str += " AND ";
		where_str += " (id in ( SELECT id2 FROM ( SELECT id2 FROM OBJ_ASSOC WHERE id1 in(" + ids_ndir_str + ")  UNION SELECT id1 FROM OBJ_ASSOC WHERE id2 in(" + ids_ndir_str + ")) AS ID_NDIR  ))";
	}
	if (obj_ids_right.size() > 0)
	{
		if (!where_str.empty()) where_str += " AND ";
		where_str += " id in (SELECT id1 FROM OBJ_ASSOC WHERE id2 in(" + ids_right_str + ") )";
	}

	query += "WHERE " + where_str;

	//	query += " LIMIT 10000";
	wxLogMessage(" BiblDB::GetAssocObjIDs Q1: %s \n", query.c_str());

	std::vector<std::string> str_arr;
	SQLQuery(query.c_str(), &str_arr);

	for (std::string s : str_arr)
	{
		const int id = std::stoi(s);
		f_ids_set.insert(id);
	}
	str_arr.clear();

	f_ids_vec.reserve(f_ids_set.size());
	for (const int i : f_ids_set)
	{
		f_ids_vec.push_back(i);
	}
	return f_ids_vec;

}

int BiblDB::GetObjType(int obj_id)
{
	std::string query;
	query = "SELECT type FROM OBJECTS where id = " + std::to_string(obj_id);
	std::vector<std::string> resp;
	SQLQuery(query, &resp);
	if (!resp.empty()) return std::stoi(resp[0]);
	return 0;
}

int BiblDB::GetKWForCategory(std::vector<std::string>& str_arr, std::string cat_str, std::vector<int>* sel_kw_ids_right_par, std::vector<int>* sel_kw_ids_left_par )
{
	std::string query;
    query = "SELECT id2 ";
	query += " FROM OBJ_ASSOC INNER JOIN OBJECTS ON OBJ_ASSOC.id2 = OBJECTS.id AND OBJECTS.type =  " + boost::str( boost::format("%d") % KEYWORD_OBJ );

	bool have_filters_left = false;
	bool have_filters_right = false;

	if (sel_kw_ids_right_par != NULL && sel_kw_ids_right_par->size() > 0) have_filters_right = true;
	if (sel_kw_ids_left_par != NULL && sel_kw_ids_left_par->size() > 0) have_filters_right = true;

	boost::to_lower(cat_str);
	if (cat_str.empty() || cat_str == "all")
	{
		query += " WHERE 1 = 1 ";
		if (!sel_kw_ids_right_par && !sel_kw_ids_right_par) return FALSE;
	}
	else 
	{
		int cat_id = GetKeyWordID(cat_str.c_str(), FALSE);
		if (cat_id < 1) return FALSE;
		query += " WHERE id1 = ";
		query += boost::str(boost::format("%d") % cat_id);
	}

	int i;
	if( have_filters_right )
	{
		int nsel = sel_kw_ids_right_par->size();
	    query += " AND ";
	    query += " id2 in (";
	    query += " SELECT id2 FROM OBJ_ASSOC WHERE ";
	    
		for( i= 0; i < nsel; i++)
		{
		  if( i > 0)
		  {
			 query += " OR ";
		  }
		  query += boost::str(boost::format(" id1 = %d ") % (*sel_kw_ids_right_par)[i]);  
		}
		query += " ) ";
	}

	if( have_filters_left )
	{
		int nsel = sel_kw_ids_left_par->size();
	    query += " AND ";
	    query += " id2 in (";
	    query += " SELECT id1 FROM OBJ_ASSOC WHERE ";
	    
		for( i= 0; i < nsel; i++)
		{
		  if( i > 0)
		  {
			 query += " OR ";
		  }
		  query += boost::str(boost::format(" id2 = %d ") % (*sel_kw_ids_left_par)[i]);  
		}
		query += " ) ";
	}
	query += " LIMIT 1000";

//	wxLogMessage(" BiblDB::GetKWForCategory() Q: %s \n",query.c_str() );

	std::vector<std::string> str_arr_tmp;

	SQLQuery(query.c_str(), &str_arr_tmp);
    
	str_arr.clear();

	int nrow = GetLastSQLResNumRows();
    str_arr.reserve(nrow);
	if (nrow == 0) return FALSE;

	for(i=0; i < nrow; i++)
	{
		int kw_id = atoi(str_arr_tmp[i].c_str());
		std::string kw = GetKeyWordByID(kw_id);
		if(!kw.empty()) str_arr.push_back(kw);
	}

	return FALSE;
}

ObjVec BiblDB::GetObjectsForCategory(std::string cat_str)
{
	ObjVec obj_vec;
	std::vector<int> id_vec;
	std::vector<int> id_filter_both;

	int cat_id = GetKeyWordID(cat_str.c_str());
	if (cat_id < 1) return obj_vec;
	
	id_filter_both.push_back(cat_id);

	id_vec = GetAssocObjIDs(id_filter_both);
	// id_vec  = GetAssocObjIDs(id_filter_both);
	obj_vec = LoadDataObjects(id_vec); 
	return obj_vec;
}

int BiblDB::EraseKeyWord(const char* kw1)
{
	int kw_id = this->GetKeyWordID(kw1);
	if( kw_id == 0) return FALSE;

	std::string kw_id_str = boost::str( boost::format("%d") % kw_id);

	wxLogMessage("Try to delete keyword %s with id= %d \n",kw1, kw_id);

	std::string query;
	query = "DELETE FROM OBJ_ASSOC ";
	query += "WHERE ID1 = " + kw_id_str;
	query += " OR ID2 = " +  kw_id_str;

	SQLQuery(query.c_str());

	query = "DELETE FROM KEYWORDS_LIST ";
	query += "WHERE KEYWORD_ID = " + kw_id_str;

	SQLQuery(query.c_str());

	query = "DELETE FROM OBJECTS ";
	query += "WHERE ID = " + kw_id_str;

	SQLQuery(query.c_str());

	return TRUE;
}

int BiblDB::MergeKeyWords(const char* kw1, const char* kw2)
{
    SetKW2RefKW1(kw1,kw2);
	EraseKeyWord(kw1);
	return TRUE;
}

int BiblDB::SetKW2RefKW1(const char* kw1, const char* kw2)
{
	int kw1_id = this->GetKeyWordID(kw1);
	if( kw1_id == 0) return FALSE;

	int kw2_id = this->GetKeyWordID(kw2,TRUE);
	if( kw2_id == 0) return FALSE;

	std::string kw1_id_str = boost::str( boost::format("%d") % kw1_id);
	std::string kw2_id_str = boost::str( boost::format("%d") % kw2_id);
	std::string type_str   = boost::str( boost::format("%d") % KEYWORD_TAG ); 

	std::string query;
	query = "INSERT INTO OBJ_ASSOC (ID2, ID1, TYPE ) ";
	query += " SELECT " + kw2_id_str + ",ID1," + type_str;
	query += " FROM OBJ_ASSOC ";
	query += " WHERE ID2 = " + kw1_id_str;
	query += " AND ID1 NOT IN ( SELECT ID1 FROM OBJ_ASSOC ";
	query += " WHERE ID2 = " + kw2_id_str + " AND TYPE = " + type_str + " ) ";

	SQLQuery(query.c_str());

	return TRUE;
}

int BiblDB::SaveRefPDF(const BiblRef& bref, std::string pdf_path_utf8, bool overwrite)
{
	if (pdf_path_utf8.empty())
	{
		wxLogMessage("Error in BiblDB::SaveRefPDF():  pdf_path in empty");
		return FALSE;
	}

	// wxString pdf_path_wx = wxString::FromUTF8(pdf_path_utf8);   // wxWidget is doing the conversion properly
	// bool exist_wx = wxFileName::FileExists(pdf_path_wx);

	// pdf_path = pdf_path_wx.c_str();

	// Get the default locale  // Should we do it just once in the beginning of the program??
	//std::locale loc = boost::locale::generator().generate("");
	// Set the global locale to loc
	//std::locale::global(loc);    // this is necessary for proper conversion of filename from UTF8??
	// Make boost.filesystem use it by default
	//boost::filesystem::path::imbue(std::locale());
	// Create the path (pdf_path_utf8 should be utf-8)
	boost::filesystem::path pdf_path(pdf_path_utf8);

	fs::path pdf_dir = pdf_path.parent_path();

	//std::string fname;    // check the name of the file as it read by 
	//fs::directory_iterator itr(pdf_dir);
	//while (itr != fs::directory_iterator{})
	//{
	//	fname = (*itr).path().string();
	//	itr++;
	//}
	//bool exists_boost_1 = fs::exists(fname);

	if (!boost::filesystem::exists(pdf_path))
	{
		wxLogMessage("Error in BiblDB::SaveRefPDF():  file %s doesn't exist ", wxString::FromUTF8(pdf_path.string()));
		return FALSE;
	}

	fs::path pdf_path_db = GetLocPathPDF_default(bref);
	if (pdf_path_db.empty())
	{
		wxLogMessage("Error in BiblDB::SaveRefPDF(): Can not determine the reference standard path");
		return FALSE;
	}
	wxLogMessage("Default DB Local name of the PDF file is %s \n ", wxString::FromUTF8(pdf_path_db.string()));
	fs::path pdf_d = pdf_path_db.parent_path();
	bool bres = true;
	if (!fs::exists(pdf_d))
	{
		bres = fs::create_directories(pdf_d);
	}
	if (!bres)
	{
		wxLogMessage("PDF directory %s does not exist and can not be created ", wxString::FromUTF8(pdf_d.string()));
		return FALSE;
	}

	if( !fs::exists(pdf_path_db) || (fs::exists(pdf_path_db) && overwrite) || (fs::exists(pdf_path_db) && (fs::file_size(pdf_path_db) < 100)) )
	{
		wxLogMessage("Copy PDF file %s to DB location %s ", wxString::FromUTF8(pdf_path.string()), wxString::FromUTF8(pdf_path_db.string()));
		fs::copy_file(pdf_path, pdf_path_db, fs::copy_option::overwrite_if_exists);
	}
	return TRUE;
}

int BiblDB::MoveRefPDFToStdLocation(const BiblRef& bref)
{
	fs::path path_def = GetLocPathPDF_default(bref);
	if (path_def.empty())
	{
		wxLogMessage(" Can not determine default PDF location ");
		return FALSE;
	}
	fs::path fpath_old = GetLocPathPDF_no_subdir( bref );
	fs::path supp_path_old = GetLocPathSupp_no_subdir( bref );
	fs::path ref_dir = GetLocDir_default(bref);
	if (!ref_dir.empty() )
	{
		if (fs::exists(fpath_old))
		{
			wxLogMessage("Saving file %s in the standard location ", fpath_old.string().c_str());
			SaveRefPDF(bref, fpath_old.string(), true);
			wxLogMessage(" Delete old Reference PDF file %s ", fpath_old.string().c_str());
			bool bres = fs::remove(fpath_old);
			if (!bres)
			{
				wxLogMessage(" Error to delete Reference PDF file %s ", fpath_old.string().c_str());
			}
		}
	}

	return TRUE;
}



int BiblDB::LoadRefFromWeb(const BiblRef& bref)
{
	wxLogMessage("Publisher ID: \n");
	wxLogMessage(bref.jrn.publisher_id.c_str());
	wxLogMessage("\n");

	std::string pub_id = bref.jrn.publisher_id;

	RefWebInfo web_info;

	int ires;
	if (!bref.doi.empty())
	{
//		if (bref.doi.find("10.1021/acs") >= 0)
		if (bref.doi.find("10.1021") == 0)  // ACS journal 
		{
			web_info.pdf_url = "http://pubs.acs.org/doi/pdf/";
			web_info.pdf_url += bref.doi;
		} 
		else if (bref.doi.find("10.1063") == 0) // APS journal
		{
			web_info.pdf_url = "http://aip.scitation.org/doi/pdf/";
			web_info.pdf_url += bref.doi;
		};
	}

	if (web_info.pdf_url.empty())
	{
		if (pub_id.find("ACS:") != std::string::npos )
		{
			ires = GetACSRefInfo(bref, web_info);
		}
		else if (pub_id.find("APS:") != std::string::npos )
		{
			ires = GetAPSRefInfo(bref, web_info);
		}
		else if (pub_id.find("AIP:") != std::string::npos )
		{
			ires = GetAIPRefInfo(bref, web_info);
		}
		else if (pub_id.find("SPRJ:") != std::string::npos )
		{
			ires = GetSpringerJRefInfo(bref, web_info);
		}
		else if (pub_id.find("SPRS:") != std::string::npos )
		{
			ires = GetSpringerSRefInfo(bref, web_info);
		}
		else if (pub_id.find("WILEY:") != std::string::npos )
		{
			ires = GetSpringerSRefInfo(bref, web_info);
		}
		else if (pub_id.find("SCIDIR:") != std::string::npos )
		{
			ires = GetSciDirRefInfo(bref, web_info);
			return ires;
		}
		else if (pub_id.find("STD1:") != std::string::npos )
		{
			ires = GetSTD1RefInfo(bref, web_info);
		}
		else if (pub_id.find("AR:") != std::string::npos )
		{
			ires = GetAnnuRevRefInfo(bref, web_info);
		}
		else
		{
			printf("No Publisher info or Unsupported Publisher \n");
		}
	}

	if( !bref.doi.empty() && bref.doi.find("acs.") != std::string::npos)
	{
		web_info.pdf_url = "http://pubs.acs.org/doi/pdf/" + bref.doi;
	}

//	bool bres;
	
	if( !web_info.pdf_url.empty() )
	{
		wxLogMessage(" PDF URL: %s", wxString::FromUTF8( web_info.pdf_url.c_str() ) );
		
		fs::path pdf_path = GetLocPathPDF(bref);
		if (fs::exists(pdf_path) && fs::is_regular_file(pdf_path) && fs::file_size(pdf_path) > 1000)
		{
			wxLogMessage(" Local PDF file already exist: %s \n", pdf_path.string() );
		}

		pdf_path = GetLocPathPDF_default(bref);
		wxLogMessage("Default Local name of the PDF file is %s \n ", wxString::FromUTF8( pdf_path.string() ) );
		fs::path pdf_d = pdf_path.parent_path();
		if (!fs::exists(pdf_d)) fs::create_directory( pdf_d );

		std::string fstr_init = pdf_path.string();
		std::string fstr;
		for (char ch : fstr_init)  // Make \ -> \\ to transfer the file path to python
		{
			fstr += ch;
			if( ch == '\\' ) fstr += ch;  
		}

		ires = PyRun_SimpleString("import biblpy");
		ires = PyRun_SimpleString("import requests");
		std::string cmd = std::string("biblpy.save_url_to_file(") + "\"" + web_info.pdf_url + "\",\"" + fstr + "\")";
		wxLogMessage("cmd: %s \n", cmd.c_str());
		ires = PyRun_SimpleString(cmd.c_str());
		if (ires != 0)
		{
			wxLogMessage("Failed to load PDF from %s \n", web_info.pdf_url );
		}
	}

	return ires;
}

int BiblDB::GetACSRefInfo(const BiblRef& bref, RefWebInfo& web_info)
{
	web_info.doi =            "";
	web_info.html_url =       "";
	web_info.issue_list_url = "";
	web_info.issue_url =      "";
	web_info.pdf_url =        "";
	web_info.support_info =   "";

	wxString jid = bref.jrn.publisher_id;
	jid = jid.Strip(wxString::both);
	jid = jid.Mid(4);

    PyRun_SimpleString("import http.cookiejar, urllib.request \n"
			           "cj = http.cookiejar.CookieJar() \n"
                       "opener = urllib.request.build_opener(urllib2.HTTPCookieProcessor(cj)) \n"
					   "opener.addheaders = [(\"User-agent\", \"Firefox/1.0.7\")] \n"
					   "pars = {} \n"
					   "adr = \"http://pubs.acs.org/wls/journals/query/citationFindResults.html\" \n"
                       "pars[\"op\"] = \"findCitation\" \n"
					   "pars[\"submit\"] = \"FIND\" \n"
					   );
	wxString cmd;
	cmd.Printf("pars[\"cit_qjrn\"] = \"%s\"",jid.c_str());
	PyRun_SimpleString(cmd.ToStdString().c_str());
	cmd.Printf("pars[\"vol\"] = \"%s\"",bref.vol.c_str());
	PyRun_SimpleString(cmd.ToStdString().c_str());
	cmd.Printf("pars[\"spn\"] = \"%s\"",bref.first_page.c_str());
	PyRun_SimpleString(cmd.ToStdString().c_str());
	PyRun_SimpleString("params_req = urllib.urlencode(pars)");
//	PyRun_SimpleString("f1 = opener.open(adr,params_req)");
    PyRun_SimpleString("adr_get = \"%s?%s\" % (adr,params_req)");
	PyRun_SimpleString("print(adr_get)");
	PyRun_SimpleString("f1 = opener.open(adr_get)");
    PyRun_SimpleString("doi_line = \"\"");
    PyRun_SimpleString("abstract_line = \"\"");
    PyRun_SimpleString("html_line = \"\"");
    PyRun_SimpleString("pdf_line = \"\"");
    PyRun_SimpleString("support_line = \"\"");
    PyRun_SimpleString("for line in f1:                                 \n"
                       "  if(line.find(\">DOI: <\") != -1):             \n"
                       "    idx = line.find(\"dx.doi.org\")             \n"
                       "    doi_line = line[idx+11:]                    \n"
					   "    idx = doi_line.find(\"\\\"\")               \n"
					   "    doi_line = doi_line[0:idx]                  \n"
					   "  if(line.find(\">Abstract</a>\") != -1):       \n"
					   "    idx = line.find(\"href=\")                  \n"
					   "    abstract_line = line[idx+6:]                \n"
					   "    idx = abstract_line.find(\"\\\"\")          \n"
					   "    abstract_line = abstract_line[0:idx]        \n"
					   "  if(line.find(\">HTML<\") != -1):              \n"
					   "    idx = line.find(\"href=\")                  \n"
					   "    html_line = line[idx+6:]                    \n"
					   "    idx = html_line.find(\"\\\"\")              \n"
					   "    html_line = html_line[0:idx]                \n"
					   "  if(line.find(\">PDF<\") != -1 and line.find(\"onClick\") == -1): \n"
					   "    idx = line.find(\"href=\")                  \n"
					   "    pdf_line = line[idx+6:]                     \n"
					   "    idx = pdf_line.find(\"\\\"\")               \n"
					   "    pdf_line = pdf_line[0:idx]                    \n"
					   "  if(line.find(\"Supporting Information\")!= -1): \n"
					   "    idx = line.find(\"href=\")                  \n"
					   "    support_line = line[idx+6:]                 \n"
					   "    idx = support_line.find(\"\\\"\")           \n"
					   "    support_line = support_line[0:idx]          \n"
					   );
	PyRun_SimpleString("f1.close()");

	PyObject* main_module = PyImport_AddModule("__main__");
	PyObject* global_dict = PyModule_GetDict(main_module);
	
    PyObject* str_doi_py      = PyDict_GetItemString(global_dict,"doi_line");
	PyObject* str_abstract_py = PyDict_GetItemString(global_dict,"abstract_line");
	PyObject* str_html_py     = PyDict_GetItemString(global_dict,"html_line");
	PyObject* str_pdf_py      = PyDict_GetItemString(global_dict,"pdf_line");
	PyObject* str_support_py  = PyDict_GetItemString(global_dict,"support_line");

	wxString str_doi = PyBytes_AsString(str_doi_py);
	wxString str_abstract = PyBytes_AsString(str_abstract_py);
	wxString str_html = PyBytes_AsString(str_html_py);
	wxString str_pdf = PyBytes_AsString(str_pdf_py);
	wxString str_support = PyBytes_AsString(str_support_py);

	wxLogMessage("DOI:      %s\n", str_doi.c_str()      );	               
	wxLogMessage("ABSTRACT: %s\n", str_abstract.c_str() );	               
	wxLogMessage("HTML:     %s\n", str_html.c_str()     );	               
	wxLogMessage("PDF:      %s\n", str_pdf.c_str()      );	               
	wxLogMessage("SUPPORT:  %s\n", str_support.c_str()  );	               

	if(str_pdf.empty()) return FALSE;

	wxString acs_site = "http://pubs.acs.org";

	web_info.doi          = str_doi;
		
	int idx = str_doi.Find("/");
	wxString art_name = str_doi.Mid(idx+1);

	web_info.pdf_url = acs_site + "/cgi-bin/article.cgi/";
	web_info.pdf_url += jid;
	web_info.pdf_url += "/";
	web_info.pdf_url += bref.pub_year;
	web_info.pdf_url += "/";
	web_info.pdf_url += bref.vol;
	web_info.pdf_url += "/i";
	web_info.pdf_url += bref.iss;
	web_info.pdf_url += "/pdf/";
	web_info.pdf_url += art_name;
	web_info.pdf_url += ".pdf";
	web_info.pdf_url += "?sessid=7593";
	
	web_info.html_url     = acs_site + str_html;
//	web_info.pdf_url      = acs_site + str_pdf;
	web_info.support_info = acs_site + str_support;

	return TRUE;
}

int BiblDB::GetAPSRefInfo(const BiblRef& bref, RefWebInfo& web_info)
{
	std::string site_url = "https://journals.aps.org/";
	std::string j_abbr = bref.jrn.publisher_id.substr(4);
	boost::trim(j_abbr);
	boost::to_lower(j_abbr);
	
	web_info.pdf_url = site_url + j_abbr + "/pdf/";
	web_info.pdf_url += bref.doi;

	return TRUE;
}

int BiblDB::GetAIPRefInfo(const BiblRef& bref, RefWebInfo& web_info)
{
	return FALSE;
}

int BiblDB::GetSpringerJRefInfo(const BiblRef& bref, RefWebInfo& web_info)
{
	return FALSE;
}

int BiblDB::GetSpringerSRefInfo(const BiblRef& bref, RefWebInfo& web_info)
{
	std::string site_url = "https://onlinelibrary.wiley.com/";
	//web_info.pdf_url = site_url + "doi/pdf/" + bref.doi;
	web_info.pdf_url = site_url + "doi/pdfdirect/" + bref.doi;
	return FALSE;
}

int BiblDB::GetSciDirRefInfo(const BiblRef& bref, RefWebInfo& web_info)
{
	if (bref.doi.empty()) return FALSE;
	
	std::string doi_url = "http://dx.doi.org/";
	doi_url += bref.doi;
	
	if (!this->webdriver_flag) this->InitWebDriver(); 
	int ires = RunPythonScriptInString("import biblpy \n"
		                               "sdir = biblpy.scidir( driver )");

	// int ires = PyRun_SimpleString("import biblpy");
	// ires = PyRun_SimpleString("sdir = biblpy.scidir( driver )");

	std::string pdf_fname_init = GetLocPathPDF(bref);
	std::string pdf_fname;
	int i;
	for (i = 0; i < pdf_fname_init.size(); i++)
	{
		char ch = pdf_fname_init[i];
		pdf_fname += ch;
		if (ch == '\\') pdf_fname += ch;
	}
	std::string cmd = std::string("sdir.get_pdf_url_by_doi(") + "\"" + bref.doi + "\",\"" + pdf_fname + "\")";

	wxLogMessage("cmd: %s \n", cmd.c_str());

	ires = RunPythonScriptInString(cmd);
	// ires = PyRun_SimpleString(cmd.c_str());
	if (ires != 0)
	{
		wxLogMessage("Failed to load PDF from %s \n", bref.doi );
	}	
	return ires;
}

int BiblDB::GetSTD1RefInfo(const BiblRef& bref, RefWebInfo& web_info)
{
	std::string j_url = bref.jrn.publisher_id.substr(5);
	boost::trim(j_url);

	std::string site_url = "http://";
	site_url += j_url;
	site_url += "/";

	web_info.html_url = site_url;
	web_info.html_url += "cgi/content/full/";
	web_info.html_url += bref.vol;
	web_info.html_url += "/";
	web_info.html_url += bref.iss;
	web_info.html_url += "/";
	web_info.html_url += bref.first_page;

	web_info.pdf_url = site_url;
	web_info.pdf_url += "cgi/reprint/";
	web_info.pdf_url += bref.vol;
	web_info.pdf_url += "/";
	web_info.pdf_url += bref.iss;
	web_info.pdf_url += "/";
	web_info.pdf_url += bref.first_page; 
	web_info.pdf_url += ".pdf";

	return TRUE;
}

int BiblDB::GetAnnuRevRefInfo(const BiblRef& bref, RefWebInfo& web_info)
{
	web_info.html_url = "";
	web_info.pdf_url =  "";
	
	if( !bref.doi.empty() )
	{
		web_info.pdf_url = "http://www.annualreviews.org/doi/pdf/" + bref.doi;
		web_info.html_url = "http://www.annualreviews.org/doi/" + bref.doi;
	}

	return TRUE;
}

std::string BiblDB::GetPDFDir(const BiblRef& bref) // need to merge with GetLocPathPDF()
{
	std::string sh_j_name = bref.jrn.fname_abbr;
//	std::string pdf_dir = "c:\\bibl\\pdf\\"; 
//	std::string pdf_dir = "d:\\Google Drive\\bibl\\pdf\\";
	std::string pdf_dir = "c:\\Users\\igor\\Google Drive\\bibl\\pdf\\";
	pdf_dir += sh_j_name;
	pdf_dir += "\\";
	return pdf_dir;
}

bool BiblDB::OpenRefDir(const BiblRef& bref)
{
	std::string ref_dir = GetLocDir_default( bref);
	if (fs::is_directory(ref_dir))
	{
		std::string cmd = "explorer " + ref_dir;
		wxExecute(cmd);
		//boost::process::child c(cmd);
		//c.wait();
		return true;
	} 
	std::string ref_doi_dir = this->GetLocDir_DOI(bref);
	if (fs::is_directory(ref_doi_dir))
	{
		std::string cmd = "explorer " + ref_doi_dir;
		wxExecute(cmd);
		//boost::process::child c(cmd);
		//c.wait();
		return true;
	}
	return false;
}

bool BiblDB::DeleteRefDir(const BiblRef& bref)
{

	return true;
}

bool BiblDB::DeleteRefPDF_no_subdir(const BiblRef& bref)
{

	return true;
}


std::string BiblDB::GetLocPrefix(const BiblRef& bref)
{	
	fs::path path = bref.jrn.fname_abbr;
	path.append( GetStdPrefix(bref) );
	return path.string();
}

std::string BiblDB::GetPDFName(const BiblRef& bref)
{
	std::string fname = GetStdPrefix(bref);
	if (fname.empty()) return fname;
	fname += ".pdf";
	return fname;
}



std::string BiblDB::GetBiblDir()
{
	//	std::string path = "c:\\bibl\\";
	//	std::string path = "d:\\Google Drive\\bibl\\";
	std::string path = "c:\\users\\igor\\Google Drive\\bibl\\";

	return path;
}

std::string BiblDB::GetPaperpileDir()
{
	//	std::string path = "c:\\bibl\\";
	//	std::string path = "d:\\Google Drive\\bibl\\";
	std::string path = "c:/users/igor/Google Drive/Paperpile/";

	return path;
}

std::string BiblDB::GetLocDir_DOI(const BiblRef& bref)
{
	fs::path bibl_dir_path = GetBiblDir();
	fs::path pdf_dir_path = bibl_dir_path;
	pdf_dir_path.append("pdf");

	if (!bref.doi.empty())
	{
		std::vector<std::string> tokens;
		boost::split(tokens, bref.doi, boost::is_any_of("/"));

		pdf_dir_path.append("doi");
		for (std::string s : tokens)
		{
			pdf_dir_path.append(s);
		}
		return pdf_dir_path.string();
	}
	return "";
}

bool BiblDB::IsSourceInfoComplete(const BiblRef& bref) const
{
	bool complete = false;

	if (!bref.jrn.fname_abbr.empty())
	{
		if (!bref.pub_year.empty() && (!bref.vol.empty() || !bref.iss.empty()))
		{
			if (!bref.first_page.empty())
			{
				if (bref.first_page != "art. no.") return true;
				if (!bref.last_page.empty()) return true;
			}
		}
		if ( (bref.ref_type == BIB_REF_PREPRINT || IsPreprintJournal(bref.jrn)) && !bref.vol.empty()) return true;
	}
	return false;
}

std::string BiblDB::GetStdPrefix(const BiblRef& bref) const
{
	std::string prefix;

	std::string sh_j_name = bref.jrn.fname_abbr;
	std::string sh_pub_year = bref.pub_year;
	int len = bref.pub_year.size();
	if (len >= 2)
		sh_pub_year = bref.pub_year.substr(len - 2);

	std::string vol_str;
	if (!bref.vol.empty())
	{
		vol_str = bref.vol;
	}
	else if (!bref.iss.empty())
	{
		vol_str = bref.iss;
	}

	std::string fst_page_str = bref.first_page;
	if (bref.first_page == "art. no.")
	{
		fst_page_str = bref.last_page;
	}

	int i;
	for (i = 0; i < 5; i++)
	{
		if (fst_page_str.size() < 5)
		{
			fst_page_str = (std::string)"0" + fst_page_str;
		}
	}

	prefix += sh_j_name;
	prefix += "_";

	if (bref.ref_type == BIB_REF_PREPRINT || IsPreprintJournal(bref.jrn))
	{
		prefix += vol_str;
	}
	else
	{
		prefix += sh_pub_year;
		prefix += "_";
		prefix += vol_str;
		prefix += "_";
		prefix += fst_page_str;
	}

	return prefix;
}

std::string BiblDB::GetLocDir_default(const BiblRef& bref)
{
	fs::path bibl_dir_path = GetBiblDir();
	fs::path pdf_dir_path = bibl_dir_path;
	pdf_dir_path.append("pdf");
	if (IsSourceInfoComplete(bref))
	{
		pdf_dir_path.append(bref.jrn.fname_abbr);
		pdf_dir_path.append( GetStdPrefix(bref) );
		return pdf_dir_path.string();
	}

	if (!bref.doi.empty())
	{
		std::vector<std::string> tokens;
		boost::split(tokens, bref.doi, boost::is_any_of("/"));
		
		pdf_dir_path.append("doi");
		for (std::string s : tokens)
		{
			pdf_dir_path.append(s);
		}
		return pdf_dir_path.string();
	}
	return "";
}

std::string BiblDB::GetLocPathPDF_no_subdir(const BiblRef& bref)
{
	fs::path bibl_dir_path = GetBiblDir();
	fs::path pdf_dir_path = bibl_dir_path;
	pdf_dir_path.append("pdf");
	if (IsSourceInfoComplete(bref))
	{
		pdf_dir_path.append(bref.jrn.fname_abbr);
		std::string fname = GetStdPrefix(bref) + ".pdf";
		pdf_dir_path.append(fname);
		return pdf_dir_path.string();
	}
	return "";
	
}


std::string BiblDB::GetLocPathPDF_default(const BiblRef& bref) 
{
	std::string pdf_dir = GetLocDir_default(bref); 
	if (pdf_dir.empty()) return "";

	std::string fname = "paper.pdf";

	if (IsSourceInfoComplete(bref))
	{
		fname = GetStdPrefix(bref) + ".pdf";
	}
	else if (!bref.doi.empty())
	{
		std::vector<std::string> tokens;
		boost::split(tokens, bref.doi, boost::is_any_of("/"));
		if (tokens.size() > 1)
		{
			fname = tokens[tokens.size() - 1];
			fname += ".pdf";
		}
	}
	else
	{
		return "";
	}

	fs::path pdf_file_path = pdf_dir;
	pdf_file_path.append(fname);

	return pdf_file_path.string();
}

std::string BiblDB::GetLocPathPDF(const BiblRef& bref) 
{	
	std::string pdf_path_def = GetLocPathPDF_default(bref);
	if (pdf_path_def.empty() || !fs::exists(pdf_path_def))
	{
		std::string pdf_path_no_subdir = GetLocPathPDF_no_subdir(bref);
		if (fs::exists(pdf_path_no_subdir)) return pdf_path_no_subdir;
	}
	return pdf_path_def;
}

std::string BiblDB::GetLocPathSupp_no_subdir(const BiblRef& bref)
{	
	std::string path = GetBiblDir();
	path += GetLocPrefix(bref);
	path += "_supp";
	path += ".pdf";

	return path;
}

bool BiblDB::HasSupp(const BiblRef& bref)
{
	std::string old_supp_path = GetLocPathSupp_no_subdir(bref);
	if (!old_supp_path.empty() && fs::exists(old_supp_path)) return true;

	fs::path ref_dir = GetLocDir_default(bref);
	fs::directory_iterator end_itr;
	if (!ref_dir.empty() && fs::exists(ref_dir))
	{
		for (fs::directory_iterator itr(ref_dir); itr != end_itr; itr++)
		{
			std::string curr_file = itr->path().string();
			if (curr_file.find("_supp") != std::string::npos || curr_file.find("_si") != std::string::npos) return true;
		}
	}
	return false;
}


JournalRef BiblDB::GetJournalByID(int journal_id_a)
{
    JournalRef journal_ref;

#if defined(MYSQL_DB) 

    MYSQL_RES *res;
	MYSQL_ROW row;
	wxString query;

	wxString where_str;
    where_str.Printf("JOURNAL_ID = %d",journal_id_a);

    query = "SELECT ";
	query += "JOURNAL_ID, FULL_NAME, STD_ABBREVIATION, STD_SHORT_ABBREVIATION,";
	query += "FNAME_ABBREVIATION, PUBLISHER_ID, ABBR_29, ISSN, ESSN, NLM_ID, PUBLISHER_STR, PRIMARY_CLASS ";
	query += "FROM JOURNALS WHERE ";
	query += where_str;

	if(db_mysql)
	{
	   if( !mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length()) )
	   {
          res = mysql_store_result((MYSQL*)db_mysql);
		  int nrow = mysql_num_rows(res);

		  row = mysql_fetch_row(res);
		  if(row)
		  {
			 journal_ref.obj_id  = journal_id_a;
	         if(row[1]) journal_ref.full_name    = row[1];
	         if(row[2]) journal_ref.std_abbr     = row[2];
	         if(row[3]) journal_ref.short_abbr   = row[3];
	         if(row[4]) journal_ref.fname_abbr   = row[4];
	         if(row[5]) journal_ref.publisher_id = row[5];
			 if(row[6]) journal_ref.abbr_29      = row[6];
			 if(row[7]) journal_ref.issn         = row[7];
			 if(row[8]) journal_ref.essn         = row[8];
			 if(row[9]) journal_ref.nlm_id         = row[9];
			 if(row[10]) journal_ref.publisher_str = row[10];
			 if (row[11]) journal_ref.primary_class = row[11];
		  }
          mysql_free_result(res);
	   }
	}

#endif

    return journal_ref;
}

int BiblDB::GetJournalID(const JournalRef& jrn_ref, int create_new_journal)
{
	if(jrn_ref.obj_id > 0)
	{
		return jrn_ref.obj_id;
	}
	int jrn_id;
		
	jrn_id = GetJournalID(jrn_ref.std_abbr.c_str());
   
	if( jrn_id < 1) jrn_id = GetJournalID(jrn_ref.full_name.c_str());
	if( jrn_id < 1) jrn_id = GetJournalID(jrn_ref.short_abbr.c_str());
	if( jrn_id < 1) jrn_id = GetJournalID(jrn_ref.fname_abbr.c_str());
	if( jrn_id < 1) jrn_id = GetJournalID(jrn_ref.abbr_29.c_str());
//	if( jrn_id < 1) jrn_id = GetJournalID(jrn_ref.issn.c_str());

	if(jrn_id == 0 && create_new_journal != 0)
	{
		jrn_id = CreateNewJournal(jrn_ref);
	}

	return jrn_id;
}

int BiblDB::UpdateJournal(const JournalRef& jrn_ref, int force_update)
{
	int jrn_id = jrn_ref.obj_id;
	if( jrn_id < 1) 
	{
		jrn_id = GetJournalID(jrn_ref, TRUE);
	}
	if( jrn_id < 1) return FALSE;
	
	JournalRef jrn_ref_old = GetJournalByID(jrn_id);

	std::set<std::string> synonyms_new;

	synonyms_new = jrn_ref.synonyms; 

	if( !force_update)
	{
		for( std::string str : jrn_ref_old.synonyms )
		{
			synonyms_new.insert(str);
		}
	}

	std::string field_list;
	if( !jrn_ref.full_name.empty())
	{
		if( jrn_ref_old.full_name.empty() || ( force_update && (jrn_ref.full_name != jrn_ref_old.full_name)) )
		{
			field_list += str(boost::format("FULL_NAME = \"%s\",") % jrn_ref.full_name);
		}
		std::string str = boost::to_upper_copy(jrn_ref.full_name);
		boost::trim(str);
		synonyms_new.insert(str);
		boost::replace_all(str," ","_");
		synonyms_new.insert(str);
	}

	if( !jrn_ref.std_abbr.empty())
	{
		if( jrn_ref_old.std_abbr.empty() || ( force_update && (jrn_ref.std_abbr != jrn_ref_old.std_abbr)) )
		{
			field_list += str(boost::format("STD_ABBREVIATION = \"%s\",") % jrn_ref.std_abbr); 
		}
		std::string str = boost::to_upper_copy(jrn_ref.std_abbr);
		boost::trim(str);
		synonyms_new.insert(str);
		boost::replace_all(str,". ",".");
		synonyms_new.insert(str);
		str = boost::to_upper_copy(jrn_ref.std_abbr);
		boost::trim(str);
		int len = str.size();
		if( len > 0 )
		{
			boost::replace_all(str,". "," ");
			if(boost::ends_with(str, ".")) boost::erase_last(str,".");
			synonyms_new.insert(str);
			boost::replace_all(str," ","_");
			synonyms_new.insert(str);
		}
	}
	
	if( !jrn_ref.short_abbr.empty())
	{
		if( jrn_ref_old.short_abbr.empty() || ( force_update && (jrn_ref.short_abbr != jrn_ref_old.short_abbr)) )
		{
			field_list += str(boost::format("STD_SHORT_ABBREVIATION = \"%s\",") % jrn_ref.short_abbr);
			if( !jrn_ref_old.short_abbr.empty() ) synonyms_new.erase(jrn_ref_old.short_abbr); 
		}
		std::string str = boost::to_upper_copy(jrn_ref.short_abbr);
		boost::trim(str);
		synonyms_new.insert(str);
		str = boost::to_upper_copy(jrn_ref.short_abbr);
		boost::replace_all(str," ","_"); 
		synonyms_new.insert(str);
	}

	if( !jrn_ref.fname_abbr.empty())
	{
		if( jrn_ref_old.fname_abbr.empty() || ( force_update && (jrn_ref.fname_abbr != jrn_ref_old.fname_abbr)) )
		{
			field_list += str(boost::format("FNAME_ABBREVIATION = \"%s\",") % jrn_ref.fname_abbr);
		}
		std::string str = boost::to_upper_copy(jrn_ref.fname_abbr);
		boost::trim(str);
		synonyms_new.insert(str);
		str = boost::to_upper_copy(jrn_ref.fname_abbr);
		boost::replace_all(str," ","_"); 
		synonyms_new.insert(str);
	}

	if( !jrn_ref.publisher_id.empty())
	{
		if( jrn_ref_old.publisher_id.empty() || ( force_update && (jrn_ref.publisher_id != jrn_ref_old.publisher_id)) )
		{
			field_list += str(boost::format("PUBLISHER_ID = \"%s\",") % jrn_ref.publisher_id);
		}
	}

	if (!jrn_ref.primary_class.empty())
	{
		if (jrn_ref_old.primary_class.empty() || (force_update && (jrn_ref.primary_class != jrn_ref_old.primary_class)))
		{
			field_list += str(boost::format("PRIMARY_CLASS = \"%s\",") % jrn_ref.primary_class);
		}
	}


	if( !jrn_ref.abbr_29.empty())
	{
		if( jrn_ref_old.abbr_29.empty() || ( force_update && (jrn_ref.abbr_29 != jrn_ref_old.abbr_29 )) )
		{
			field_list += str(boost::format("ABBR_29 = \"%s\",") % jrn_ref.abbr_29);
			if( !jrn_ref_old.abbr_29.empty() ) synonyms_new.erase(jrn_ref_old.abbr_29); 
		}
		synonyms_new.insert(jrn_ref.abbr_29); 
	}

	if( !jrn_ref.issn.empty())
	{
		if( jrn_ref_old.issn.empty() || ( force_update && (jrn_ref.issn != jrn_ref_old.issn )) )
		{
			field_list += str(boost::format("ISSN = \"%s\",") % jrn_ref.issn);
		}
	}

	if( !jrn_ref.essn.empty())
	{
		if( jrn_ref_old.essn.empty() || ( force_update && (jrn_ref.essn != jrn_ref_old.essn )) )
		{
			field_list += str(boost::format("ESSN = \"%s\",") % jrn_ref.essn);
		}
	}

	if( !jrn_ref.nlm_id.empty())
	{
		if( jrn_ref_old.nlm_id.empty() || ( force_update && (jrn_ref.nlm_id != jrn_ref_old.nlm_id )) )
		{
			field_list += str(boost::format("NLM_ID = \"%s\",") % jrn_ref.nlm_id);
		}
	}

	if( !jrn_ref.publisher_str.empty())
	{
		if( jrn_ref_old.publisher_str.empty() || ( force_update && (jrn_ref.publisher_str != jrn_ref_old.publisher_str )) )
		{
			field_list += str(boost::format("PUBLISHER_STR = \"%s\",") % jrn_ref.publisher_str);
		}
	}

	if( !field_list.empty() )
	{
		boost::erase_tail(field_list,1);
		std::string query;
		query = str(boost::format("UPDATE JOURNALS SET %s WHERE JOURNAL_ID= %d") % field_list % jrn_id);
		wxLogMessage("Update Journal query:\n%s\n\n",query.c_str());
		SQLQuery(query.c_str()); 
	}

  	std::string query = str(boost::format("DELETE FROM JOURNALS_SYN WHERE JOURNAL_ID = %d ") % jrn_id);
	wxLogMessage("Delete synonyms query:\n%s\n\n",query.c_str());
	SQLQuery(query.c_str());  

	if( synonyms_new.size() > 0)
	{
		for(std::string ss : synonyms_new)
		{
			query = "INSERT INTO JOURNALS_SYN  (JOURNAL_ID,JOURNAL_NAME) ";
			query += str(boost::format("VALUES (%d, \'%s\')") % jrn_id % ss) ;
			wxLogMessage("Add synonyms query:\n%s\n\n",query.c_str());
			SQLQuery(query.c_str());
		}
	}
	return TRUE;
}

std::vector<std::string> BiblDB::GetSynForJournal(int journal_id)
{	
	std::string query;
	std::vector<std::string> res;
	query = boost::str(boost::format("SELECT JOURNAL_NAME FROM JOURNALS_SYN WHERE JOURNAL_ID = %d") % journal_id); 
	SQLQuery(query.c_str(),&res);
	return res;
}


int BiblDB::GetJournalID(const char* jrn_name)
{
	if( strlen(jrn_name) == 0) return 0;
	std::string query;
	query = boost::str(boost::format("SELECT JOURNAL_ID FROM JOURNALS_SYN WHERE JOURNAL_NAME = \'%s\'") % jrn_name );
	
	std::vector<std::string> str_arr;
	SQLQuery(query.c_str(), &str_arr);

	if( str_arr.size() > 0) 
	{
		if( str_arr.size() > 1) 
		{
			wxLogMessage("There are more than one journal ID for name = %s \n", (wxString) jrn_name);	
		}
		return atoi(str_arr[0].c_str());
	}
	return 0;
}

int BiblDB::GetJournalIDByFullName(const char* jrn_full_name)
{
	if( strlen(jrn_full_name) == 0) return 0;
	std::string query;
	query = boost::str(boost::format("SELECT JOURNAL_ID FROM JOURNALS WHERE FULL_NAME = \'%s\'") % jrn_full_name  );
	
	std::vector<std::string> str_arr;
	SQLQuery(query.c_str(), &str_arr);

	if( str_arr.size() > 0)
	{
		if( str_arr.size() > 1) 
		{
			wxLogMessage("There are more than one journal ID with full name = %s \n",jrn_full_name);	
		}			
		return atoi(str_arr[0].c_str());
	}
	return 0;
}

int BiblDB::CreateNewJournal(const JournalRef& jrn_ref)
{
	int id_jrn = 0;

    MYSQL_RES *res;
	MYSQL_ROW row;

	int ires;
	
	if(db_mysql)
	{
       int id_jrn = CreateNewObj( JOURNAL_OBJ );	

	   if( id_jrn < 1) return 0;
		
	   wxString j_std;
	   wxString j_f;
	   wxString j_sh;
	   wxString j_fn;

	   if( !jrn_ref.std_abbr.empty() )
	   {
		   j_std = jrn_ref.std_abbr;
		   j_f   = jrn_ref.std_abbr;
		   j_sh  = jrn_ref.std_abbr;
		   j_fn  = jrn_ref.std_abbr;
	   }
	   if( !jrn_ref.full_name.empty() )
	   {
		   j_f   = jrn_ref.full_name;
		   if(j_std.IsEmpty()) j_std = jrn_ref.full_name;
		   if(j_sh.IsEmpty() ) j_sh  = jrn_ref.full_name;
		   if(j_fn.IsEmpty() ) j_fn  = jrn_ref.full_name;
	   }

	   if( !jrn_ref.short_abbr.empty() )
	   {
		   j_sh   = jrn_ref.short_abbr;
		   j_fn   = j_sh;
		   if(j_std.IsEmpty()) j_std = jrn_ref.short_abbr;
		   if(j_f.IsEmpty())   j_f   = jrn_ref.short_abbr;
	   }  

	   j_std = j_std.Strip(wxString::both);
	   j_f   = j_f.Strip(wxString::both);
	   j_sh  = j_sh.Strip(wxString::both);
	   j_fn  = j_fn.Strip(wxString::both);

	   j_fn.Replace(" ","_");
	   j_fn.LowerCase();
	
	   if(j_std.IsEmpty()) return 0;

	   wxString id_str;
	   id_str.Printf("%d",id_jrn);
	   id_str = id_str.Strip(wxString::both);

	   wxString query = "INSERT INTO JOURNALS (JOURNAL_ID, STD_ABBREVIATION, FULL_NAME, STD_SHORT_ABBREVIATION, FNAME_ABBREVIATION";
	   if( !jrn_ref.abbr_29.empty()) 
	   {
		   query += ",";
		   query += "ABBR_29";
	   }
	   if( !jrn_ref.issn.empty()) 
	   {
		   query += ",";
		   query += "ISSN";
	   }
	   if( !jrn_ref.essn.empty()) 
	   {
		   query += ",";
		   query += "ESSN";
	   }
	   if( !jrn_ref.nlm_id.empty()) 
	   {
		   query += ",";
		   query += "NLM_ID";
	   }
	   if( !jrn_ref.publisher_str.empty()) 
	   {
		   query += ",";
		   query += "PUBLISHER_STR";
	   }
	   if (!jrn_ref.primary_class.empty())
	   {
		   query += ",";
		   query += "PRIMARY_CLASS";
	   }
	   query += ") VALUES (";
	   query += "\'";
	   query += id_str;
	   query += "\',\'";
	   query += j_std;
	   query += "\',\'";
	   query += j_f;
	   query += "\',\'";
	   query += j_sh;
	   query += "\',\'";
	   query += j_fn;
	   query += "\'";
	   if( !jrn_ref.abbr_29.empty()) 
	   {
		  query += ",\'";
		  query += jrn_ref.abbr_29;
		  query += "\'";
	   }
	   if( !jrn_ref.issn.empty()) 
	   {
		  query += ",\'";
		  query += jrn_ref.issn;
		  query += "\'";
	   }
	   if( !jrn_ref.essn.empty()) 
	   {
		  query += ",\'";
		  query += jrn_ref.essn;
		  query += "\'";
	   }
	   if( !jrn_ref.nlm_id.empty()) 
	   {
		  query += ",\'";
		  query += jrn_ref.nlm_id;
		  query += "\'";
	   }
	   if( !jrn_ref.publisher_str.empty()) 
	   {
		  query += ",\'";
		  query += jrn_ref.publisher_str;
		  query += "\'";
	   }
	   if (!jrn_ref.primary_class.empty())
	   {
		   query += ",\'";
		   query += jrn_ref.primary_class;
		   query += "\'";
	   }
	   query += ")";

	   wxLogMessage(" SQL Query to insert journal %s \n", query.c_str() );

	   ires = mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.Length());
	   if( ires ) 
	   {
		   wxLogMessage(" Error Creating new journal \n");
		   return 0;
	   }
	   wxString str;

		SetSynForJournal(id_jrn,j_f.ToStdString().c_str());
		str = j_f;
		str.Replace(" ","_");
		SetSynForJournal(id_jrn,str.ToStdString().c_str());

		SetSynForJournal(id_jrn,j_std.ToStdString().c_str());
		str = j_std;
		str.Replace(". ",".");
		SetSynForJournal(id_jrn,str.ToStdString().c_str());

		SetSynForJournal(id_jrn,j_sh.ToStdString().c_str());
		str = j_sh;
		str.Replace(" ","_");
		SetSynForJournal(id_jrn,str.ToStdString().c_str());
	 }

	return id_jrn;
}


int BiblDB::SetSynForJournal(int journal_id, const char* jrn_name)
{
	if(journal_id < 1) return FALSE;
	std::string jrn_str = jrn_name;
	boost::to_upper(jrn_str);
	boost::trim(jrn_str);

	if(jrn_str.empty()) return FALSE;
	std::string query;

	query = boost::str( boost::format("SELECT COUNT(*) FROM JOURNALS_SYN WHERE (JOURNAL_ID = %d AND JOURNAL_NAME = ") % (journal_id) );
	query += "\"" + jrn_str + "\")";
	int nfound = SQLIntFunc( query );

	if(nfound > 0) return TRUE;

	query = boost::str(boost::format("REPLACE INTO JOURNALS_SYN (JOURNAL_ID,JOURNAL_NAME) VALUES (%d,") % (journal_id));
	query += "\"" + jrn_str + "\")";
	SQLQuery( query );

	return TRUE;
}

int BiblDB::RemoveSynForJournal( int journal_id, const char* jrn_name)
{
	if(journal_id < 1) return FALSE;
	std::string jrn_str = jrn_name;
	boost::to_upper(jrn_str);
	boost::trim(jrn_str);

	if(jrn_str.empty()) return FALSE;

	std::string query;
	query = boost::str(boost::format("DELETE FROM JOURNALS_SYN WHERE (JOURNAL_ID = %d AND JOURNAL_NAME= ") % (journal_id));
	query += "\"" + jrn_str + "\")";
	SQLQuery( query );
	return TRUE;
}

int BiblDB::LoadJournalList1(FILE* finp)
{
	TiXmlDocument doc;
	doc.LoadFile(finp);

	const TiXmlElement* root_el = doc.RootElement();

	if( root_el == NULL)
	{
		wxLogMessage(" No Root Element \n");
		return FALSE;
	}
		
	wxString str;

	const TiXmlElement* body_el = root_el->FirstChildElement("body");
	if( body_el == NULL) return FALSE;
	wxLogMessage(body_el->Value());

	const TiXmlElement* form_el = body_el->FirstChildElement("form");
	if( form_el == NULL) return FALSE;
	wxLogMessage(form_el->Value());

	const TiXmlElement* dl_el = form_el->FirstChildElement("dl");
	if( dl_el == NULL) return FALSE;
	wxLogMessage(dl_el->Value());

	const TiXmlElement* dt_el = dl_el->FirstChildElement("dt");
	while( dt_el != NULL)
	{
		JournalRef jref;

		wxString str;
		wxString jrn_full_name;
		wxString issn;
		wxString publisher;
		const TiXmlElement* strong_el = dt_el->FirstChildElement("strong");
		if( strong_el == NULL) 
		{
			wxLogMessage("DT element does not have <strong> element \n");
			return FALSE;
		}
		str = strong_el->GetText();
		str = str.AfterFirst('.');
		jrn_full_name = str.Strip(wxString::both);
		wxLogMessage(jrn_full_name);
		jref.full_name = jrn_full_name;

		int jid = GetJournalID(jrn_full_name.c_str());
		int jid_f = GetJournalIDByFullName(jrn_full_name.c_str());
		wxLogMessage("Journal ID = %d",jid);
		if( jid != jid_f)
		{
			wxLogMessage("WARNING: Journal ID FROM FULL NAME IS DIFFERENT = %d",jid);
		}

		const TiXmlElement* dd_el = dt_el->NextSiblingElement("dd");
		
		dd_el = dd_el->NextSiblingElement("dd");
		str = dd_el->GetText();
		str = str.AfterFirst(':');
		issn = str.Strip(wxString::both);
		wxLogMessage(issn);
		jref.issn = issn;

		dd_el = dd_el->NextSiblingElement("dd");
		str = dd_el->GetText();
		str = str.BeforeFirst(',');
		publisher = str.Strip(wxString::both);
		wxLogMessage(publisher);
		wxLogMessage("  ");
		jref.publisher_str = publisher;

		if( jid == 0)
		{
			CreateNewJournal(jref);	
		}
		else
		{
			jref.obj_id = jid;
			UpdateJournal(jref);
		}

		dt_el = dd_el->NextSiblingElement("dt");
	}

	return TRUE;
}

int BiblDB::LoadJournalList2(FILE* finp)
{
	TiXmlDocument doc;
	doc.LoadFile(finp);

	const TiXmlElement* root_el = doc.RootElement();

	if( root_el == NULL)
	{
		wxLogMessage(" No Root Element \n");
		return FALSE;
	}
		
	wxString str;

	const TiXmlElement* body_el = root_el->FirstChildElement("body");
	if( body_el == NULL) return FALSE;
	wxLogMessage(body_el->Value());

	const TiXmlElement* font_el = body_el->FirstChildElement("font");
	if( font_el == NULL) return FALSE;
	wxLogMessage(font_el->Value());

	const TiXmlElement* table_el = font_el->FirstChildElement("table");
	if( table_el == NULL) return FALSE;
	wxLogMessage(table_el->Value());

	const TiXmlElement* tbody_el = table_el->FirstChildElement("tbody");
	if( tbody_el == NULL) return FALSE;
	wxLogMessage(tbody_el->Value());

	const TiXmlElement* tr_el = tbody_el->FirstChildElement("tr");
	while( tr_el != NULL)
	{
		const TiXmlElement* td_el_1 = tr_el->FirstChildElement("td");
		const TiXmlElement* td_el_2 = NULL;
		if( td_el_1 != NULL) td_el_2 = td_el_1->NextSiblingElement("td");
		if( td_el_2 != NULL )
		{
			const TiXmlElement* font_el_1 = td_el_1->FirstChildElement("font");
			const TiXmlElement* font_el_2 = td_el_2->FirstChildElement("font");
			if( font_el_1 != NULL && font_el_2 != NULL)
			{
				JournalRef jref;
				
				wxString std_abbr  = font_el_1->GetText();
				wxString full_name = font_el_2->GetText();

				int jid = GetJournalID(full_name.ToStdString().c_str());

				wxLogMessage("%d :  %s       -       %s",jid,full_name,std_abbr);
				if( jid > 0)
				{
					jref = GetJournalByID(jid);
					jref.full_name = full_name;
					jref.std_abbr  = std_abbr;
					UpdateJournal(jref,TRUE); 
				}
			}
		}
		tr_el = tr_el->NextSiblingElement("tr");
	}

	return TRUE;
}

int BiblDB::LoadJournalList3(const char* fname)
{
	wxFFile file_inp(fname);
	if( !file_inp.IsOpened() ) 
	{
		wxLogMessage("\nFailed to open file %s \n", fname);
		return FALSE;
	}
	wxFFileInputStream finp_stream(file_inp);
	wxTextInputStream  text_stream(finp_stream );

	wxString line;
	line = text_stream.ReadLine();
	while(!line.IsEmpty())
	{
		if(line.Contains("<DT>"))
		{
			wxString full_name = line.AfterLast('>');
			full_name = full_name.Strip(wxString::both);
			line = text_stream.ReadLine();
			if( line.Contains("<DD>"))
			{
				wxString short_name = line.AfterLast('>');
				short_name = short_name.Strip(wxString::both);

				wxLogMessage("\n\n");
	
				int jid = GetJournalID(full_name.ToStdString().c_str());
				wxLogMessage("%d  :  %s ",jid,full_name);
				wxLogMessage("%d  :  %s ",jid,short_name);
				wxLogMessage("  ");

				if( jid > 0)
				{	
					JournalRef jref;
					jref = GetJournalByID(jid);
					jref.short_abbr = short_name;
					UpdateJournal(jref,TRUE); 
				}
			}
		}
		line = text_stream.ReadLine();
	}

	return TRUE;
}

int BiblDB::MergeJournals(int jrn_id_from, int jrn_id_to)
{
	std::string query;
	query = "UPDATE REFS ";
	query += str(boost::format("SET JOURNAL_ID = %d ") % jrn_id_to);
	query += str(boost::format("WHERE JOURNAL_ID = %d") % jrn_id_from);
	wxLogMessage(" Merge Journals Query: %s \n", query.c_str());
	SQLQuery(query.c_str());
	return TRUE;
}

bool BiblDB::IsPreprintJournal(int journal_id) const
{
	if (journal_id == 681122) return true; // for arXiv 
	return false;
}

bool BiblDB::IsPreprintJournal(const JournalRef& jrn_ref) const
{
	if (jrn_ref.obj_id > 0) return IsPreprintJournal(jrn_ref.obj_id);
	if (jrn_ref.full_name == "ArXiv" || jrn_ref.fname_abbr == "arxiv") return true;
	return false;
}


std::vector<int> BiblDB::FilterJournalIDs(const JournalRequest& jreq)
{
	std::vector<int> ids;
#if defined(MYSQL_DB) 

    MYSQL_RES *res;
	MYSQL_ROW row;
	std::string query;

	std::string jstr = jreq.journal_name_flt;
	
	boost::trim(jstr);
	boost::to_upper(jstr);
	boost::replace_all(jstr,"*","%");

	int first_condition = TRUE;
	std::string where_clause;

	if(!jstr.empty())
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		where_clause += boost::str(boost::format("( JOURNAL_NAME LIKE \"%s\" )") % jstr );
	}

	if( !jreq.jrn_id_from.empty() )
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		where_clause += boost::str(boost::format("( JOURNAL_ID >= %s )") % jreq.jrn_id_from );			
	}

	if( !jreq.jrn_id_to.empty() )
	{
		if(first_condition) 
		{
			first_condition = FALSE;
		}
		else 
		{
			where_clause += " AND ";
		}
		where_clause += boost::str(boost::format("( JOURNAL_ID <= %s )") % jreq.jrn_id_to );			
	}

	std::vector<std::string> sql_res;

    query =  "SELECT JOURNAL_ID FROM JOURNALS_SYN WHERE ";
	query += where_clause;

	std::set<int> jid_set;

	SQLQuery(query.c_str(),&sql_res);
	if(sql_res.size() > 0)
	{
		int i;
		for( i= 0; i < sql_res.size(); i++)
		{
			int id  =  atoi(sql_res[i].c_str());
			jid_set.insert(id);
		}
	}
	
	if( jreq.journal_name_flt.empty() )
	{
		first_condition = TRUE;
		where_clause = "";

		if( !jreq.jrn_id_from.empty() )
		{
			if(first_condition) 
			{
				first_condition = FALSE;
			}
			else 
			{
				where_clause += " AND ";
			}
			where_clause += boost::str(boost::format("( JOURNAL_ID >= %s )") % jreq.jrn_id_from );			
		}

		if( !jreq.jrn_id_to.empty() )
		{
			if(first_condition) 
			{
				first_condition = FALSE;
			}
			else 
			{
				where_clause += " AND ";
			}
			where_clause += boost::str(boost::format("( JOURNAL_ID <= %s )") % jreq.jrn_id_to );			
		}
		
		if(!where_clause.empty())
		{
			query =  "SELECT JOURNAL_ID FROM JOURNALS WHERE ";
			query += where_clause;
			SQLQuery(query.c_str(),&sql_res);
			if(sql_res.size() > 0)
			{
				int i;
				for( i= 0; i < sql_res.size(); i++)
				{
					int id  =  atoi(sql_res[i].c_str());
					jid_set.insert(id);
				}
			}
		}
	}
	

	int nid = jid_set.size();
	ids.resize(nid);

	std::set<int>::iterator itr;
	int i = 0;
	for( itr = jid_set.begin(); itr != jid_set.end(); itr++)
	{
		int id = (*itr);
		ids[i] = id;
		i++;
	}
	
#endif

    return ids;
}

AuthorRef BiblDB::GetAuthByID(int auth_id_a)
{
    AuthorRef auth_ref;

#if defined(MYSQL_DB)

    MYSQL_RES *res;
	MYSQL_ROW row;
	std::string query;
	
	std::string where_str;
	where_str = boost::str(boost::format("AUTH_ID = %d") % auth_id_a);

    query = "SELECT ";
	query += "AUTH_ID, LAST_NAME, FIRST_NAME, INITIALS,";
	query += "MIDDLE_NAME, SUFFIX, ADDRESS, IMPORTANCE, URL, NOTE ";
	query += "FROM AUTHORS WHERE ";
	query += where_str;

	if(db_mysql)
	{
	   if( !mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length()) )
	   {
          res = mysql_store_result((MYSQL*)db_mysql);
		  int nrow = mysql_num_rows(res);

		  row = mysql_fetch_row(res);
		  if(row)
		  {
             auth_ref.obj_id = auth_id_a;
	         if(row[1]) auth_ref.last_name   = row[1];
             if(row[2]) auth_ref.first_name  = row[2];
	         if(row[3]) auth_ref.initials    = row[3];
	         if(row[4]) auth_ref.middle_name = row[4];
	         if(row[5]) auth_ref.suffix      = row[5];
	         if(row[6]) auth_ref.address     = row[6];	 
			 if(row[7]) auth_ref.importance  = atoi(row[7]);
			 if(row[8]) auth_ref.url         = row[8];
			 if(row[9]) auth_ref.note        = row[9];
		  }
          mysql_free_result(res);
	   }
	}
#endif

    return auth_ref;
}

int BiblDB::GetAuthsByID(std::vector<int>& auth_id_vec, std::vector<AuthorRef>& auth_vec )
{
#if defined(MYSQL_DB)

    MYSQL_RES *res;
	MYSQL_ROW row;
	std::string query;

	int i;
	int ival;

	int n = auth_id_vec.size();
	auth_vec.clear();
	
	if(!db_mysql || n == 0) 
	{
		return FALSE;
	}

	std::string where_str ="AUTH_ID IN (";
	std::string str;

	for(i = 0; i < n; i++)
	{
		if( i != (n-1) )
		{
			str = boost::str(boost::format("%d,") % auth_id_vec[i]);
		}
		else
		{
			str = boost::str(boost::format("%d)") % auth_id_vec[i]);
		}
		where_str += str;
	}

    query = "SELECT ";
	query += "AUTH_ID, LAST_NAME, FIRST_NAME, INITIALS,";
	query += "MIDDLE_NAME, SUFFIX, ADDRESS, IMPORTANCE, URL, NOTE ";
	query += "FROM AUTHORS WHERE ";
	query += where_str;
	query += " ORDER BY IMPORTANCE DESC, LAST_NAME ";

	if(db_mysql)
	{
	   if( !mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length()) )
	   {
          res = mysql_store_result((MYSQL*)db_mysql);
		  int nrow = mysql_num_rows(res);
		  auth_vec.resize(nrow);

		  i = 0;
		  while(row = mysql_fetch_row(res))
		  {
			 std::string auth_id_str = row[0]; 
			 try
			 {
				ival = boost::lexical_cast<int>(auth_id_str);
			 }
			 catch(boost::bad_lexical_cast&) {}

			 AuthorRef& auth_ref = auth_vec[i];

             auth_ref.obj_id = ival;
	         if(row[1]) auth_ref.last_name   = row[1];
             if(row[2]) auth_ref.first_name  = row[2];
	         if(row[3]) auth_ref.initials    = row[3];
	         if(row[4]) auth_ref.middle_name = row[4];
	         if(row[5]) auth_ref.suffix      = row[5];
	         if(row[6]) auth_ref.address     = row[6];	 
			 if(row[7]) auth_ref.importance  = atoi(row[7]);
			 if(row[8]) auth_ref.url         = row[8];
			 if(row[9]) auth_ref.note        = row[9];
			 i++;
		  }
          mysql_free_result(res);
	   }
	}
#endif

    return TRUE;
}

int BiblDB::GetRefID(const char* jrn_name, const char* pub_year, const char* vol, const char* issue, const char* fst_page) //!< Get Ref ID  return (-1) if not found and (-2) if not unique.. 
{
	JournalRequest jreq;
	jreq.journal_name_flt = jrn_name;

	std::vector<int> jrn_ids = FilterJournalIDs(jreq);
	
	int njrn = jrn_ids.size();

	if( njrn == 0)
	{
		wxLogMessage("Unknown Journal %s \n", jrn_name);
		return (-1);
	}

	if( njrn > 0)
	{
		wxLogMessage("Warning: Multiple Journals with name %s \n", jrn_name);
	}

	BibRefRequest breq;

	breq.pub_year_from = pub_year;
	breq.pub_year_to   = pub_year;
	breq.journal_flt   = jrn_name;
	breq.volume_flt    = vol;	
	breq.issue_flt     = issue;
	breq.pages_flt  = fst_page;
 
	std::vector<int> refs_ids = SearchRefs(breq);
		
	if( refs_ids.size() == 1)
	{
		return refs_ids[0];
	}

	if( refs_ids.size() > 0)
	{
		wxLogMessage("Warning in BiblDB::GetRefID() \n");
		wxLogMessage("Multiple IDs found for refernce: \n");
		wxLogMessage("ref: %s (%s) vol.%s(%s),p.%s \n",jrn_name,pub_year, vol,issue,fst_page);
		wxLogMessage(" Return first found \n");
		return refs_ids[0];
	}

	return -1;
}


int BiblDB::GetRefID(const BiblRef& bref)
{
	int ref_id = 0;

	if (bref.obj_id > 0) return bref.obj_id;

	int ires;
	std::string query, tmp;
	std::vector<std::string> str_arr;

	if (bref.doi.size() > 0)
	{
		query = "SELECT REF_ID FROM REFS WHERE ( DOI = \'" + bref.doi + "\')";
		ires = SQLQuery(query, &str_arr);

		if (str_arr.size() > 0)
		{
			if (str_arr.size() > 1)
			{
				wxLogMessage(" Warning: More than one reference is matching reference DOI\n");
				PrintRef1(bref);
				wxLogMessage("\n");
			}
			ref_id = atoi(str_arr[0].c_str());
		}
		return ref_id;
	}

	int jrn_id = GetJournalID(bref.jrn);
	if (jrn_id < 1) return 0;

	if (IsPreprintJournal(jrn_id))
	{
		if(bref.vol.empty()) return 0;
	}
	else
	{
		if (bref.pub_year.empty() && bref.vol.empty() && bref.iss.empty()) return 0;
		if (bref.first_page.empty()) return 0;
	}
	
	query = "SELECT REF_ID FROM REFS WHERE ( ";
	
	query += boost::str(boost::format("JOURNAL_ID = %d") % jrn_id );  
		

	if( IsPreprintJournal(jrn_id)) 
	{
		query += " AND VOLUME = " + bref.vol;
	}
	else
	{
		if (!bref.vol.empty())
		{
			query += " AND VOLUME = " + bref.vol;
		}
		else if (!bref.iss.empty())
		{
			query += " AND ISSUE = " + bref.iss;
		}
		else if (!bref.pub_year.empty())
		{
			query += " AND PUB_YEAR = " + bref.pub_year;
		}

		if (!bref.first_page.empty())
		{
			query += " AND FIRST_PAGE = \'" + bref.first_page + "\'";
		}
	}
    
	query += ")";

	wxLogMessage( "Query for BiblDB::GetRefID(): %s\n", wxString::FromUTF8( query.c_str()) );

	ires = SQLQuery( query, &str_arr );

	if( str_arr.size() > 0 ) 
	{
		if( str_arr.size() > 1)
		{
			wxLogMessage(" Warning: More than one reference is matching reference \n");
			PrintRef1(bref);
			wxLogMessage("\n");
		}
		ref_id = atoi(str_arr[0].c_str());
	}
	return ref_id;
}

int BiblDB::CreateNewRefDB()
{
	int ref_id = CreateNewObj( REF_OBJ ); 
	if( ref_id == 0 ) return 0;

	std::string query;
	
	query = boost::str( boost::format("INSERT INTO REFS (REF_ID,REF_TYPE,JOURNAL_ID) VALUES (%d,0,0)") % ref_id);
	SQLQuery( query );

	return ref_id;
}

bool invalidChar(char c)
{
	return !(c >= 0 && c < 128);
}


int BiblDB::UpdateReferenceDB( const BiblRef& bref, int force_update)
{
	int ref_id;
	std::string str;

	ref_id = bref.obj_id;
	if( ref_id < 1) ref_id = GetRefID(bref);
	if( ref_id < 1) 
	{
		wxLogMessage("Can not find the reference \n"); 
		wxLogMessage("No such Reference in the database: \n");
		PrintRefFullStr(bref,str);
		wxLogMessage("\n%s\n",str.c_str());
		return FALSE;
	}

//	BiblRef bref_old = GetRefByID(ref_id);

	std::string field_list;

	field_list = "REF_TYPE = ";
	field_list += boost::str( boost::format("%d") % bref.ref_type);
	
	int incomplete_auth_flag_old = GetIncompleteFlagForRef(ref_id);

	if( bref.auth_vec.size() > 0 || force_update )
	{
		int do_update = 1;
		if( bref.incomplete_auth_flag && incomplete_auth_flag_old == 0) 
		{
			do_update = 0;
		}

		if( do_update)
		{
			UpdateAuthorsForRef(ref_id, bref.auth_vec);
		}
	}

	if( !bref.ext_ref_id.empty() || force_update )
	{
		field_list  += ",EXT_REF_ID = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.ext_ref_id);
	}

	if( !bref.title.empty() || force_update )
	{
		str = bref.title;
		boost::replace_all(str,"\"","&quot;");
		boost::replace_all(str,"\'","&apos;");

		// str.erase(remove_if(str.begin(), str.end(), invalidChar), str.end());

		field_list  += ",TITLE = ";
		field_list += boost::str(boost::format("\"%s\"") % str);
	}

	if( !bref.book_title.empty() || force_update )
	{
		str = bref.book_title;
		boost::replace_all(str,"\"","&quot;");
		boost::replace_all(str,"\'","&apos;");
		field_list  += ",BOOK_TITLE = ";
		field_list += boost::str(boost::format("\"%s\"") % str);
	}

	int journal_id_new = GetJournalID(bref.jrn,TRUE);
	
	if( journal_id_new > 0 || force_update )
	{
		field_list  += ",JOURNAL_ID = ";
		field_list += boost::str(boost::format("%d") % journal_id_new);
	}

	if( !bref.vol.empty() || force_update )
	{
		field_list  += ",VOLUME = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.vol);
	}

	if( !bref.iss.empty() || force_update )
	{
		field_list  += ",ISSUE = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.iss);
	}

	if( !bref.pub_year.empty() || force_update )
	{
		field_list  += ",PUB_YEAR = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.pub_year);
	}

	if( !bref.pub_month.empty() || force_update )
	{
		field_list  += ",PUB_MONTH = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.pub_month);
	}

	if( !bref.first_page.empty() || force_update )
	{
		field_list  += ",FIRST_PAGE = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.first_page);
	}

	if( !bref.last_page.empty() || force_update )
	{
		field_list  += ",LAST_PAGE = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.last_page);
	}

	if( !bref.isi_id.empty() || force_update )
	{
		field_list  += ",ISI_ID = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.isi_id);
	}

	if( bref.isi_id_int > 0 || force_update)
	{
		field_list  += ",ISI_ID_INT = ";
		field_list += boost::str(boost::format("%d") % bref.isi_id_int);
	}

	if( !bref.ga_code.empty() || force_update )
	{
		field_list  += ",GA_CODE = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.ga_code);
	}

	if( !bref.pubmed_id.empty() || force_update )
	{
		field_list  += ",PUBMED_ID = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.pubmed_id);
	}

	if( !bref.reprint_status.empty() || force_update )
	{
		field_list  += ",REPRINT_STATUS = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.reprint_status);
	}

	if( bref.importance > 0 || force_update )
	{
		field_list  += ",IMPORTANCE = ";
		field_list += boost::str(boost::format("%d") % bref.importance);
	}

	if( !bref.doi.empty() || force_update )
	{
		field_list  += ",DOI = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.doi);
	}

	if( !bref.url.empty() || force_update )
	{
		field_list  += ",URL = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.url);
	}

	if( !bref.pii_id.empty() || force_update )
	{
		field_list  += ",PII_ID = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.pii_id);
	}

	if( !bref.medline_id.empty() || force_update )
	{
		field_list  += ",MEDLINE_ID = ";
		field_list += boost::str(boost::format("\"%s\"") % bref.medline_id);
	}

	if( bref.num_cited_in > 0 || force_update )
	{
		field_list  += ",NUM_CITED_IN = ";
		field_list += boost::str(boost::format("%d") % bref.num_cited_in);
	}

	if( bref.num_citing > 0 || force_update )
	{
		field_list  += ",NUM_CITING = ";
		field_list += boost::str(boost::format("%d") % bref.num_citing);
	}

	std::string query = boost::str(boost::format("UPDATE REFS SET %s WHERE REF_ID= %d") % field_list % ref_id);
	SQLQuery(query);

	std::vector<std::string> str_arr;

	if( bref.abstract_str.size() > 10  || force_update )
	{
		str = bref.abstract_str;
		boost::replace_all(str,"\"","&quot;");
		boost::replace_all(str,"\'","&apos;");
		std::string note_str = "";
		
		query = boost::str(boost::format("DELETE FROM ABSTRACTS WHERE REF_ID = (%d)") % ref_id );
		SQLQuery(query);
		query = boost::str(boost::format("INSERT INTO ABSTRACTS (REF_ID,ABSTRACT,NOTE) VALUES (%d,\"%s\",\"%s\")") % ref_id % str % note_str );
		SQLQuery(query);
	}

 //	if( !bref.notes.empty() || force_update)
//	{
//		str = bref.notes;
//		str.Replace("\"","&quot;");
//		str.Replace("\'","&apos;");

//		query.Printf("REPLACE INTO ABSTRACTS (REF_ID,NOTE) VALUES (%d,\"%s\")", ref_id, str.c_str());
//		SQLQuery(query.c_str());
//	}
	
	int ik;
	int nkw_old = 0;

	std::set<int> kwid_old_set;

	if( force_update ) 
	{
		DelObjKW_All( ref_id );
	}
	else
	{
		std::vector<int> kwid_old_array = GetObjKWID(ref_id);
		nkw_old = kwid_old_array.size();

		for(ik = 0; ik < nkw_old; ik++)
		{
			int kw_id = kwid_old_array[ik];
			if( kwid_old_set.count(kw_id) == 0) kwid_old_set.insert(kw_id);
		}
	}

	std::vector<std::string> keyw_arr;

	boost::split(keyw_arr, bref.keywords_str, boost::is_any_of(";"));

	for( std::string keyw : keyw_arr)
	{
		boost::trim(keyw);
		boost::to_lower(keyw);
		if(keyw.empty())continue;
		int kw_id = GetKeyWordID( keyw.c_str(), TRUE );
		if( kwid_old_set.count(kw_id) == 0 ) SetObjKWID( ref_id, kw_id ); 
	}

	return TRUE;    
}

int BiblDB::UpdateTextFieldForRef(int ref_id, const char* value, const char* field_name, const char* table_name ) 
{
	if( ref_id < 1) return FALSE;
	std::string query;

	query = boost::str(boost::format("UPDATE %s SET %s = \"%s\" WHERE REF_ID= %d") % table_name % field_name % value % ref_id) ;
	SQLQuery(query);
	
	return TRUE;
}

int BiblDB::UpdateIntFieldForRef(int ref_id, int value, const char* field_name, const char* table_name ) 
{
	if( ref_id < 1) return FALSE;
	std::string query;

	query = boost::str(boost::format("UPDATE %s SET %s = %d WHERE REF_ID= %d") % table_name % field_name % value % ref_id) ;
	SQLQuery(query.c_str());
	
	return TRUE;
}

std::string BiblDB::GetTextFieldForRef(int ref_id, const char* field_name, const char* table_name )
{
	std::vector<std::string> str_arr;
	std::string query;
	query = str(boost::format("SELECT %s FROM %s WHERE REF_ID = %d") % field_name % table_name % ref_id);
	int ires = SQLQuery(query.c_str(),&str_arr);

	if( ires == 0) return "";
	if( str_arr.size() == 0) return "";

	return str_arr[0];
}

int BiblDB::GetIntFieldForRef(int ref_id, const char* field_name, const char* table_name )
{
	std::vector<std::string> str_arr;
	std::string query;
	query = boost::str(boost::format("SELECT %s FROM %s WHERE REF_ID = %d") % field_name % table_name % ref_id);
	int ires = SQLQuery(query.c_str(),&str_arr);

	if( ires == 0) return -1;
	if( str_arr.size() == 0) return -1;

	return atoi(str_arr[0].c_str());
}

int BiblDB::SetImportanceForRef(int ref_id, int value ) 
{
	return UpdateIntFieldForRef(ref_id,value,"IMPORTANCE");
}

int BiblDB::UpdateAuthorsForRef(int ref_id, const std::vector<AuthorRef>& auth_vec)
{
	std::string query;
	query = boost::str(boost::format("DELETE FROM AUTH_ORDER WHERE REF_ID = %d") % ref_id);
	SQLQuery(query.c_str());

	int na = auth_vec.size();
	int ia;
	for( ia = 0; ia < na; ia++)
	{
		int auth_id = GetAuthorID(auth_vec[ia],TRUE);
		if( auth_id != 0)
		{
			query = "INSERT INTO AUTH_ORDER (REF_ID, AUTH_ID, AUTH_POS) ";
			query += "VALUES ";
			query += boost::str(boost::format("(%d,%d,%d)") % ref_id % auth_id % (ia+1));
			SQLQuery(query.c_str());
		}
	}

 	std::string auth_str = AuthorRef::AuthVecToString(auth_vec);
	query= boost::str(boost::format("UPDATE REFS SET AUTHORS_STR = \"%s\" WHERE REF_ID = %d ") % auth_str % ref_id);
	SQLQuery(query);

	return TRUE;
}

int BiblDB::GetAuthorID(const AuthorRef& aref, int create_new_author)
{
	int id_auth = 0;

	if(aref.obj_id > 0) return aref.obj_id;

	std::string query;
	std::string last_name  = boost::to_upper_copy(aref.last_name);  
	std::string first_name = boost::to_upper_copy(aref.first_name);	 
	std::string initials   = boost::to_upper_copy(aref.initials);  

	query = "SELECT AUTH_ID FROM AUTHORS WHERE ";
	query += "LAST_NAME =";
	query += '"';
    query += last_name;
	query += '"';
	query += " AND INITIALS = ";
	query += '"';
	query += initials;
	query += '"';

	std::vector<std::string> str_arr;
	int nrow = SQLQuery(query.c_str(), &str_arr);
	if(nrow > 0) 
	{
		id_auth = atoi(str_arr[0].c_str());
		return id_auth;
	}
	if(create_new_author)
	{
		id_auth = CreateNewAuthor(aref);
	}
	return id_auth;
}

int BiblDB::CreateNewObj( int obj_type )
{
	std::string query = boost::str(boost::format("INSERT INTO OBJECTS (ID,TYPE) VALUES (NULL,%d)") % obj_type);

	int nrow = SQLQuery( query );
	std::vector<std::string> str_arr;
	nrow = SQLQuery("SELECT MAX(ID) FROM OBJECTS", &str_arr);
	if( nrow == 0 ) return 0;
	int id_obj = atoi(str_arr[0].c_str());

	return id_obj;
}


int BiblDB::CreateNewAuthor(const AuthorRef& aref)
{
	if( aref.last_name.empty() ) return 0;
	
	int id_auth = CreateNewObj( AUTHOR_OBJ );

	std::string str_id_auth;
	str_id_auth = boost::str(boost::format("%d") % id_auth);

	std::string query;
	std::string last_name  = boost::to_upper_copy(aref.last_name);  
	std::string first_name = boost::to_upper_copy(aref.first_name);	 
	std::string initials   = boost::to_upper_copy(aref.initials); 

	query = "INSERT INTO AUTHORS (AUTH_ID,LAST_NAME,INITIALS,FIRST_NAME)";
	query += " VALUES(";
	query += str_id_auth;
	query += ",";
	query += '"';
	query += last_name;
	query += '"';
    query += ",";
	query += '"';
	query += initials;
	query += '"';
    query += ",";
	query += '"';
	query += first_name;
	query += '"';
	query += ")";
	
	wxLogMessage(" CreateNewAuthor query:\n %s \n", (wxString) query);

	SQLQuery(query);

	return id_auth;
}



std::vector<AuthorRef> BiblDB::GetAuthForRef(int ref_id_a)
{
	std::vector<AuthorRef> auth_vec;

#if defined(MYSQL_DB)

    MYSQL_RES *res;
	MYSQL_ROW row;
	wxString query;

    query = "SELECT AUTH_ID FROM AUTH_ORDER WHERE " ;  

	wxString where_str;
    where_str.Printf("REF_ID = %d",ref_id_a);

	query += where_str.c_str();
	
	query += " ORDER BY AUTH_POS ";

	if(db_mysql)
	{
	   if( !mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length()) )
	   {
          res = mysql_store_result((MYSQL*)db_mysql);
		  int nrow = mysql_num_rows(res);
 
		  if(res) 
		  {
		    while( row = mysql_fetch_row(res) )
			{
				int auth_id = atoi(row[0]);
		        AuthorRef auth_ref = GetAuthByID(auth_id);
                auth_vec.push_back(auth_ref);
			}
		  }
          mysql_free_result(res);
	   }
	}

#endif

	return auth_vec;
}

int BiblDB::GetLastSQLResNumRows() const
{
	return num_rows_sql;	
}

int BiblDB::GetLastSQLResNumCols() const
{
	return num_cols_sql;
}

int BiblDB::GetLastSQLResError() const
{
	return sql_error_code;
}

int BiblDB::SQLQuery(const std::string& query, std::vector<std::string>* p_resp)
{
	char query_safe[10000];
	unsigned long len_safe;
	num_rows_sql = 0;
	num_cols_sql = 0;
	sql_error_code = 0;

#if defined(MYSQL_DB)
	if(db_mysql)
	{
	   //len_safe = mysql_real_escape_string((MYSQL*)db_mysql, query_safe, query.c_str(), query.size());
	   //sql_error_code = mysql_real_query((MYSQL*)db_mysql, query_safe, len_safe);
	   sql_error_code  = mysql_real_query((MYSQL*)db_mysql, query.c_str() , query.size() ); 
       if(sql_error_code) return sql_error_code; 
		  
	   if( p_resp != NULL) 
	   {
		   p_resp->clear();
		   
		   MYSQL_RES *res;
		   MYSQL_ROW row;
		   
		   res = mysql_store_result((MYSQL*)db_mysql);
		   num_rows_sql = mysql_num_rows(res);
		   num_cols_sql = mysql_num_fields(res);
		   
		   p_resp->reserve(num_rows_sql * num_cols_sql);
		   
		   if(res) 
		   {
			   while( row = mysql_fetch_row(res) )
			   {
				   unsigned long* lengths;
				   lengths = mysql_fetch_lengths(res);

				   int j;
				   for(j = 0; j < num_cols_sql; j++)
				   {
					   if(row[j] == NULL)
					   {
							p_resp->push_back("");
					   }
					   else
					   {
						   p_resp->push_back( row[j] );
					   }
				   }
			   }
		   }
		   mysql_free_result(res);
	   }
	}
#endif
	return num_rows_sql;
}


int BiblDB::SQLIntFunc(const std::string& query)
{
	std::vector<std::string> str_arr;
	SQLQuery(query,&str_arr);
	if( str_arr.size() == 0) 
	{
		sql_error_code = 901;
		return 0;
	}
	int ires;
	try
	{
		ires = boost::lexical_cast<int>(str_arr[0]);
		return ires;
	}
	catch(boost::bad_lexical_cast&) 
	{
		sql_error_code = 902;
		return 0;	
	}
}

std::vector<AuthorRef> BiblDB::GetAuthForRef_SH(int ref_id_a)
//!
//! Short and fast version of get Authors for the reference
//!
{
	std::vector<AuthorRef> auth_vec;

#if defined(MYSQL_DB)

    MYSQL_RES *res;
	MYSQL_ROW row;
	wxString query;

//
// Can program this:
//
// SELECT LAST_NAME,INITIALS FROM AUTH_ORDER, AUTHORS WHERE REF_ID = 38 AND
// AUTH_ORDER.AUTH_ID = AUTHORS.AUTH_ID ORDER BY AUTH_POS;

//    query = "SELECT AUTH_ID FROM AUTH_ORDER WHERE " ;  

    query =  " SELECT AUTHORS.AUTH_ID, LAST_NAME, INITIALS ";
	query += " FROM AUTHORS, AUTH_ORDER ";
	query += " WHERE AUTH_ORDER.AUTH_ID = AUTHORS.AUTH_ID AND ";

	wxString where_str;
    where_str.Printf("REF_ID = %d",ref_id_a);

	query += where_str.c_str();
	
	query += " ORDER BY AUTH_POS ";

	if(db_mysql)
	{
	   if( !mysql_real_query((MYSQL*)db_mysql,query.c_str(),query.length()) )
	   {
          res = mysql_store_result((MYSQL*)db_mysql);
		  int nrow = mysql_num_rows(res);
 
		  if(res) 
		  {
		    while( row = mysql_fetch_row(res) )
			{
				int auth_id = atoi(row[0]);
		        AuthorRef auth_ref;
                if(row[0]) auth_ref.obj_id     = atoi(row[0]);
	            if(row[1]) auth_ref.last_name   = row[1];
	            if(row[2]) auth_ref.initials    = row[2];
                auth_vec.push_back(auth_ref);
			}
		  }
          mysql_free_result(res);
	   }
	}
#endif

	return auth_vec;
}

int BiblDB::InitWOSRemote()
{
	wxArrayString output;
	wxExecute("ssh kurnikov@pandora get_wos_id",output);
	int n = output.GetCount();

	if( n == 0) 
	{
		wxLogMessage("Error to extract Remote WOS ID \n");
		return FALSE;
	}
	wos_sid = output[0].AfterLast('=');
	boost::trim(wos_sid);
	wxLogMessage("WOS SID= %s \n",wos_sid.c_str());

	return TRUE;
}

int BiblDB::InitWebDriver()
{
	wxLogMessage("Init WEB driver \n");

	PyGILState_STATE gstate = PyGILState_Ensure();
	if (PyErr_Occurred()) {  // PyErr_Print();  
		PyErr_Clear();
	}
	int ires = PyRun_SimpleString("import biblpy");
	ires = PyRun_SimpleString("driver = biblpy.InitWebDriver()");
	ires = PyRun_SimpleString("wos = biblpy.Wos(driver)");
	ires = PyRun_SimpleString("gs_m  = biblpy.GScholar(driver)");
	// ires = PyRun_SimpleString("wos.open_main_page()");
	if (PyErr_Occurred()) {  // PyErr_Print(); 
		PyErr_Clear();
	}
	PyGILState_Release(gstate);

 	webdriver_flag = TRUE;
	return TRUE;
}

int BiblDB::InitWOS()
{
	wxString wos_site = wxT("www.webofknowledge.com");
	wxString page = wxT("/?DestApp=WOS");
//	wxString wos_site = wxT("www.google.com");
//	wxString page = wxT("/intl/en/about.html");

	wxLogMessage(" WOS URL: %s \n",wos_site );

	wxHTTP get;
	get.SetHeader(_T("Content-type"), _T("text/html; charset=utf-8"));
    get.SetTimeout(10); // 10 seconds of timeout instead of 10 minutes ...

	// this will wait until the user connects to the internet. It is important in case of dialup (or ADSL) connections
    while (!get.Connect(wos_site))  // only the server, no pages here yet ...
    wxSleep(5);

	if( get.GetError() != wxPROTO_NOERR) return FALSE;

	wxInputStream* ins = get.GetInputStream(page);

	wxString out;
	wxStringOutputStream os(&out);
	ins->Read( os );

	wxLogMessage("String From Server: \n %s \n\n",out.c_str());
	
	wxString loc = get.GetHeader("Location");
	wxLogMessage("Location : \n %s \n\n",loc );

	loc = loc.Strip(wxString::both);
	wos_start_url = loc;

	wxString set_cookie = get.GetHeader("Set-Cookie");
	wxLogMessage("Set-Cookie: \n %s \n\n", set_cookie );

	wos_sid = "";
	std::string tmp = loc.ToStdString();
	size_t ip = tmp.find("SID=");
	if (ip != std::string::npos) tmp = tmp.substr(ip + 4);

	size_t ip1 = tmp.find('\"');
	size_t ip2 = tmp.find('&');
	if (ip1 != std::string::npos) ip = ip1;
	if (ip2 != std::string::npos && ip2 < ip1 ) ip = ip2;

	wos_sid = tmp.substr(0, ip);

	wxLogMessage("WOS SID= %s ", wxString(wos_sid) );
	wxLogMessage("WOS START URL= %s \n" ,wxString(wos_start_url) );
    
	return TRUE;
}	

int BiblDB::GotoWOSMainPage()
{
	wxLogMessage("BiblDB::GotoWOSMainPage() \n");
//	int ires = PyRun_SimpleString("print(driver)");
//	ires = PyRun_SimpleString("biblpy.driver.get(\"https://www.isiknowledge.com\")");
    int ires = PyRun_SimpleString("driver.get('https://www.isiknowledge.com')");
	return TRUE;
}


int BiblDB::SearchWOSPars(const BibRefRequest& breq)
{

	if(wos_sid.empty()) this->InitWOS();

	//wxHTTPBuilder* m_http = new wxHTTPBuilder();
	//m_http->InitContentTypes(); // Initialise the content types on the page

	//m_http->SetValue( "Search.x", "10", wxHTTPBuilder::wxHTTP_TYPE_GET );
	//m_http->SetValue( "Search.y", "10", wxHTTPBuilder::wxHTTP_TYPE_GET );
	//m_http->SetValue( "RQ", "", wxHTTPBuilder::wxHTTP_TYPE_GET );
	//m_http->SetValue( "Func", "GeneralSearch", wxHTTPBuilder::wxHTTP_TYPE_GET );
	//m_http->SetValue( "Form", "GeneralSearch", wxHTTPBuilder::wxHTTP_TYPE_GET );
	//m_http->SetValue( "SID",   wos_sid, wxHTTPBuilder::wxHTTP_TYPE_GET );
	//m_http->SetValue( "editions",  "D", wxHTTPBuilder::wxHTTP_TYPE_GET);
	//m_http->SetValue( "editions",  "S", wxHTTPBuilder::wxHTTP_TYPE_GET);
	//m_http->SetValue( "editions",  "H", wxHTTPBuilder::wxHTTP_TYPE_GET);
 //   m_http->SetValue( "topic",  breq.keywords_refs_flt, wxHTTPBuilder::wxHTTP_TYPE_GET);
 //   m_http->SetValue( "author", breq.author_flt_str,    wxHTTPBuilder::wxHTTP_TYPE_GET);
	//m_http->SetValue( "gr_author", "",             wxHTTPBuilder::wxHTTP_TYPE_GET);
	//m_http->SetValue( "journal", breq.journal_flt,     wxHTTPBuilder::wxHTTP_TYPE_GET);

	wxString pub_year_str;
	if( !breq.pub_year_from.empty()) 
	{
		pub_year_str = breq.pub_year_from;
		if( !breq.pub_year_to.empty() )
		{
			pub_year_str += "-";
			pub_year_str += breq.pub_year_to;
		}
	}
	else if( !breq.pub_year_to.empty())
	{
		pub_year_str = breq.pub_year_to;
	}
    
	/*m_http->SetValue( "pubyear", pub_year_str, wxHTTPBuilder::wxHTTP_TYPE_GET);
	m_http->SetValue( "address", "", wxHTTPBuilder::wxHTTP_TYPE_GET);

	m_http->SetValue( "source_languagetype", "All languages", wxHTTPBuilder::wxHTTP_TYPE_GET);
	m_http->SetValue( "source_doctype", "All document types", wxHTTPBuilder::wxHTTP_TYPE_GET);

	wxString out = m_http->GetInputString(wos_cgi);

	wxLogMessage("String From Server: \n %s \n\n",out.c_str());

	wxString resp = m_http->GetResponseString();
	wxLogMessage("Response String: \n %s \n\n",resp.c_str());

    delete m_http;
    m_http = NULL; */

	wxString go_str = wos_cgi;
	go_str += "?";
	go_str += "RQ=";
	go_str += "";
	go_str += "&Func=";
	go_str += "GeneralSearch";
	go_str += "&Form=";
	go_str += "GeneralSearch";
	go_str += "&SID=";
	go_str += wos_sid;
	go_str += "&editions=";
	go_str += "S";
	go_str += "&editions=";
	go_str += "D";
	go_str += "&editions=";
	go_str += "H";
	go_str += "&topic=";
	go_str += breq.keywords_refs_flt;
	go_str += "&author=";
	go_str += breq.author_flt_str;
	go_str += "&source_languagetype=";
	go_str += "All languages";
	go_str += "&source_doctype=";
	go_str += "All document types";
	go_str += "&Search.x=";
	go_str += "10";
	go_str += "&Search.y=";
	go_str += "10";

	go_str.Replace(" ","+");

	wxLaunchDefaultBrowser(go_str);

	return TRUE;

}

int BiblDB::ImportRefsPubMedXmlStr(const char* refs_str, const char* kw_str)
{
	FILE* tmp_file = tmpfile();

	if( tmp_file)
	{
		fputs(refs_str, tmp_file);
		rewind(tmp_file);
		ImportRefsPubMedXmlFile(tmp_file, kw_str);
		fclose(tmp_file);
	}
	
	return TRUE;
	
}	

int BiblDB::ImportRefsPubMedXmlFile(FILE* finp, const char* kw_str)
{
	TiXmlDocument doc;
	doc.LoadFile(finp);

	const TiXmlElement* root_el = doc.RootElement();

	if( root_el == NULL)
	{
		wxLogMessage(" No Root Element \n");
		return FALSE;
	}
	
	wxLogMessage("Root Element: %s\n", root_el->Value());
	wxString root_name = root_el->Value();

	const TiXmlElement* pubmed_article_el;

	if( root_name == "PubmedArticle")
	{ 
		pubmed_article_el = root_el;
		const TiXmlElement* medline_citation_el = pubmed_article_el->FirstChildElement("MedlineCitation");
		
		BiblRef bref;

		if( medline_citation_el != NULL)
		{	
			const TiXmlElement* pmid_el = medline_citation_el->FirstChildElement("PMID");
			if( pmid_el != NULL )
			{
				bref.pubmed_id = pmid_el->GetText();
			}
			const TiXmlElement* article_el = medline_citation_el->FirstChildElement("Article");
			if( article_el != NULL)
			{
				const TiXmlElement* journal_el = article_el->FirstChildElement("Journal");
				if( journal_el != NULL)
				{	
					const TiXmlElement* isoabbreviation_el = journal_el->FirstChildElement("ISOAbbreviation");
					if( isoabbreviation_el != NULL )
					{
						bref.jrn.std_abbr =  isoabbreviation_el->GetText();
					}
					const TiXmlElement* title_el = journal_el->FirstChildElement("Title");
					if( title_el != NULL)
					{
						bref.jrn.full_name = title_el->GetText();
					}
					const TiXmlElement* issn_el = journal_el->FirstChildElement("ISSN");
					if( issn_el != NULL)
					{
						bref.jrn.issn = issn_el->GetText();
					}
					const TiXmlElement* journal_issue_el = journal_el->FirstChildElement("JournalIssue");
					if( journal_issue_el != NULL)
					{
						const TiXmlElement* volume_el = journal_issue_el->FirstChildElement("Volume");
						if( volume_el != NULL)
						{
							bref.vol = volume_el->GetText();
						}
						const TiXmlElement* issue_el = journal_issue_el->FirstChildElement("Issue");
						if( issue_el != NULL)
						{
							bref.iss = issue_el->GetText();
						}
						const TiXmlElement* pub_date_el = journal_issue_el->FirstChildElement("PubDate");
						if( pub_date_el != NULL)
						{
							const TiXmlElement* year_el = pub_date_el->FirstChildElement("Year");
							if( year_el != NULL)
							{
								bref.pub_year = year_el->GetText();
							}
							const TiXmlElement* month_el = pub_date_el->FirstChildElement("Month");
							if( month_el != NULL)
							{
								bref.pub_month = month_el->GetText();
							}
						}
					}
				}
			
				const TiXmlElement* article_title_el = article_el->FirstChildElement("ArticleTitle");
				if( article_title_el != NULL)
				{
					bref.title = article_title_el->GetText();
				}
				const TiXmlElement* pagination_el = article_el->FirstChildElement("Pagination");
				if( pagination_el != NULL)
				{
					wxString pages_str;
					const TiXmlElement* medlinepgn_el = pagination_el->FirstChildElement("MedlinePgn");
					if( medlinepgn_el != NULL ) 
					{	
						pages_str = medlinepgn_el->GetText();
						bref.first_page = pages_str.BeforeFirst('-');
						bref.last_page  = pages_str.AfterFirst('-');

						int nfst  = bref.first_page.size();
						int nlst = bref.last_page.size();

						int ic;

						for(ic = nlst+1; ic <= nfst; ic++)
						{
							bref.last_page = bref.first_page[nfst-ic] + bref.last_page;
						}
					}
				}
				const TiXmlElement* abstract_el = article_el->FirstChildElement("Abstract");
				if( abstract_el != NULL)
				{
					const TiXmlElement* abstract_text_el = abstract_el->FirstChildElement("AbstractText");
					if( abstract_text_el != NULL ) 
					{
						bref.abstract_str = abstract_text_el->GetText();
					}
				}

				const TiXmlElement* author_list_el = article_el->FirstChildElement("AuthorList");
				if( author_list_el != NULL)
				{
					const TiXmlElement* author_el = author_list_el->FirstChildElement("Author");	
					while( author_el != NULL)
					{ 
						AuthorRef auth_ref;

						const TiXmlElement* last_name_el = author_el->FirstChildElement("LastName");
						const TiXmlElement* fore_name_el = author_el->FirstChildElement("ForeName");
						const TiXmlElement* initials_el  = author_el->FirstChildElement("Initials");
						
						if( last_name_el != NULL) auth_ref.last_name  = last_name_el->GetText();
						if( fore_name_el != NULL) auth_ref.first_name = fore_name_el->GetText();
						if( initials_el  != NULL) auth_ref.initials   = initials_el->GetText();
						
						bref.auth_vec.push_back(auth_ref);

						author_el = author_el->NextSiblingElement("Author");
					}
				}
			}
		}

		const TiXmlElement* pubmed_data_el = pubmed_article_el->FirstChildElement("PubmedData");
		if( pubmed_data_el != NULL )
		{
			const TiXmlElement* article_id_list_el = pubmed_data_el->FirstChildElement("ArticleIdList");	
			if( article_id_list_el != NULL )
			{	
				const TiXmlElement* article_id_el = article_id_list_el->FirstChildElement("ArticleId");	
                while( article_id_el != NULL )
				{
                    wxString id_type = article_id_el->Attribute("IdType");
					
					id_type.MakeLower();

					if( id_type == "doi")       bref.doi          = article_id_el->GetText();
					if( id_type == "pubmed" )   bref.pubmed_id    = article_id_el->GetText();
					if( id_type == "pii")       bref.pii_id       = article_id_el->GetText();
					if( id_type == "medline")   bref.medline_id   = article_id_el->GetText();

					article_id_el = article_id_el->NextSiblingElement("ArticleId");
				}
			}
		}

		std::string str;
		PrintRefFullStr( bref, str );
		wxLogMessage("Imported Reference Before Saving \n\n%s: \n", str.c_str());

		int ref_id = GetRefID(bref);
		if( ref_id < 1) ref_id = CreateNewRefDB();
		if( ref_id < 1) return FALSE;
		bref.obj_id = ref_id;
		UpdateReferenceDB(bref);

		if( strlen(kw_str) > 0) SetObjKW(ref_id, kw_str);

		bref.Clear();
		bref = GetRefByID(ref_id, GET_REF_FULL);
		PrintRefFullStr( bref, str );
		wxLogMessage("Imported Reference After Saving \n\n%s: \n", str.c_str());
		
	}

	return TRUE;
	
}	

int BiblDB::ImportRefsBibTeXStr(std::string refs_str, const BibRefInfo* p_ref_info )
{
	//std::vector<std::string> str_arr;
	//std::string query = "SELECT AUTH_ID FROM AUTHORS WHERE LAST_NAME =\"LAGARDËRE\" AND INITIALS = \"L\"";
	//std::string query = R"(SELECT AUTH_ID FROM AUTHORS WHERE LAST_NAME ="LAGARDËRE" AND INITIALS = "L")";
	//int nrow = SQLQuery(query, &str_arr);
	//return TRUE;

	// refs_str.erase(std::remove(refs_str.begin(), refs_str.end(), '\n'), refs_str.end());
	refs_str.erase(std::remove(refs_str.begin(), refs_str.end(), '\r'), refs_str.end());

	std::vector<std::string> lines;

	boost::split(lines, refs_str, boost::is_any_of("\n"));

	BiblRef bref;
	size_t pos;
	size_t pos_e;
	int set_update_current = TRUE;
	
	for (std::string line : lines )
	{
		boost::trim(line);
		boost::replace_first(line, " =", "=");

		if (line[0] == '@')
		{
			bref.Clear();
			pos = line.find("{");
			if (pos != std::string::npos)
			{
				std::string type_str = line.substr(1, pos);
				if (type_str == "misc" || type_str == "article") bref.ref_type = BIB_REF_TYPE_JOURNAL;
				if (type_str == "series") bref.ref_type = BIB_REF_TYPE_IN_SERIES;
			}
			continue;
		}
		if ( line.substr(0,6) == "title=" )
		{
			pos   = line.find("{");
			pos_e = line.rfind("}");
			if (pos != std::string::npos && pos_e != std::string::npos)
			{
				std::string title_str = line.substr(pos + 1, pos_e - pos - 1);
				bref.title = title_str;
			}
			continue;
		}
		if (line.substr(0, 5) == "year=")
		{
			pos = line.find("{");
			pos_e = line.find("}");
			if (pos != std::string::npos && pos_e != std::string::npos)
			{
				std::string year_str = line.substr(pos + 1, pos_e - pos - 1);
				bref.pub_year = year_str;
			}
			continue;
		}
		if (line.substr(0, 14) == "archivePrefix=")
		{
			pos = line.find("{");
			pos_e = line.find("}");
			if (pos != std::string::npos && pos_e != std::string::npos)
			{
				std::string archive_str = line.substr(pos + 1, pos_e - pos - 1);
				bref.jrn.full_name = archive_str;
			}
			continue;
		}
		if (line.substr(0, 7) == "eprint=")
		{
			pos = line.find("{");
			pos_e = line.find("}");
			if (pos != std::string::npos && pos_e != std::string::npos)
			{
				std::string eprint_str = line.substr(pos + 1, pos_e - pos - 1);
				bref.vol = eprint_str;
				bref.incomplete_auth_flag = 0;
			}
			continue;
		}
		if (line.substr(0, 13) == "primaryClass=")
		{
			pos = line.find("{");
			pos_e = line.find("}");
			if (pos != std::string::npos && pos_e != std::string::npos)
			{
				std::string class_str = line.substr(pos + 1, pos_e - pos - 1);
				bref.iss = class_str;
			}
			continue;
		}
		if (line.substr(0, 8) == "journal=")
		{
			pos = line.find("{");
			pos_e = line.rfind("}");
			if (pos != std::string::npos && pos_e != std::string::npos)
			{
				std::string journal_str = line.substr(pos + 1, pos_e - pos - 1);
				bref.jrn.full_name = journal_str;
			}
			continue;
		}
		if (line.substr(0, 7) == "volume=")
		{
			pos = line.find("{");
			pos_e = line.find("}");
			if (pos != std::string::npos && pos_e != std::string::npos)
			{
				std::string vol_str = line.substr(pos + 1, pos_e - pos - 1);
				bref.vol = vol_str;
			}
			continue;
		}
		if (line.substr(0, 6) == "issue=")
		{
			pos = line.find("{");
			pos_e = line.find("}");
			if (pos != std::string::npos && pos_e != std::string::npos)
			{
				std::string issue_str = line.substr(pos + 1, pos_e - pos - 1);
				bref.iss = issue_str;
			}
			continue;
		}
		if (line.substr(0, 6) == "pages=")
		{
			pos = line.find("{");
			pos_e = line.rfind("}");
			if (pos != std::string::npos && pos_e != std::string::npos)
			{
				std::string pages_str = line.substr(pos + 1, pos_e - pos - 1);
				std::vector<std::string> tokens;
				boost::split(tokens, pages_str, boost::is_any_of("-"));
				boost::trim(tokens[0]);
				bref.first_page = tokens[0];
				if (tokens.size() > 1)
				{
					boost::trim(tokens[1]);
					bref.last_page = tokens[1];
					if (tokens.size() > 1 && bref.last_page.size() == 0) bref.last_page = tokens[2];
				}
			}
			continue;
		}
		if (line.substr(0, 4) == "doi=")
		{
			pos = line.find("{");
			pos_e = line.rfind("}");
			if (pos != std::string::npos && pos_e != std::string::npos)
			{
				std::string doi_str = line.substr(pos + 1, pos_e - pos - 1);
				boost::trim(doi_str);
				bref.doi = doi_str;
			}
			continue;
		}
		if (line.substr(0, 4) == "url=")
		{
			pos = line.find("{");
			pos_e = line.rfind("}");
			if (pos != std::string::npos && pos_e != std::string::npos)
			{
				std::string url_str = line.substr(pos + 1, pos_e - pos - 1);
				boost::trim(url_str);
				bref.url = url_str;
			}
			continue;
		}
		if (line.substr(0, 7) == "author=")
		{
			pos = line.find("{");
			pos_e = line.rfind("}");
			if (pos != std::string::npos && pos_e != std::string::npos)
			{
				std::string str = line.substr(pos + 1, pos_e - pos - 1);
				std::vector<std::string> auth_str_vec; 
				while ( (pos = str.find(" and ")) != std::string::npos )
				{
					std::string token = str.substr(0, pos);
					boost::trim(token);
					auth_str_vec.push_back(token);
					str.erase(0, pos + 5);
				}
				boost::trim(str);
				auth_str_vec.push_back(str);

				for (std::string str : auth_str_vec)
				{
					boost::trim(str);
					boost::to_upper(str);
					AuthorRef auth_ref;

					std::vector<std::string> tokens;
					boost::split(tokens, str, boost::is_any_of(","));
					int n = tokens.size();
					if (n == 0) continue;
					boost::trim(tokens[0]);
					auth_ref.last_name = tokens[0];

					if (n > 1)
					{
						std::string str2 = tokens[1];
						boost::trim(str2);
						boost::split(tokens, str2, boost::is_any_of(" "));
						int n2 = tokens.size();

						for (int i = 0; i < n2; i++)
						{
							boost::trim(tokens[i]);
							int len = tokens[i].size();
							if (tokens[i][len - 1] == '.')
							{
								tokens[i] = tokens[i].substr(0, len - 1);
							}
							else
							{
								if (i == 0)
								{
									auth_ref.first_name = tokens[i];
								}
								else if (i == 1)
								{
									auth_ref.middle_name = tokens[i];
								}
							}
							auth_ref.initials += tokens[i][0];
						}
					}
					bref.auth_vec.push_back(auth_ref);
				}
			}
			continue;
		}
		if (line[0] == '}')
		{
			std::string  str;
			PrintRefFullStr(bref, str);
			wxLogMessage("Imported Reference Before Saving %s \n", wxString(str));

			int ref_id = GetRefID(bref);
			if (ref_id < 1)
			{
				ref_id = CreateNewRefDB();
			}
			else
			{
				wxLogMessage("Reference already in the Database, refid = %d\n", ref_id);
			}

			if (ref_id < 1) return FALSE;
			bref.obj_id = ref_id;

			UpdateReferenceDB(bref);
			SetIncompleteFlagForRef(ref_id, FALSE);
			if (set_update_current) SetCurrentUpdateTime(ref_id);

			if (p_ref_info != NULL) // Set Reference info
			{
				if (!p_ref_info->keywords_str.empty()) SetObjKW(ref_id, p_ref_info->keywords_str);
				if (!p_ref_info->cited_in_ref.empty())
				{
					try
					{
						int cited_in_ref = boost::lexical_cast<int>(p_ref_info->cited_in_ref);
						this->SetCitation(cited_in_ref, ref_id);
					}
					catch (boost::bad_lexical_cast&)
					{
					}
				}
				if (!p_ref_info->cited_ref.empty())
				{
					try
					{
						int cited_ref = boost::lexical_cast<int>(p_ref_info->cited_ref);
						this->SetCitation(ref_id, cited_ref);
					}
					catch (boost::bad_lexical_cast&)
					{
					}
				}
			}
			bref.Clear();
			bref = GetRefByID(ref_id, GET_REF_FULL);
			PrintRefFullStr(bref, str);
			wxLogMessage("Imported Reference After Saving \n%s\n", wxString::FromUTF8(str));

			continue;
		}
	}

	return FALSE;
}

int BiblDB::GetIncompleteFlagForRef(int ref_id)
{
	std::vector<std::string> str_arr;
	std::string query;
	query = boost::str(boost::format("SELECT INCOMPLETE_FLAG FROM REFS WHERE REF_ID = %d") % ref_id);
	int ires = SQLQuery(query,&str_arr);

	if( ires == 0) return 0;
	if( str_arr.size() == 0) return 0;

	return atoi(str_arr[0].c_str());
}


int BiblDB::SetIncompleteFlagForRef(int ref_id, int value_new )
{
	std::string query;
	query= boost::str( boost::format("UPDATE REFS SET INCOMPLETE_FLAG = %d WHERE REF_ID = %d") % value_new % ref_id );
	int ires = SQLQuery(query);
	return ires;
}

int BiblDB::SetCurrentUpdateTime(int ref_id )
{
	if( ref_id < 1) return FALSE;
	std::string query;

	time_t time_now = wxDateTime::GetTimeNow();

	query = boost::str( boost::format("UPDATE REFS SET LAST_UPDATE_TIME = %d WHERE REF_ID = %d") % time_now % ref_id );
	int ires = SQLQuery(query);
	return ires;
}

int BiblDB::ImportRefsISI( wxInputStream& stream, const BibRefInfo* p_ref_info )
{	
	wxTextInputStream tstr(stream);
	wxString str;
	wxString val;

	wxString tag_old = "";
	wxString tag_new  = "";

	int incomplete_kw = FALSE;
	int incomplete_cit_ref = FALSE;

	BiblRef bref;
	wxArrayString cited_refs_strs;
	int set_update_current = FALSE;

	for(;;)
	{	
		str = tstr.ReadLine();
		if( stream.Eof()) return TRUE;

		tag_new = str.Mid(0,2);
		tag_new = tag_new.Strip( wxString::both);
		tag_new.MakeUpper();

		if( tag_new.IsEmpty() ) tag_new = tag_old;
	
		val = str.Mid(2);
		val = val.Strip(wxString::both);

		if( tag_new == "PT")
		{
			bref.Clear();
			set_update_current = FALSE;
			if(val[0] == 'J') bref.ref_type = BIB_REF_TYPE_JOURNAL;
			if(val[0] == 'S') bref.ref_type = BIB_REF_TYPE_IN_SERIES;
			incomplete_kw = FALSE;
			tag_old = "PT";
			cited_refs_strs.Clear();
		}

		if( tag_old.IsEmpty()) continue;

		if( tag_new == "AU")
		{
			val.MakeUpper();
            AuthorRef auth_ref;
			auth_ref.last_name = val.BeforeFirst(',');
			auth_ref.initials  = val.AfterFirst(',');
			boost::trim(auth_ref.initials);
			bref.auth_vec.push_back(auth_ref);
		}

		if( tag_new == "TI") 
		{
			if( !bref.title.empty() && bref.title[bref.title.size() - 1] != '-' ) bref.title += " ";
			bref.title += val;
		}

		if( tag_new == "SO")
		{
			if( bref.ref_type == BIB_REF_TYPE_JOURNAL)
			{
				if( !bref.jrn.full_name.empty())
				{
					int len = bref.jrn.full_name.size();
					if(bref.jrn.full_name[len-1] != '-') bref.jrn.full_name += " ";
				}
				bref.jrn.full_name += val;
			}
			if( bref.ref_type == BIB_REF_TYPE_IN_SERIES)
			{
				if(!bref.book_title.empty() ) bref.book_title += " ";
				bref.book_title += val;
			}
		}

		if( tag_new == "SE")
		{
			if( bref.ref_type == BIB_REF_TYPE_IN_SERIES)
			{
				if(!bref.jrn.full_name.empty())
				{
					int len = bref.jrn.full_name.size();
					if(bref.jrn.full_name[len-1] != '-' ) bref.jrn.full_name += " ";
				}
				bref.jrn.full_name += val;
			}
		}

		if( tag_new == "ID")
		{	
			if( !bref.keywords_str.empty() ) bref.keywords_str += " ";
			bref.keywords_str += val;
		}

		if( tag_new == "AB") 
		{
			if( !bref.abstract_str.empty() && bref.abstract_str[bref.abstract_str.size() - 1] != '-' ) bref.abstract_str += " ";
			bref.abstract_str += val;
		}

		long ltmp; 
		
		if( tag_new == "BP") bref.first_page = val;
		if( tag_new == "EP") bref.last_page = val;
		if( tag_new == "AR") bref.first_page = val; // Article number for journals without page number 
		if( tag_new == "VL") bref.vol = val;
		if( tag_new == "IS") bref.iss = val;
		if( tag_new == "PY") bref.pub_year = val;
		if( tag_new == "PD") bref.pub_month = val;
		
		if( tag_new == "JI") bref.jrn.std_abbr = val;
		if( tag_new == "J9") bref.jrn.short_abbr = val;
		if( tag_new == "GA") bref.ga_code = val;
		if( tag_new == "UT") 
		{
			if( val.find("WOS:") != std::string::npos )
			{
				bref.isi_id  = val.AfterFirst(':');
			}
			else if( val.find("MEDLINE:") != std::string::npos )
			{
				bref.medline_id = val.AfterFirst(':');
			}
		}
		if( tag_new == "DI") bref.doi  = val;
		if( tag_new == "NR") 
		{
			if(val.ToLong(&ltmp))
			{
				bref.num_cited_in = ltmp;
			}
		}
		if( tag_new == "TC") 
		{
			if(val.ToLong(&ltmp))
			{
				bref.num_citing = ltmp;
				set_update_current = TRUE;
			}
		}

		if( tag_new == "CR") 
		{
			if( !incomplete_cit_ref)
			{
				cited_refs_strs.Add(val);
			}
			else
			{
				int nlast = cited_refs_strs.GetCount();
				cited_refs_strs[nlast-1] += " ";
				cited_refs_strs[nlast-1] += val;
			}

			incomplete_cit_ref = FALSE;

            int val_len = val.length();
			wxString end_str;
			if(val_len > 2)
			{
				end_str = val.Mid(val_len - 3,3);
				end_str.MakeUpper();
				if(end_str == "DOI") incomplete_cit_ref = TRUE;
			}
		}

		if( tag_new == "ER")
		{
			std::string  str;
			PrintRefFullStr( bref, str );
			wxLogMessage("Imported Reference Before Saving %s \n", wxString::FromUTF8(str.c_str()) );

			int ref_id = GetRefID(bref);
			if( ref_id < 1) 
			{
				ref_id = CreateNewRefDB();
			}
			else
			{
				wxLogMessage("Reference already in the Database, refid = %d\n",ref_id);
			}
				
			if( ref_id < 1) return FALSE;
			bref.obj_id = ref_id;
			UpdateReferenceDB(bref);
			SetIncompleteFlagForRef(ref_id,FALSE);
			if(set_update_current) SetCurrentUpdateTime(ref_id);
			if( p_ref_info != NULL) // Set Reference info
			{
				if( !p_ref_info->keywords_str.empty() ) SetObjKW(ref_id, p_ref_info->keywords_str);
				if( !p_ref_info->cited_in_ref.empty() )
				{
					try
					{
						int cited_in_ref = boost::lexical_cast<int>(p_ref_info->cited_in_ref);
						this->SetCitation(cited_in_ref,ref_id);
					}
					catch(boost::bad_lexical_cast &)
					{
					}
				}
				if( !p_ref_info->cited_ref.empty() )
				{
					try
					{
						int cited_ref = boost::lexical_cast<int>(p_ref_info->cited_ref);
						this->SetCitation(ref_id,cited_ref);
					}
					catch(boost::bad_lexical_cast &)
					{
					}
				}
			}
			
            int ncit = cited_refs_strs.GetCount();
			if( ncit > 0 )
			{
				wxLogMessage("Import cited references: \n\n");

				int ic;
				for( ic= 0; ic < ncit; ic++)
				{
					BiblRef bref_cit;
					bref_cit.incomplete_auth_flag = TRUE;
					std::vector<std::string> fields;
					std::vector<std::string>::iterator sitr;
					
					boost::split(fields, cited_refs_strs[ic], boost::is_any_of(","));

					wxLogMessage("%s \n",cited_refs_strs[ic]);
//					continue;
					
					for( sitr = fields.begin(); sitr != fields.end(); )
					{
						boost::trim(*sitr);
						if((*sitr).empty() ) 
						{
							sitr = fields.erase(sitr);
						}
						else
						{
							sitr++;
						}
					}
					int nf = fields.size();
					if( nf > 3 )
					{
						std::string auth_str = fields[0];
						boost::to_upper(auth_str);
						AuthorRef auth_ref;
						std::vector<std::string> name_part_arr;
						boost::split(name_part_arr,auth_str, boost::is_any_of(" "));
						int ip = 0;
						for( std::string name_part : name_part_arr)
						{
							boost::trim(name_part);
							if(name_part.empty()) continue;
							if(ip == 0) auth_ref.last_name = name_part;
							if(ip == 1) auth_ref.initials  = name_part;
							ip++;
						} 
						bref_cit.auth_vec.push_back(auth_ref);

						bref_cit.pub_year        = fields[1];

						int jrn_id = GetJournalID(fields[2].c_str());
						if( jrn_id < 1) 
						{
							wxLogMessage("Unknown Journal: %s \n", fields[2].c_str());
							wxLogMessage("Can not process citation:\n%s \n\n",cited_refs_strs[ic].c_str());
							continue;
						}	
						bref_cit.jrn.obj_id = jrn_id;

						JournalRef jref = GetJournalByID(jrn_id);
						wxLogMessage("Journal ID = %d",jref.obj_id);
						wxLogMessage("Journal Full Name = %s",jref.full_name.c_str() );
	
						int ii;
						for(ii = 3; ii < nf; ii++)
						{
							std::string axx = fields[ii];
							
							if(axx[0] == 'V' || axx[0] == 'v')
							{
								bref_cit.vol = axx.substr(1);
							}
							if(axx[0] == 'P' || axx[0] == 'p' )
							{
								bref_cit.first_page = axx.substr(1);
							}
							if( axx.substr(0,4) == "ARTN")
							{
								std::string artn_str = axx.substr(4);
								boost::trim(artn_str);
								bref_cit.first_page = artn_str;
							}
							if( axx.substr(0,3) == "DOI")
							{
								std::string doi_str = axx.substr(3);
								boost::trim(doi_str);
								bref_cit.doi = doi_str;
							}
						}
						if( (bref_cit.jrn.obj_id > 0) && ( !bref_cit.vol.empty() || !bref_cit.first_page.empty()) )
						{
							try
							{
								int pub_year_val = boost::lexical_cast<int>( bref_cit.pub_year );
								if( pub_year_val > 0 )
								{
									int ref_id_cit = GetRefID(bref_cit);
									if( ref_id_cit < 1) 
									{
										ref_id_cit = CreateNewRefDB();
										if(ref_id_cit > 1) SetIncompleteFlagForRef(ref_id_cit);
									}
									else
									{
										wxLogMessage("Cited Reference already in the Database, ref_id_cit = %d \n\n", ref_id_cit);
									}
									if( ref_id_cit < 1) continue;

									bref_cit.obj_id = ref_id_cit;
									UpdateReferenceDB(bref_cit);
									SetCitation(ref_id, ref_id_cit);
								}
							}
							catch(boost::bad_lexical_cast&)
							{
							}
						}
						else
						{
							wxLogMessage(" ref str %s \n has less than 4 fields. Not processed \n",cited_refs_strs[ic]);
						}
					}
				}
			}

			bref.Clear();
			bref = GetRefByID(ref_id, GET_REF_FULL);
			PrintRefFullStr( bref, str );
			wxLogMessage("Imported Reference After Saving \n\n");

			tag_new = "";
		}
		tag_old = tag_new;
	}
	return TRUE;
}	

int BiblDB::ImportRefsRIS(std::istream& stream, const BibRefInfo* p_ref_info )
{
	std::string str;
	std::string val;

	std::string tag_old = "";
	std::string tag_new = "";

	int incomplete_kw = FALSE;
	int incomplete_cit_ref = FALSE;

	BiblRef bref;
	std::vector<std::string> cited_refs_strs;
	int set_update_current = FALSE;
	std::string pdf_loc_path_paperpile = ""; // local path to PDF file of the paper indicated in Paperpile RIS file 

	bool already_have_journal_name = false;

	for (;;)
	{
		std::getline(stream,str);
		if ( stream.eof() ) return TRUE;

		tag_new = str.substr(0,2);
		boost::trim(tag_new);
		boost::to_upper(tag_new);
	
		if (tag_new.empty() ) tag_new = tag_old;

		if (str.size() < 6) continue;

		val = str.substr(6);
		boost::trim(val);


		if (tag_new == "TY")
		{
			bref.Clear();
			pdf_loc_path_paperpile = "";
			set_update_current = FALSE;
			if (val[0] == 'J') bref.ref_type = BIB_REF_TYPE_JOURNAL;
			if (val[0] == 'S') bref.ref_type = BIB_REF_TYPE_IN_SERIES;
			if (val == "INPR") bref.ref_type = BIB_REF_PREPRINT;
			incomplete_kw = FALSE;
			tag_old = "TY";
			cited_refs_strs.clear();
		}

		if (tag_old.empty()) continue;

		size_t pos;
		std::vector<std::string> tokens;
		int i;

		if (tag_new == "AU" || tag_new == "A1")
		{
			boost::to_upper(val);
			AuthorRef auth_ref;
			pos = val.find(",");
			if (pos != std::string::npos) auth_ref.last_name = val.substr(0, pos);
			std::string s2 = val.substr(pos + 1);
			boost::trim(s2);
			boost::split(tokens, s2, boost::is_any_of(" "));
			if (tokens.size() == 0) continue;

			auth_ref.first_name = tokens[0];
			auth_ref.initials = auth_ref.first_name[0];

			if (tokens.size() > 1)
			{
				for (i = 1; i < tokens.size(); i++)
				{
					auth_ref.initials += tokens[i][0];
				}
			}

			bref.auth_vec.push_back(auth_ref);
		}

		if (tag_new == "TI" || tag_new == "T1")
		{
			if (!bref.title.empty() && bref.title[bref.title.size() - 1] != '-') bref.title += " ";
			bref.title += val;
		}

		if (tag_new == "T2")
		{
			if (bref.ref_type == BIB_REF_TYPE_JOURNAL)
			{
				if (!bref.jrn.full_name.empty())
				{
					int len = bref.jrn.full_name.size();
					if (bref.jrn.full_name[len - 1] != '-') bref.jrn.full_name += " ";
				}
				bref.jrn.full_name += val;
				already_have_journal_name = true;
			}
			if (bref.ref_type == BIB_REF_TYPE_IN_SERIES)
			{
				if (!bref.book_title.empty()) bref.book_title += " ";
				bref.book_title += val;
			}
			if (bref.ref_type == BIB_REF_PREPRINT)
			{
				bref.jrn.full_name = val;
				bref.jrn.std_abbr = val;
				pos = val.find('[');
				if (pos != std::string::npos)
				{
					size_t pos_end = val.find(']');
					if (pos_end != std::string::npos) bref.jrn.primary_class = val.substr(pos + 1, pos_end - pos - 1);
					bref.jrn.short_abbr = val.substr(0, pos);
					boost::trim(bref.jrn.short_abbr);
					bref.jrn.fname_abbr = val.substr(0, pos);
					boost::trim(bref.jrn.fname_abbr);
					boost::algorithm::to_lower(bref.jrn.fname_abbr);
				}
				else
				{
					bref.jrn.short_abbr = val;
					bref.jrn.fname_abbr = val;
					boost::algorithm::to_lower(bref.jrn.fname_abbr);
				}
			}
		}

		if (tag_new == "SE" )
		{
			if (bref.ref_type == BIB_REF_TYPE_IN_SERIES)
			{
				if (!bref.jrn.full_name.empty())
				{
					int len = bref.jrn.full_name.size();
					if (bref.jrn.full_name[len - 1] != '-') bref.jrn.full_name += " ";
				}
				bref.jrn.full_name += val;
			}
		}

		if (tag_new == "JO")
		{
			if (bref.ref_type == BIB_REF_TYPE_JOURNAL && !already_have_journal_name )
			{
				if (!bref.jrn.full_name.empty())
				{
					int len = bref.jrn.full_name.size();
					if (bref.jrn.full_name[len - 1] != '-') bref.jrn.full_name += " ";
				}
				bref.jrn.full_name += val;
			}
		}

		if (tag_new == "SN")
		{
			if (bref.ref_type == BIB_REF_PREPRINT)
			{
				bref.vol = val;
			}
			else
			{
				bref.jrn.issn = val;
			}
		}

		if (tag_new == "PB")
		{
			bref.jrn.publisher_str = val;
		}

		if (tag_new == "KW")
		{
			if (!bref.keywords_str.empty()) bref.keywords_str += ";";
			bref.keywords_str += val;
		}

		if (tag_new == "AB")
		{
			if (!bref.abstract_str.empty() && bref.abstract_str[bref.abstract_str.size() - 1] != '-') bref.abstract_str += " ";
			bref.abstract_str += val;
		}
		//if (tag_new == "N2")
		//{
		//	if (val.find("Abstract") == 0) val = val.substr(9);
		//	bref.abstract_str += val;
		//}

		long ltmp;

		if (val == "n/a") continue;
		if (tag_new == "SP") bref.first_page = val;
		if (tag_new == "EP") bref.last_page = val;
		if (tag_new == "AR") bref.first_page = val; // Article number for journals without page number 
		if (tag_new == "VL")
		{
			bref.vol = val;
		}
		if (tag_new == "IS") bref.iss = val;
		if (tag_new == "PY" || tag_new == "Y1" ) bref.pub_year = val;
		if (tag_new == "Y1")
		{
			pos = val.find("/");
			if (pos > 0)
			{
				bref.pub_year = val.substr(0, pos);
				bref.pub_month = val.substr(pos+1);
			}
			else
			{
				bref.pub_year = val;
			}
		}
		if (tag_new == "PD") bref.pub_month = val;

		if (tag_new == "JA") bref.jrn.short_abbr = val;
		if (tag_new == "J2") bref.jrn.std_abbr = val;
		if (tag_new == "J9") bref.jrn.short_abbr = val;
		if (tag_new == "GA") bref.ga_code = val;
		if (tag_new == "DO")
		{
			pos = val.find("doi.org");
			if( pos != std::string::npos ) val = val.substr(pos + 8);
			bref.doi = val;
		}
		if (tag_new == "L1")
		{
			pdf_loc_path_paperpile = val;
		}

		if (tag_new == "ER")
		{
			std::string  str;
			PrintRefFullStr(bref, str);
			wxLogMessage("Imported Reference Before Saving %s \n", wxString::FromUTF8(str.c_str()));

			int ref_id = GetRefID(bref);
			if (ref_id < 1)
			{
				ref_id = CreateNewRefDB();
			}
			else
			{
				wxLogMessage("Reference already in the Database, refid = %d\n", ref_id);
			}

			if (ref_id < 1) return FALSE; 
			bref.obj_id = ref_id;
			UpdateReferenceDB(bref);
			SetIncompleteFlagForRef(ref_id, FALSE);
			if (set_update_current) SetCurrentUpdateTime(ref_id);
			if (p_ref_info != NULL) // Set Reference info
			{
				if (!p_ref_info->keywords_str.empty()) SetObjKW(ref_id, p_ref_info->keywords_str);
				if (!p_ref_info->cited_in_ref.empty())
				{
					try
					{
						int cited_in_ref = boost::lexical_cast<int>(p_ref_info->cited_in_ref);
						this->SetCitation(cited_in_ref, ref_id);
					}
					catch (boost::bad_lexical_cast&)
					{
					}
				}
				if (!p_ref_info->cited_ref.empty())
				{
					try
					{
						int cited_ref = boost::lexical_cast<int>(p_ref_info->cited_ref);
						this->SetCitation(ref_id, cited_ref);
					}
					catch (boost::bad_lexical_cast&)
					{
					}
				}
			}
			bref.Clear();
			bref = GetRefByID(ref_id, GET_REF_FULL);
			PrintRefFullStr(bref, str);
			wxLogMessage("Imported Reference After Saving \n%s\n", wxString::FromUTF8(str));

			if (!pdf_loc_path_paperpile.empty())
			{
				std::string paperpile_dir = this->GetPaperpileDir();
				if (paperpile_dir.size() > 0)
				{
					std::string pdf_full_path_paperpile = paperpile_dir + pdf_loc_path_paperpile;
					wxLogMessage("Copy PDF File from Paperpile storage: \n %s \n", wxString::FromUTF8(pdf_full_path_paperpile));
					SaveRefPDF(bref, pdf_full_path_paperpile);
				}
				pdf_loc_path_paperpile = "";
			}


			tag_new = "";
			already_have_journal_name = false;
		}
		tag_old = tag_new;
	}
	return TRUE;
}

int BiblDB::ImportRefsNBIB(std::istream& stream, const BibRefInfo* p_ref_info)
{
	std::string str;
	std::string val;

	std::string tag_old = "";
	std::string tag_new = "";

	int incomplete_kw = FALSE;
	int incomplete_cit_ref = FALSE;

	BiblRef bref;
	std::vector<std::string> cited_refs_strs;
	int set_update_current = FALSE;

	bool already_have_journal_name = false;

	for (;;)
	{
		std::getline(stream, str);
		if (stream.eof()) return TRUE;

		tag_new = str.substr(0, 4);
		boost::trim(tag_new);
		boost::to_upper(tag_new);

		if (tag_new.empty()) tag_new = tag_old;

		if (str.size() < 6) continue;

		val = str.substr(6);
		boost::trim(val);

		if (tag_new == "PMID")
		{
			bref.Clear();
			cited_refs_strs.clear();
			set_update_current = FALSE;
			incomplete_kw = FALSE;
			tag_old = "TY";
			bref.pubmed_id = val;
		}

		if (tag_new == "PT")
		{
			if (val[0] == 'J') bref.ref_type = BIB_REF_TYPE_JOURNAL;
			if (val[0] == 'S') bref.ref_type = BIB_REF_TYPE_IN_SERIES;
		}

		if (tag_old.empty()) continue;

		size_t pos;
		std::vector<std::string> tokens;
		int i;

		if (tag_new == "FAU" )
		{
			boost::to_upper(val);
			AuthorRef auth_ref;
			pos = val.find(",");
			if (pos != std::string::npos) auth_ref.last_name = val.substr(0, pos);
			std::string s2 = val.substr(pos + 1);
			boost::trim(s2);
			boost::split(tokens, s2, boost::is_any_of(" "));
			if (tokens.size() == 0) continue;

			auth_ref.first_name = tokens[0];
			auth_ref.initials = auth_ref.first_name[0];

			if (tokens.size() > 1)
			{
				for (i = 1; i < tokens.size(); i++)
				{
					auth_ref.initials += tokens[i][0];
				}
			}

			bref.auth_vec.push_back(auth_ref);
		}

		if (tag_new == "TI" )
		{
			if (!bref.title.empty() && bref.title[bref.title.size() - 1] != '-') bref.title += " ";
			bref.title += val;
		}

		if (tag_new == "JT")
		{
			if (bref.ref_type == BIB_REF_TYPE_JOURNAL && !already_have_journal_name)
			{
				if (!bref.jrn.full_name.empty())
				{
					int len = bref.jrn.full_name.size();
					if (bref.jrn.full_name[len - 1] != '-') bref.jrn.full_name += " ";
				}
				bref.jrn.full_name += val;
			}
		}
		if (tag_new == "TA") bref.jrn.short_abbr = val;

		if (tag_new == "SN")
		{
			bref.jrn.issn = val;
			boost::trim(bref.jrn.issn);
		}

		if (tag_new == "PB")
		{
			bref.jrn.publisher_str = val;
			boost::trim(bref.jrn.publisher_str);
		}

		if (tag_new == "MH")
		{
			if (!bref.keywords_str.empty()) bref.keywords_str += ";";
			bref.keywords_str += val;
		}

		if (tag_new == "AB")
		{
			if (!bref.abstract_str.empty() && bref.abstract_str[bref.abstract_str.size() - 1] != '-') bref.abstract_str += " ";
			bref.abstract_str += val;
		}

		long ltmp;

		if (val == "n/a") continue;
		if (tag_new == "PG")
		{
			boost::split(tokens, val, boost::is_any_of("-"));
			if(tokens.size() > 0) bref.first_page = tokens[0];
			if(tokens.size() > 1) bref.last_page = tokens[0];
		}
		if (tag_new == "VI")
		{
			bref.vol = val;
		}
		if (tag_new == "IP") bref.iss = val;
		if (tag_new == "DP")
		{
			boost::split(tokens, val, boost::is_any_of(" "));
			if (tokens.size() > 0)
			{
				boost::trim(tokens[0]);
				bref.pub_year = tokens[0];
			}
		}
		if (tag_new == "PD") bref.pub_month = val;

		if (tag_new == "LID")
		{
			pos = val.find("[doi]");
			if (val.find("[doi]") != std::string::npos)
			{
				val = val.substr(0, pos);
				boost::trim(val);

				pos = val.find("doi.org");
				if (pos != std::string::npos) val = val.substr(pos + 8);
				bref.doi = val;
			}
		}

		if (tag_new == "SO")
		{
			std::string  str;
			PrintRefFullStr(bref, str);
			wxLogMessage("Imported Reference Before Saving %s \n", wxString::FromUTF8(str.c_str()));

			int ref_id = GetRefID(bref);
			if (ref_id < 1)
			{
				ref_id = CreateNewRefDB();
			}
			else
			{
				wxLogMessage("Reference already in the Database, refid = %d\n", ref_id);
			}

			if (ref_id < 1) return FALSE;
			bref.obj_id = ref_id;
			UpdateReferenceDB(bref);
			SetIncompleteFlagForRef(ref_id, FALSE);
			if (set_update_current) SetCurrentUpdateTime(ref_id);
			if (p_ref_info != NULL) // Set Reference info
			{
				if (!p_ref_info->keywords_str.empty()) SetObjKW(ref_id, p_ref_info->keywords_str);
				if (!p_ref_info->cited_in_ref.empty())
				{
					try
					{
						int cited_in_ref = boost::lexical_cast<int>(p_ref_info->cited_in_ref);
						this->SetCitation(cited_in_ref, ref_id);
					}
					catch (boost::bad_lexical_cast&)
					{
					}
				}
				if (!p_ref_info->cited_ref.empty())
				{
					try
					{
						int cited_ref = boost::lexical_cast<int>(p_ref_info->cited_ref);
						this->SetCitation(ref_id, cited_ref);
					}
					catch (boost::bad_lexical_cast&)
					{
					}
				}
			}
			bref.Clear();
			bref = GetRefByID(ref_id, GET_REF_FULL);
			PrintRefFullStr(bref, str);
			wxLogMessage("Imported Reference After Saving \n%s\n", wxString::FromUTF8(str));

			tag_new = "";
			already_have_journal_name = false;
		}
		tag_old = tag_new;
	}
	return TRUE;
}



int BiblDB::GotoWOSSearchPage()
{
#if defined(WXHTTPENGINE) 

	if(wos_sid.empty()) this->InitWOS();

	wxHTTPBuilder* m_http = new wxHTTPBuilder();
	m_http->InitContentTypes(); // Initialise the content types on the page

//	m_http->ClearHeaders();
	m_http->SetValue( "Func", "Search", wxHTTPBuilder::wxHTTP_TYPE_GET );
	m_http->SetValue( "Form", "HomePage",   wxHTTPBuilder::wxHTTP_TYPE_GET );
	m_http->SetValue( "SID",   wos_sid, wxHTTPBuilder::wxHTTP_TYPE_GET );

	m_http->SetValue( "General Search.x", "10", wxHTTPBuilder::wxHTTP_TYPE_GET);
	m_http->SetValue( "General Search.y", "10", wxHTTPBuilder::wxHTTP_TYPE_GET);

	wxString out = m_http->GetInputString(wos_cgi);

	wxLogMessage("String From Server: \n %s \n\n",out.c_str());

	wxString resp = m_http->GetResponseString();
	wxLogMessage("Response String: \n %s \n\n",resp.c_str());

    delete m_http;
    m_http = NULL;

	wxString go_str = wos_cgi;
	go_str += "?";
	go_str += "Func=";
	go_str += "Search";
	go_str += "&Form=";
	go_str += "HomePage";
	go_str += "&SID=";
	go_str += wos_sid;
	go_str += "&General+Search.x=";
	go_str += "10";
	go_str += "&General+Search.y=";
	go_str += "10";
	
//	go_str = wxHTTPBuilder::URLEncode(go_str);

	wxLaunchDefaultBrowser(go_str);

#endif

	return TRUE;

}

int BiblDB::GotoWOSRef(BiblRef& ref)
{
	if(wos_sid.empty()) InitWOS();
		
	wxString tmp;

	wxString go_str =  wos_cgi;
	go_str += "?";
	go_str += "SID=";
	go_str += wos_sid;

	if(!ref.isi_id.empty())
	{
//		go_str += "&Func=DispCitingRec&UT=";
		go_str += "&product=WOS&UT=";
		go_str += ref.isi_id.c_str();
		go_str += "&SrcApp=EndNote&Init=Yes&action=retrieve&SrcAuth=ResearchSoft&Func=Frame&customersID=ResearchSoft&IsProductCode=Yes&mode=FullRecord";
		wxLogMessage("HTTP request: \n %s \n\n",go_str.c_str());
	}
	else
	{
		return FALSE;
	}

	wxLaunchDefaultBrowser(go_str);

	return TRUE;
}

int BiblDB::GotoWOSMarkedList()
{
	if (!this->webdriver_flag) this->InitWebDriver();
	pwos->DelWosSaveFile();
	std::string cmd = std::string("wos.open_marked_list()");
	RunPythonScriptInString(cmd);

	return TRUE;
}

int BiblDB::GotoWOSRefDriver(BiblRef& ref)
{
	if (!this->webdriver_flag) this->InitWebDriver();
	pwos->DelWosSaveFile();
	if (!ref.isi_id.empty())
	{
		std::string cmd = std::string("wos.open_ref(") + "\"" + ref.isi_id + "\"" + ")";
		RunPythonScriptInString(cmd);
	}
	else
	{
		wxLogMessage("Reference does not have isi_id set");
	}
	return TRUE;
}

int BiblDB::GotoWOSCitedRefDriver(BiblRef& ref)
{
	if (!this->webdriver_flag) this->InitWebDriver();
	pwos->DelWosSaveFile();
	if (!ref.isi_id.empty())
	{
		std::string cmd = std::string("wos.open_ref_cited(") + "\"" + ref.isi_id + "\"" + ")";
		RunPythonScriptInString(cmd);
	}
	else
	{
		wxLogMessage("Reference does not have isi_id set");
	}
	return TRUE;
}

int BiblDB::GotoGScholarRefDriver(BiblRef& ref)
{
	if (!this->webdriver_flag) this->InitWebDriver();
	std::string cmd;
	if (!ref.title.empty())
	{
		cmd = std::string("gs_m.open_main_page()");
		RunPythonScriptInString(cmd);
		if (ref.doi.size() > 5)
			cmd = std::string("gs_m.find_ref_by_doi(") + "\"" + ref.doi + "\"" + ")";
		else if (ref.title.size() > 5)
			cmd = std::string("gs_m.find_ref_by_title(") + "\"" + ref.title + "\"" + ")";
		else
			return FALSE;
		RunPythonScriptInString(cmd);
		cmd = std::string("import biblpy");
		RunPythonScriptInString(cmd);
		cmd = std::string("dlg_gs = biblpy.GSRefDlg(gs_m = gs_m)");
		RunPythonScriptInString(cmd);
		cmd = "dlg_gs.Show()";
		RunPythonScriptInString(cmd);
	}
	else
	{
		wxLogMessage("Reference does not have title set - can not find it on Google Scholar");
	}
	return TRUE;
}

int BiblDB::GotoWOSCitingRefDriver(BiblRef& ref)
{
	if (!this->webdriver_flag) this->InitWebDriver();
	pwos->DelWosSaveFile();
	if (!ref.isi_id.empty())
	{
		std::string cmd = std::string("wos.open_ref_citing(") + "\"" + ref.isi_id + "\"" + ")";
		RunPythonScriptInString(cmd);
	}
	else
	{
		wxLogMessage("Reference does not have isi_id set");
	}
	return TRUE;
}

int BiblDB::GotoWOSCitedRef2(BiblRef& ref)
{
	if(wos_sid.empty()) InitWOS();

	wxString url;
	url="http://apps.isiknowledge.com/CitedRefList.do?product=UA&search_mode=CitedRefList&";
	url += wxString::Format("SID=%s&db_id=WOS&", wos_sid.c_str());

	if(ref.isi_id_int > 0)
	{
		url +=  wxString::Format("isickref=%d",ref.isi_id_int);
	}
	else if(!ref.isi_id.empty())
	{
		url += wxString::Format("UT=%s",ref.isi_id.c_str());
	}
	else
	{
		return FALSE;
	}

	wxLogMessage("Cited References URL: %s\n",url.c_str());

	wxLaunchDefaultBrowser(url.c_str());

	return TRUE;
}

int BiblDB::GotoWOSCitingRef(BiblRef& ref)
{
	if(wos_sid.empty()) InitWOS();

	wxString url;
	url="http://apps.isiknowledge.com/CitingArticles.do?product=WOS&search_mode=CitingArticles&";
	url += wxString::Format("SID=%s&db_id=WOS&", wos_sid.c_str());

	if(ref.isi_id_int > 0)
	{
		url +=  wxString::Format("isickref=%d",ref.isi_id_int);
	}
	else if(!ref.isi_id.empty())
	{
		url += wxString::Format("UT=%s",ref.isi_id.c_str());
	}
	else
	{
		return FALSE;
	}

	wxLogMessage("Citing References URL: %s\n",url.c_str());

	wxLaunchDefaultBrowser(url.c_str());

	return TRUE;
}


int BiblDB::GotoWOSRef2(BiblRef& ref)
{
	if(wos_sid.empty()) InitWOS();
		
	wxString tmp;

	wxString go_str =  wos_cgi_2;
	go_str += "full_record.do?product=UA&db_id=WOS&";
	go_str += "SID=";
	go_str += wos_sid;

	if(ref.isi_id_int > 0)
	{
		go_str += "&search_mode=CitedFullRecord&isickref=";
		go_str += wxString::Format("%d",ref.isi_id_int);
	}
	else if(!ref.isi_id.empty())
	{
		go_str += "&Func=DispCitingRec&UT=";
		go_str += ref.isi_id.c_str();
	}
	else
	{
		return FALSE;
	}

	wxLaunchDefaultBrowser(go_str);

	return TRUE;
}
