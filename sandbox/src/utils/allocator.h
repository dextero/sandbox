#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#ifdef _DEBUG

#	include <cstdio>
#	include <string>
#	include <vector>
#	include <map>

	class _Allocator
	{
	protected:
		struct AllocData {
			size_t bytes;
			size_t fileId;
			int line;

			AllocData(): bytes((size_t)0), fileId((size_t)-1), line(-1) {}
			AllocData(size_t bytes, size_t fileId, int line): bytes(bytes), fileId(fileId), line(line) {}
		};

		static std::vector<std::wstring>		mFileNames;
		static std::map<void*, AllocData>	mAllocations;
		static unsigned __int64				mTotalAllocated, mTotalFreed;
		static bool mIsAlive;
	
		// _Allocator is only an interface!
		_Allocator() {}
		~_Allocator() { mIsAlive = false; }

		static size_t GetFileNameId(const std::wstring& file);

	public:
		void* operator new(size_t size, const wchar_t* file, int line);
		void* operator new[](size_t size, const wchar_t* file, int line);
		void operator delete(void* p, const wchar_t* file, int line);
		void operator delete[](void* p, const wchar_t* file, int line);
		void operator delete(void* p);
		void operator delete[](void* p);

		static void* malloc(size_t size, const wchar_t* file, int line);
		static void free(void* p);

		static void PrintMemleakReport(const wchar_t* file = NULL);
		
	friend void* operator new(size_t size, const wchar_t* file, int line);
	friend void* operator new[](size_t size, const wchar_t* file, int line);
	friend void operator delete(void* p, const wchar_t* file, int line);
	friend void operator delete[](void* p, const wchar_t* file, int line);
	friend void operator delete(void* p);
	friend void operator delete[](void* p);
	};

	// global operators
	void* operator new(size_t size, const wchar_t* file, int line);
	void* operator new[](size_t size, const wchar_t* file, int line);
	void operator delete(void* p, const wchar_t* file, int line);
	void operator delete[](void* p, const wchar_t* file, int line);
	void operator delete(void* p);
	void operator delete[](void* p);

//	TODO: some magic to allow placement new
#	ifndef DONT_TOUCH_NEW_MACRO
#		if !defined(TEXT)
#			define __TEXT(x)			L ## x
#			define TEXT(x)				__TEXT(x)
#		endif //!defined(TEXT)
#		define new					new(TEXT(__FILE__), __LINE__)
#		define malloc(size)			_Allocator::malloc(size, TEXT(__FILE__), __LINE__)
#	endif
#	define PRINT_MEMLEAK_REPORT(foo)		_Allocator::PrintMemleakReport(foo)

#else

	// dummy class stub for compatibility in release builds
	class _Allocator
	{
	public:
		static void PrintMemleakReport(const wchar_t* foo)
		{
		}
	};

#	define PRINT_MEMLEAK_REPORT(foo)		_Allocator::PrintMemleakReport(foo)

#endif //_DEBUG

#define OWN_ALLOC	public _Allocator

#endif //__ALLOCATOR_H__