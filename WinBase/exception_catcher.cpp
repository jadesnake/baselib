//if you want to use this class separately,
//you should comment log sentences in "CreateMiniDump" and all handlers,
//and replace CSelfPath with your exe path getting function in "PrepareDumpFilePath"
#include "stdafx.h"
#include <string>
#include <functional>
#include <algorithm>
#include <new.h>
#include <signal.h>
#include <Windows.h>
#include <Dbghelp.h>
#include <intrin.h>
#include "exception_catcher.h"

#pragma comment(lib, "Dbghelp.lib")

//copy from invarg.c of VC9.0
#if defined (_AMD64_)

PRUNTIME_FUNCTION
RtlLookupFunctionEntry (
                        IN ULONG64 ControlPc,
                        OUT PULONG64 ImageBase,
                        IN OUT PVOID HistoryTable OPTIONAL
                        );

PVOID
RtlVirtualUnwind (
                  IN ULONG HandlerType,
                  IN ULONG64 ImageBase,
                  IN ULONG64 ControlPc,
                  IN PRUNTIME_FUNCTION FunctionEntry,
                  IN OUT PCONTEXT ContextRecord,
                  OUT PVOID *HandlerData,
                  OUT PULONG64 EstablisherFrame,
                  IN OUT PVOID ContextPointers OPTIONAL
                  );

#endif



namespace
{
	static LPCTSTR stc_appName=NULL;
	static LPCTSTR stc_appVersion=NULL;
	static LPCTSTR stc_appDesc=NULL;
	static LPCTSTR stc_appParams=NULL;


	static time_t getDateFromMacro(char const *date, char const *time) {
		char s_month[5];
		int month, day, year;
		struct tm t = { 0 };
		static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

		sscanf(date, "%s %d %d", s_month, &day, &year);

		month = (strstr(month_names, s_month) - month_names) / 3;

		t.tm_mon = month;
		t.tm_mday = day;
		t.tm_year = year - 1900;
		t.tm_isdst = -1;

		std::string sTime = time;
		for (int j = 0, i = sTime.length() - 1; i >= 0; i--)
		{
			if (sTime[i] == ':')
			{
				if (j == 0)
				{
					t.tm_sec = atoi(&sTime[i + 1]);
					sTime[i] = 0;
					j++;
				}
				else if (j == 1)
				{
					t.tm_min = atoi(&sTime[i + 1]);
					sTime[i] = 0;
					j++;
					t.tm_hour = atoi(sTime.c_str());
					break;
				}

			}
		}

		return mktime(&t);
	}
    std::wstring prepare_dump_file_path()
    {
		USES_CONVERSION;

        std::wstring file_path;
		TCHAR szTemp[MAX_PATH] = { 0 };
		::GetModuleFileName(::GetModuleHandle(NULL), szTemp, MAX_PATH);
		LPTSTR pszFileName = _tcsrchr(szTemp, _T('\\'));
		if (pszFileName) {
			pszFileName++;
			*pszFileName = _T('\0');
		}
		//trim datetime
		time_t t1 = getDateFromMacro(__DATE__, __TIME__);
		tm* myTime = localtime(&t1);
		file_path = szTemp;

        SYSTEMTIME systime = {0};
        GetLocalTime(&systime);

        const size_t name_buf_size = 200;
        TCHAR name_buf[name_buf_size] = {0};
        //PS: pid 5 numbers
		_stprintf_s(name_buf, L"_lnk%04d%02d%02d%02d%02d%02d_%04d%02d%02d%02d%02d%02d",
            systime.wYear, systime.wMonth, systime.wDay,
            systime.wHour, systime.wMinute, systime.wSecond
			, myTime->tm_year+1900, myTime->tm_mon+1, myTime->tm_mday
			, myTime->tm_hour, myTime->tm_min, myTime->tm_sec
			);
		file_path += stc_appVersion;
		file_path += name_buf;
		file_path += L".dmp";
        return file_path;
    }

    bool create_minidump(EXCEPTION_POINTERS* pExceptionPtrs)
    {
		static bool bOnce = false;
		if (bOnce)
		{
			return true;
		}
		bOnce = true;
        bool ret = false;
		bool bIsDebugFlag = false;
		{
			//is exist .test file
			std::wstring testFile = _T(".test");
			FILE* hf = _wfopen(testFile.c_str(), L"r");
			if (hf)
			{
				bIsDebugFlag = true;
				fclose(hf);
			}
		}
        MINIDUMP_EXCEPTION_INFORMATION dumpinfo;
        dumpinfo.ThreadId = GetCurrentThreadId();
        dumpinfo.ExceptionPointers = pExceptionPtrs;
        dumpinfo.ClientPointers = 0;
		DWORD dwDumpType = MiniDumpNormal;
		if (bIsDebugFlag)
		{
			dwDumpType = MiniDumpWithDataSegs | MiniDumpWithProcessThreadData | MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithCodeSegs;
		}

        std::wstring file_path = prepare_dump_file_path();

        HANDLE hFile = CreateFile(file_path.c_str(),
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            //ErrorLogLastErr("CreateFile(for create dump) fail, file path: %s", file_path.c_str());
			return 0;
        }
        else
        {
            if (!MiniDumpWriteDump(GetCurrentProcess(),
                GetCurrentProcessId(),
                hFile,
				(MINIDUMP_TYPE)dwDumpType,
                &dumpinfo,
                NULL,
                NULL))
            {
				return 0;
                //ErrorLogLastErr("MiniDumpWriteDump fail, file path: %s", file_path.c_str());
            }
            else
            {
                ret = true;
            }

            CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
        }
		TCHAR szTemp[MAX_PATH] = { 0 };
		::GetModuleFileName(::GetModuleHandle(NULL), szTemp, MAX_PATH);
		std::wstring crashApp = szTemp;      
		LPTSTR pszFileName = _tcsrchr(szTemp, _T('\\'));
		if (pszFileName) {
			pszFileName++;
			*pszFileName = _T('\0');
		}
		std::wstring reporterPath = szTemp + std::wstring(_T("NNCrashReporter.exe")); 
		//MessageBox(0, file_path.c_str(), reporterPath.c_str(), 0);
		//MessageBox(0, crashApp.c_str(), _T("s"), 0);

		std::wstring swparam;
		if(stc_appName!=NULL)
			swparam += _T(" -nnapp ")+std::wstring(stc_appName);							//重启程序
		if(stc_appParams!=NULL)
			swparam += _T(" -nnparam \"")+std::wstring(stc_appParams)+_T("\"");								//重启参数
		if(stc_appDesc!=NULL)
			swparam += _T(" -nndesc ")+std::wstring(stc_appDesc);							//程序描述
		if(!file_path.empty())
			swparam += _T(" -nnfile ")+file_path;												//DUMP文件
		if(stc_appVersion!=NULL)
			swparam += _T(" -nnurl crashreport_")+ std::wstring(stc_appVersion) +_T("/"); //DUMP上传路径

		//MessageBox(0, swparam.c_str(), _T("s"), 0);
        //InfoLog("create dmp file[%s] %s", file_path.c_str(), ret ? "success" : "fail");
		HINSTANCE hInstCrashReporter = ShellExecute(NULL, _T("open"), reporterPath.c_str(), swparam.c_str(), szTemp, SW_SHOW); 
        return ret;
    }

    EXCEPTION_POINTERS* get_exception_pointers(const DWORD dwExceptionCode)
    {
        //copy from function _invoke_watson in invarg.c of VC9.0
        EXCEPTION_RECORD   ExceptionRecord = {0};
        CONTEXT ContextRecord;

#ifdef _X86_

        __asm {
            mov dword ptr [ContextRecord.Eax], eax
                mov dword ptr [ContextRecord.Ecx], ecx
                mov dword ptr [ContextRecord.Edx], edx
                mov dword ptr [ContextRecord.Ebx], ebx
                mov dword ptr [ContextRecord.Esi], esi
                mov dword ptr [ContextRecord.Edi], edi
                mov word ptr [ContextRecord.SegSs], ss
                mov word ptr [ContextRecord.SegCs], cs
                mov word ptr [ContextRecord.SegDs], ds
                mov word ptr [ContextRecord.SegEs], es
                mov word ptr [ContextRecord.SegFs], fs
                mov word ptr [ContextRecord.SegGs], gs
                pushfd
                pop [ContextRecord.EFlags]
        }

        ContextRecord.ContextFlags = CONTEXT_CONTROL;
#pragma warning(push)
#pragma warning(disable:4311)
        ContextRecord.Eip = (ULONG)_ReturnAddress();
        ContextRecord.Esp = (ULONG)_AddressOfReturnAddress();
#pragma warning(pop)
        ContextRecord.Ebp = *((ULONG *)_AddressOfReturnAddress()-1);

#elif defined (_AMD64_)

        ULONG64 ControlPc;
        ULONG64 EstablisherFrame;
        PRUNTIME_FUNCTION FunctionEntry;
        PVOID HandlerData;
        ULONG64 ImageBase;

        RtlCaptureContext(&ContextRecord);
        ControlPc = ContextRecord.Rip;
        FunctionEntry = RtlLookupFunctionEntry(ControlPc, &ImageBase, NULL);
        if (FunctionEntry != NULL) {
            RtlVirtualUnwind(/*UNW_FLAG_NHANDLER*/0x00,
                ImageBase,
                ControlPc,
                FunctionEntry,
                &ContextRecord,
                &HandlerData,
                &EstablisherFrame,
                NULL);
        } else {
            ContextRecord.Rip = (ULONGLONG) _ReturnAddress();
            ContextRecord.Rsp = (ULONGLONG) _AddressOfReturnAddress();
        }

#elif defined (_IA64_)

        /* Need to fill up the Context in IA64. */
        RtlCaptureContext(&ContextRecord);

#else  /* defined (_IA64_) */

        ZeroMemory(&ContextRecord, sizeof(ContextRecord));

#endif  /* defined (_IA64_) */


        ExceptionRecord.ExceptionCode = dwExceptionCode;
        ExceptionRecord.ExceptionFlags    = EXCEPTION_NONCONTINUABLE;
        ExceptionRecord.ExceptionAddress = _ReturnAddress();

        //end copy

        EXCEPTION_RECORD* pExceptionRecord = new EXCEPTION_RECORD;
        memcpy(pExceptionRecord, &ExceptionRecord, sizeof(EXCEPTION_RECORD));
        CONTEXT* pContextRecord = new CONTEXT;
        memcpy(pContextRecord, &ContextRecord, sizeof(CONTEXT));

        EXCEPTION_POINTERS* pExceptionPointers = new EXCEPTION_POINTERS;
        pExceptionPointers->ContextRecord = pContextRecord;
        pExceptionPointers->ExceptionRecord = pExceptionRecord;

        return pExceptionPointers;
    }


    //handlers
    LONG WINAPI catched_seh_handler(EXCEPTION_POINTERS* pExceptionPtrs)
    {
        //InfoLog("SEH exception occurs");
        create_minidump(pExceptionPtrs);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    void __cdecl catched_terminate_handler()
    {
        //InfoLog("terminate call occurs");
        create_minidump(get_exception_pointers(0));
    }

    void __cdecl catched_unexpected_handler()
    {
        //InfoLog("unexpected call occurs");
        create_minidump(get_exception_pointers(0));
    }

    void __cdecl catched_pure_call_handler()
    {
        //InfoLog("pure virtual function call occurs");
        create_minidump(get_exception_pointers(0));
    }

    void __cdecl catched_invalid_parameter_handler(const wchar_t* expression,
        const wchar_t* function,
        const wchar_t* file,
        unsigned int line,
        uintptr_t pReserved)
    {
        pReserved;
        //InfoLog(L"invalid parameter call occurs, expression[%s], function[%s], file[%s], line[%d]",expression, function, file, line);
        create_minidump(get_exception_pointers(0));
    }

    int __cdecl catched_new_handler(size_t)
    {
        //InfoLog("new operator memory allocation exception occurs");
        create_minidump(get_exception_pointers(0));
        return 0;
    }

    void catched_sigabrt_handler(int)
    {
        //InfoLog("signal SIGABRT occurs");
        create_minidump(get_exception_pointers(0));
    }

    void catched_sigfpe_handler(int code, int subcode)
    {
        //InfoLog("signal SIGFPE occurs, code[%d], subcode[%d]", code, subcode);
        create_minidump(reinterpret_cast<EXCEPTION_POINTERS *>(_pxcptinfoptrs));
    }

    void catched_sigint_handler(int)
    {
        //InfoLog("signal SIGINT occurs");
        create_minidump(get_exception_pointers(0));
    }

    void catched_sigill_handler(int)
    {
        //InfoLog("signal SIGILL occurs");
        create_minidump(get_exception_pointers(0));
    }

    void catched_sigsegv_handler(int)
    {
        //InfoLog("signal SIGSEGV occurs");
        create_minidump(reinterpret_cast<EXCEPTION_POINTERS *>(_pxcptinfoptrs));
    }

    void catched_sigterm_handler(int)
    {
        //InfoLog("signal SIGTERM occurs");
        create_minidump(get_exception_pointers(0));
    }
    //end handlers
}

void exception_catcher::init(LPCTSTR appName,LPCTSTR appParams,LPCTSTR appDesc,LPCTSTR appVersion)
{
	stc_appName = appName;
	stc_appParams = appParams;
	stc_appDesc = appDesc;
	stc_appVersion = appVersion;
}

void exception_catcher::set_process_exception_handlers()
{
    // Install top-level SEH handler
    SetUnhandledExceptionFilter(catched_seh_handler);

    // Catch pure virtual function calls.
    // Because there is one _purecall_handler for the whole process,
    // calling this function immediately impacts all threads. The last
    // caller on any thread sets the handler.
    // http://msdn.microsoft.com/en-us/library/t296ys27.aspx
    _set_purecall_handler(catched_pure_call_handler);

    // Catch new operator memory allocation exceptions
    _set_new_handler(catched_new_handler);

    // Catch invalid parameter exceptions.
    _set_invalid_parameter_handler(catched_invalid_parameter_handler);

    // Set up C++ signal handlers

    _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);

    // Catch an abnormal program termination
    signal(SIGABRT, catched_sigabrt_handler);

    // Catch illegal instruction handler
    signal(SIGINT, catched_sigint_handler);

    // Catch a termination request
    signal(SIGTERM, catched_sigterm_handler);
}

void exception_catcher::set_thread_exception_handlers()
{
    // Catch terminate() calls.
    // In a multithreaded environment, terminate functions are maintained
    // separately for each thread. Each new thread needs to install its own
    // terminate function. Thus, each thread is in charge of its own termination handling
    // http://msdn.microsoft.com/en-us/library/t6fk7h29.aspx
    set_terminate(catched_terminate_handler);

    // Catch unexpected() calls.
    // In a multithreaded environment, unexpected functions are maintained
    // separately for each thread. Each new thread needs to install its own
    // unexpected function. Thus, each thread is in charge of its own unexpected handling.
    // http://msdn.microsoft.com/en-us/library/h46t5b69.aspx
    set_unexpected(catched_unexpected_handler);

    // Catch a floating point error
    typedef void (*sigh)(int);
    signal(SIGFPE, (sigh)catched_sigfpe_handler);

    // Catch an illegal instruction
    signal(SIGILL, catched_sigill_handler);

    // Catch illegal storage access errors
    signal(SIGSEGV, catched_sigsegv_handler);
}