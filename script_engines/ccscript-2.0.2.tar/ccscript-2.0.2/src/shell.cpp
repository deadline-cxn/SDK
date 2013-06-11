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
#include <cc++/process.h>
#include <cc++/strchar.h>
#include <cstdlib>
#include <sys/wait.h>
#include <iostream>
#include "script.h"

#ifndef	WEXITSTATUS
#define	WEXITSTATUS(x)	(x)
#endif

#ifdef	CCXX_NAMESPACES
using namespace ost;
#endif

class shScript : public ScriptCommand
{
public:
	inline int symsize(void)
		{return atoi(getLast("symsize"));};

	inline int pagesize(void)
		{return atoi(getLast("pagesize"));};

	shScript();
} _image;

class shInterp : public ScriptInterp
{
private:
	friend class shScript;

	bool scrEcho(void);
	bool scrSystem(void);
	bool scrAlarm(void);

	void exit(void)
		{std::cout << "exiting..." << std::endl; std::exit(0);};

public:
	void signal(const char *sigid)
		{ScriptInterp::signal(sigid);};
	shInterp();
} interp;

class shImage : public ScriptImage
{
private:
	Name *getScript(const char *name);

public:
	shImage();
} shImage;

shImage::shImage() :
ScriptImage(&_image)
{
	commit();
};

Script::Name *shImage::getScript(const char *name)
{
	char fname[128];
	Name *line = ScriptImage::getScript(name);
	if(!line)
	{
		strcpy(fname, name);
		strcat(fname, ".scr");
		compile(fname);
		line = ScriptImage::getScript(name);
	}
	return line;
}

shInterp::shInterp() :
ScriptInterp(&_image, _image.symsize(), _image.pagesize())
{
}

bool shInterp::scrEcho(void)
{
	char *val;
	while(NULL != (val = getValue(NULL)))
		std::cout << val;

	std::cout << std::endl;
	advance();
	return true;
}

bool shInterp::scrAlarm(void)
{
	alarm(atoi(getValue("0")));
	advance();
	return true;
}

bool shInterp::scrSystem(void)
{
	char *argv[33];
	int argc = 0;
	char *val;
	int pid, status;

	while(argc < 32 && (NULL != (val = getValue(NULL))))
		argv[argc++] = val;

	argv[argc] = NULL;

	pid = vfork();
	if(!pid)
	{
		execvp(*argv, argv);
		std::exit(-1);
	}
#ifdef	__FreeBSD__
	wait4(pid, &status, 0, NULL);
#else
	waitpid(pid, &status, 0);
#endif
	status = WEXITSTATUS(status);
	if(status)
		error("sys-error");
	else
		advance();
	return false;
}

shScript::shScript() :
ScriptCommand("/script/define")
{
	static Script::Define interp[] = {
		{"echo", (Method)&shInterp::scrEcho, &ScriptCommand::chkHasArgs},
		{"sys", (Method)&shInterp::scrSystem, &ScriptCommand::chkHasArgs},
		{"alarm", (Method)&shInterp::scrAlarm, &ScriptCommand::chkHasArgs},
		{NULL, NULL, NULL}};

	static Keydata::Define keys[] = {
		{"symsize", "128"},
		{"pagesize", "4096"},
		{NULL, NULL}};

	Keydata::load("/script/memory");
	Keydata::load("~script/define");
	Keydata::load(keys);

	trap("alarm");
	trap("hangup");

	load(interp);
}

static RETSIGTYPE handler(int signo)
{
	switch(signo)
	{
	case SIGINT:
		interp.signal("exit");
		break;
	case SIGHUP:
		interp.signal("hangup");
		break;
	case SIGALRM:
		interp.signal("alarm");
		break;
	}
}

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		std::cerr << "use: ccscript scriptname" << std::endl;
		exit(-1);
	}

	interp.attach(argv[1]);
	interp.autoloop(false);

	Process::setPosixSignal(SIGINT, &handler);
	Process::setPosixSignal(SIGHUP, &handler);
	Process::setPosixSignal(SIGALRM, &handler);

	for(;;)
		interp.step();

	exit(-1);
}

