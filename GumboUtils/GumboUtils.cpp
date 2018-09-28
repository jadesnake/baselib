#include "stdafx.h"
#include "GumboUtils.h"

namespace GumboUtils
{
	std::string QueryNodeAttr(GumboNode *node,const char* attrName)
	{
		std::string ret;
		if(node==NULL)	return ret;
		if(node->type==GUMBO_NODE_WHITESPACE)	return ret;
		if(node->v.element.attributes.length==0) return ret;
		GumboAttribute* class_attr = gumbo_get_attribute(&node->v.element.attributes,attrName);
		if(class_attr==NULL)	return ret;
		ret = class_attr->value;
		return ret;
	}
	GumboNode*  QueryNodeById(GumboNode *node,const char* id)
	{
		if(node==NULL) return NULL;
		if(node->type!=GUMBO_NODE_ELEMENT)	return NULL;
		GumboAttribute* class_attr = gumbo_get_attribute(&node->v.element.attributes,"id");
		if(class_attr)
		{
			CAtlStringA trimA(class_attr->value),trimB(id);
			trimA.Trim();trimB.Trim();
			if(trimA==trimB)
			{
				return node;
			}
		}
		GumboVector* div_children = &node->v.element.children;
		for(int i=0;i<div_children->length;i++) 
		{
			GumboNode* child = (GumboNode*)div_children->data[i];
			GumboNode* ret = QueryNodeById(child,id);
			if(ret)
				return ret;
		}
		return NULL;
	}
	size_t  QueryNodeByName(GumboNode *node,const char* name,std::vector<GumboNode*>& out)
	{
		if(node==NULL) return NULL;
		if(node->type!=GUMBO_NODE_ELEMENT)	return out.size();
		std::string nameAttr = QueryNodeAttr(node,"name");
		if(nameAttr==name && !nameAttr.empty())
		{
			out.push_back(node);
		}
		GumboVector* div_children = &node->v.element.children;
		for(int i=0;i<div_children->length;i++) 
		{
			GumboNode* child = (GumboNode*)div_children->data[i];
			QueryNodeByName(child,name,out);     
		}
		return out.size();
	}
	size_t  QueryNodeByText(std::vector<GumboNode*> &out,GumboNode *node,GumboTag tag,const char* pText,const char* pClass,const char* pStyle)
	{
		if(node==NULL)	return out.size();
		if(node->type!=GUMBO_NODE_ELEMENT)	return out.size();
		if(node->v.element.tag==tag)
		{
			int nA=0,nB=0;
			if(pClass) nA+=1;
			if(pStyle) nA+=1;
			if(pText)  nA+=1;
			GumboAttribute* class_attr = gumbo_get_attribute(&node->v.element.attributes,"class");
			GumboAttribute* style_attr = gumbo_get_attribute(&node->v.element.attributes,"style");
			if(pClass && class_attr)
			{
				CAtlStringA trimA(class_attr->value),trimB(pClass);
				trimA = trimA.Trim();
				trimB = trimB.Trim();
				if(0==trimA.CompareNoCase(trimB))
					nB += 1;
			}
			if(pStyle && style_attr)
			{
				CAtlStringA trimA(style_attr->value),trimB(pStyle);
				trimA = trimA.Trim();
				trimB = trimB.Trim();
				if(0==trimA.CompareNoCase(trimB))
					nB += 1;
			}
			if(pText&&node->v.element.children.length)
			{
				GumboNode* txtNode = (GumboNode*)node->v.element.children.data[0];
				if(txtNode->type==GUMBO_NODE_TEXT)
				{
#ifdef _DEBUG
					OutputDebugStringA("\n");
					OutputDebugString(CA2CT(txtNode->v.text.text,CP_UTF8));
					OutputDebugStringA("\n");
#endif
					if(strstr(txtNode->v.text.text,pText))
						nB += 1;
				}
			}
			if(nA==nB)
			{
				out.push_back(node);
				return out.size();
			}
		}
		GumboVector* div_children = &node->v.element.children;
		for(int i=0;i<div_children->length;i++) 
		{
			GumboNode* child = (GumboNode*)div_children->data[i];
			QueryNodeByText(out,child,tag,pText,pClass,pStyle);
		}
		return out.size();
	}
	size_t QueryFromBody(std::vector<GumboNode*> &out,GumboNode *node,GumboTag tag,const char* pClass,const char* pStyle,int tarLv,int curLv)
	{
		if(node==NULL)
			return out.size();
		if(node->type!=GUMBO_NODE_ELEMENT)	return out.size();
		if(node->v.element.tag==tag)
		{
			int nA=0,nB=0;
			if(pClass) nA+=1;
			if(pStyle) nA+=1;
			GumboAttribute* class_attr = gumbo_get_attribute(&node->v.element.attributes,"class");
			GumboAttribute* style_attr = gumbo_get_attribute(&node->v.element.attributes,"style");
			if(pClass && class_attr)
			{
				CAtlStringA trimA(class_attr->value),trimB(pClass);
				trimA = trimA.Trim();
				trimB = trimB.Trim();
				if(0==trimA.CompareNoCase(trimB))
					nB += 1;
			}
			if(pStyle && style_attr)
			{
				CAtlStringA trimA(style_attr->value),trimB(pStyle);
				trimA = trimA.Trim();
				trimB = trimB.Trim();
				if(0==trimA.CompareNoCase(trimB))
					nB += 1;
			}
			if(nA==nB)
			{
				out.push_back(node);
				return out.size();
			}
		}
		if(curLv==tarLv && tarLv!=-1)
			return out.size();
		curLv += 1;
		GumboVector* div_children = &node->v.element.children;
		for(int i=0;i<div_children->length;i++) 
		{
			GumboNode* child = (GumboNode*)div_children->data[i];
			QueryFromBody(out,child,tag,pClass,pStyle,tarLv,curLv);
		}
		return out.size();
	}
	GumboNode* RollbackByTag(GumboNode *node,GumboTag tag,const char* pClass,const char* pStyle)
	{
		GumboNode* ret = NULL;
		if(node==NULL)	return ret;
		if(node->v.element.tag==tag)
		{
			int nA=0,nB=0;
			if(pClass) nA+=1;
			if(pStyle) nA+=1;
			GumboAttribute* class_attr = gumbo_get_attribute(&node->v.element.attributes,"class");
			GumboAttribute* style_attr = gumbo_get_attribute(&node->v.element.attributes,"style");
			if(pClass && class_attr)
			{
				CAtlStringA trimA(class_attr->value),trimB(pClass);
				trimA = trimA.Trim();
				trimB = trimB.Trim();
				if(0==trimA.CompareNoCase(trimB))
					nB += 1;
			}
			if(pStyle && style_attr)
			{
				CAtlStringA trimA(style_attr->value),trimB(pStyle);
				trimA = trimA.Trim();
				trimB = trimB.Trim();
				if(0==trimA.CompareNoCase(trimB))
					nB += 1;
			}
			if(nA==nB)
			{
				ret = node;
				return ret;
			}
		}
		if(node->parent)
			ret = RollbackByTag(node->parent,tag,pClass,pStyle);
		return ret;
	}

	std::vector<Node> Node::Convert(std::vector<GumboNode*>& inSrc)
	{
		std::vector<Node> ret;
		for(size_t t=0;t<inSrc.size();t++)
			ret.push_back( Node(inSrc[t]) );
		return ret;
	}
	Node::Node()
		:m_Node(NULL)
	{

	}
	Node::Node(GumboNode *node)
		:m_Node(node)
	{

	}
	void Node::Set(GumboNode *node)
	{
		m_Node = node;
	}
	bool Node::empty()
	{
		return (m_Node==NULL);
	}
	std::string Node::val()
	{
		return other("value");
	}
	std::string Node::name()
	{
		return other("name");
	}
	std::string Node::id()
	{
		return other("id");
	}
	std::string Node::other(const char* attr)
	{
		return QueryNodeAttr(m_Node,attr);
	}
	GumboNode* Node::Gumbo()
	{
		return m_Node;
	}
	Node Node::QueryById(const char* id)
	{
		return Node(QueryNodeById(m_Node,id));
	}
	std::vector<Node> Node::QueryByName(const char* name)
	{
		std::vector<GumboNode*> out;
		QueryNodeByName(m_Node,name,out);
		return Node::Convert(out);
	}
	std::vector<Node> Node::QueryByText(const char* pText,GumboTag tag,const char* pClass,const char* pStyle)
	{
		std::vector<GumboNode*> out;
		QueryNodeByText(out,m_Node,tag,pText,pClass,pStyle);
		return Node::Convert(out);
	}
	std::vector<Node> Node::QueryFromBody(GumboTag tag,const char* pClass,const char* pStyle,int tarLv,int curLv)
	{
		std::vector<GumboNode*> ret;
		GumboUtils::QueryFromBody(ret,m_Node,tag,pClass,pStyle,tarLv,curLv);
		return Node::Convert(ret);
	}
	
	Wapper::Wapper()
		:body(NULL),head(NULL),outDoc(NULL),curNode(NULL) 
	{

	}
	Wapper::Wapper(const std::string& html)
		:body(NULL),head(NULL),outDoc(NULL),curNode(NULL) 
	{
		Parse(html);
	}
	Wapper::~Wapper()
	{
		if(outDoc)
			gumbo_destroy_output(&kGumboDefaultOptions,outDoc);
	}
	bool Wapper::IsSuc()
	{
		return (body&&head);
	}
	void Wapper::SetCurNode(GumboNode *node)
	{
		curNode = node;
	}
	void Wapper::SetCurNode(const Node &node)
	{
		curNode = node.m_Node;
	}
	bool Wapper::Parse(const std::string& html)
	{
		outDoc = gumbo_parse_with_options(&kGumboDefaultOptions,html.c_str(),html.size());	
		if(outDoc==NULL) return false;
		const GumboVector* root_children = &outDoc->root->v.element.children;
		for(int i = 0; i < root_children->length; ++i) 
		{
			GumboNode* child = (GumboNode*)root_children->data[i];
			if(child->type == GUMBO_NODE_ELEMENT&&child->v.element.tag==GUMBO_TAG_BODY)
			{
				body = child;
			}
			else if(child->type == GUMBO_NODE_ELEMENT&&child->v.element.tag==GUMBO_TAG_HEAD)
			{
				head = child;
			}
			if(body&&head)	break;
		}
		return (body&&head);
	}
	std::vector<std::string> Wapper::QueryScripts()
	{
		GumboNode *node = (curNode==NULL?body:curNode);
		std::vector<std::string> ret;
		if(node)
		{
			GumboVector* children = &node->v.element.children;
			for(int i=0;i<children->length;++i) 
			{
				GumboNode* child = (GumboNode*)children->data[i];
				if(child->type==GUMBO_NODE_WHITESPACE)	continue;
				if(child->v.element.tag==GUMBO_TAG_SCRIPT && child->v.element.children.length ) 
				{
					GumboNode* text = (GumboNode*)child->v.element.children.data[0];
					if(text->type==GUMBO_NODE_TEXT && text->v.text.original_text.length)
						ret.push_back(text->v.text.text);
				}
			}
		}
		return ret;
	}
	Node Wapper::QueryById(const char* id)
	{
		Node ret;
		if(curNode==NULL)
			ret.Set(QueryNodeById(body,id));
		else
			ret.Set(QueryNodeById(curNode,id));
		return ret;
	}
	std::vector<Node> Wapper::QueryFromBody(GumboTag tag,const char* pClass,const char* pStyle,int tarLv,int curLv)
	{
		std::vector<GumboNode*> ret;
		if(curNode==NULL)
			curNode = body;
		GumboUtils::QueryFromBody(ret,curNode,tag,pClass,pStyle,tarLv,curLv);
		return Node::Convert(ret);
	}
	std::vector<Node> Wapper::QueryByName(const char* name)
	{
		std::vector<GumboNode*> results;
		if(curNode==NULL)
			QueryNodeByName(body,name,results);
		else
			QueryNodeByName(curNode,name,results);
		return Node::Convert(results);
	}
	std::vector<Node> Wapper::QueryByText(const char* pText,GumboTag tag,const char* pClass,const char* pStyle)
	{
		std::vector<GumboNode*> results;
		if(curNode==NULL)
			QueryNodeByText(results,body,tag,pText,pClass,pStyle);
		else
			QueryNodeByText(results,curNode,tag,pText,pClass,pStyle);
		return Node::Convert(results);
	}
}