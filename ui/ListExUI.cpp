#include "StdAfx.h"
#include "ListExUI.h"

namespace CustomUI
{
	ListUI::ListUI(void)
	{

	}
	void ListUI::SetPos(RECT rc)
	{
		__super::SetPos(rc);
		RECT rcBody = m_pList->GetPos();
		m_pList->SetPos(rcBody);
	}
}