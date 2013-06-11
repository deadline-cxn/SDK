// Copyright (C) 1999-2001 Open Source Telecom Corporation.
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// 
// As a special exception to the GNU General Public License, permission is 
// granted for additional uses of the text contained in its release 
// of ccscript.
// 
// The exception is that, if you link the ccscript library with other
// files to produce an executable, this does not by itself cause the
// resulting executable to be covered by the GNU General Public License.
// Your use of that executable is in no way restricted on account of
// linking the ccscript library code into it.
// 
// This exception does not however invalidate any other reasons why
// the executable file might be covered by the GNU General Public License.
// 
// This exception applies only to the code released under the 
// name ccscript.  If you copy code from other releases into a copy of
// ccscript, as the General Public License permits, the exception does
// not apply to the code that you add in this way.  To avoid misleading
// anyone as to the status of such modified files, you must delete
// this exception notice from them.
// 
// If you write modifications of your own for ccscript, it is your choice
// whether to permit this exception to apply to your modifications.
// If you do not wish that, delete this exception notice.  

#include <cc++/url.h>
#include "script.h"

#ifdef	CCXX_NAMESPACES
namespace ost {
#endif

static class URLProperty : public Script::Property
{
public:
	URLProperty() : Property("url") {};
private:
	void setProperty(char *dp, char *tp, size_t size)
		{urlEncode(tp, dp, size);};
	void getProperty(char *dp, char *tp, size_t size)
		{urlDecode(dp, tp);};
} url;

static class BINProperty : public Script::Property
{
public:
	BINProperty() : Script::Property("bin") {};
private:
	void setProperty(char *dp, char *tp, size_t size)
		{b64Encode(tp, dp, size);};
	void getProperty(char *dp, char *tp, size_t size)
		{b64Decode(dp, tp);};
} bin;

#ifdef	CCXX_NAMESPACES
};
#endif

