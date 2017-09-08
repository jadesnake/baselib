#include "stdafx.h"
#include "SharedMemory.h"
#include "SecurityUtil.h"
namespace base
{
	bool SharedMemory::Create(const wchar_t *name, int size)
	{
		assert(mapping_ == NULL);
		mapping_ = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
			0, size, name);
		if(mapping_)
		{
			base::SetObjectToLowIntegrity(mapping_);
		}
		return mapping_ != NULL;
	}
	bool SharedMemory::Open(const wchar_t *name)
	{
		assert(mapping_ == NULL);
		mapping_ = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, name);
		return mapping_ != NULL;
	}
	void SharedMemory::Close()
	{
		if (mapping_ != NULL)
		{
			CloseHandle(mapping_);
			mapping_ = NULL;
		}
	}
	bool SharedMemory::MappedView::OpenView(SharedMemory *shared_memory, bool read_write)
	{
		assert(!view_);
		DWORD access = FILE_MAP_READ;
		if (read_write) access |= FILE_MAP_WRITE;
		view_ = reinterpret_cast<unsigned char*>(::MapViewOfFile(shared_memory->mapping_, access, 0, 0, 0));
		return view_ != NULL;
	}
	void SharedMemory::MappedView::CloseView()
	{
		if (view_)
		{
			::UnmapViewOfFile(view_);
			view_ = NULL;
		}
	}
}