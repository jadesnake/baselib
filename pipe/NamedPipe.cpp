#include "StdAfx.h"
#include "NamedPipe.h"

NamedPipe::NamedPipe(void)
{
	mIdentity = BLANK;
	mPipe = NULL;
	mClosed = false;
}
NamedPipe::NamedPipe(HANDLE hPipe)
{
	mPipe = hPipe;
	mIdentity = RCVCLIENT;
	mClosed = false;
}
NamedPipe::~NamedPipe(void)
{
	if(mIdentity==RCVCLIENT)
		close();
}
void NamedPipe::close()
{
	if(mPipe)
	{
		if(mIdentity==SERVER&&IsAlive())
			DisconnectNamedPipe(mPipe);
		CloseHandle(mPipe);
		mPipe = NULL;
		mClosed = true;
		mNamed.Empty();
	}
}
bool NamedPipe::WriteBytes(const char* buf,size_t size)
{
	DWORD cbWritten=0;
	BOOL fSuccess = FALSE;
	mLastMsg.Empty();
	//先发送数据长度
	std::stringstream ss;
	ss<<size<<"\n";
	fSuccess = WriteFile(mPipe,ss.str().c_str(),ss.str().size(),&cbWritten,NULL);
	DWORD dwErr = GetLastError();
	if(dwErr==ERROR_BROKEN_PIPE||dwErr==ERROR_NO_DATA)
	{
		mLastMsg = L"pipe disconnected";
		mClosed = true;
		return false;
	}
	if(!fSuccess || ss.str().size()!=cbWritten)
	{
		mLastMsg = L"WriteFile failed";
		return false;
	}
	cbWritten=0;
	do
	{
		DWORD sendbytes=0;
		fSuccess = WriteFile(mPipe,buf+cbWritten,size-cbWritten,&sendbytes,NULL);
		dwErr = GetLastError();
		if(dwErr==ERROR_BROKEN_PIPE||dwErr==ERROR_NO_DATA)
		{
			mLastMsg = L"pipe disconnected";
			mClosed = true;
			return false;
		}
		if(!fSuccess)
		{
			mLastMsg = L"WriteFile failed";
			return false;
		}
		cbWritten += sendbytes;
	}while(size!=cbWritten);
	return true;
}
bool NamedPipe::ReadLine(std::string& outBuf)
{
	mLastMsg.Empty();
	DWORD cbBytesRead=0,needBytes=0,lastCode=0;
	BOOL fSuccess = FALSE;
	bool bPacket = false;
	std::string rcvBuffer,lenBuffer;
	while(1)
	{
		char chBuf = 0;
		fSuccess = ReadFile(mPipe,&chBuf, 1, &cbBytesRead,NULL);
		lastCode = GetLastError();
		if(lastCode== ERROR_BROKEN_PIPE)
		{
			mLastMsg = L"pipe disconnected";
			mClosed = true;
			return false;
		}
		if(fSuccess==0)
			break;
		if(cbBytesRead==0)
		{
			mLastMsg = L"read zero length";
			return false;
		}
		if(chBuf!='\n')
		{		
			lenBuffer += chBuf;
			continue;
		}
		needBytes = atol(lenBuffer.c_str());
		bPacket = true;
		break;
	}
	if(needBytes==0)
	{
		outBuf = rcvBuffer;
		return true;
	}
	lenBuffer = rcvBuffer;
	while(bPacket && needBytes)
	{
		memset(mRcvData,0,BUFFER_PIPE_SIZE);
		fSuccess = ReadFile(mPipe,mRcvData, needBytes, &cbBytesRead,NULL);
		DWORD lastCode = GetLastError();
		if(lastCode== ERROR_BROKEN_PIPE)
		{
			mLastMsg = L"pipe disconnected";
			mClosed = true;
			return false;
		}
		if(fSuccess==0)
			break;
		if(cbBytesRead==0)
		{
			mLastMsg = L"read zero length";
			return false;
		}
		rcvBuffer.append(mRcvData,cbBytesRead);
		if(rcvBuffer.size()>=needBytes)
		{
			outBuf = rcvBuffer;
			return true;
		}
	}
	mLastMsg = L"pipe read error";
	return false;
}
bool NamedPipe::IsClosed()
{
	return mClosed;
}
const CAtlString NamedPipe::GetLastMsg()
{
	return mLastMsg;
}
bool NamedPipe::connect(LPCTSTR named)
{
	//for client
	mLastMsg.Empty();
	mIdentity = CLIENT;
	if(mPipe)
	{
		if(mNamed!=named)
 			close();
		else
			return true;
 	}
	for(;;)
	{
		WaitNamedPipe(named, NMPWAIT_WAIT_FOREVER);
		mPipe = CreateFile(named,   // pipe name 
			GENERIC_READ|GENERIC_WRITE, 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file
		DWORD errCode = GetLastError();
		if(errCode==ERROR_PIPE_BUSY)
			continue;
		if (mPipe==INVALID_HANDLE_VALUE) 
		{
			mLastMsg.Format(L"connect error %d",errCode);
			return false;
		}
		break;
	}
	if (mPipe == INVALID_HANDLE_VALUE) 
	{
		mLastMsg = L"Could not open pipe";
		return false;
	}
	// The pipe connected; change to message-read mode. 
	DWORD dwMode = PIPE_TYPE_BYTE; 
	BOOL fSuccess = SetNamedPipeHandleState( 
		mPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if(!fSuccess) 
	{
		mLastMsg = L"SetNamedPipeHandleState failed";
		return false;
	}
	mNamed = named;
	return true;
}
bool NamedPipe::open(LPCTSTR named)
{
	//for service
	mLastMsg.Empty();
	mIdentity = SERVER;
	mPipe = CreateNamedPipe(named, PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_BYTE|PIPE_WAIT, 
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		BUFFER_PIPE_SIZE,      // output buffer size 
		BUFFER_PIPE_SIZE,      // input buffer size 
		0,         // client time-out 
		NULL);     // default security attribute 
	if(mPipe == INVALID_HANDLE_VALUE) 
	{
		mLastMsg = L"CreateNamedPipe failed";
		return false;
	}
	mNamed = named;
	return true;
}

void NamedPipe::Flush()
{
	if(mPipe)
		FlushFileBuffers(mPipe); 
}
bool NamedPipe::IsAlive()
{
	char chTmp[2] = {0};
	DWORD dwBytes=0,dwLeft=0;
	if(::PeekNamedPipe(mPipe,chTmp,2,&dwBytes,&dwBytes,&dwLeft))
		return true;
	DWORD dwCode = GetLastError();
	return false;
}
NamedPipe* NamedPipe::WaitClient(unsigned int timeout)
{
	mLastMsg.Empty();
	if(mIdentity!=SERVER)
	{
		mLastMsg = L"WaitForConnection is not supported on client pipe";
		return NULL;
	}
	OVERLAPPED lpOverlapped = {0};
	lpOverlapped.hEvent = CreateEvent(NULL,1,1,NULL);
	if(ConnectNamedPipe(mPipe, &lpOverlapped)==0)
	{
		DWORD dwCode = GetLastError();
		if(dwCode==ERROR_PIPE_CONNECTED)
		{
			if (!SetEvent(lpOverlapped.hEvent))
			{
				mLastMsg = L"AsyncWaitForConnection failed";
				if(lpOverlapped.hEvent)
					CloseHandle(lpOverlapped.hEvent);
				return NULL;
			}
		}
		int result = WaitForSingleObject(lpOverlapped.hEvent,timeout);
		if(lpOverlapped.hEvent)
			CloseHandle(lpOverlapped.hEvent);
		if(WAIT_OBJECT_0==result && IsAlive())
		{
			HANDLE client = mPipe;
			open(mNamed);
			return new NamedPipe(client);
		}
		return NULL;
	}
	mLastMsg = L"AsyncWaitForConnection failed";
	return NULL;
}
