/* SPDX-License-Identifier: MIT */
/*
  This software is provided by AJA Video, Inc. "AS IS"
  with no express or implied warranties.
*/

#ifndef _ERRORLIST_H
#define _ERRORLIST_H

#include <string>
#include <list>
#include "export_gpu.h"




//class CErrorList;

class CErrorList
{
public:
	GPU_EXPORT CErrorList();
	GPU_EXPORT virtual ~CErrorList();
	
	GPU_EXPORT void Error(const std::string& message);
	GPU_EXPORT void Acquire(CErrorList& errorList);
	
	GPU_EXPORT std::string GetErrorMessage() const;
	GPU_EXPORT void Clear();

private:
	std::list<std::string> _errors;
};

#endif

