#pragma once

#include <WinCrypt.h>

namespace encrypt
{
	typedef enum 
	{
		BIN_BASE64,
		BIN_HEX
	}BIN_TYPE;
}
// 计算Hash，成功返回0，失败返回GetLastError()
//  CONST BYTE *pbData, 输入数据
//  DWORD dwDataLen,    输入数据字节长度
//  ALG_ID algId        Hash 算法：CALG_MD5,CALG_SHA
DWORD  GetHash(BYTE *pbData, DWORD dwDataLen, ALG_ID algId,CAtlStringA &out);
