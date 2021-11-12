#include "StdAfx.h"

#include "TiXmlDocEx.h"

TiXmlDocumentEx::TiXmlDocumentEx(void)
{
}

TiXmlDocumentEx::~TiXmlDocumentEx(void)
{
}
bool TiXmlDocumentEx::GetNodePointerByName(TiXmlElement* pRootEle, string strNodeName,TiXmlElement* &Node)   
{   
	// 假如等于根节点名，就退出   
	if (strNodeName==pRootEle->Value())   
	{   
		Node = pRootEle;   
		return true;   
	}   
	TiXmlElement* pEle = pRootEle;     
	for (pEle = pRootEle->FirstChildElement(); pEle; pEle = pEle->NextSiblingElement())     
	{     
		//递归处理子节点，获取节点指针   
		if(GetNodePointerByName(pEle,strNodeName,Node))   
			return true;   
	}     
	return false;   
}  
bool TiXmlDocumentEx::QueryNode_Text(string strNodeName, string &strText)   
{   

	TiXmlElement *pRootEle = RootElement();   
	if (NULL==pRootEle)   
	{   
		return false;   
	}   
	TiXmlElement *pNode = NULL;   
	GetNodePointerByName(pRootEle,strNodeName,pNode);   
	if (NULL!=pNode)   
	{   
		const char *pText = pNode->GetText();
		if(pText)
		{
			strText = pText;
		}
		else
		{
			strText = "";
		}
		
		return true;   
	}   
	else  
	{   
		return false;   
	}   

}   

bool TiXmlDocumentEx::QueryNode_Attribute(string strNodeName, map<string, string> &AttMap)   
{   
	TiXmlElement *pNode = NULL;   
	GetNodePointerByName(RootElement(), strNodeName,pNode);   
	if (NULL!=pNode)   
	{   
		TiXmlAttribute* pAttr = NULL;    
		for (pAttr = pNode->FirstAttribute(); pAttr; pAttr = pAttr->Next())     
		{     
			std::string strAttName = pAttr->Name();   
			std::string strAttValue = pAttr->Value();   
			AttMap.insert(map<string, string>::value_type(strAttName,strAttValue));   
		}     
		return true;   
	}   
	else  
	{   
		return false;   
	}   
	return true;   
} 


bool TiXmlDocumentEx::DelNode(string strNodeName)   
{   

	TiXmlElement *pRootEle = RootElement();   
	if (NULL==pRootEle)   
	{   
		return false;   
	}   
	TiXmlElement *pNode = NULL;   
	GetNodePointerByName(RootElement(), strNodeName, pNode);   
	// 假如是根节点   
	if (pRootEle==pNode)   
	{   
		if(RemoveChild(pRootEle))   
		{   
			SaveFile();
			return true;   
		}   
		else    
			return false;   
	}   
	// 假如是其它节点   
	if (NULL!=pNode)   
	{   
		TiXmlNode *pParNode =  pNode->Parent();   
		if (NULL==pParNode)   
		{   
			return false;   
		}   

		TiXmlElement* pParentEle = pParNode->ToElement();   
		if (NULL!=pParentEle)   
		{   
			if(pParentEle->RemoveChild(pNode))   
				SaveFile();   
			else  
				return false;   
		}   
	}   
	else  
	{   
		return false;   
	}   
	return false;   
} 