#include "stdafx.h"
#include "xmlutils.h"
namespace xmlutils
{
	tinyxml2::XMLElement *QueryChildElement(tinyxml2::XMLElement *root,const CAtlString &ElementName)
	{
		tinyxml2::XMLElement *itemRet = NULL;
		std::string strName= CT2CA(ElementName); 
		tinyxml2::XMLNode *node = root->FirstChild();
		while(node)
		{
			tinyxml2::XMLElement *item  = node->ToElement();
			if(item)
			{
				std::string curName = item->Name();
				if(curName == strName)
				{
					itemRet = item;
					break;
				}
			}
			node = node->NextSibling();
		}	
		if(itemRet==NULL && root->FirstChild())
		{
			tinyxml2::XMLElement *item = root->FirstChild()->ToElement();
			if(item)
				itemRet = QueryChildElement(item,ElementName);
		}
		return itemRet;
	}

	CAtlString GetChildTxT(tinyxml2::XMLElement *root,const CAtlString &ElementName,UINT pagecode)
	{
		CAtlString ret;
		tinyxml2::XMLElement *item = QueryChildElement(root,ElementName);
		if(item==NULL)
			return ret;
		ret = (TCHAR*)CA2CT(item->GetText(),pagecode);
		return ret;
	}
}
