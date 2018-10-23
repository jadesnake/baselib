#pragma once;
#include <map>
#include <vector>
#include <atlstr.h>



class PeInfoVal
{
public:
	PeInfoVal(){	}
 	virtual ~PeInfoVal();
	//”Ô—‘±‡¬Î
	struct CodePage
	{
		WORD wLanguage;
		WORD wCodePage;
	};	 
	typedef std::map<CAtlString,CAtlString> Values;
	void Set(void* data);
	bool IsSuc();
	// Accessors to the different version properties.
	// Returns an empty string if the property is not found.
	CAtlString company_name();
	CAtlString company_short_name();
	CAtlString product_name();
	CAtlString product_short_name();
	CAtlString internal_name();
	CAtlString product_version() ;
	CAtlString private_build() ;
	CAtlString special_build() ;
	CAtlString comments() ;
	CAtlString original_filename() ;
	CAtlString file_description() ;
	CAtlString file_version() ;
	CAtlString legal_copyright() ;
	CAtlString legal_trademarks() ;
	CAtlString last_change() ;
	bool is_official_build() ;
 protected:
	// Get the fixed file info if it exists. Otherwise NULL
	const VS_FIXEDFILEINFO& fixed_file_info() { return fixed_file_info_; }
	CAtlString GetValue(LPCTSTR name,void* data);
private:
	Values data_;
	int language_;
	int code_page_;
	VS_FIXEDFILEINFO fixed_file_info_;
};
PeInfoVal GetPeInfo(PCTSTR pcszFileName);