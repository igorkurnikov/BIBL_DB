//
// PYTHON dependent functions
//
#include <string>
#include <vector>
#include <set>

#include <boost/filesystem.hpp>

#include "Python.h"
#include "bibldb.h"

extern "C" {
#if PY_VERSION_HEX >= 0x03000000
	extern PyObject* PyInit__bibldbc();
#else
	extern void init_bibldbc();
#endif
}

int BiblDB::InitPython()
{
	char buf[256]; 

	wchar_t* prog_name = Py_DecodeLocale("BIBLDB", NULL);
	Py_SetProgramName(prog_name);
	Py_Initialize();

	boost::filesystem::path full_path(boost::filesystem::current_path());
	std::string cur_path = full_path.string();

	int ires = PyRun_SimpleString("import sys");
	ires = PyRun_SimpleString("sys.path.append(\".\")");
	ires = PyRun_SimpleString("import biblpy");

	return 1;
}