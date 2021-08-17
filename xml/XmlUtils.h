#pragma once
#include "tinyxml2.h"
#include <string>
#include <atlstr.h>
namespace xmlutils
{
	tinyxml2::XMLElement *QueryChildElement(tinyxml2::XMLElement *root,const CAtlString &ElementName);

	CAtlString GetChildTxT(tinyxml2::XMLElement *root,const CAtlString &ElementName,UINT pagecode=CP_ACP);
}