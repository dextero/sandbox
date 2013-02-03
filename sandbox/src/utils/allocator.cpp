#define DONT_TOUCH_NEW_MACRO
#include "allocator.h"

#ifdef _DEBUG

	// characters from c:\.. to ..\src, to shorten report a bit
#	define CHARS_TO_SRC_FOLDER	59
	
	std::vector<std::wstring> _Allocator::mFileNames = std::vector<std::wstring>();
	std::map<void*, _Allocator::AllocData> _Allocator::mAllocations = std::map<void*, _Allocator::AllocData>();
	unsigned __int64 _Allocator::mTotalAllocated = 0, _Allocator::mTotalFreed = 0;
	bool _Allocator::mIsAlive = true;
	
	
	size_t _Allocator::GetFileNameId(const std::wstring& file)
	{
		for (size_t i = 0; i < mFileNames.size(); ++i)
			if (mFileNames[i] == file)
				return i;
	
		// file name not in vector - add
		mFileNames.push_back(file);
		return mFileNames.size() - 1;
	}
	
	void* _Allocator::operator new(size_t size, const wchar_t* file, int line)
	{
		void* p = ::malloc(size);
		if (!p)
			throw std::bad_alloc();
	
		mAllocations.insert(std::make_pair(p, AllocData(size, GetFileNameId(file), line)));
		mTotalAllocated += size;
	
		return p;
	}
	
	void* _Allocator::operator new[](size_t size, const wchar_t* file, int line) 
	{
		void* p = ::malloc(size);
		if (!p)
			throw std::bad_alloc();
	
		mAllocations.insert(std::make_pair(p, AllocData(size, GetFileNameId(file), line)));
		mTotalAllocated += size;
	
		return p;
	}

	void _Allocator::operator delete(void* p, const wchar_t* file, int line)
	{
		delete p;
	}

	void _Allocator::operator delete[](void* p, const wchar_t* file, int line)
	{
		delete p;
	}
	
	void _Allocator::operator delete(void* p)
	{
		if (mIsAlive && mAllocations.find(p) != mAllocations.end())
		{
			mTotalFreed += mAllocations[p].bytes;
			mAllocations.erase(p);
		}
	
		free(p);
	}
	
	void _Allocator::operator delete[](void* p)
	{
		if (mIsAlive && mAllocations.find(p) != mAllocations.end())
		{
			mTotalFreed += mAllocations[p].bytes;
			mAllocations.erase(p);
		}
	
		free(p);
	}

	void* _Allocator::malloc(size_t size, const wchar_t* file, int line)
	{
		void* p = ::malloc(size);
		if (!p)
			return NULL;

		mAllocations.insert(std::make_pair(p, AllocData(size, GetFileNameId(file), line)));
		mTotalAllocated += size;

		return p;
	}

	void _Allocator::free(void* p)
	{
		if (mIsAlive && mAllocations.find(p) != mAllocations.end())
		{
			mTotalFreed += mAllocations[p].bytes;
			mAllocations.erase(p);
		}

		::free(p);
	}
	
	void _Allocator::PrintMemleakReport(const wchar_t* filename)
	{
		FILE* out = stdout;
		if (filename != NULL)
		{
			out = _wfopen(filename, L"w");
			if (out == NULL)
				out = stdout;
		}

		fprintf(out, "--- MEMORY LEAK REPORT ---\n\n");
		fprintf(out, "- bytes allocated: %llu\n- bytes freed: %llu\n", mTotalAllocated, mTotalFreed);
		fprintf(out, "- unfreed allocations: %u\n\n--- UNFREED ALLOCATIONS LIST ---\n\n", mAllocations.size());
	
		for (size_t fileId = 0; fileId < mFileNames.size(); ++fileId)
		{
			std::map<int, size_t> leaksInLines;
	
			for (std::map<void*, AllocData>::iterator it = mAllocations.begin(); it != mAllocations.end(); ++it)
				if (it->second.fileId == fileId)
				{
					if (leaksInLines.find(it->second.line) == leaksInLines.end())
						leaksInLines.insert(std::make_pair(it->second.line, it->second.bytes));
					else
						leaksInLines[it->second.line] += it->second.bytes;
				}
	
			if (leaksInLines.size())
			{
				fprintf(out, "- in file %ls:\n", mFileNames[fileId].c_str() + CHARS_TO_SRC_FOLDER);
				for (std::map<int, size_t>::iterator it = leaksInLines.begin(); it != leaksInLines.end(); ++it)
					fprintf(out, "  - %u bytes at line %d\n", it->second, it->first);
			}
		}
		
		fprintf(out, "\n--- END OF MEMORY LEAK REPORT ---\n");

		if (out != stdout)
		{
			fclose(out);
			fprintf(stdout, "Memleak report saved to %ls.\n", filename);
		}
	}
	
	
	// global operators
	void* operator new(size_t size, const wchar_t* file, int line)
	{
		void* p = malloc(size);
		if (!p)
			throw std::bad_alloc();
		
		_Allocator::mAllocations.insert(std::make_pair(p, _Allocator::AllocData(size, _Allocator::GetFileNameId(file), line)));
		_Allocator::mTotalAllocated += size;
		
		return p;
	}
	
	void* operator new[](size_t size, const wchar_t* file, int line) 
	{
		void* p = malloc(size);
		if (!p)
			throw std::bad_alloc();
		
		_Allocator::mAllocations.insert(std::make_pair(p, _Allocator::AllocData(size, _Allocator::GetFileNameId(file), line)));
		_Allocator::mTotalAllocated += size;
		
		return p;
	}
	
	void operator delete(void* p, const wchar_t* file, int line)
	{
		delete p;
	}

	void operator delete[](void* p, const wchar_t* file, int line)
	{
		delete p;
	}

	void operator delete(void* p)
	{
		if (_Allocator::mIsAlive && _Allocator::mAllocations.find(p) != _Allocator::mAllocations.end())
		{
			_Allocator::mTotalFreed += _Allocator::mAllocations[p].bytes;
			_Allocator::mAllocations.erase(p);
		}
	
		free(p);
	}
	
	void operator delete[](void* p)
	{
		if (_Allocator::mIsAlive && _Allocator::mAllocations.find(p) != _Allocator::mAllocations.end())
		{
			_Allocator::mTotalFreed += _Allocator::mAllocations[p].bytes;
			_Allocator::mAllocations.erase(p);
		}
	
		free(p);
	}

#endif //_DEBUG