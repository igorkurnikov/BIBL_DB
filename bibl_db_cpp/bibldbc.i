%module bibldbc

%begin %{
#include <Python.h>
%}

%include typemaps.i            
%include cpointer.i                      
%include stl.i                                           
%include std_iostream.i                                
%include file.i           
%include "std_string.i"
//%include "std_vector.i"
%include "std_list.i"
%include "std_map.i"

namespace std {
//    %template(IntVector) vector<int>;
//    %template(FloatVector) vector<float>;
//	%template(DoubleVector) vector<double>;
	%template(StringVector) vector<string>;
}

%{ 
#include "cwos.h"
#include "DataObject.h"
#include "bibldb.h"
%}

%include "cwos.h"
%include "DataObject.h"
%include "bibldb.h"

