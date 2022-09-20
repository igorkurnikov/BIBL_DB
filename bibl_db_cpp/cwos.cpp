//! class to handle interaction with Web of Science website  (implementation)
#include "cwos.h"
#include <stdio.h>
#include <istream>


CWos::CWos() 
{ 
	wos_save_fname = "C:\\Users\\igor\\Downloads\\savedrecs.ciw"; 
}

CWos::~CWos() 
{
	
}

void CWos::DelWosSaveFile()
{
	FILE* frec = fopen(wos_save_fname.c_str(), "r");
	if (frec)
	{
		fclose(frec);
		remove(wos_save_fname.c_str());
	}
}