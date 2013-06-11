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
#include <cc++/url.h>
#include <cc++/export.h>
#include <cstdlib>
#include <cstdio>
#include "script.h"

#ifdef	CCXX_NAMESPACES
namespace ost {
using namespace std;
#endif

static void logerror(const char *script, unsigned id, const char *msg)
{
	slog(Slog::levelError) << script;
	if(id)
		slog() << "(" << id << ")";
	slog() << ": " << msg << endl;
}

static void adjustValue(char *buffer, int diff)
{
	int value = atoi(buffer);
	int len = strlen(buffer);

	sprintf(buffer, "%d", value + diff);
}

ScriptInterp::ScriptInterp(ScriptCommand *cmdset, size_t sym, size_t pg) :
ScriptSymbol(sym, pg)
{
	session = NULL;
	once = true;
	loop = true;
	signalmask = 0;
	packtoken = ',';
	stack = 0;
	cmd = cmdset;
	image = NULL;
	memset(temps, 0, sizeof(temps));

	for(tempidx = 0; tempidx < SCRIPT_TEMP_SPACE; ++tempidx)
		temps[tempidx] = new char[getSymbolSize() + 1];
	tempidx = 0;
	symsize = sym;
	pgsize = pg;
};

ScriptInterp::~ScriptInterp()
{
	for(tempidx = 0; tempidx < SCRIPT_TEMP_SPACE; ++tempidx)
		if(temps[tempidx])
			delete[] temps[tempidx];
}

Script::Name *ScriptInterp::getScriptCopy(const char *name)
{
	char buffer[256];
	Name *scr;

	snprintf(buffer, 255, "%s::%p", name, this);
	scr = image->dupScript(name, buffer);
	if(scr && !strcmp(script[stack].script->name, name))
		script[stack].script = scr;
	return scr;
}

unsigned long ScriptInterp::getMask(void)
{
	return script[stack].line->mask & script[stack].script->mask;
}

Script::Name *ScriptInterp::getScriptImage(const char *name)
{
	char buffer[256];
	Name *scr = image->getScript(name);

	if(!scr)
		return NULL;

	if(scr->mode == Name::COPIED)
	{
		snprintf(buffer, 255, "%s::%p", name, this);
		scr = image->getScript(buffer);
	}
	return scr;
}

void ScriptInterp::getTrigger(bool jump)
{
	char buffer[256];
	Symbol *sym = NULL;
	Name *scr;
	char *id;

	if(script[stack].local)
		sym = script[stack].local->getTrigger();

	if(sym)
		ScriptSymbol::getTrigger();
	else
		sym = ScriptSymbol::getTrigger();
	if(!jump || !sym)
		return;

	id = strchr(sym->id, '.');
	if(id)
		++id;
	else
		id = sym->id;

	setSymbol("script.trigger", sym->data);
	snprintf(buffer, 255, "%s::%s_%s", script[stack].script->name,
		id, sym->data);

	scr = getScriptImage(buffer);
	if(!scr)
	{
		snprintf(buffer, 256, "%s::%s",  script[stack].script->name, id);
		scr = getScriptImage(buffer);
	}
	if(!scr)
	{
		snprintf(buffer, 256, "%s::%s", id, sym->data);
		scr = getScriptImage(buffer);
	}
	if(scr)
	{
		once = true;
		script[stack].caseflag = false;
		script[stack].script = scr;
		script[stack].line = scr->first;
		script[stack].index = 0;
	}
}

bool ScriptInterp::scrEnable(void)
{
	char buffer[256];
	char *name = getValue(NULL);
	Name *scr;
	unsigned long mask, id;
	char *cp;

	if(!name)
	{
		advance();
		return true;
	}

	if(!strncmp(name, "::", 2))
	{
		strcpy(buffer, script[stack].script->name);
		cp = strstr(buffer, "::");
		if(cp)
			*cp = 0;
		strcat(buffer, name);
		name = buffer;
	}
	scr = getScriptCopy(name);
	if(!scr)
	{
		error("script-not-found");
		return true;
	}

	while(NULL != (name = getValue(NULL)))
	{
		id = cmd->getTrapId(name);
		mask = cmd->getTrapMask(name);
		if(!mask)
		{
			error("handler-invalid");
			return true;
		}
		if(!scr->trap[id])
		{
			error("handler-not-found");
			return true;
		}
		scr->mask |= mask;
	}
	advance();
	return true;
}

bool ScriptInterp::scrDisable(void)
{
	char buffer[256];
	char *name = getValue(NULL);
	Name *scr;
	unsigned long mask, id;
	char *cp;

	if(!name)
	{
		advance();
		return true;
	}

	if(!strncmp(name, "::", 2))
	{
		strcpy(buffer, script[stack].script->name);
		cp = strstr(buffer, "::");
		if(cp)
			*cp = 0;
		strcat(buffer, name);
		name = buffer;
	}
	scr = getScriptCopy(name);
	if(!scr)
	{
		error("script-not-found");
		return true;
	}

	while(NULL != (name = getValue(NULL)))
	{
		id = cmd->getTrapId(name);
		mask = cmd->getTrapMask(name);
		if(!mask)
		{
			error("handler-invalid");
			return true;
		}
		if(!scr->trap[id])
		{
			error("handler-not-found");
			return true;
		}
		scr->mask &= ~mask;
	}
	advance();
	return true;
}

void ScriptInterp::advance(void)
{
	script[stack].line = script[stack].line->next;
	if(!script[stack].line && loop)
		script[stack].line = script[stack].script->first;
}

void ScriptInterp::error(const char *errmsg)
{
	setSymbol("script.error", errmsg);
	if(script[stack].script->mask & 0x02)
		trap(1);
	else
		advance();
}

void ScriptInterp::trap(const char *trapid)
{
	unsigned trap = cmd->getTrapId(trapid);
	if(!trap)
	{
		if(!stricmp(trapid, "first") || !stricmp(trapid, "top"))
		{
			script[stack].caseflag = false;
			script[stack].line = script[stack].script->first;
			return;
		}
	}
	ScriptInterp::trap(trap);
}

void ScriptInterp::trap(unsigned id)
{
	Line *trap = NULL;

	if(getGlobalTrap(id))
		return;

retry:
	trap = script[stack].script->trap[id];
	if(!trap && id)
	{
		advance();
		return;
	}
	if(!trap && !id && stack)
	{
		pull();
		goto retry;
	}

	script[stack].caseflag = false;
	script[stack].line = trap;
}

bool ScriptInterp::step(const char *trapname)
{
	unsigned long mask, cmask;
	bool rtn;
	Symbol *sym = NULL;

	script[stack].index = 0;

	if(session)
	{
		session->waitHandler();
		delete session;
		session = NULL;
	}

	if(trapname)
	{
		getTrigger(false);
		mask = cmd->getTrapMask(trapname);
	}
	else
	{
		mask = 0;
		getTrigger(true);
	}

	// auto return code

retry:
	while(!script[stack].line && stack)
	{
		if(script[stack - 1].local == script[stack].local)
			break;
		pull();
		if(script[stack].line)
			advance();
	}

	if(!script[stack].line)
	{
		exit();
		return false;
	}

	cmask = script[stack].line->cmask;
	if(cmask)
	{
		if((cmask & script[stack].script->mask) != cmask)
		{
			advance();
			goto retry;
		}
	}

	if(mask & script[stack].line->mask)
		trap(trapname);

	if(script[stack].line->error)
		sym = getEntry("script.error");
	if(sym)
		sym->flags.type = TRIGGER;
	rtn = execute(script[stack].line->method);
	if(sym)
		sym->flags.type = NORMAL;
	return rtn;
}

bool ScriptInterp::signal(const char *trapname)
{
	unsigned long mask = cmd->getTrapMask(trapname);
	mask &= script[stack].line->mask;
	mask &= script[stack].script->mask;
	if(!mask)
		return false;

	stop(mask);
	trap(trapname);
	return true;
}

bool ScriptInterp::signal(unsigned id)
{
	if(id >= TRAP_BITS)
		return false;

	unsigned long mask = cmd->getTrapMask(id);
	mask &= script[stack].line->mask;
	if(!mask)
	{
		signalmask |= id;
		return false;
	}

	stop(mask);
	trap(id);
	return true;
}

Script::Symbol *ScriptInterp::getLocal(const char *id, size_t size)
{
	Symbol *sym = NULL;

	if(script[stack].local)
		sym = script[stack].local->getEntry(id, size);

	if(!sym)
		sym = getEntry(id, size);

	return sym;
}

bool ScriptInterp::scrSkip(void)
{
	char *val;
	char *id = getValue(NULL);
	Line *line;
	int argc;
	bool cf = false; 

	if(!id)
	{
		advance();
		if(script[stack].line)
			if(script[stack].line->method == &ScriptInterp::scrCase)
				cf = true;

		advance();
		while(script[stack].line && cf)
		{
			if(script[stack].line->method != &ScriptInterp::scrCase)
				break;

			advance();
		}
		return true;
	}

	script[stack].line = script[stack].script->first;
	while(NULL != (line = script[stack].line))
	{
		advance();
		if(line->method != &ScriptInterp::scrLabel)
			continue;

		argc = 0;
		while(argc < line->argc)
		{
			val = getContent(line->args[argc++]);
			if(!stricmp(val, id))
				return true;
		}
	}
	return true;
}

bool ScriptInterp::scrData(void)
{
	advance();
	return true;
}

bool ScriptInterp::scrSlog(void)
{
	unsigned id = getId();
	const char *member = getMember();
	char *val;

	if(!member)
		member = getKeyword("level");

	if(member)
	{
		if(!strnicmp(member, "err", 3))
			slog(Slog::levelError);
		else if(!strnicmp(member, "warn", 4))
			slog(Slog::levelWarning);
		else if(!stricmp(member, "debug"))
			slog(Slog::levelDebug);
		else if(!strnicmp(member, "crit", 4))
			slog(Slog::levelCritical);
		else
			slog(Slog::levelInfo);
	}
	else
		slog(Slog::levelInfo);

	slog() << script[stack].script->name;
	if(id)
		slog() << "(" << id << ")";
	slog() << ": ";
	while(NULL != (val = getValue(NULL)))
		slog() << val;
	slog() << endl;
	advance();
	return true;
}

bool ScriptInterp::scrUse(void)
{
#ifdef	HAVE_MODULES
	advance();
#else
	error("missing-dynloader-support");
#endif
	return true;
}

bool ScriptInterp::scrMissing(void)
{
	char *chk = getValue(NULL);

	if(!chk)
	{
		error("missing-no-keyword");
		return true;
	}

	while(chk)
	{
		if(!cmd->getHandler(chk))
			return scrGoto();
		chk = getValue(NULL);
		chk = getValue(NULL);
	}

	advance();
	return true;
}

bool ScriptInterp::scrHas(void)
{
	char *chk = getValue(NULL);

	if(!chk)
	{
		error("has-no-keyword");
		return true;
	}

	while(chk)
	{
		if(cmd->getHandler(chk))
			return scrGoto();
		chk = getValue(NULL);
		chk = getValue(NULL);
	}

	advance();
	return true;
}

bool ScriptInterp::scrSelect(void)
{
	const char *value = getValue(NULL);
	const char *match = getMember();
	int len, l2;
	int offset = 0;

	enum
	{
		SEL_NORMAL,
		SEL_LENGTH,
		SEL_PREFIX,
		SEL_SUFFIX,
		SEL_VALUE
	} sm = SEL_NORMAL;

	if(!value)
	{
		error("branch-no-select");
		return true;
	}

	if(match)
	{
		if(!strnicmp(match, "len", 3))
			sm = SEL_LENGTH;
		else if(!strnicmp(match, "pre", 3))
			sm = SEL_PREFIX;
		else if(!strnicmp(match, "suf", 3))
			sm = SEL_SUFFIX;
		else if(!strnicmp(match, "val", 3))
			sm = SEL_VALUE;
		else if(!strnicmp(match, "end", 3))
			sm = SEL_SUFFIX;
		else
		{
			offset = atoi(match);
			sm = SEL_PREFIX;
		}
	}
	len = strlen(value);
	if(offset < 0)
		offset = len + offset;
	if(offset < 0)
		offset = 0;
	if(offset > len)
		offset = len;

	while(NULL != (match = getValue(NULL)))
	{
		l2 = strlen(match);
		switch(sm)
		{
		case SEL_SUFFIX:
			if(len < l2)
				break;
			if(!stricmp(match, value + len - l2))
				return scrGoto();
			break;
		case SEL_PREFIX:
			if(!strnicmp(match + offset, value, l2))
				return scrGoto();
			break;
		case SEL_NORMAL:
			if(!stricmp(match, value))
				return scrGoto();
			break;
		case SEL_LENGTH:
			if(atoi(match) == len)
				return scrGoto();
			break;
		case SEL_VALUE:
			if(atoi(match) == atoi(value))
				return scrGoto();
			break;
		}
		getValue(NULL);
	}
	advance();
	return true;
}

bool ScriptInterp::scrOnce(void)
{
	if(getOnce())
		return scrGoto();
	advance();
	return true;
}

bool ScriptInterp::scrOn(void)
{
	char *sig = getValue(NULL);
	unsigned long mask;

	if(!sig)
	{
		error("on-no-signal");
		return true;
	}

	mask = cmd->getTrapMask(sig);
	if(!mask)
	{
		error("on-invalid-signal");
		return true;
	}
	if(signalmask & mask)
	{
		signalmask &= ~mask;
		return scrGoto();
	}
	advance();
	return true;
}

bool ScriptInterp::scrGoto(void)
{
	char namebuf[256];
	char *label = getValue(NULL);
	char *ext;
	int len;
	Line *skip;
	Name *scr;

	if(!label)
	{
		error("branch-failed");
		return true;
	}

	if(*label == '^')
	{
		initKeywords(0);
		trap(++label);
		return true;
	}

	if(*label >= '0' && *label <= '9' && !label[1])
	{
		initKeywords(0);
		Line *skip = script[stack].script->skip[atoi(label)];
		if(!skip)
			advance();

		return true;
	}

	len = strlen(label);
	if(!strncmp(label, "::", 2))
	{
		strcpy(namebuf, script[stack].script->name);
		ext = strstr(namebuf, "::");
		if(ext)
			strcpy(ext, label);
		else
			strcat(namebuf, label);
		label = namebuf;
	}
	else if(label[len - 1] == ':')
	{
		strcpy(namebuf, script[stack].script->name);
		ext = strstr(namebuf, "::");
		if(ext)
			strcpy(ext + 2, label);
		else
		{
			strcat(namebuf, "::");
			strcat(namebuf, label);
		}
		label = namebuf;
		len = strlen(label);
		label[len - 1] = 0;
	}

	scr = getScriptImage(label);
	if(!scr)
	{
		error("script-not-found");
		return true;
	}
	initKeywords(0);
	once = true;
	script[stack].caseflag = false;
	script[stack].script = scr;
	script[stack].line = scr->first;
	script[stack].index = 0;
	return true;
}

bool ScriptInterp::scrTry(void)
{
	char namebuf[256];
	Name *scr;
	char *label, *ext;

	while(NULL != (label = getValue(NULL)))
	{
		if(!strncmp(label, "::", 2))
		{
			strcpy(namebuf, script[stack].script->name);
			ext = strstr(namebuf, "::");
			if(ext)
				strcpy(ext, label);
			else
				strcat(namebuf, label);
			label = namebuf;
		}
		scr = getScriptImage(label);
		if(!scr)
			continue;
		once = true;
		script[stack].caseflag = false;
		script[stack].script = scr;
		script[stack].line = scr->first;
		script[stack].index = 0;
		return true;
	}
	advance();
	return true;
}

bool ScriptInterp::scrArm(void)
{
	Symbol *opt;

	while(opt = getVariable(0))
	{
		if(opt->flags.type == NORMAL)
			opt->flags.type = TRIGGER;
	}
	advance();
	return true;
}

bool ScriptInterp::scrDisarm(void)
{
	Symbol *opt;

	while(opt = getVariable(0))
	{
		if(opt->flags.type == TRIGGER)
			opt->flags.type = NORMAL;
	}
	advance();
	return true;
}

bool ScriptInterp::scrSwap(void)
{
	Symbol *s1, *s2;
	s1 = getVariable(getSymbolSize());
	s2 = getVariable(getSymbolSize());
	char *id;

	if(!s1 || !s2)
	{
		error("symbol-not-found");
		return true;
	}
	enterMutex();
	id = s1->id;
	s1->id = s2->id;
	s2->id = id;
	leaveMutex();
	advance();
	return true;
}

bool ScriptInterp::scrRead(void)
{
	Symbol *sym;
	Name *scr;
	Line *rd;
	const char *mem = getMember();
	int argc = 0;
	char *value = NULL;
	char namebuf[256];

	if(!mem)
		mem = "";
	if(!stricmp(mem, "from"))
	{
		value = getValue(script[stack].script->name);
		if(!strnicmp(value, "::", 2))
		{
			strcpy(namebuf, script[stack].script->name);
			strcat(namebuf, value);
			value = namebuf;
		}
		scr = getScriptImage(value);
		if(!scr)
		{
			error("no-source-to-read");
			return true;
		}
		script[stack].read = scr->first;
		value = getValue(NULL);
		advance();
		return true;
	}
	rd = script[stack].read;
	while(rd)
	{
		if(rd->method == &ScriptInterp::scrData)
			break;
		rd = rd->next;
	}
	if(!rd)
	{
		script[stack].read = NULL;
		error("end-of-data");
		return true;
	}
	else
		script[stack].read = rd->next;
	while(argc < rd->argc)
	{
		value = getContent(rd->args[argc++]);
		if(!value)
			break;
		sym = getVariable(strlen(value));
		if(!sym)
			break;
		if(sym->flags.readonly)
			continue;
		strcpy(sym->data, value);
		sym->flags.initial = false;
		if(sym->flags.commit)
			commit(sym);
	}

	advance();
	return true;
}

bool ScriptInterp::scrRepeat(void)
{
	unsigned short loop = script[stack].line->loop;
	Line *line;
	int index = script[stack].index;
	int count;

	script[stack].index = 0;
	count = atoi(getValue("0"));

	if(index >= count)
	{
		line = script[stack].line->next;
		while(line)
		{
			if(line->loop == loop)
			{
				script[stack].line = line;
				advance();
				return true;
			}
			line = line->next;
		}
		error("loop-overflow");
		return true;
	}
	script[stack].index = ++index;
	if(!push())
		return true;

	advance();
	return true;
}

bool ScriptInterp::scrForeach(void)
{
	Symbol *sym;
	unsigned short loop = script[stack].line->loop;
	Line *line;
	int index = script[stack].index;
	const char *value = getMember();
	int len = 0;

	script[stack].index = 0;

	sym = getVariable(getSymbolSize());

	if(!sym)
	{
		error("symbol-not-found");
		return true;
	}

	if(sym->flags.readonly)
	{
		error("symbol-readonly");
		return true;
	}

	sym->data[sym->flags.size] = 0;

	if(value && !index)
		len = atoi(value);

	value = getValue();
	if(!value)
	{
		error("list-missing");
		return true;
	}

	while(value[index] && (len--) > 1)
	{
		while(value[index] && value[index] != packtoken)
			++index;
		if(value[index] == packtoken)
			++index;
	}

	if(!value[index])
	{
		line = script[stack].line->next;
		while(line)
		{
			if(line->loop == loop)
			{
				script[stack].line = line;
				advance();
				return true;
			}
			line = line->next;
		}
		error("loop-overflow");
		return true;
	}
	len = 0;
	while(value[index] && value[index] != packtoken && len < sym->flags.size)
		sym->data[len++] = value[index++];
	if(value[index] == packtoken)
		++index;
	sym->data[len] = 0;
	sym->flags.initial = false;
	if(sym->flags.commit)
		commit(sym);

	script[stack].index = index;
	if(!push())
		return true;

	advance();
	return true;
}

bool ScriptInterp::scrFor(void)
{
	Symbol *sym;
	unsigned short loop = script[stack].line->loop;
	Line *line;
	int index = script[stack].index;
	script[stack].index = 0;

	if(!index)
		++index;

	sym = getVariable(getSymbolSize());
	char *value;

	if(!sym)
	{
		error("symbol-not-found");
		return true;
	}

	if(sym->flags.readonly)
	{
		error("symbol-readonly");
		return true;
	}

	sym->data[sym->flags.size] = 0;
	script[stack].index = index;
	value = getValue(NULL);
	if(!value)
	{
		line = script[stack].line->next;
		while(line)
		{
			if(line->loop == loop)
			{
				script[stack].line = line;
				advance();
				return true;
			}
			line = line->next;
		}
		error("loop-overflow");
		return true;
	}

	strncpy(sym->data, value, sym->flags.size);
	sym->flags.initial = false;
	if(sym->flags.commit)
		commit(sym);

	if(!push())
		return true;

	advance();
	return true;
}

bool ScriptInterp::scrDo(void)
{
	unsigned short loop = script[stack].line->loop;
	Line *line;

	script[stack].index = 0;	// always reset

	if(script[stack].line->argc)
	{
		if(!conditional())
		{
			line = script[stack].line->next;
			while(line)
			{
				if(line->loop == loop)
				{
					script[stack].line = line;
					advance();
					return true;
				}
				line = line->next;
			}
			error("loop-overflow");
			return true;
		}
	}

	if(!push())
		return true;

	advance();
	return true;
}

bool ScriptInterp::scrLoop(void)
{
	unsigned short loop;

	if(stack < 1)
	{
		error("stack-underflow");
		return true;
	}

	loop = script[stack - 1].line->loop;
	if(!loop)
	{
		error("stack-not-loop");
		return true;
	}

	if(script[stack].line->argc)
	{
		if(!conditional())
		{
			script[stack - 1] = script[stack];
			--stack;
			advance();
			return true;
		}
	}

	--stack;
	return execute(script[stack].line->method);
}

bool ScriptInterp::scrContinue(void)
{
	Line *line;
	unsigned short loop;

	if(script[stack].line->argc)
	{
		if(!conditional())
		{
			advance();
			return true;
		}
	}

	if(stack < 1)
	{
		error("stack-underflow");
		return true;
	}

	loop = script[stack - 1].line->loop;
	line = script[stack].line->next;

	if(!loop)
	{
		error("stack-not-loop");
		return true;
	}

	while(line)
	{
		if(line->loop == loop)
		{
			script[stack].line = line;
			return true;
		}
		line = line->next;
	}
	error("loop-overflow");
	return true;
}

bool ScriptInterp::scrBreak(void)
{
	Line *line;
	unsigned short loop;

	if(script[stack].line->argc)
	{
		if(!conditional())
		{
			advance();
			return true;
		}
	}

	if(stack < 1)
	{
		error("stack-underflow");
		return true;
	}

	loop = script[stack - 1].line->loop;
	line = script[stack].line->next;

	if(!loop)
	{
		error("stack-not-loop");
		return true;
	}

	while(line)
	{
		if(line->loop == loop)
		{
			--stack;
			script[stack].line = line;
			advance();
			return true;
		}
		line = line->next;
	}
	error("loop-overflow");
	return true;
}

bool ScriptInterp::scrEndcase(void)
{
	advance();
	return true;
}

bool ScriptInterp::scrEndif(void)
{
	advance();
	return true;
}

bool ScriptInterp::scrLabel(void)
{
	advance();
	return true;
}

bool ScriptInterp::scrElse(void)
{
	int level = 0;
	Line *line;

	advance();

	while(NULL != (line = script[stack].line))
	{
		advance();

		if(line->method == &ScriptInterp::scrThen)
			++level;
		else if(line->method == &ScriptInterp::scrEndif)
		{
			if(!level)
				return true;
		}
	}
	return true;
}

bool ScriptInterp::scrThen(void)
{
	int level = 0;
	Line *line;

	advance();

	while(NULL != (line = script[stack].line))
	{
		advance();

		if(line->method == &ScriptInterp::scrThen)
			++level;
		else if(line->method == &ScriptInterp::scrElse)
		{
			if(!level)
				return true;
		}
		else if(line->method == &ScriptInterp::scrEndif)
		{
			if(!level)
				return true;
			--level;
		}
	}
	return true;
}

bool ScriptInterp::scrIfThen(void)
{
	if(!conditional())
		advance();
	advance();
	return true;
}

bool ScriptInterp::scrCase(void)
{
	unsigned short loop = 0xffff;
	Line	*line;

	if(!script[stack].caseflag)
		if(conditional() || !script[stack].line->argc)
		{
			script[stack].caseflag = true;
			advance();
			while(script[stack].line)
			{
				if(script[stack].line->method == &ScriptInterp::scrCase)
					advance();
				else
					return true;
			}
			return true;
		}

	if(stack)
		loop = script[stack - 1].line->loop;

	advance();
	while(NULL != (line = script[stack].line))
	{
		if(line->loop == loop)
			return true;

		if(line->method == &ScriptInterp::scrCase && !script[stack].caseflag)
			return true;

		if(line->method == &ScriptInterp::scrEndcase)
			return true;

		advance();
	}
	return true;
}

bool ScriptInterp::scrIf(void)
{
	if(conditional())
	{
		if(script[stack].index < script[stack].line->argc)
			return scrGoto();
		advance();
		if(script[stack].line->method == &ScriptInterp::scrThen)
			advance();
		return true;
	}
	advance();
	return true;
}

bool ScriptInterp::scrLock(void)
{
	unsigned id = atoi(getValue("0"));
	const char *member = getMember();

	if(!member)
		member = "wait";

	if(!stricmp(member, "try"))
	{
		if(!locks.lock(this, id))
			error("lock-busy");
		else
			advance();
		return true;
	}

	if(!stricmp(member, "unlock") || !stricmp(member, "ulock"))
	{
		if(!locks.unlock(this, id))
			error("lock-invalid");
		else
			advance();
		return true;
	}

	if(!locks.lock(this, id))
		return true;

	advance();
	return true;
}

bool ScriptInterp::scrCall(void)
{
	int index = script[stack].index;
	int id = 0;
	Symbol *sym;
	char symname[8];
	char *arg;
	const char *member = getMember();

	if(!member)
		member = "none";

	if(!push())
		return true;

	if(!stricmp(member, "global") || !stricmp(member, "public"))
		script[stack].local = NULL;
	else if(stricmp(member, "local") && stricmp(member, "protected"))
		script[stack].local = new ScriptSymbol(symsize, pgsize);

	while(NULL != (arg = getValue(NULL)))
	{
		sprintf(symname, "%d", id++);
		sym = getLocal(symname, pgsize);
		if(sym)
		{
			sym->flags.initial = false;
			strcpy(sym->data, arg);
		}
	}

	script[stack].index = index;
	return scrGoto();
}

bool ScriptInterp::scrPop(void)
{
	char *level = getValue(NULL);
	int max;

	if(level)
	{
		max = atoi(level);
		if(max < stack)
			stack = max;
		advance();
		return true;
	}

	if(stack < 1)
	{
		error("stack-underflow");
		return true;
	}
	if(script[stack].local != script[stack - 1].local)
	{
		if(script[stack].local)
			delete script[stack].local;
		script[stack].local = script[stack - 1].local;
	}
	script[stack - 1] = script[stack];
	--stack;
	advance();
	return true;
}

bool ScriptInterp::scrReturn(void)
{
	bool erflag;
	char *label = getValue(NULL), *ext;
	char namebuf[256];
	int len;
	Name *scr;

	do {
		if(!pull())
			return true;

	} while(script[stack].line->loop != 0);

retry:
	if(!label)
	{
		advance();
		return true;
	}

	if(!*label)
	{
		advance();
		return true;
	}
	if(*label == '^')
	{
		trap(++label);
		return true;
	}
	if(*label >= '0' && *label <= '9' && !label[1])
	{
		Line *skip =
			script[stack].script->skip[atoi(label)];
               	if(!skip)
                       	advance();
                return true;
	}
        len = strlen(label);
        if(!strncmp(label, "::", 2))
        {
                strcpy(namebuf, script[stack].script->name);
                ext = strstr(namebuf, "::");
                if(ext)
                        strcpy(ext, label);
                else
                        strcat(namebuf, label);
                label = namebuf;
        }
        else if(label[len - 1] == ':')
        {
                strcpy(namebuf, script[stack].script->name);
                ext = strstr(namebuf, "::");
                if(ext)
                        strcpy(ext + 2, label);
                else
                {
                        strcat(namebuf, "::");
                        strcat(namebuf, label);
                }
                label = namebuf;
		len = strlen(label);
		label[len - 1] = 0;
        }

        scr = getScriptImage(label);
        if(!scr)
        {
		label = getValue(NULL);
		if(label)
			goto retry;
                error("script-not-found");
                return true;
        }
        once = true;
	script[stack].caseflag = false;
        script[stack].script = scr;
        script[stack].line = scr->first;
        script[stack].index = 0;
        return true;
}

bool ScriptInterp::scrExit(void)
{
	while(stack)
		pull();

	script[stack].line = NULL;
	return true;
}

bool ScriptInterp::scrRemove(void)
{
	Symbol *sym = getVariable(0);
	char *val;

	if(!sym)
	{
		error("symbol-missing");
		return true;
	}

	if(sym->flags.type != FIFO &&
	   sym->flags.type != STACK &&
	   sym->flags.type != SEQUENCE &&
	   sym->flags.type != CACHE)
	{
		error("symbol-invalid");
		return true;
	}

	while(NULL != (val = getValue()))
		removeSymbol(sym, val);

	advance();
	return true;
}

bool ScriptInterp::scrClear(void)
{
	Symbol *sym = getVariable(0);

	while(sym)
	{
		if(sym->flags.type == FIFO || sym->flags.type == SEQUENCE || sym->flags.type == STACK || sym->flags.type == CACHE)
		{
			sym->data[1] = sym->data[2] = 0;
			continue;
		}
		if(sym->flags.readonly && !sym->flags.commit)
		{
			sym = getVariable(0);
			continue;
		}
		sym->data[0] = 0;
		sym->flags.initial = true;
		if(sym->flags.commit)
			commit(sym);
		sym = getVariable(0);
	}
	advance();
	return true;
}

bool ScriptInterp::scrPost(void)
{
	Symbol *sym;
	char *opt = getOption(NULL);
	if(!opt)
	{
		error("symbol-missing");
		return true;
	}

	if(*opt != '%')
	{
		error("symbol-invalid");
		return true;
	}

	sym = getLocal(++opt, 0);
	if(!sym)
	{
		error("symbol-missing");
		return true;
	}

	if(sym->flags.type != FIFO && sym->flags.type != SEQUENCE && sym->flags.type != STACK && sym->flags.type != CACHE)
	{
		error("symbol-type-invalid");
		return true;
	}

	while(NULL != (opt = getValue(NULL)))
		postSymbol(sym, opt);

	advance();
	return true;
}

bool ScriptInterp::scrList(void)
{
	unsigned count = 0;
	Symbol *sym;
	const char *opt;
	const char *name = getOption(NULL);
	char def[96], buf[8];

	if(!name)
	{
		error("list-missing");
		return true;
	}

	if(*name != '%')
	{
		error("list-missing-array-name");
		return true;
	}
	++name;

	strcpy(def, name);
	strcat(def, ".#####");
	if(!setAlias(name, def))
	{
		error("array-unavailable");
		return true;
	}

	while(NULL != (opt = getOption(NULL)))
	{
		++count;
		snprintf(def, sizeof(def), "%s.%d", name, count);
		setConst(def, opt);
	}
	sprintf(buf, "%d", count);
	strcpy(def, name);
	strcat(def, ".count");
	setConst(def, buf);
	strcpy(def, name);
	strcat(def, ".limit");
	setConst(def, buf);
	strcpy(def, name);
	strcat(def, ".index");
	sym = getEntry(def, 5);
	if(!sym)
	{
		error("array-no-index");
		return true;
	}
	sym->flags.initial = false;
	sym->flags.commit = true;
	sym->flags.type = INDEX;
	strcpy(sym->data, "0");
	advance();
	return true;
}

bool ScriptInterp::scrArray(void)
{
	unsigned size = getSymbolSize();
	unsigned count = atoi(getValue("0"));
	const char *mem = getMember();
	char def[96], buf[8];
	unsigned index;
	Symbol *sym;

	if(mem)
		size = atoi(mem);

	if(!count || !size)
	{
		error("array-no-size");
		return true;
	}

	while(NULL != (mem = getOption(NULL)))
	{
		strcpy(def, mem);
		strcat(def, ".#####");
		if(!setAlias(mem, def))
		{
			error("array-unavailable");
			return true;
		}

		snprintf(def, sizeof(def), "%s.index", mem);
		sym = getEntry(def, 5);
		if(!sym)
		{
			error("array-no-index");
			return true;
		}
		sym->flags.initial = false;
		sym->flags.commit = true;
		sym->flags.type = INDEX;
		strcpy(sym->data, "0");

		sprintf(buf, "%d", count);
		snprintf(def, sizeof(def), "%s.count", mem);
		setConst(def, buf);
		snprintf(def, sizeof(def), "%s.limit", mem);
		setConst(def, buf);

		for(index = 1; index <= count; ++index)
		{
			snprintf(def, sizeof(def), "%s.%d", mem, index);
			setSymbol(def, size);
		}
	 }
	advance();
	return true;
}

bool ScriptInterp::scrFifo(void)
{
	unsigned char rec = getSymbolSize();
	unsigned char count = atoi(getValue("0"));
	const char *mem = getMember();
	int rtn;
	if(mem)
		rec = atoi(mem);

	if(!count || !rec)
	{
		error("symbol-no-size");
		return true;
	}

	while(NULL != (mem = getOption(NULL)))
	{
		if(strchr(mem, '.') || !script[stack].local)
			rtn = makeFifo(mem, count, rec);
		else
			rtn = script[stack].local->makeFifo(mem, count,  rec);
		if(!rtn)
		{
			error("fifo-make-failed");
			return true;
		}
	}
	advance();
	return true;
}

bool ScriptInterp::scrCounter(void)
{
	const char *id;
	int rtn;

	while(NULL != (id = getOption(NULL)))
	{
		if(strchr(id, '.') || !script[stack].local)
			rtn = makeCounter(id);
		else
			rtn = script[stack].local->makeCounter(id);
		if(!rtn)
		{
			error("counter-make-failed");
			return true;
		}
	}
	advance();
	return true;
}

bool ScriptInterp::scrStack(void)
{
	unsigned char rec = getSymbolSize();
	unsigned char count = atoi(getValue("0"));
	const char *mem = getMember();
	if(mem)
		rec = atoi(mem);

	if(!count || !rec)
	{
		error("symbol-no-size");
		return true;
	}

	while(NULL != (mem = getOption(NULL)))
	{
		if(!makeStack(mem, count, rec))
		{
			error("stack-make-failed");
			return true;
		}
	}
	advance();
	return true;
}


bool ScriptInterp::scrSequence(void)
{
	unsigned char rec = getSymbolSize();
	unsigned char count = atoi(getValue("0"));
	const char *mem = getMember();
	int rtn;
	if(mem)
		rec = atoi(mem);

	if(!count || !rec)
	{
		error("symbol-no-size");
		return true;
	}

	mem = getOption(NULL);
	if(!mem)
	{
		error("symbol-missing");
		return true;
	}

	if(strchr(mem, '.') || !script[stack].local)
		rtn = makeSequence(mem, count, rec);
	else
		rtn = script[stack].local->makeSequence(mem, count, rec);

	if(!rtn)
	{
		error("sequence-make-failed");
		return true;
	}
	--script[stack].index;
	scrPost();
}

bool ScriptInterp::scrCache(void)
{
	unsigned char rec = getSymbolSize();
	unsigned char count = atoi(getValue("0"));
	const char *mem = getMember();
	int rtn;
	if(mem)
		rec = atoi(mem);

	if(!count || !rec)
	{
		error("symbol-no-size");
		return true;
	}

	mem = getOption(NULL);
	if(!mem)
	{
		error("symbol-missing");
		return true;
	}

	if(strchr(mem, '.') || !script[stack].local)
		rtn = makeCache(mem, count, rec);
	else
		rtn = script[stack].local->makeCache(mem, count, rec);

	if(!rtn)
	{
		error("cache-make-failed");
		return true;
	}
	--script[stack].index;
	scrPost();
}

bool ScriptInterp::scrSize(void)
{
	Symbol *sym;
	int size = atoi(getValue("0"));
	char *opt = getOption(NULL);

	if(!opt)
	{
		error("symbol-not-specified");
		return true;
	}

	if(!size)
	{
		error("symbol-no-size");
		return true;
	}

	while(opt)
	{
		if(*opt == '@')
		{
			opt = getSymbol(++opt);
			if(!opt)
			{
				opt = getOption(NULL);
				continue;
			}
		}
		else if(*opt != '%')
		{
			error("symbol-not-constant");
			return true;
		}

		if(*opt == '%')
			++opt;
		if(strchr(opt, '.') || !script[stack].local)
			sym = getEntry(opt, size);
		else
			sym = script[stack].local->getEntry(opt, size);
		opt = getOption(NULL);
	}

	advance();
	return true;
}

bool ScriptInterp::scrInc(void)
{
	Property *prop = NULL;
	Symbol *sym = getVariable(getSymbolSize());
	int len;
	int value = 0;
	const char *opt;

	if(!sym)
	{
		error("symbol-not-found");
		return true;
	}

	if(sym->flags.readonly)
	{
		error("symbol-readonly");
		return true;
	}

	len = sym->flags.size;
	opt = getValue(NULL);
	if(opt)
	{
		while(opt)
		{
			value += atoi(opt);
			opt = getValue(NULL);
		}
	}
	else
		++value;


	opt = getMember();
	if(opt)
		prop = Property::find(opt);
	if(prop)
		prop->adjProperty(sym->data, sym->flags.size, value);
	else
		adjustValue(sym->data, value);
	if(sym->flags.commit)
		commit(sym);

	advance();
	return true;
}

bool ScriptInterp::scrDec(void)
{
	Property *prop = NULL;
	Symbol *sym = getVariable(getSymbolSize());
	int len;
	int value = 0;
	const char *opt;

	if(!sym)
	{
		error("symbol-not-found");
		return true;
	}

	if(sym->flags.readonly)
	{
		error("symbol-readonly");
		return true;
	}

	len = sym->flags.size;
	opt = getValue(NULL);
	if(opt)
	{
		while(opt)
		{
			value -= atoi(opt);
			opt = getValue(NULL);
		}
	}
	else
		--value;

	opt = getMember();
	if(opt)
		prop = Property::find(opt);
	if(prop)
		prop->adjProperty(sym->data, sym->flags.size, value);
	else
		adjustValue(sym->data, value);
	if(sym->flags.commit)
		commit(sym);

	advance();
	return true;
}

bool ScriptInterp::scrTryeach(void)
{
	char *gp = getValue(NULL);
	const char *mem = getMember();
	int offset = 0;
	char namebuf[256];
	char *np;
	Name *scr;

	if(mem)
		offset = atoi(mem);

	if(!gp)
	{
		error("list-not-found");
		return true;
	}

	while(*gp && (offset--) > 1)
	{
		gp = strchr(gp, packtoken);
		if(gp)
			++gp;
		else
			gp = "";
	}

	while(*gp)
	{
		np = namebuf;
		while(*gp && *gp != packtoken)
			*(np++) = *(gp++);

		*np = 0;
		if(*gp == packtoken)
			++gp;

		scr = getScriptImage(namebuf);
		if(!scr)
			continue;

		once = true;
		script[stack].caseflag = false;
		script[stack].script = scr;
		script[stack].line = scr->first;
		script[stack].index = 0;
		return true;
	}
	advance();
	return true;
}

bool ScriptInterp::scrUnpack(void)
{
	Symbol *sym;
	char *pp;
	char *name;
	char *gp = getValue(NULL);
	int len;
	const char *mem = getMember();
	int offset = 0;

	if(mem)
		offset = atoi(mem);

	if(!gp)
	{
		error("source-not-found");
		return true;
	}

	while(*gp && (offset--) > 1)
	{
		gp = strchr(gp, packtoken);
		if(!gp)
			gp = "";
		else
			++gp;
	}

	while(*gp && NULL != (name = getOption(NULL)))
	{
		if(*name == '@')
			sym = getIndirect(++name);
		if(*name == '%')
			sym = getEntry(++name, getSymbolSize());
		else
			sym = NULL;

		if(sym)
		{
			if(sym->flags.readonly)
				pp = NULL;
			else
				pp = sym->data;
		}
		else
			pp = NULL;

		len = 0;
		while(pp && *gp && *gp != packtoken)
		{
			if(++len > sym->flags.size)
				break;

			*(pp++) = *(gp++);
		}

		if(pp)
		{
			*pp = 0;
			sym->flags.initial = false;
			if(sym->flags.commit)
				commit(sym);
		}

		while(*gp && *gp != packtoken)
			++gp;

		if(*gp == packtoken)
			++gp;
	}
	advance();
	return true;
}

bool ScriptInterp::scrPack(void)
{
	Symbol *sym;
	char *name = getOption(NULL);
	char *value;
	int len = 0;
	char *pp;

	if(!name)
	{
		error("symbol-not-found");
		return true;
	}

	if(*name == '@')
		sym = getIndirect(++name);
	else if(*name == '%')
		sym = getEntry(++name, getSymbolSize());

	if(!sym)
	{
		error("symbol-not-found");
		return true;
	}

	if(sym->flags.readonly)
	{
		error("symbol-read-only");
		return true;
	}

	pp = sym->data;
	len = strlen(pp);
	pp += len;

	value = getValue(NULL);
	while(len < sym->flags.size && value != NULL)
	{
		if(len)
		{
			*(pp++) = packtoken;
			++len;
		}
		while(*value && len++ < sym->flags.size)
			*(pp++) = *(value++);

		value = getValue(NULL);
	}
	*pp = 0;
	sym->flags.initial = false;
	if(sym->flags.commit)
		commit(sym);
	advance();
	return true;
}

bool ScriptInterp::scrPacked(void)
{
	char *value = getValue(NULL);
	if(value)
		packtoken = *value;
	advance();
	return true;
}

bool ScriptInterp::scrLoadable(void)
{
	Line *line = getScript();
	char *cmd = line->cmd;
	char keybuf[33];
	int len = 0;
	char *kw = keybuf;
	ScriptModule *mod = ScriptModule::first;

	while(len ++ < 32 && *cmd && *cmd != '.')
		*(kw++) = *(cmd++);
	*kw = 0;

	while(mod)
	{
		if(!stricmp(keybuf, mod->cmd))
			break;
		mod = mod->next;
	}

	if(!mod)
	{
		error("module-not-found");
		return true;
	}
	kw = mod->getSession(this, line, &session);
	if(line != getScript())
		return true;

	if(kw)
	{
		error(kw);
		return true;
	}
	if(!session)
	{
		advance();
		return true;
	}
	return false;
}

bool ScriptInterp::scrRef(void)
{
	Symbol *sym;
	char *ref;
	char refbuf[256];
	int len = 0;

	while(NULL != (ref = getValue(NULL)) && len < 250)
	{
		if(*ref == '%')
			++ref;

		if(script[stack].line->argc <= script[stack].index)
			break;

		if(len)
			snprintf(refbuf + len, sizeof(refbuf) - len,
				".%s", ref);
		else
			snprintf(refbuf + len, sizeof(refbuf) - len,
				"%s", ref);
		len = strlen(refbuf);
	}
	
	sym = getLocal(ref, len + sizeof(ScriptSymbol *));
	if(!sym->flags.initial)
	{
		error("alias-invalid-reference-object");
		return true;
	}
	advance();
	enterMutex();
	*((ScriptSymbol **)(sym->data)) = this;
	strcpy(sym->data + sizeof(ScriptSymbol *), refbuf);	
	sym->flags.initial = false;
	sym->flags.readonly = true;
	sym->flags.type = REF;
	leaveMutex();
	return true;
}	

bool ScriptInterp::scrAlias(void)
{
	char *sym;
	char *src;

	while(NULL != (sym = getValue(NULL)))
	{
		src = getValue(NULL);
		if(!src)
		{
			error("alias-no-source");
			return true;
		}

		if(!setAlias(sym, src))
		{
			error("alias-failure");
			return true;
		}
	}

	advance();
	return true;
}

bool ScriptInterp::scrConst(void)
{
	char *sym = getValue(NULL);
	char *val;
	char buffer[getSymbolSize()];
	strstream str(buffer, getSymbolSize());
	int count = 0;
	int idx = 0;
	Line *line = getScript();

	buffer[0] = 0;
	if(!sym)
	{
		while(idx < line->argc)
		{
			val = line->args[idx++];
			if(*val != '=')
				continue;
			if(*(++val) == '%')
				++val;
			setConst(val, line->args[idx++]);
			++count;
		}

		if(count)
			advance();
		else
			error("const-not-specified");
		return true;
	}

	while(NULL != (val = getValue(NULL)))
		str << val;
	str << ends;
	if(!setConst(sym, buffer))
	{
		error("const-not-set");
		return true;
	}
	advance();
	return true;
}

bool ScriptInterp::scrDup(void)
{
	char *id;
	Symbol *src, *dup;
	while(NULL != (src = getVariable(0)))
	{
		dup = getVariable(src->flags.size);
		if(!dup)
		{
			error("no-target");
			return true;

		}
		enterMutex();
		if(!dup->flags.initial)
		{
			leaveMutex();
			error("target-exists");
			return true;
		}
		id = dup->id;
		memcpy(dup, src, sizeof(Symbol) + src->flags.size);
		dup->id = id;
		leaveMutex();
	}
	advance();
	return true;
}

bool ScriptInterp::scrInit(void)
{
	Property *prop = NULL;
	Symbol *sym;
	int len;
	const char *value = getMember();
	int size = 0;
	bool number = false;
	int count = 0;
	int idx = 0;
	Line *line = getScript();

	enum
	{
		VAL_NONE,
		VAL_MIN,
		VAL_MAX
	}	minmax = VAL_NONE;

	if(value)
	{
		if(!stricmp(value, "size"))
			size = atoi(getValue("0"));
		else if(!stricmp(value, "max"))
			minmax = VAL_MAX;
		else if(!stricmp(value, "min"))
			minmax = VAL_MIN;
		else if(!stricmp(value, "val") || !stricmp(value, "num"))
		{
			size = 11;
			number = true;
		}
		else if(NULL == (prop = Property::find(value)))
			size = atoi(value);
	}

	if(!size)
		size = getSymbolSize();

	sym = getVariable(size);
	if(!sym)
	{
		while(idx < line->argc)
		{
			value = line->args[idx++];
			if(*value != '=')
				continue;
			if(*(++value) == '%')
				++value;
			++count;
			sym = getEntry(value, size);
			value = line->args[idx++];
			if(!sym)
				continue;
			if(!sym->flags.initial || sym->flags.readonly)
				continue;
			setSymbol(sym->id, value);
		}
		if(count)
			advance();
		else
			error("symbol-not-found");
		return true;
	}

	if(!sym->flags.initial)
	{
		advance();
		return true;
	}

	if(sym->flags.readonly)
	{
		error("symbol-readonly");
		return true;
	}

	sym->data[sym->flags.size] = 0;
	len = 0;

	while(len < sym->flags.size && NULL != (value = getValue(NULL)))
	{
		if(minmax == VAL_MIN && atoi(value) >= atoi(sym->data) && len)
			continue;
		if(minmax == VAL_MAX && atoi(value) <= atoi(sym->data) && len)
			continue;
		if(minmax == VAL_NONE)
			strncpy(sym->data + len, value, sym->flags.size - len);
		else
			strncpy(sym->data, value, sym->flags.size);
		sym->data[sym->flags.size] = 0;
		len = strlen(sym->data);
	}

	if(number)
		sprintf(sym->data, "%ld", atol(sym->data));
	else if(prop)
	{
		strcpy(temps[tempidx], sym->data);
		prop->setProperty(sym->data, temps[tempidx],  sym->flags.size);
	}
	sym->flags.initial = false;
	if(sym->flags.commit)
		commit(sym);

	advance();
	return true;
}

bool ScriptInterp::scrSet(void)
{
	Property *prop = NULL;
	Symbol *sym;
	int size = 0;
	int len;
	char *cp, *ep;
	const char *value;
	bool number = false;

	enum
	{
		VAL_NONE,
		VAL_MIN,
		VAL_MAX
	}	minmax = VAL_NONE;

	value = getMember();
	if(value)
	{
		if(!stricmp(value, "size"))
			size = atoi(getValue("0"));
		else if(!stricmp(value, "min"))
			minmax = VAL_MIN;
		else if(!stricmp(value, "max"))
			minmax = VAL_MAX;
		else if(!strnicmp(value, "val", 3) || !strnicmp(value, "num", 3))
		{
			number = true;
			size = 11;
		}
		else if(NULL ==(prop = Property::find(value)))
			size = atoi(value);
	}
	if(!size && prop)
		size = prop->getPropertySize();
	if(!size)
		size = getSymbolSize();
	sym = getVariable(size);
	if(!sym)
	{
		if(!initKeywords(size))
			error("symbol-not-found");
		else
			advance();
		return true;
	}

	if(sym->flags.readonly)
	{
		error("symbol-readonly");
		return true;
	}

	sym->data[sym->flags.size] = 0;
	len = 0;

	while(len < sym->flags.size && NULL != (value = getValue(NULL)))
	{
		if(minmax == VAL_MIN && atoi(value) >= atoi(sym->data) && len)
			continue;

		if(minmax == VAL_MAX && atoi(value) <= atoi(sym->data) && len)
			continue;
		if(minmax == VAL_NONE)
			strncpy(sym->data + len, value, sym->flags.size - len);
		else
			strncpy(sym->data, value, sym->flags.size);
		sym->data[sym->flags.size] = 0;
		len = strlen(sym->data);
	}

	if(number)
		sprintf(sym->data, "%ld", atol(sym->data));
	else if(prop)
	{
		strcpy(temps[tempidx], sym->data);
		prop->setProperty(sym->data, temps[tempidx], sym->flags.size);
	}

	sym->flags.initial = false;
	if(sym->flags.commit)
		commit(sym);

	advance();
	return true;
}

bool ScriptInterp::scrDump(void)
{
	Symbol *index[64];
	unsigned pos = 0;
	unsigned count = gather(index, 63, getValue(NULL), NULL);	

	while(pos < count)
	{
		slog(Slog::levelDebug) << index[pos]->id << " = " << index[pos]->data << endl;
		++pos;
	}
	advance();
	return true;
}

bool ScriptInterp::scrGather(void)
{
	Symbol *sym = getVariable(getSymbolSize());
	char *suffix = getValue(NULL);
	Name *list[33];
	int count = 0, idx;
	char *ext;
	Name *scr;

	if(!sym)
	{
		error("symbol-not-found");
		return true;
	}

	if(sym->flags.readonly)
	{
		error("symbol-readonly");
		return true;
	}

	count = image->gather(suffix, list, 32);
	if(!count)
	{
		error("no-scripts-found");
		return true;
	}
	sym->data[0] = 0;
	for(idx = 0; idx < count; ++idx)
	{
		if(idx)
			strcat(sym->data, ",");
		strcat(sym->data, list[idx]->name);
	}
	sym->flags.initial = false;
	if(sym->flags.commit)
		commit(sym);
	advance();
	return true;
}

bool ScriptInterp::scrFullpath(void)
{
	char *cp;
	char *pro;
	int len;
	Symbol *sym = getVariable(getSymbolSize());
	char *prefix = getValue(NULL);

	if(!sym)
	{
		error("symbol-not-found");
		return true;
	}

	if(sym->flags.readonly)
	{
		error("symbol-readonly");
		return true;
	}

	if(!prefix)
	{
		error("no-prefix");
		return true;
	}

	cp = strchr(sym->data, '/');
	pro = strchr(sym->data, ':');
	if(cp == sym->data || pro < cp)
	{
		advance();
		return true;
	}
	len = strlen(prefix);
	if(sym->flags.size < len + strlen(sym->data) + 3)
	{
		error("no-space");
		return true;
	}
	cp = sym->data;
	sym->data[len++] = '/';
	while(*cp)
		sym->data[len++] = *(cp++);
	*cp = 0;
	memcpy(sym->data, prefix, strlen(prefix));
	sym->flags.initial = false;
	if(sym->flags.commit)
		commit(sym);
	advance();
	return true;
}

bool ScriptInterp::scrDirname(void)
{
	char *cp;
	Symbol *sym = getVariable(1);

	if(!sym)
	{
		error("symbol-not-found");
		return true;
	}

	if(sym->flags.readonly)
	{
		error("symbol-readonly");
		return true;
	}

	cp = strrchr(sym->data, '/');
	if(!cp)
		strcpy(sym->data, ".");
	else
		*cp = 0;

	sym->flags.initial = false;
	if(sym->flags.commit)
		commit(sym);

	advance();
	return true;
}

bool ScriptInterp::scrBasename(void)
{
	const char *ext;
	Symbol *sym = getVariable(0);
	char *cp;

	if(!sym)
	{
		error("symbol-not-found");
		return true;
	}

	if(sym->flags.readonly)
	{
		error("symbol-readonly");
		return true;
	}

	cp = strrchr(sym->data, '/');
	if(cp)
		strcpy(sym->data, cp + 1);

	while(NULL != (ext = getValue(NULL)))
	{
		cp = sym->data + strlen(sym->data) - strlen(ext) - 1;
		if(cp > sym->data)
			if(*cp == '.')
				if(!stricmp(cp + 1, ext))
					*cp = 0;
	}
	sym->flags.initial = false;
	if(sym->flags.commit)
		commit(sym);
	advance();
	return true;
}

bool ScriptInterp::conditional(void)
{
	Line *line = script[stack].line;
	char *joiner;
	bool rtn;
	bool andfalse = false;
	bool ortrue = false;

	for(;;)
	{
		rtn = expConditional();
		if(script[stack].index < line->argc)
			joiner = line->args[script[stack].index];
		else
			joiner = "";

		if(!stricmp(joiner, "and"))
		{
			if(!rtn)
				andfalse = true;
		}
		else if(!stricmp(joiner, "or"))
		{
			if(rtn)
				ortrue = true;
		}
		else
			break;

		++script[stack].index;
	}
	if(andfalse)
		return false;

	if(ortrue)
		return true;

	return rtn;
}
		

bool ScriptInterp::expConditional(void)
{
	char *v1 = getOption(NULL);
	char *op = getValue(NULL);
	char *v2 = getOption(NULL);
	int l1, l2;

	// no first parm, invalid

	if(!v1)
		return false;

	v1 = getContent(v1);

	// sym/label by itself, invalid

	if(!op)
	{
		script[stack].index = 0;
		return false;
	}

	// ifdef sym ... format assumed

	if(!v2)
	{
		script[stack].index = 1;
		if(v1 && *v1)
			return true;
		return false;
	}

	v2 = getContent(v2);

	if(!v1)
		v1 = "";

	if(!v2)
		v2 = "";

	if(!stricmp(op, "=") || !stricmp(op, "-eq"))
	{
		if(atol(v1) == atol(v2))
			return true;
		return false;
	}

	if(!stricmp(op, "<>") || !stricmp(op, "-ne"))
	{
		if(atol(v1) != atol(v2))
			return true;
		return false;
	}

	if(!stricmp(op, "==") || !stricmp(op, ".eq."))
	{
		if(!stricmp(v1, v2))
			return true;

		return false;
	}

	if(!stricmp(op, "!=") || !stricmp(op, ".ne."))
	{
		if(stricmp(v1, v2))
			return true;

		return false;
	}

	if(!stricmp(op, "$") || !stricmp(op, ".in."))
	{
		if(strstr(v2, v1))
			return true;
		return false;
	}

	if(!stricmp(op, "$<") || !stricmp(op, "$+") || !stricmp(op, ".prefix."))
	{
		if(!strnicmp(v1, v2, strlen(v1)))
			return true;
		return false;
	}

	if(!stricmp(op, "$>") || !stricmp(op, "$-") || !stricmp(op, ".suffix."))
	{
		l1 = strlen(v1);
		l2 = strlen(v2);
		if(l1 <= l2)
			if(!strnicmp(v1, v2 + l2 - l1, l1))
				return true;

		return false;
	}

	if(!stricmp(op, "<") || !stricmp(op, "-lt"))
	{
		if(atol(v1) < atol(v2))
			return true;
		return false;
	}

	if(!stricmp(op, ".le."))
	{
		if(stricmp(v1, v2) <= 0)
			return true;
		return false;
	}

	if(!stricmp(op, ".ge."))
	{
		if(stricmp(v1, v2) >= 0)
			return true;
		return false;
	}

	if(!stricmp(op, "<=") || !stricmp(op, "=<") || !stricmp(op, "-le"))
	{
		if(atol(v1) <= atol(v2))
			return true;
		return false;
	}

	if(!stricmp(op, ">") || !stricmp(op, "-gt"))
	{
		if(atol(v1) > atol(v2))
			return true;
		return false;
	}

	if(!stricmp(op, ">=") || !stricmp(op, "=>") || !stricmp(op, "-ge"))
	{
		if(atol(v1) >= atol(v2))
			return true;
		return false;
	}

	// if no op, assume ifdef format

	script[stack].index = 1;
	if(*v1)
		return true;

	return false;
}

bool ScriptInterp::push(void)
{
	if(stack >= (SCRIPT_STACK_SIZE - 1))
	{
		error("stack-overflow");
		return false;
	}

	script[stack + 1] = script[stack];
	script[stack + 1].caseflag = false;
	++stack;
	return true;
}

bool ScriptInterp::pull(void)
{
	if(!stack)
	{
		error("stack-underflow");
		return false;
	}

	if(script[stack - 1].local != script[stack].local)
	{
		if(script[stack].local)
			delete script[stack].local;
	}

	--stack;
	return true;
}

bool ScriptInterp::getOnce(void)
{
	bool ret = once;
	once = false;
	return ret;
}

bool ScriptInterp::redirect(const char *scriptname)
{
	Name *scr;
	char namebuf[128];
	char *ext;

	if(!strncmp(scriptname, "::", 2))
	{
		strcpy(namebuf, script[stack].script->name);
		ext = strstr(namebuf, "::");
		if(ext)
			strcpy(ext, scriptname);
		else
			strcat(namebuf, scriptname);
	}
	else
		strcpy(namebuf, scriptname);

	scr = getScriptImage(namebuf);
	if(scr)
	{
		script[stack].script = scr;
		script[stack].line = scr->first;
		return true;
	}
	return false;
}

bool ScriptInterp::attach(const char *scriptname)
{
	ScriptModule *mod;
	session = NULL;
	stack = 0;
	cmd->enterMutex();
	image = cmd->active;
	if(!image)
	{
		cmd->leaveMutex();
		return false;
	}
	setSymbol("script.error", symsize);
	setSymbol("script.trigger", symsize);
	setSymbol("script.home", symsize);
	setSymbol("script.error", "none");
	setSymbol("script.trigger", "none");
	setSymbol("script.home", scriptname);
	script[stack].local = NULL;
	script[stack].script = getScriptImage(scriptname);
	if(script[stack].script)
	{
		mod = ScriptModule::first;
		while(mod)
		{
			mod->moduleAttach(this);
			mod = mod->next;
		}
		script[stack].line = script[stack].script->first;
		script[stack].index = 0;
		script[stack].read = NULL;
		script[stack].caseflag = false;
		++image->refcount;
		cmd->leaveMutex();
		return true;
	}
	cmd->leaveMutex();
	packtoken = ',';
	once = true;
	signalmask = 0;
	logerror(scriptname, getId(), "missing; attach failed");
	return false;
}

void ScriptInterp::detach(void)
{
	ScriptModule *mod;

	if(!image)
		return;

	if(session)
	{
		delete session;
		session = NULL;
	}

	cmd->enterMutex();
	--image->refcount;
	mod = ScriptModule::first;
	while(mod)
	{
		mod->moduleDetach(this);
		mod = mod->next;
	}
	if(image)
		if(!image->refcount && image != cmd->active)
			delete image;

	cmd->leaveMutex();
	image = NULL;

	while(stack)
		pull();
	locks.release(this);
}

int ScriptInterp::initKeywords(int size)
{
	int idx = 0;
	int count = 0;
	Line *line = script[stack].line;
	char *opt;

	if(!size)
		size = getSymbolSize();

	while(idx < line->argc)
	{
		opt = line->args[idx++];
		if(*opt != '=')
			continue;
		if(*(++opt) == '%')
			++opt;
		++count;
		setSymbol(opt, getSymbolSize());
		setSymbol(opt, getContent(line->args[idx++]));
	}
	return count;
}

char *ScriptInterp::getKeyword(const char *kw)
{
	int idx = 0;
	Line *line = script[stack].line;
	char *opt;
	while(idx < line->argc)
	{
		opt = line->args[idx++];
		if(*opt == '=')
		{
			if(!strnicmp(kw, opt + 1, strlen(kw)))
				return getContent(line->args[idx]);
			++idx;
		}
	}
	return NULL;
}

char *ScriptInterp::getOption(const char *def)
{
	for(;;)
	{
		if(script[stack].index >= script[stack].line->argc)
			return (char *)def;

		if(*script[stack].line->args[script[stack].index] != '=')
			break;

		script[stack].index += 2;
	}

	return script[stack].line->args[script[stack].index++];
}

Script::Symbol *ScriptInterp::getVariable(size_t size)
{
	char *opt;
	Symbol *sym;

	for(;;)
	{
		if(script[stack].index >= script[stack].line->argc)
			return NULL;

		opt = script[stack].line->args[script[stack].index++];

		if(*opt != '=')
			break;

		script[stack].index += 2;
	}

	if(*opt != '%' && *opt != '@')
		return NULL;

	if(*opt == '@')
	{
		sym = getIndirect(++opt);
		if(sym)
			return sym;

		if(strchr(opt, '.'))
			sym = getEntry(opt);
		else
			sym = getLocal(opt);

		if(sym)
			opt = sym->data;
		else
			return NULL;
	}
	else
		++opt;

	if(strchr(opt, '.'))
		return getEntry(opt, size);
	else
		return getLocal(opt, size);
}

char *ScriptInterp::getContent(char *opt)
{
	Property *prop;
	Symbol *sym;
	char *ext = strrchr(opt, '.');
	char *buf;

	if(!opt)
		return NULL;

	if(*opt != '%' && *opt != '@')
		return opt;

	if(*opt == '@')
	{
		++opt;
		sym = getIndirect(opt);
		if(sym)
			return readSymbol(sym);

		sym = getLocal(opt);
		if(!sym)
			return NULL;

		opt = sym->data;
	}
	else
		++opt;

	sym = getLocal(opt);
	if(sym)
		return readSymbol(sym);

	if(!ext)
		return NULL;

	buf = new char[strlen(opt) + 1];
	strcpy(buf, opt);
	*strrchr(buf, '.') = 0;

	sym = getLocal(buf);
	delete[] buf;
	if(!sym)
		return NULL;

	if(++tempidx >= SCRIPT_TEMP_SPACE)
		tempidx = 0;
	buf = temps[tempidx];

	if(!strnicmp(ext, ".len", 4))
		sprintf(buf, "%d", strlen(sym->data));
	else if(!stricmp(ext, ".size"))
		sprintf(buf, "%d", sym->flags.size);
	else if(!strnicmp(ext, ".val", 4) || !strnicmp(ext, ".int", 4))
		sprintf(buf, "%d", atoi(sym->data));
	else if(!stricmp(ext, ".bool"))
	{
		strcpy(buf, "false");
		switch(*sym->data)
		{
		case 'y':
		case 'Y':
		case 't':
		case 'T':
			strcpy(buf, "true");
			break;
		default:
			if(atoi(sym->data))
				strcpy(buf, "true");
		}
	}
	else if(!stricmp(ext, ".count"))
		switch(sym->flags.type)
		{
		case CACHE:
		case SEQUENCE:
		case FIFO:
		case STACK:
			sprintf(buf, "%d", sym->data[3]);
			break;
		default:
			return "1";
		}
	else if(NULL != (prop = Property::find(ext + 1)))
		prop->getProperty(sym->data, buf, getSymbolSize());
	else if(!stricmp(ext, ".max"))
		switch(sym->flags.type)
		{
		case CACHE:
		case SEQUENCE:
		case FIFO:
		case STACK:
			sprintf(buf, "%d", sym->data[4]);
			break;
		default:
			sprintf(buf, "%d", sym->flags.size);
		}
	else if(!stricmp(ext, ".type"))
		switch(sym->flags.type)
		{
		case CACHE:
			return "cache";
		case ALIAS:
			return "alias";
		case INDEX:
			return "index";
		case SEQUENCE:
			return "sequence";
		case STACK:
			return "stack";
		case COUNTER:
			return "counter"; 
		case FIFO:
			return "fifo";
		default:
			return "string";
		}
	else
		buf = NULL;

	return buf;
}

char *ScriptInterp::getValue(const char *def)
{
	char *opt = getOption(NULL);

	if(!opt)
		return (char *)def;

	opt = getContent(opt);
	if(!opt)
		return (char *)def;

	return opt;
}	

const char *ScriptInterp::getMember(void)
{
	char *cmd = script[stack].line->cmd;

	while(*cmd && *cmd != '.')
		++cmd;

	if(*cmd)
		return ++cmd;
	return NULL;
}

#ifdef	CCXX_NAMESPACES
};
#endif
