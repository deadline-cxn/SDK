// utilities.h
// Gregory Rosenblatt

#ifndef Utilities_H_
#define Utilities_H_

#include <string>
#include <stdio.h>
#include <assert.h>

namespace MetaLang
{
    // data buffer to encapsulate copy-on-write functionality
    class Buffer
    {
    public:
        Buffer() : _size(0), _data(0)   {}
        Buffer(const char* data, size_t size)
            : _size(size), _data(new char[_size])
        { memcpy(_data, data, _size); }
        Buffer(const Buffer& buf) : _size(buf._size), _data(new char[_size])
        { memcpy(_data, buf._data, _size); }
        ~Buffer()   { delete[] _data; }
        void operator=(const Buffer& buf)
        {
            if (&buf == this)
                return;
            delete[] _data;
            _size = buf._size;
            _data = new char[_size];
            memcpy(_data, buf._data, _size);
        }
        const char* Data() const    { return _data; }
    private:
        size_t  _size;
        char*   _data;
    };
    
    // stack for pushing and popping of data groups by scope
    template <class T>
    class ScopeStack
    {
    public:
        ScopeStack() : _stackBase(0)    {}
        ScopeStack(size_t reserveSize) : _stackBase(0)
            { _stack.reserve(reserveSize); }
        // scope interface
        void SetInitialScope(size_t scopeSize)
        {
            assert(_stack.empty() && _indices.empty()); // initial scope should only be set once
            _stack.resize(scopeSize);
        }        
        void PushScope(size_t scopeSize)
        {
            _indices.push_back(_stackBase);         // store the current index
            _stackBase = _stack.size();             // get new index
            _stack.resize(_stackBase+scopeSize);    // accomodate new scope data
        }
        void PopScope()
        {
            assert(!_indices.empty());      // don't try to pop a non-existent scope
            _stack.resize(_stackBase);      // shrink back to the end of the former scope
            _stackBase = _indices.back();   // grab the base index for the former scope
            _indices.pop_back();            // take that index off the index stack
        }
        // element interface
        void Push(const T& element) { _stack.push_back(element); }
        void Pop()  { assert(!_stack.empty()); _stack.pop_back(); }
        // accessors
        const T& Top() const    { assert(!_stack.empty()); return _stack.back(); }
        T& Top()    { assert(!_stack.empty()); return _stack.back(); }
        const T& Get(size_t index) const    { assert((_stackBase+index)<_stack.size()); return _stack[_stackBase+index]; }
        T& Get(size_t index)    { assert((_stackBase+index)<_stack.size()); return _stack[_stackBase+index]; }
        const T& GetAtDepth(size_t index, size_t depth) const   { assert((GetScopeIndex(depth)+index)<_stack.size()); return _stack[GetScopeIndex(depth)+index]; }
        T& GetAtDepth(size_t index, size_t depth)   { assert((GetScopeIndex(depth)+index)<_stack.size()); return _stack[GetScopeIndex(depth)+index]; }
        const T& operator[](size_t index) const { return Get(index); }
        T& operator[](size_t index) { return Get(index); }
        // properties
        size_t Size() const { return _stack.size(); }
        size_t ScopeDepth() const   { return _indices.size(); }
        size_t GetScopeIndex(size_t depth) const  { assert(depth<_indices.size()); return _indices[depth]; }
    private:
        std::vector<T>      _stack;
        std::vector<size_t> _indices;
        size_t              _stackBase;
    };

	/** File **
	* Basic file handle interface.
	*/
	class File
	{
	public:
		bool Close()		{ assert(Good()); return (fclose(_fp) == 0); }
		bool Flush()		{ assert(Good()); return (fflush(_fp) == 0); }
		bool Good() const	{ return (_fp != 0); }
		bool Bad() const	{ return (_fp == 0); }
		// file detection
		static bool Exists(const std::string& fileName)
        { FILE* fp; bool exists = ((fp = fopen(fileName.c_str(), "rb")) != 0); if (fp) fclose(fp); return exists; }
		// file length    (only use before reading/writing)
		long Length()	{ assert(Good()); fseek(_fp, 0, SEEK_END); long len = ftell(_fp); fseek(_fp, 0, SEEK_SET); return len; }
	protected:
		File() : _fp(0)	{}    // this class should not be directly instantiate
		~File()	{ if (_fp) fclose(_fp); }
		// open modes for deriving classes
		bool OpenForRead(const std::string& fileName)	{ _fp = fopen(fileName.c_str(), "rb"); return Good(); }
		bool OpenForWriteTrunc(const std::string& fileName)	{ _fp = fopen(fileName.c_str(), "wb"); return Good(); }
		bool OpenForWriteAppend(const std::string& fileName)	{ _fp = fopen(fileName.c_str(), "ab"); return Good(); }
		bool OpenForReadText(const std::string& fileName)	{ _fp = fopen(fileName.c_str(), "rt"); return Good(); }
		bool OpenForWriteTextTrunc(const std::string& fileName)	{ _fp = fopen(fileName.c_str(), "wt"); return Good(); }
		bool OpenForWriteTextAppend(const std::string& fileName)	{ _fp = fopen(fileName.c_str(), "at"); return Good(); }
		FILE*	_fp;
	};

	/** ReaderFile **
	* File handle used only for binary-mode reading.
	* Provides an interface for reading basic data-types and strings.
	** usage notes **
	* Dynamic-binding not supported.
	*/
	class ReaderFile : public File
	{
	public:
		ReaderFile()	{}
		ReaderFile(const std::string& fileName)	{ Open(fileName); }
		bool Open(const std::string& fileName)	{ return OpenForRead(fileName); }
		// reading
		int Read(char* dstBuf, size_t len)	{ assert(Good()); return fread(reinterpret_cast<void*>(dstBuf), sizeof(char), len, _fp); }
		int Read(std::string& str)			{ long len; int val = Read(len); str.resize(len); val += Read(&str[0], len); return val; }
		int Read(char& val)					{ return Read(reinterpret_cast<char*>(&val), sizeof(char)); }
		int Read(unsigned char& val)		{ return Read(reinterpret_cast<char*>(&val), sizeof(unsigned char)); }
		int Read(short& val)				{ return Read(reinterpret_cast<char*>(&val), sizeof(short)); }
		int Read(unsigned short& val)		{ return Read(reinterpret_cast<char*>(&val), sizeof(unsigned short)); }
		int Read(long& val)					{ return Read(reinterpret_cast<char*>(&val), sizeof(long)); }
		int Read(unsigned long& val)		{ return Read(reinterpret_cast<char*>(&val), sizeof(unsigned long)); }
		int Read(size_t& val)				{ return Read(reinterpret_cast<char*>(&val), sizeof(size_t)); }
		int Read(float& val)				{ return Read(reinterpret_cast<char*>(&val), sizeof(float)); }
		int Read(double& val)				{ return Read(reinterpret_cast<char*>(&val), sizeof(double)); }
	};

	/** WriterFile **
	* File handle used only for binary-mode writing.
	* Provides an interface for writing basic data-types and strings.
	** usage notes **
	* When opening a file which already exists, new data written to this file will erase the old.
	* Dynamic-binding not supported.
	*/
	class WriterFile : public File
	{
	public:
		WriterFile()	{}
		WriterFile(const std::string& fileName)	{ Open(fileName); }
		WriterFile(const std::string& fileName, bool append)	{ Open(fileName, append); }
		bool Open(const std::string& fileName)	{ return OpenForWriteTrunc(fileName); }
		bool Open(const std::string& fileName, bool append)	{ if (append) return OpenForWriteAppend(fileName); else return OpenForWriteTrunc(fileName); }
		// writing
		int Write(const char* srcBuf, size_t len)	{ assert(Good()); return fwrite(reinterpret_cast<const void*>(srcBuf), sizeof(char), len, _fp); }
		int Write(const std::string& str)			{ long len = str.length(); int val = Write(len); return (val+Write(str.data(), len)); }
		int Write(char val)							{ return Write(reinterpret_cast<const char*>(&val), sizeof(char)); }
		int Write(unsigned char val)				{ return Write(reinterpret_cast<const char*>(&val), sizeof(unsigned char)); }
		int Write(short val)						{ return Write(reinterpret_cast<const char*>(&val), sizeof(short)); }
		int Write(unsigned short val)				{ return Write(reinterpret_cast<const char*>(&val), sizeof(unsigned short)); }
		int Write(long val)							{ return Write(reinterpret_cast<const char*>(&val), sizeof(long)); }
		int Write(unsigned long val)				{ return Write(reinterpret_cast<const char*>(&val), sizeof(unsigned long)); }
		int Write(size_t val)						{ return Write(reinterpret_cast<const char*>(&val), sizeof(size_t)); }
		int Write(float val)						{ return Write(reinterpret_cast<const char*>(&val), sizeof(float)); }
		int Write(double val)						{ return Write(reinterpret_cast<const char*>(&val), sizeof(double)); }
	};

	/** TextReaderFile **
	* File handle for reading in text-based mode.
	** usage notes **
	* Dynamic-binding not supported.
	*/
	class TextReaderFile : public File
	{
	public:
		TextReaderFile()	{}
		TextReaderFile(const std::string& fileName)	{ Open(fileName); }
		bool Open(const std::string& fileName)	{ return OpenForReadText(fileName); }
		// reading
		int Read(char* buffer, size_t len)		{ return fread(reinterpret_cast<void*>(buffer), sizeof(char), len, _fp); }
		int Read(std::string& str, size_t len)	{ str.resize(len); return fread(reinterpret_cast<void*>(&str[0]), sizeof(char), len, _fp); }
	};

	/** TextWriterFile **
	* File handle for writing in text-based mode.
	** usage notes **
	* Dynamic-binding not supported.
	*/
	class TextWriterFile : public File
	{
	public:
		TextWriterFile()	{}
		TextWriterFile(const std::string& fileName)	{ Open(fileName); }
		TextWriterFile(const std::string& fileName, bool append)	{ Open(fileName, append); }
		bool Open(const std::string &fileName)	{ return OpenForWriteTextTrunc(fileName); }
		bool Open(const std::string& fileName, bool append)	{ if (append) return OpenForWriteTextAppend(fileName); else return OpenForWriteTextTrunc(fileName); }
		// writing
		int Write(const char* str)			{ size_t len = strlen(str); return fwrite(reinterpret_cast<const void*>(str), sizeof(char), len, _fp); }
		int Write(const std::string& str)	{ return fwrite(reinterpret_cast<const void*>(str.data()), sizeof(char), str.length(), _fp); }
	};
}

#endif

