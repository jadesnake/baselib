#include "stdafx.h"
#include "encrypt.h"
#include "WinAES.h"
#include "base64.h"

void encodeHex(const char *data,DWORD dlen,CAtlStringA &to) 
{
	char *toDigits = "0123456789abcdef";
	int l = dlen;
	char *out = new char[l << 1];
	int i = 0,j = 0;
	for( ; i < l; i++)
	{
		out[j++] = toDigits[(0xF0 & data[i]) >> 4];
		out[j++] = toDigits[0x0F & data[i]];
	}
	to.Empty();
	to.Append(out,j);
	delete [] out;
	return ;
}

byte nibbleFromChar(char c)
{
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;
	return 255;
}
bool decodeHex(const char *inhex,DWORD hexlen,std::string &to)
{
	byte *retval;
	byte *p;
	int len, i;
	if( inhex == NULL )	return false;
	len = hexlen/ 2;
	if( len == 0 )		return false;
	retval = (byte*)malloc(len+1);
	for(i=0, p = (byte *) inhex; i<len; i++) 
	{
		retval[i] = (nibbleFromChar(*p) << 4) | nibbleFromChar(*(p+1));
		p += 2;
	}
	retval[len] = 0;
	to.clear();
	to.append((char*)retval);
	free(retval);
	return true;
}

DWORD  GetHash(BYTE *pbData, DWORD dwDataLen, ALG_ID algId,CAtlStringA &out)
{
	DWORD dwReturn = 0;
	if(dwDataLen<1)
	{
		return dwReturn;
	}
	HCRYPTPROV hProv;
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		return (dwReturn = GetLastError());
	HCRYPTHASH hHash;
	if(!CryptCreateHash(hProv, algId, 0, 0, &hHash))
	{
		dwReturn = GetLastError();
		CryptReleaseContext(hProv, 0);
		return dwReturn;
	}
	if(!CryptHashData(hHash, pbData, dwDataLen, 0))
	{
		dwReturn = GetLastError();
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return dwReturn;
	}
	DWORD dwSize;
	DWORD dwLen = sizeof(dwSize);
	CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)(&dwSize), &dwLen, 0);

	BYTE* pHash = new BYTE[dwSize];
	dwLen = dwSize;
	CryptGetHashParam(hHash, HP_HASHVAL, pHash, &dwLen, 0);

	char szTemp[3];
	for (DWORD i = 0; i < dwLen; ++i)
	{
		sprintf(szTemp, "%02X", pHash[i]);
		out += szTemp;
	}

	delete [] pHash;
	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	return dwReturn;
}