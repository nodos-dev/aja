/* SPDX-License-Identifier: MIT */
/*
  This software is provided by AJA Video, Inc. "AS IS"
  with no express or implied warranties.
*/

#include "errorList.h"

CErrorList::CErrorList()
{
	
}

CErrorList::~CErrorList()
{
	
}

void CErrorList::Error(const std::string& message)
{
	_errors.push_back(message);
}

void CErrorList::Acquire(CErrorList& errorList)
{
	_errors.insert(_errors.end(),
		errorList._errors.begin(), errorList._errors.end());
	
	errorList._errors.clear();
}

std::string CErrorList::GetErrorMessage() const
{
	std::string result;
	for(std::list<std::string>::const_iterator itr = _errors.begin();
		itr != _errors.end();
		itr++)
	{
		result += "error: " + (*itr) + "\n";
	}
	
	return result;
}

void CErrorList::Clear()
{
	_errors.clear();
}

