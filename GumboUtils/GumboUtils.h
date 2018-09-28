#pragma once
#include "gumbo-parser/src/gumbo.h"
#include <vector>
namespace GumboUtils
{
	class Wapper;
	class Node
	{
	public:
		std::string val();
		std::string name();
		std::string id();
		std::string other(const char* attr);
		bool empty();
		GumboNode* Gumbo();
		Node QueryById(const char* id);
		std::vector<Node> QueryByName(const char* name);
		std::vector<Node> QueryByText(const char* pText,GumboTag tag,const char* pClass,const char* pStyle);
		std::vector<Node> QueryFromBody(GumboTag tag,const char* pClass,const char* pStyle,int tarLv,int curLv=0);
	protected:
		Node();
		Node(GumboNode *node);
		void Set(GumboNode *node);
		static std::vector<Node> Convert(std::vector<GumboNode*>& inSrc);
	protected:
		GumboNode *m_Node;
		friend Wapper;
	};

	class Wapper
	{
	public:
		Wapper();
		Wapper(const std::string& html);
		~Wapper();
		bool IsSuc();
		bool Parse(const std::string& html);
		void SetCurNode(GumboNode *node);
		void SetCurNode(const Node &node);
		Node QueryById(const char* id);
		std::vector<Node> QueryByName(const char* name);
		std::vector<Node> QueryByText(const char* pText,GumboTag tag,const char* pClass,const char* pStyle);
		std::vector<std::string> QueryScripts();
		//tarLv ÷’÷π µ›πÈ≤„
		//curLv µ±«∞ µ›πÈ≤„
		std::vector<Node> QueryFromBody(GumboTag tag,const char* pClass,const char* pStyle,int tarLv,int curLv=0);
 	public:
		GumboNode *body;
		GumboNode *head;
	protected:
		GumboOutput *outDoc;
		GumboNode *curNode;
	};
	//tarLv ÷’÷π µ›πÈ≤„
	//curLv µ±«∞ µ›πÈ≤„
	GumboNode* RollbackByTag(GumboNode *node,GumboTag tag,const char* pClass,const char* pStyle);
	//ªÒ»°node Ù–‘
	std::string QueryNodeAttr(GumboNode *node,const char* attrName);
}