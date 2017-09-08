#ifndef BASE_WIN32_SCOPED_WIN_HANDLE_H_
#define BASE_WIN32_SCOPED_WIN_HANDLE_H_


#include <windows.h>
#include "../MacroX.h"
namespace base
{

class  ScopedWinHandle
{
public:

	ScopedWinHandle() : handle_(INVALID_HANDLE_VALUE) 
	{
	}
	ScopedWinHandle(HANDLE handle) : handle_(handle) 
	{

	}
	~ScopedWinHandle() 
	{
		Reset(INVALID_HANDLE_VALUE);
	}

	bool Valid() const
	{ 
		return handle_ != INVALID_HANDLE_VALUE;
	}
	HANDLE Get() const 
	{ 
		return handle_;
	}
	HANDLE Release()
	{ 
		HANDLE old_handle = handle_;
		handle_ = INVALID_HANDLE_VALUE; 
		return old_handle;
	}
	void Reset(HANDLE handle) 
	{ 
		if (Valid()) 
			::CloseHandle(handle_); 
		handle_ = handle;
	}
	HANDLE* operator&() 
	{
		return &handle_;
	}
	operator HANDLE() const 
	{ 
		return handle_;
	}
private:
	HANDLE handle_;
	DISALLOW_COPY_AND_ASSIGN(ScopedWinHandle);
};

} 

#endif // BASE_WIN32_SCOPED_WIN_HANDLE_H_
