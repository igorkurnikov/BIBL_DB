#pragma once
#include <string>

class CWos
//! class to handle interaction with Web of Science website
{
	public:

	CWos();
	virtual ~CWos();

	void DelWosSaveFile();  //!< delete file for Saving Records from Web of Science website

	std::string wos_save_fname; //!< name of the file Web of Science save records

};