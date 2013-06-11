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

#include <cc++/config.h>
#include <cc++/misc.h>
#include <cc++/slog.h>
#include <cc++/export.h>
#include <cstdlib>
#include "script.h"

#ifdef	CCXX_NAMESPACES
namespace ost {
#endif

Script::Locks ScriptInterp::locks;

Script::Locks::Locks() :
Mutex()
{
	unsigned id;

	for(id = 0; id < MAX_LOCKS; ++id)
		locks[id] = NULL;
}

void Script::Locks::release(ScriptInterp *interp)
{
	unsigned id;
	enterMutex();

	for(id = 0; id < MAX_LOCKS; ++id)
		if(locks[id] == interp)
			locks[id] = NULL;

	leaveMutex();
}

bool Script::Locks::lock(ScriptInterp *interp, unsigned id)
{
	bool rtn = false;

	if(id >= MAX_LOCKS)
		return false;

	enterMutex();
	if(!locks[id])
	{
		locks[id] = interp;
		rtn = true;
	}
	leaveMutex();
	return rtn;
}

bool Script::Locks::unlock(ScriptInterp *interp, unsigned id)
{
	bool rtn = false;

	if(id >= MAX_LOCKS)
		return false;

	enterMutex();
	if(locks[id] == interp)
	{
		locks[id] = NULL;
		rtn = true;
	}
	leaveMutex();
	return rtn;
}

#ifdef	CCXX_NAMESPACES
};
#endif












