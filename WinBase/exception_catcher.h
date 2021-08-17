#ifndef _EXCEPTION_CATCHER_H_
#define _EXCEPTION_CATCHER_H_
#include <ShellAPI.h>
namespace exception_catcher
{
    //some exception handlers work on per process, others work on per thread
    //per process: SEH exception, pure virtual function call, C++ new exception, runtime invalid parameter error, signal: SIGABRT, SIGINT, SIGTERM
    //per thread: terminate() call, unexpected() call, signal: SIGFPE, SIGILL, SIGSEGV
    void init(LPCTSTR appName,LPCTSTR appParams,LPCTSTR appDesc,LPCTSTR appVersion);
	void set_process_exception_handlers();
    void set_thread_exception_handlers();
}


#endif//_EXCEPTION_CATCHER_H_