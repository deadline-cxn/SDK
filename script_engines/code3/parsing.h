// parsing.h
// Gregory Rosenblatt

#ifndef Parsing_H_
#define Parsing_H_

#include "virtualmachine.h"

namespace MetaLang
{
    // container typedefs
    typedef std::vector<char>           CharArray;
    typedef std::vector<std::string>    StringArray;

    // converts a string into an array of meaningful values
    class Scanner
    {
    public:
        Scanner(const StringArray& codeNames) : _codeNames(codeNames)   {}
        Scanner(const std::string* codeNames, size_t nameCount)
        {
            _codeNames.resize(nameCount);
            const std::string* end = codeNames+nameCount;
            std::copy(codeNames, end, _codeNames.begin());
        }
        bool ScanLine(const std::string& line); // tokenize a line and return success
        const CharArray& GetTokens() const  { return _tokBuffer; }  // get tokens from most recent scan
    private:
        bool SkipSpacing(const std::string& line)
        {
            while (isspace(line.c_str()[_offset]))
                ++_offset;
            if (line.c_str()[_offset] == 0)
                return false;
            return true;
        }
        bool ScanCode(const std::string& line)
        {
            size_t begin = _offset;
            while (isalpha(line.c_str()[_offset]))
                ++_offset;
            return MatchCode(std::string(line, begin, _offset-begin));
        }
        bool ScanNum(const std::string& line)
        {
            size_t begin = _offset;
            while (isdigit(line.c_str()[_offset]))
                ++_offset;
            if (_offset == begin)   // were any digits scanned?
                return false;
            std::string number(line, begin, _offset-begin);
            _tokBuffer.push_back(static_cast<char>(atoi(number.c_str())));
            return true;
        }
        bool MatchCode(const std::string& str)
        {
            char codeVal;
            StringArray::iterator itr = _codeNames.begin();
            for (codeVal = 0; itr != _codeNames.end(); ++itr, ++codeVal)
            {
                if (str == *itr)
                {
                    _tokBuffer.push_back(codeVal);
                    return true;
                }
            }
            return false;
        }
    private:
        StringArray _codeNames;
        CharArray   _tokBuffer;
        size_t      _offset;
    };
}

#endif

