// parsing.cpp
// Gregory Rosenblatt

#include "parsing.h"
using namespace MetaLang;

bool Scanner::ScanLine(const std::string& line)
{
    // reset offset and token buffer
    _offset = 0;
    _tokBuffer.clear();

    // check for an empty line
    if (line.empty())
        return false;

    // check for valid line content
    if (!SkipSpacing(line))
        return false;

    // check for a valid opcode
    if (!ScanCode(line))
        return false;

    size_t len = line.length();
    while (_offset < len)   // scan args until the end of line
    {
        if (!SkipSpacing(line)) // get to next arg
            return true;        // unless we're done
        if (!ScanNum(line))
            return false;
    }

    return true;
}

