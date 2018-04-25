// A wrapper class of Windows shared memory

#ifndef BASE_SHARED_MEMORY_H_
#define BASE_SHARED_MEMORY_H_


#include <assert.h>
#include <windows.h>

namespace base
{
	class  SharedMemory
	{
	public:
		SharedMemory() : mapping_(NULL) {}
		~SharedMemory() { Close(); };
		bool Create(const wchar_t *name, int size);
		bool Open(const wchar_t *name);
		void Close();
		int  GetSize();

		class MappedView
		{
		public:
			MappedView() : view_(NULL) {}
			~MappedView() { CloseView(); }
			bool OpenView(SharedMemory *shared_memory, bool read_write);
			int  GetSize();
			void CloseView();
			unsigned char *view() { return view_; }
		private:
			unsigned char *view_;
		};

		template<class T>
		class MappedViewOf : public MappedView
		{
		public:
			T *get() { return reinterpret_cast<T*>(view()); }
			T* operator->()
			{
				assert(view() != NULL);
				return reinterpret_cast<T*>(view());
			}
		};
	private:
		friend class MappedView;
		HANDLE mapping_;
	};
}

#endif // BASE_SHARED_MEMORY_H_
