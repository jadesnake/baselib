#pragma once

#include <string>
#include <map>
#include "tinyxml.h"

using namespace std;

class TiXmlDocumentEx :
	public TiXmlDocument
{
public:
	TiXmlDocumentEx(void);
	~TiXmlDocumentEx(void);
	bool GetNodePointerByName(TiXmlElement* pRootEle, string strNodeName,TiXmlElement* &Node);
	bool QueryNode_Text(string strNodeName, string &strText);
	bool QueryNode_Attribute(string strNodeName, map<string, string> &AttMap);
	bool DelNode(string strNodeName);
};
