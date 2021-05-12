#include "StdAfx.h"
#include "MacroX.h"
#include "PeVerTool.h"
#include <sstream>
#include <wincrypt.h>
#include <wintrust.h>
#pragma comment(lib, "crypt32.lib")
#pragma comment(linker, "/defaultlib:version.lib")
namespace base{
	BOOL GetProgAndPublisherInfo(PCMSG_SIGNER_INFO pSignerInfo,SIGN_INFO &signinfo)
	{
		DWORD ENCODING = (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING);
		BOOL fReturn = FALSE;
		PSPC_SP_OPUS_INFO OpusInfo = NULL;  
		DWORD dwData;
		BOOL fResult;
		__try
		{
			// Loop through authenticated attributes and find
			// SPC_SP_OPUS_INFO_OBJID OID.
			for (DWORD n = 0; n < pSignerInfo->AuthAttrs.cAttr; n++)
			{           
				if (lstrcmpA(SPC_SP_OPUS_INFO_OBJID, pSignerInfo->AuthAttrs.rgAttr[n].pszObjId) == 0)
				{
					// Get Size of SPC_SP_OPUS_INFO structure.
					fResult = CryptDecodeObject(ENCODING,
						SPC_SP_OPUS_INFO_OBJID,
						pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].pbData,
						pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].cbData,
						0,
						NULL,
						&dwData);
					if (!fResult)
					{
						_tprintf(_T("CryptDecodeObject failed with %x\n"),
							GetLastError());
						__leave;
					}
					// Allocate memory for SPC_SP_OPUS_INFO structure.
					OpusInfo = (PSPC_SP_OPUS_INFO)LocalAlloc(LPTR, dwData);
					if (!OpusInfo)
					{
						_tprintf(_T("Unable to allocate memory for Publisher Info.\n"));
						__leave;
					}
					// Decode and get SPC_SP_OPUS_INFO structure.
					fResult = CryptDecodeObject(ENCODING,
						SPC_SP_OPUS_INFO_OBJID,
						pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].pbData,
						pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].cbData,
						0,
						OpusInfo,
						&dwData);
					if (!fResult)
					{
						_tprintf(_T("CryptDecodeObject failed with %x\n"),
							GetLastError());
						__leave;
					}
					// Fill in Program Name if present.
					if (OpusInfo->pwszProgramName)
					{
						signinfo.programName = OpusInfo->pwszProgramName;
					}
					// Fill in Publisher Information if present.
					if (OpusInfo->pPublisherInfo)
					{

						switch (OpusInfo->pPublisherInfo->dwLinkChoice)
						{
						case SPC_URL_LINK_CHOICE:
							signinfo.publisherLink = OpusInfo->pPublisherInfo->pwszUrl;
							break;
						case SPC_FILE_LINK_CHOICE:
							signinfo.publisherLink = OpusInfo->pPublisherInfo->pwszFile;
							break;
						}
					}
					// Fill in More Info if present.
					if (OpusInfo->pMoreInfo)
					{
						switch (OpusInfo->pMoreInfo->dwLinkChoice)
						{
						case SPC_URL_LINK_CHOICE:
							signinfo.moreInfoLink = OpusInfo->pMoreInfo->pwszUrl;
							break;
						case SPC_FILE_LINK_CHOICE:
							signinfo.moreInfoLink = OpusInfo->pMoreInfo->pwszFile;
							break;
						}
					}
					fReturn = TRUE;
					break; // Break from for loop.
				} // lstrcmp SPC_SP_OPUS_INFO_OBJID                 
			} // for 
		}
		__finally
		{
			if (OpusInfo != NULL) LocalFree(OpusInfo);      
		}
		return fReturn;
	}

	BOOL GetDateOfTimeStamp(PCMSG_SIGNER_INFO pSignerInfo, SYSTEMTIME *st)
	{   
		DWORD ENCODING = (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING);
		BOOL fResult;
		FILETIME lft, ft;   
		DWORD dwData;
		BOOL fReturn = FALSE;
		// Loop through authenticated attributes and find
		// szOID_RSA_signingTime OID.
		for (DWORD n = 0; n < pSignerInfo->AuthAttrs.cAttr; n++)
		{           
			if (lstrcmpA(szOID_RSA_signingTime,pSignerInfo->AuthAttrs.rgAttr[n].pszObjId) == 0)
			{               
				// Decode and get FILETIME structure.
				dwData = sizeof(ft);
				fResult = CryptDecodeObject(ENCODING,
					szOID_RSA_signingTime,
					pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].pbData,
					pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].cbData,
					0,
					(PVOID)&ft,
					&dwData);
				if (!fResult)
				{
					_tprintf(_T("CryptDecodeObject failed with %x\n"),
						GetLastError());
					break;
				}
				// Convert to local time.
				FileTimeToLocalFileTime(&ft, &lft);
				FileTimeToSystemTime(&lft, st);
				fReturn = TRUE;
				break; // Break from for loop.
			} //lstrcmp szOID_RSA_signingTime
		} // for 
		return fReturn;
	}

	BOOL GetTimeStampSignerInfo(PCMSG_SIGNER_INFO pSignerInfo, PCMSG_SIGNER_INFO *pCounterSignerInfo)
	{   
		DWORD ENCODING = (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING);
		PCCERT_CONTEXT pCertContext = NULL;
		BOOL fReturn = FALSE;
		BOOL fResult;       
		DWORD dwSize;
		__try
		{
			*pCounterSignerInfo = NULL;
			// Loop through unathenticated attributes for
			// szOID_RSA_counterSign OID.
			for (DWORD n = 0; n < pSignerInfo->UnauthAttrs.cAttr; n++)
			{
				if (lstrcmpA(pSignerInfo->UnauthAttrs.rgAttr[n].pszObjId, 
					szOID_RSA_counterSign) == 0)
				{
					// Get size of CMSG_SIGNER_INFO structure.
					fResult = CryptDecodeObject(ENCODING,
						PKCS7_SIGNER_INFO,
						pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].pbData,
						pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].cbData,
						0,
						NULL,
						&dwSize);
					if (!fResult)
					{
						_tprintf(_T("CryptDecodeObject failed with %x\n"),
							GetLastError());
						__leave;
					}
					// Allocate memory for CMSG_SIGNER_INFO.
					*pCounterSignerInfo = (PCMSG_SIGNER_INFO)LocalAlloc(LPTR, dwSize);
					if (!*pCounterSignerInfo)
					{
						_tprintf(_T("Unable to allocate memory for timestamp info.\n"));
						__leave;
					}
					// Decode and get CMSG_SIGNER_INFO structure
					// for timestamp certificate.
					fResult = CryptDecodeObject(ENCODING,
						PKCS7_SIGNER_INFO,
						pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].pbData,
						pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].cbData,
						0,
						(PVOID)*pCounterSignerInfo,
						&dwSize);
					if (!fResult)
					{
						_tprintf(_T("CryptDecodeObject failed with %x\n"),
							GetLastError());
						__leave;
					}
					fReturn = TRUE;
					break; // Break from for loop.
				}           
			}
		}
		__finally
		{
			// Clean up.
			if (pCertContext != NULL) CertFreeCertificateContext(pCertContext);
		}
		return fReturn;
	}
	
	SIGN_INFO GetSoftSign(PCTSTR pcszFileName)
	{
		DWORD ENCODING = (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING);
 		HCERTSTORE		  hStore = NULL;
		HCRYPTMSG		  hMsg = NULL;
		PCCERT_CONTEXT    pCertContext = NULL;
		BOOL			  bResult;
		DWORD dwEncoding, dwContentType, dwFormatType;
		PCMSG_SIGNER_INFO pSignerInfo = NULL;
		PCMSG_SIGNER_INFO pCounterSignerInfo = NULL;
		DWORD			  dwSignerInfo;
		CERT_INFO		  CertInfo;
		DWORD             dwDataSize = 0;
		SIGN_INFO signinfo;
		//从签名文件中获取存储句柄
		bResult = CryptQueryObject(CERT_QUERY_OBJECT_FILE,pcszFileName,
			CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,CERT_QUERY_FORMAT_FLAG_BINARY,
			0,&dwEncoding,&dwContentType,&dwFormatType,&hStore,&hMsg,NULL);
		if(!bResult)
		{
			return signinfo;
		}
		//获取签名信息所需的缓冲区大小
		bResult = CryptMsgGetParam(hMsg,CMSG_SIGNER_INFO_PARAM,0,NULL,&dwSignerInfo);
		if(!bResult)
		{
			return signinfo;
		}
		//分配缓冲区
		pSignerInfo = (PCMSG_SIGNER_INFO)LocalAlloc(LPTR, dwSignerInfo);
		if(pSignerInfo == NULL)
		{
			if(pSignerInfo)
				LocalFree((HLOCAL)pSignerInfo);
			if(hMsg)
				CryptMsgClose(hMsg);
			if(pCertContext)
				CertFreeCertificateContext(pCertContext);
			if(hStore)
				CertCloseStore(hStore,CERT_CLOSE_STORE_FORCE_FLAG);
			hStore = NULL;
			pCertContext = NULL;
			hMsg = NULL;
			pSignerInfo = NULL;
			return signinfo;
		}
		//获取签名信息
		if(CryptMsgGetParam(hMsg,CMSG_SIGNER_INFO_PARAM,0,pSignerInfo,&dwSignerInfo))
		{
			SYSTEMTIME sysTime;
			CertInfo.Issuer = pSignerInfo->Issuer;
			CertInfo.SerialNumber = pSignerInfo->SerialNumber;
			GetProgAndPublisherInfo(pSignerInfo,signinfo);
 			if(GetTimeStampSignerInfo(pSignerInfo, &pCounterSignerInfo))
			{
				// Find Date of timestamp.
				if (GetDateOfTimeStamp(pCounterSignerInfo, &sysTime))
				{
					signinfo.timestamp.Format(L"%d-%02d-%02d %0d:%0d:%0d",
						sysTime.wYear,sysTime.wMonth,sysTime.wDay,
						sysTime.wHour,sysTime.wMinute,sysTime.wSecond);
				}
			}
			if(pCertContext = CertFindCertificateInStore(hStore,ENCODING,0,CERT_FIND_SUBJECT_CERT,(PVOID)&CertInfo,NULL))
			{
				if(dwDataSize = CertGetNameString(pCertContext,CERT_NAME_SIMPLE_DISPLAY_TYPE,0,NULL,NULL,0))
				{
					dwDataSize = sizeof(wchar_t)*(dwDataSize+1);
					wchar_t *certName = (wchar_t*)malloc(dwDataSize);
					if(certName)
					{
						memset(certName,0,dwDataSize);
						CertGetNameString(pCertContext,CERT_NAME_SIMPLE_DISPLAY_TYPE,0,NULL,certName,dwDataSize);
						signinfo.author = certName;
						free(certName);
					}
				}
			}
			if(pSignerInfo)
				LocalFree((HLOCAL)pSignerInfo);
		}
		if(hMsg)
			CryptMsgClose(hMsg);
		if(pCertContext)
			CertFreeCertificateContext(pCertContext);
		if(hStore)
			CertCloseStore(hStore,CERT_CLOSE_STORE_FORCE_FLAG);
		hStore = NULL;
		pCertContext = NULL;
		hMsg = NULL;
		pSignerInfo = NULL;
		return signinfo;
	}

	CAtlString PeInfoVal::GetValue(LPCTSTR name,void* data) 
	{
		CAtlString retStr;

		WORD lang_codepage[8];
		int i = 0;
		// Use the language and codepage from the DLL.
		lang_codepage[i++] = language_;
		lang_codepage[i++] = code_page_;
		// Use the default language and codepage from the DLL.
		lang_codepage[i++] = ::GetUserDefaultLangID();
		lang_codepage[i++] = code_page_;
		// Use the language from the DLL and Latin codepage (most common).
		lang_codepage[i++] = language_;
		lang_codepage[i++] = 1252;
		// Use the default language and Latin codepage (most common).
		lang_codepage[i++] = ::GetUserDefaultLangID();
		lang_codepage[i++] = 1252;
		i = 0;
		while (i < arraysize(lang_codepage)) 
		{
			CAtlString	sub_block;
			WORD language = lang_codepage[i++];
			WORD code_page = lang_codepage[i++];
			sub_block.Format(_T("\\StringFileInfo\\%04x%04x\\%ls"),language,
				code_page, name);
			LPVOID value = NULL;
			UINT size;
			BOOL r = ::VerQueryValue(data,sub_block,&value, &size);
			if (r && value) 
			{
				retStr.SetString((LPCTSTR)value);
				return retStr;
			}
		}
		return retStr;
	}
	void PeInfoVal::Set(void* data)
	{
		PeInfoVal::CodePage *pLanguage = NULL;
		UINT  uiOtherSize = 0;
		//获取资源相关的 codepage 和language
		VerQueryValue(data,_T("\\VarFileInfo\\Translation"),(PVOID*)&pLanguage,&uiOtherSize);
		if(pLanguage)
		{
			language_ = pLanguage[0].wLanguage;
			code_page_ = pLanguage[0].wCodePage;
			VS_FIXEDFILEINFO *tmpFixInfo=NULL;
			::VerQueryValue(data,TEXT("\\"), (LPVOID*)&tmpFixInfo,&uiOtherSize);
			if(tmpFixInfo)
				memcpy(&fixed_file_info_,&tmpFixInfo,sizeof(VS_FIXEDFILEINFO));
			std::vector<CAtlString> keys;
			keys.push_back(_T("CompanyName"));
			keys.push_back(_T("CompanyShortName"));
			keys.push_back(_T("InternalName"));
			keys.push_back(_T("ProductName"));
			keys.push_back(_T("ProductShortName"));
			keys.push_back(_T("Comments"));
			keys.push_back(_T("LegalCopyright"));
			keys.push_back(_T("ProductVersion"));
			keys.push_back(_T("FileDescription"));
			keys.push_back(_T("LegalTrademarks"));
			keys.push_back(_T("PrivateBuild"));
			keys.push_back(_T("FileVersion"));
			keys.push_back(_T("OriginalFilename"));
			keys.push_back(_T("SpecialBuild"));
			keys.push_back(_T("LastChange"));
			keys.push_back(_T("Official Build"));
			for(size_t t=0;t<keys.size();t++)
			{
				data_[keys[t]] = GetValue(keys[t],data);
			}
		}
	}
	bool PeInfoVal::IsSuc()
	{
		return (data_.size()?true:false);
	}
	PeInfoVal::~PeInfoVal()
	{

	}
	CAtlString PeInfoVal::company_name() 
	{
		return data_[_T("CompanyName")];
	}
	CAtlString PeInfoVal::company_short_name()
	{
		return data_[_T("CompanyShortName")];
	}
	CAtlString PeInfoVal::internal_name() 
	{
		return data_[_T("InternalName")];
	}
	CAtlString PeInfoVal::product_name() 
	{
		return data_[_T("ProductName")];
	}
	CAtlString PeInfoVal::product_short_name() 
	{
		return data_[_T("ProductShortName")];
	}
	CAtlString PeInfoVal::comments() 
	{
		return data_[_T("Comments")];
	}
	CAtlString PeInfoVal::legal_copyright() 
	{
		return data_[_T("LegalCopyright")];
	}
	CAtlString PeInfoVal::product_version() 
	{
		return data_[_T("ProductVersion")];
	}
	CAtlString PeInfoVal::file_description() 
	{
		return data_[_T("FileDescription")];
	}
	CAtlString PeInfoVal::legal_trademarks() 
	{
		return data_[_T("LegalTrademarks")];
	}
	CAtlString PeInfoVal::private_build()
	{
		return data_[_T("PrivateBuild")];
	}
	CAtlString PeInfoVal::file_version() 
	{
		return data_[_T("FileVersion")];
	}
	CAtlString PeInfoVal::original_filename() 
	{
		return data_[_T("OriginalFilename")];
	}
	CAtlString PeInfoVal::special_build() 
	{
		return data_[_T("SpecialBuild")];
	}
	CAtlString PeInfoVal::last_change()
	{
		return data_[_T("LastChange")];
	}
	bool PeInfoVal::is_official_build() 
	{
		return (data_[_T("Official Build")].Compare(_T("1")) == 0);
	}
	PeInfoVal GetPeInfo(PCTSTR pcszFileName)
	{
		PeInfoVal ret;
		DWORD dwSize = 0;
		UINT uiSize = GetFileVersionInfoSize(pcszFileName,&dwSize);
		if (0 == uiSize)
		{
			return ret;
		}
		uiSize = uiSize + 1;
		byte *pBuffer = (byte*)malloc(uiSize);
		if (NULL == pBuffer)
		{
			return ret;
		}
		memset((void*)pBuffer,0,uiSize);
		if(!GetFileVersionInfo(pcszFileName,0,uiSize,(PVOID)pBuffer))
		{
			free(pBuffer);
			return ret;
		}
		ret.Set(pBuffer);
		free(pBuffer);
		return ret;	
	}

	CAtlString GetSelfVersion(HMODULE hModule)
	{
		HRSRC hsrc = FindResource(hModule, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
		if(hsrc==NULL)
			return L"";
		HGLOBAL hgbl = LoadResource(hModule, hsrc);
		if(hgbl==NULL)
			return L"";
		BYTE *pBt=(BYTE *)LockResource(hgbl);
		VS_FIXEDFILEINFO* pFinfo = (VS_FIXEDFILEINFO*)(pBt+40); 
		CAtlString valStr;   
		valStr.Format(L"%d.%d.%d.%d",
			(pFinfo->dwFileVersionMS >> 16) & 0xFF,
			(pFinfo->dwFileVersionMS) & 0xFF,
			(pFinfo->dwFileVersionLS >> 16) & 0xFF,
			(pFinfo->dwFileVersionLS) & 0xFF );
		UnlockResource(hgbl);
		return valStr;
	}

	int CompareVersion(LPCTSTR a,LPCTSTR b)
	{
		std::vector<long> verA;
		std::vector<long> verB;
		std::stringstream ssA;
		std::stringstream ssB;
		long nTmp=0;
		for(int n=0;;n++){
			char ch = a[n];
			nTmp = 0;
			if('0'<=ch && ch<='9' && ch!=' ' )
				ssA << ch;
			else if(!ssA.str().empty())
			{
				ssA>>nTmp;
				verA.push_back(nTmp);
				ssA.clear();
				ssA.str("");
				if(!ch) break;
			}
		}
		for(int n=0;;n++){
			char ch = b[n];
			nTmp = 0;
			if('0'<=ch && ch<='9' && ch!=' ' )
				ssB << ch;
			else if(!ssB.str().empty())
			{
				ssB>>nTmp;
				verB.push_back(nTmp);
				ssB.clear();
				ssB.str("");
				if(!ch) break;
			}
		}
		size_t posA = verA.size();
		size_t posB = verB.size();
		if(posA==0&&posB==0) return -1;
		for(int i=0;i<posA;i++)
		{
			if(verA[i]>verB[i])
				return 1;
			else if(verA[i]<verB[i])
				return 2;
		}
		return 0;
	}
}