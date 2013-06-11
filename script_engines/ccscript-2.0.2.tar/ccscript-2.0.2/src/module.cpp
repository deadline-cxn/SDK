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
#include "script.h"

#ifdef	CCXX_NAMESPACES
namespace ost {
#endif

Script::Package *Script::Package::first = NULL;
ScriptModule *ScriptModule::first = NULL;
Script::Property *Script::Property::first = NULL;

Script::Property::Property(const char *name)
{
	id = name;
	next = first;
	first = this;
}

Script::Property *Script::Property::find(const char *name)
{
	Property *prop = first;

	while(prop)
	{
		if(!stricmp(prop->id, name))
			break;
		prop = prop->next;
	}
	return prop;
}

ScriptModule::ScriptModule(const char *name)
{
	next = first;
	first = this;
	cmd = name;
}

ScriptModule *ScriptModule::find(const char *name)
{
	ScriptModule *mod = first;
	char keybuf[33];
	int len = 0;

	while(*name && *name != '.' && len < sizeof(keybuf) - 1)
		keybuf[len++] = *(name++);

	while(mod)
	{
		if(!stricmp(keybuf, mod->cmd))
			return mod;
		mod = mod->next;
	}
	return mod;
}

Script::Session::Session(ScriptInterp *i)
{
	interp = i;
}

void Script::Session::sleepScheduler(timeout_t timeout)
{
	interp->sleepScheduler(timeout);
}

void Script::Session::stepScheduler(const char *event)
{
	interp->stepScheduler(event);
}

Script::Package::Package(char *name) :
DSO(name)
{
	filename = new char[strlen(name) + 1];
	strcpy(filename, name);
	next = first;
	first = this;
}

bool Script::use(const char *name)
{
	Package *pkg = Package::first;
	char buffer[256];

	if(*name != '.' && *name != '/')
	{
		snprintf(buffer, sizeof(buffer), "%s/%s.pkg",  SCRIPT_MODULE_PATH,  name);
		name = buffer;
	}

	while(pkg)
	{
		if(!strcmp(pkg->filename, name))
			return true;
		pkg = pkg->next;
	}

	if(!canAccess(name))
	{
		slog(Slog::levelError) << "use: cannot find " << name << std::endl;
		return false;
	}

	try
	{
		new Package((char *)name);
	}
	catch (DSO *dso)
	{
		slog(Slog::levelError) << "use: cannot load " << name << std::endl;
		delete dso;
		return false;
	}
	return true;
}

#ifdef	CCXX_NAMESPACES
};
#endif
