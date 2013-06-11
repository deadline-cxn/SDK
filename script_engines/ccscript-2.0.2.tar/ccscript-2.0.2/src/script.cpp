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
#include <cc++/file.h>
#include <cc++/strchar.h>
#include <cc++/export.h>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include "script.h"

#ifdef	CCXX_NAMESPACES
namespace ost {
using namespace std;
#endif

static int keyindex(const char *keyword)
{
	unsigned key = 0;

	while(*keyword)
		key ^= key << 1 ^ (*(keyword++) & 0x1f);

	return key % KEYWORD_INDEX_SIZE;
}

ScriptCommand::ScriptCommand(const char *cfgfile) :
Keydata(cfgfile), Mutex()
{
	char *scrname, *ext;
	char buffer[256];
	ifstream cf;

	static Script::Define interp[] = {
		{"slog", &ScriptInterp::scrSlog, &ScriptCommand::chkHasArgs},
		{"exit", &ScriptInterp::scrExit, &ScriptCommand::chkNoArgs},
		{"set", &ScriptInterp::scrSet, &ScriptCommand::chkIgnore},
		{"clear", &ScriptInterp::scrClear, &ScriptCommand::chkHasVars},
		{"try", &ScriptInterp::scrTry, &ScriptCommand::chkHasArgs},
		{"goto", &ScriptInterp::scrGoto, &ScriptCommand::chkHasArgs},
		{"call", &ScriptInterp::scrCall, &ScriptCommand::chkHasArgs},
		{"gosub", &ScriptInterp::scrCall, &ScriptCommand::chkHasArgs},
		{"pop", &ScriptInterp::scrPop, &ScriptCommand::chkIgnore},
		{"return", &ScriptInterp::scrReturn, &ScriptCommand::chkIgnore},
		{"select", &ScriptInterp::scrSelect, &ScriptCommand::chkHasArgs},
		{"size", &ScriptInterp::scrSize, &ScriptCommand::chkHasArgs},
		{"init", &ScriptInterp::scrInit, &ScriptCommand::chkIgnore},
		{"if", &ScriptInterp::scrIf, &ScriptCommand::chkHasArgs},
		{"do", &ScriptInterp::scrDo, &ScriptCommand::chkIgnore},
		{"for", &ScriptInterp::scrFor, &ScriptCommand::chkHasList},
		{"read", &ScriptInterp::scrRead, &ScriptCommand::chkHasArgs},
		{"tryeach", &ScriptInterp::scrTryeach, &ScriptCommand::chkHasArgs},
		{"foreach", &ScriptInterp::scrForeach, &ScriptCommand::chkHasArgs},
		{"repeat", &ScriptInterp::scrRepeat, &ScriptCommand::chkHasArgs},
		{"dump", &ScriptInterp::scrDump, &ScriptCommand::chkHasArgs},
		{"gather", &ScriptInterp::scrGather, &ScriptCommand::chkHasArgs},
		{"loop", &ScriptInterp::scrLoop, &ScriptCommand::chkIgnore},
		{"fifo", &ScriptInterp::scrFifo, &ScriptCommand::chkHasArgs},
		{"post", &ScriptInterp::scrPost, &ScriptCommand::chkHasArgs},
		{"break", &ScriptInterp::scrBreak, &ScriptCommand::chkIgnore},
		{"continue", &ScriptInterp::scrContinue, &ScriptCommand::chkIgnore},
		{"swap", &ScriptInterp::scrSwap, &ScriptCommand::chkHasArgs},
		{"inc", &ScriptInterp::scrInc, &ScriptCommand::chkHasArgs},
		{"dec", &ScriptInterp::scrDec, &ScriptCommand::chkHasArgs},
		{"once", &ScriptInterp::scrOnce, &ScriptCommand::chkHasArgs},
		{"on", &ScriptInterp::scrOn, &ScriptCommand::chkHasArgs},
		{"pack", &ScriptInterp::scrPack, &ScriptCommand::chkHasList},
		{"unpack", &ScriptInterp::scrUnpack, &ScriptCommand::chkHasList},
		{"packed", &ScriptInterp::scrPacked, &ScriptCommand::chkHasArgs},
		{"lock", &ScriptInterp::scrLock, &ScriptCommand::chkIgnore},
		{"has", &ScriptInterp::scrHas, &ScriptCommand::chkHasArgs},
		{"missing", &ScriptInterp::scrMissing, &ScriptCommand::chkHasArgs},
		{"const", &ScriptInterp::scrConst, &ScriptCommand::chkIgnore},
		{"basename", &ScriptInterp::scrBasename, &ScriptCommand::chkHasArgs},
		{"dirname", &ScriptInterp::scrDirname, &ScriptCommand::chkHasArgs},
		{"fullpath", &ScriptInterp::scrFullpath, &ScriptCommand::chkHasArgs},
		{"alias", &ScriptInterp::scrAlias, &ScriptCommand::chkHasArgs},
		{"data", &ScriptInterp::scrData, &ScriptCommand::chkHasArgs},
		{"dup", &ScriptInterp::scrDup, &ScriptCommand::chkHasArgs},
		{"array", &ScriptInterp::scrArray, &ScriptCommand::chkHasArgs},
		{"list", &ScriptInterp::scrList, &ScriptCommand::chkHasArgs},
		{"remove", &ScriptInterp::scrRemove, &ScriptCommand::chkHasArgs},
		{"sequence", &ScriptInterp::scrSequence, &ScriptCommand::chkHasArgs},
		{"cache", &ScriptInterp::scrCache, &ScriptCommand::chkHasArgs},
		{"stack", &ScriptInterp::scrStack, &ScriptCommand::chkHasArgs},
		{"lifo", &ScriptInterp::scrStack, &ScriptCommand::chkHasArgs},
		{"counter", &ScriptInterp::scrCounter, &ScriptCommand::chkHasArgs},
		{"use", &ScriptInterp::scrUse, &ScriptCommand::chkUse},
		{"skip", &ScriptInterp::scrSkip,  &ScriptCommand::chkHasArgs},
		{"arm", &ScriptInterp::scrArm,  &ScriptCommand::chkHasArgs},
		{"disarm", &ScriptInterp::scrDisarm, &ScriptCommand::chkHasArgs},
		{"enable", &ScriptInterp::scrEnable, &ScriptCommand::chkHasArgs},
		{"disable", &ScriptInterp::scrDisable, &ScriptCommand::chkHasArgs},
		{"then", &ScriptInterp::scrThen, &ScriptCommand::chkNoArgs},
		{"else", &ScriptInterp::scrElse, &ScriptCommand::chkNoArgs},
		{"endif", &ScriptInterp::scrEndif, &ScriptCommand::chkNoArgs},
		{"case", &ScriptInterp::scrCase, &ScriptCommand::chkHasArgs},
		{"endcase", &ScriptInterp::scrEndcase, &ScriptCommand::chkNoArgs},
		{"otherwise", &ScriptInterp::scrCase, &ScriptCommand::chkNoArgs},
		{"label", &ScriptInterp::scrLabel, &ScriptCommand::chkHasArgs},
		{"ref", &ScriptInterp::scrRef, &ScriptCommand::chkHasArgs},
		{NULL, NULL, NULL}};

	int i;
	memset(&keywords, 0, sizeof(keywords));
	for(i = 0; i < TRAP_BITS; ++i)
		traps[i] = "<undefined>";

	active = NULL;
	keyword_count = 0;
	trap_count = 0;

	// add special default exit and error traps

	trap("exit");
	trap("error");
	load(interp);

	// parse global definition entries

	if(!isDir(SCRIPT_MODULE_PATH))
		return;

	if(!canAccess(SCRIPT_MODULE_PATH))
		return;

	Dir dir(SCRIPT_MODULE_PATH);
	for(;;)
	{
		scrname = dir.getName();
		if(!scrname)
			return;

		ext = strchr(scrname, '.');
		if(!ext)
			continue;
		if(stricmp(ext, "cfg"))
			continue;

		snprintf(buffer, sizeof(buffer) - 1, "%s/%s",
			SCRIPT_MODULE_PATH, scrname);

		cf.open(buffer);
		if(!cf.is_open())
			continue;

		for(;;)
		{
			cf.getline(buffer, sizeof(buffer) - 1);
			if(cf.eof())
				break;
			scrname = buffer;
			while(isspace(*scrname))
				++scrname;

			if(*scrname == '#' || !*scrname)
				continue;

			ext = scrname;

			while(*ext && !isspace(*ext))
				++ext;

			*ext = 0;
			while(isspace(*ext) || *ext == '=')
				++ext;

			setValue(scrname, ext);
		}
		cf.close();
		cf.clear();
	}
}

Script::Method ScriptCommand::getHandler(const char *keyword)
{
	Keyword *key;
	char keybuf[33];
	int len = 0;
	char *kw = keybuf;

	while(len++ < 32 && *keyword && *keyword != '.')
		*(kw++) = (*keyword++);
	*kw = 0;
	keyword = keybuf;

	key = keywords[keyindex(keyword)];

	while(key)
	{
		if(!stricmp(key->keyword, keyword))
			return key->method;

		key = key->next;
	}
	if(ScriptModule::find(keyword))
		return &ScriptInterp::scrLoadable;
	return (Method)NULL;
}

char *ScriptCommand::check(char *keyword, Line *line, ScriptImage *img)
{
	ScriptModule *mod;
	Keyword *key;
	char keybuf[33];
	int len = 0;
	char *kw = keybuf;

	while(len++ < 32 && *keyword && *keyword != '.')
		*(kw++) = *(keyword++);

	*kw = 0;
	keyword = keybuf;
	key = keywords[keyindex(keyword)];

	while(key)
	{
		if(!stricmp(key->keyword, keyword))
			return check(key->check, line, img);

		key = key->next;
	}
	mod = ScriptModule::find(keyword);
	if(mod)
		return mod->checkScript(line, img);
	return "unknown command";
}

unsigned ScriptCommand::getTrapId(const char *trapname)
{
	unsigned i;

	for(i = 0; i < TRAP_BITS; ++i)
	{
		if(!stricmp(traps[i], trapname))
			return i;
	}
	return 0;
}

unsigned long ScriptCommand::getTrapMask(unsigned id)
{
	return 1 << id;
}

unsigned long ScriptCommand::getTrapMask(const char *trapname)
{
	unsigned long mask = 1;
	int i;

	for(i = 0; i < TRAP_BITS; ++i)
	{
		if(!stricmp(traps[i], trapname))
			return mask;

		mask = mask << 1;
	}
	return 0;
}

void ScriptCommand::load(Script::Define *keydefs)
{
	int key;
	Keyword *script;

	for(;;)
	{
		if(!keydefs->keyword)
			break;

		key = keyindex(keydefs->keyword);
		script = (Keyword *)alloc(sizeof(Keyword) + strlen(keydefs->keyword));
		strcpy(script->keyword, keydefs->keyword);
		script->method = keydefs->method;
		script->check = keydefs->check;
		script->next = keywords[key];
		keywords[key] = script;
		++keydefs;
	}
}

int ScriptCommand::trap(const char *trapname)
{
	traps[trap_count++] = alloc((char *)trapname);
	return trap_count;
}

char *ScriptCommand::chkIgnore(Line *line, ScriptImage *img)
{
	return NULL;
}

char *ScriptCommand::chkUse(Line *line, ScriptImage *img)
{
	int argc = line->argc;
	char **argv = line->args;

	if(!argc)
		return "missing args";

#ifdef	HAVE_MODULES

	while(argc--)
	{
		if(!Script::use(*argv))
			return "package missing";
		++argv;
	}
	return NULL;
#else
	return "dynamic loader unsupported"
#endif
}

char *ScriptCommand::chkModule(Line *line, ScriptImage *img)
{
	ScriptModule *mod = ScriptModule::first;
	char keybuf[32];

	char *kw = line->cmd;
	int len = 0;
	while(*kw && len < sizeof(keybuf) - 1 && *kw != '.')
		keybuf[len++] = *kw;
	keybuf[len] = 0;

	while(mod)
	{
		if(!stricmp(kw, mod->cmd))
			break;
		return NULL;
	}
	return mod->checkScript(line, img);
}

char *ScriptCommand::chkHasModify(Line *line, ScriptImage *img)
{
	if(!line->argc)
		return "arguments missing";

	if(line->argc < 2)
		return "no values to modify";

	if(*(line->args[0]) != '%' && *(line->args[0]) != '@')
		return "invalid variable assignment";

	return NULL;
}

char *ScriptCommand::chkHasList(Line *line, ScriptImage *img)
{
	if(!line->argc)
		return "arguments missing";

	if(line->argc < 2)
		return "no values to assign";

	if(*(line->args[0]) != '%' && *(line->args[0]) !='@')
		return "invalid variable assignment";

	return NULL;
}

char *ScriptCommand::chkHasArgs(Line *line, ScriptImage *img)
{
	if(!line->argc)
		return "arguments missing";

	return NULL;
}

char *ScriptCommand::chkNoArgs(Line *line, ScriptImage *img)
{
	if(line->argc)
		return "invalid arguments";

	return NULL;
} 

char *ScriptCommand::chkHasVars(Line *line, ScriptImage *img)
{
	int index = 0;
	char ch;

	if(line->argc < 1)
		return "no arguments";

	while(index < line->argc)
	{
		ch = *(line->args[index++]);
		if(ch != '%' && ch != '@')
			return "variable argument required";
	}

	return NULL;
}

ScriptSymbol::ScriptSymbol(int size, int paging) :
SharedMemPager(paging)
{
	symsize = size;
	memset(index, 0, sizeof(index));
	trigger = NULL;
}

Script::Symbol *ScriptSymbol::getTrigger(void)
{
	Symbol *sym;

	if(!trigger)
		return NULL;

	sym = trigger;
	trigger = NULL;
	return sym;
}

void ScriptSymbol::purge(void)
{
	trigger = NULL;
	MemPager::purge();
	memset(index, 0, sizeof(index));
}

unsigned ScriptSymbol::gather(Symbol **idx, unsigned max, const char *prefix, const char *suffix)
{
	char *ext;
	unsigned key = 0;
	unsigned count = 0;
	unsigned pointer, marker;
	Symbol *node;

	enterMutex();

	while(max && key < SYMBOL_INDEX_SIZE)
	{
		node = index[key++];
		while(node && max)
		{
			if(strnicmp(node->id, prefix, strlen(prefix)))
			{
				node = node->next;
				continue;
			}

			if(suffix)
			{
				ext = strrchr(node->id, '.');
				if(!ext)
				{
					node = node->next;
					continue;
				}
				if(stricmp(++ext, suffix))
				{
					node = node->next;
					continue;
				}
			}

			pointer = 0;
			while(pointer < count)
			{
				if(stricmp(node->id, idx[pointer]->id) < 0)
					break;
				++pointer;
			}
			marker = pointer;
			pointer = count;
			while(pointer > marker)
			{
				idx[pointer] = idx[pointer - 1];
				--pointer;
			}
			idx[marker] = node;
			--max;
			++count;
			node = node->next;
		}
	}
	leaveMutex();
	return count;	
}


unsigned ScriptSymbol::getIndex(const char *symname)
{
	unsigned int key = 0;

	while(*symname)
		key ^= (key << 1) ^ (*(symname++) & 0x1f);		
	
	return key % SYMBOL_INDEX_SIZE;
}

void ScriptSymbol::commit(Symbol *sym)
{
	char array[96];
	char *id, *cp;
	int count;
	int index;

	if(sym->flags.type != INDEX)
		return;

	strcpy(array, sym->id);
	id = strrchr(array, '.');
	if(!id)
		return;

	strcpy(id, ".limit");
	cp = getSymbol(array);
	if(!cp)
		return;

	count = atoi(cp);
	index = atoi(sym->data);
	if(index < 1 || index > count)
		index = 0;
	sprintf(sym->data, "%d", index);
	*id = 0;
	sym = getAlias(array);
	if(!sym)
		return;
	snprintf(sym->data, sym->flags.size, "%s.%d", array, index);
}

Script::Symbol *ScriptSymbol::getEntry(const char *symname, int size)
{
	int key;
	Symbol *sym;
	ScriptSymbol *ref;

	enterMutex();

retry:
	if(*symname == '%')
		++symname;
	key = getIndex(symname);
	sym = index[key];
	while(sym)
	{
		if(!stricmp(sym->id, symname))
			break;

		sym = sym->next;
	}

	if(sym)
	{
		if(sym->flags.type == ALIAS)
		{
			symname = sym->data;
			goto retry;
		}
		else if(sym->flags.type == REF)
		{
			symname = sym->data + sizeof(ScriptSymbol *);
			ref = *((ScriptSymbol **)sym->data);
			return ref->getEntry(symname, size);
		}
	}

	if(sym || !size)
	{
		leaveMutex();
		return sym;
	}

	sym = (Symbol *)alloc(sizeof(Symbol) + size);
	sym->id = MemPager::alloc((char *)symname);
	sym->next = index[key];
	sym->flags.size = size;
	sym->flags.commit = false;
	sym->flags.initial = true;
	sym->flags.system = false;
	sym->flags.readonly = false;
	sym->flags.type = NORMAL;
	sym->data[0] = 0;
	index[key] = sym;
	leaveMutex();
	return sym;
}

Script::Symbol *ScriptSymbol::getAlias(const char *symname)
{
	int key;
	Symbol *sym;

	enterMutex();
	if(*symname == '%')
		++symname;
	key = getIndex(symname);
	sym = index[key];
	while(sym)
	{
		if(!stricmp(sym->id, symname))
			break;

		sym = sym->next;
	}

	if(sym)
	{
		if(sym->flags.type != ALIAS)
			sym = NULL;
	}
	leaveMutex();
	return sym;
}

char *ScriptSymbol::getSymbol(const char *id)
{
	Symbol *sym = getEntry(id);
	if(!sym)
		return NULL;

	if(sym->flags.initial)
		return NULL;

	return readSymbol(sym);
}

void *ScriptSymbol::getPointer(const char *id)
{
	Symbol *sym = getEntry(id);
	void *dp;

	if(!sym)
		return NULL;

	if(sym->flags.initial)
		return NULL;

	if(sym->flags.type != POINTER)
		return NULL;

	memcpy(&dp, sym->data + 1, sizeof(void *));
	return dp;
}

bool ScriptSymbol::setPointer(const char *id, void *data)
{
	Symbol *sym;

	if(*id == '%')
		++id;

	sym = getEntry(id, sizeof(void *) + 1);
	if(!sym->flags.initial && sym->flags.type != POINTER)
		return false;

	enterMutex();
	sym->data[0] = 0;
	sym->flags.initial = false;
	sym->flags.readonly = true;
	sym->flags.type = POINTER;
	memcpy(sym->data + 1, &data, sizeof(void *));
	leaveMutex();
	return true;
}

bool ScriptSymbol::setAlias(const char *id, const char *source)
{
	Symbol *sym;

	if(*id == '%')
		++id;

	if(*source == '%')
		++source;

	sym = getEntry(id, strlen(source));
	if(!sym->flags.initial)
		return false;

	enterMutex();
	strcpy(sym->data, source);
	sym->flags.initial = false;
	sym->flags.readonly = true;
	sym->flags.type = ALIAS;
	leaveMutex();
	return true;
}

bool ScriptSymbol::makeCounter(const char *id)
{
	Symbol *sym;

	if(*id == '%')
		++id;

	sym = getEntry(id, 11);
	if(!sym->flags.initial)
		return false;
	enterMutex();
	sym->flags.initial = false;
	sym->flags.type = COUNTER;
	strcpy(sym->data, "0");
	leaveMutex();
	return true;
}

bool ScriptSymbol::makeFifo(const char *id, unsigned char size, unsigned char rec)
{
	Symbol *sym;

	if(*id == '%')
		++id;

	sym = getEntry(id, size * (rec + 1) + 5);
	if(!sym->flags.initial)
		return false;

	enterMutex();
	sym->flags.initial = false;
	sym->flags.readonly = true;
	sym->flags.type = FIFO;
	sym->data[0] = 0;
	sym->data[1] = 0;	// head
	sym->data[2] = 0;	// tail
	sym->data[3] = rec;	// rec
	sym->data[4] = size;
	leaveMutex();
	return true;
}

bool ScriptSymbol::makeStack(const char *id, unsigned char size, unsigned char rec)
{
	Symbol *sym;

	if(*id == '%')
		++id;

	sym = getEntry(id, size * (rec + 1) + 5);
	if(!sym->flags.initial)
		return false;

	enterMutex();
	sym->flags.initial = false;
	sym->flags.readonly = true;
	sym->flags.type = STACK;
	sym->data[0] = 0;
	sym->data[1] = 0;	// head
	sym->data[2] = 0;	// tail
	sym->data[3] = rec;	// rec
	sym->data[4] = size;
	leaveMutex();
	return true;
}

bool ScriptSymbol::makeSequence(const char *id, unsigned char size, unsigned char rec)
{
	Symbol *sym;

	if(*id == '%')
		++id;

	sym = getEntry(id, size * (rec + 1) + 5);
	if(!sym->flags.initial)
		return false;

	enterMutex();
	sym->flags.initial = false;
	sym->flags.readonly = true;
	sym->flags.type = SEQUENCE;
	sym->data[0] = 0;
	sym->data[1] = 0;	// head
	sym->data[2] = 0;	// tail
	sym->data[3] = rec;	// rec
	sym->data[4] = size;
	leaveMutex();
	return true;
}

bool ScriptSymbol::makeCache(const char *id, unsigned char size, unsigned char rec)
{
	Symbol *sym;

	if(*id == '%')
		++id;

	sym = getEntry(id, size * (rec + 1) + 5);
	if(!sym->flags.initial)
		return false;

	enterMutex();
	sym->flags.initial = false;
	sym->flags.readonly = true;
	sym->flags.type = CACHE;
	sym->data[0] = 0;
	sym->data[1] = 0;	// head
	sym->data[2] = 0;	// tail
	sym->data[3] = rec;	// rec
	sym->data[4] = size;
	leaveMutex();
	return true;
}

bool ScriptSymbol::removeSymbol(Symbol *sym, const char *value)
{
	unsigned char size, rec, head, tail;
	unsigned i, j;

	if(sym->flags.type != FIFO &&
	   sym->flags.type != SEQUENCE &&
	   sym->flags.type != CACHE &&
	   sym->flags.type != STACK)
		return false;

	enterMutex();
	size = sym->data[4];
	rec = sym->data[3];
	head = sym->data[1];
	tail = sym->data[2];

	i = head;
	while(i != tail)
	{
		if(!strcmp(sym->data + 5 + i * (rec + 1), value))
			break;
		if(++i >= size)
			i = 0;
	}

	if(i == tail)
	{
		leaveMutex();
		return true;
	}

	if(i == head && sym->flags.type == FIFO)
	{
		if(++head >= size)
			head = 0;
		sym->data[0] = head;
		leaveMutex();
		return true;
	}

	j = i + 1;
	while(j < size)
	{
		strcpy(sym->data + 5 + (j - 1) * (rec + 1),
		       sym->data + 5 + j * (rec + 1));
		++j;
	}

	if(sym->data[1] > i)
		--sym->data[1];
	if(sym->data[2] > i)
		--sym->data[2];
	leaveMutex();
	return true;
}

bool ScriptSymbol::postSymbol(Symbol *sym, const char *value)
{
	unsigned char head, tail, pos, size, rec;
	unsigned offset = 0;

	if(sym->flags.type != FIFO && sym->flags.type != SEQUENCE && sym->flags.type != STACK && sym->flags.type != CACHE)
		return false;

	enterMutex();
	head = sym->data[1];
	pos = tail = sym->data[2];
	size = sym->data[4];
	rec = sym->data[3];

	if(tail >= size && sym->flags.type == CACHE)
	{
		while(offset < size - 1)
		{
			strcpy(sym->data + 5 + offset * (rec + 1),
				sym->data + 5 + (offset + 1) * (rec + 1));
			++offset;
		}
		tail = size;
		pos = size - 1;
	}
	else if(++tail >= size && sym->flags.type == FIFO)
		tail = 0;

	if(pos >= size)
	{
		leaveMutex();
		return false;
	}
	if(tail == head && (sym->flags.type != CACHE && sym->flags.type  != SEQUENCE))
	{
		leaveMutex();
		return false;
	}

	strncpy(sym->data + (pos * (rec + 1)) + 5, value, rec);
	sym->data[pos * (rec + 1) + rec + 5] = 0;
	sym->data[2] = tail;
	leaveMutex();
	return true;
}

char *ScriptSymbol::readSymbol(Symbol *sym)
{
	long value;
	char *data;
	unsigned char head, rec, size;
	switch(sym->flags.type)
	{
	case COUNTER:
		value = atoi(sym->data);
		sprintf(sym->data, "%ld", ++value);
		return sym->data;
	case CACHE:
		enterMutex();
		if(!sym->data[1])
			head = sym->data[1] = sym->data[2];
		if(head)
			head = --sym->data[1];
		rec = sym->data[3];
		leaveMutex();
		return sym->data + 5 + (head * (rec + 1));
	case STACK:
		enterMutex();
		if(!sym->data[2])
		{
			leaveMutex();
			return sym->data;
		}
		head = --sym->data[2];
		rec = sym->data[3];
		leaveMutex();
		return sym->data + 5 + (head * (rec + 1));
	case SEQUENCE:
		enterMutex();
		if(sym->data[1] == sym->data[2])
			sym->data[1] = 0;
		leaveMutex();
	case FIFO:
		enterMutex();
		if(sym->data[1] == sym->data[2])
		{
			leaveMutex();
			return sym->data;
		}
		size = sym->data[4];
		rec = sym->data[3];
		head = sym->data[1];
		data =  sym->data + head * (rec + 1) + 5;
		if(++head >= size)
			head = 0;
		sym->data[1] = head;
		leaveMutex();
		return data;
	default:
		return sym->data;
	}
}

char *ScriptSymbol::setConst(const char *id, const char *value)
{
	Symbol *sym;

	if(*id == '%')
		++id;

	sym = getEntry(id, strlen(value));
	if(!sym)
		return NULL;

	if(!sym->flags.initial)
		return sym->data;

	enterMutex();
	strcpy(sym->data, value);
	sym->flags.initial = false;
	sym->flags.readonly = true;
	sym->flags.type = NORMAL;
	leaveMutex();
	return sym->data;
}

char *ScriptSymbol::setSymbol(const char *id, const char *value)
{
	char array[96];
	int count;
	int index;
	char *cp;
	long val;

	Symbol *alias;
	Symbol *sym = getEntry(id);
	if(!sym)
		return NULL;

	if(sym->flags.readonly)
		return NULL;

	if(!value)
		value = "";

	enterMutex();
	sym->flags.initial = false;
	if(sym->flags.type == COUNTER)
	{
		val = atoi(value);
		sprintf(sym->data, "%ld", --val);
	}
	else
	{
		strncpy(sym->data, value, sym->flags.size);
		sym->data[sym->flags.size] = 0;
	}
	if(sym->flags.type == TRIGGER)
	{
		sym->flags.type = NORMAL;
		trigger = sym;
	}

	if(sym->flags.commit)
		commit(sym);

	leaveMutex();
	return sym->data;
}

void ScriptSymbol::clrSymbol(const char *id)
{
	Symbol *sym = getEntry(id);

	if(!sym)
		return;

	if(sym->flags.readonly || sym->flags.system)
		return;

	enterMutex();
	sym->flags.initial = true;
	sym->data[0] = 0;
	if(sym->flags.commit)
		commit(sym);
	leaveMutex();
	return;
}

char *ScriptSymbol::setSymbol(const char *id, int size)
{
	Symbol *sym;

	if(!size)
		size = symsize;

	sym = getEntry(id, size);
	if(!sym)
		return NULL;

	return sym->data;
}

ScriptImage::ScriptImage(ScriptCommand *cmd)
{
	char buffer[256];
	char *scrname, *ext;
	cmds = cmd;
	memset(index, 0, sizeof(index));
	refcount = 0;
	scrStream = (istream *)&scrSource;

	if(!isDir(SCRIPT_MODULE_PATH))
		return;

	if(!canAccess(SCRIPT_MODULE_PATH))
		return;

	Dir dir(SCRIPT_MODULE_PATH);
	for(;;)
	{
		scrname = dir.getName();
		if(!scrname)
			break;
		ext = strchr(scrname, '.');
		if(!ext)
			continue;
		if(stricmp(ext, ".src"))
			continue;
		snprintf(buffer, sizeof(buffer) - 1, "%s/%s",
			SCRIPT_MODULE_PATH, scrname);
		ScriptImage::compile(buffer);
	}
}

void ScriptImage::purge(void)
{
	MemPager::purge();
	memset(index, 0, sizeof(index));
	refcount = 0;
}

void ScriptImage::commit(void)
{
	cmds->enterMutex();
	cmds->active = this;
	cmds->leaveMutex();
}

char *ScriptImage::getToken(void)
{
	static char temp[513];
	char *cp = temp + 1;
	char *base = temp + 1;
	char q;

	if(*bp == '=')
	{
		++bp;
		if(*bp == ' ' || *bp == '\t' || !*bp)
			return "";
		if(*bp == '\"' || *bp == '\'')
		{
			q = *(bp++);
			while(q)
			{
				switch(*bp)
				{
				case '\\':
					++bp;
					if(!*bp)
					{
						q = 0;
						break;
					}
					switch(*bp)
					{
					case 't':
						*(cp++) = '\t';
						++bp;
						break;
					case 'b':
						*(cp++) = '\b';
						++bp;
						break;
					case 'n':
						*(cp++) = '\n';
						++bp;
						break;
					default:
						*(cp++) = *(bp++);
					}
					break;
				case 0:
					q = 0;
					break;
				default:
					if(*bp == q)
					{
						++bp;
						q = 0;
					}
					else
						*(cp++) = *(bp++);
				}
			}
			*cp = 0;
			return base;
		}
		while(*bp != ' ' && *bp != '\t' && *bp)
			*(cp++) = *(bp++);
		*cp = 0;
		return base;
	}

	if(!quote)
		while(*bp == ' ' || *bp == '\t')
			++bp;

	if(!quote && *bp == '#')
		return NULL;

	if(!*bp)
	{
		quote = false;
		return NULL;
	}

	if(*bp == '\"' && !quote)
	{
		++bp;
		quote = true;
	}

	if(!quote)
	{
retry:
		while(*bp && !isspace(*bp) && *bp != ',' && *bp != '=')
			*(cp++) = *(bp++);

		if(*bp == '=' && cp == base)
		{
			*(cp++) = *(bp++);
			goto retry;
		}

		if(*bp == '=' && cp == base + 1 && ispunct(*base))
		{
			*(cp++) = *(bp++);
			goto retry;
		}

		*cp = 0;
		if(*bp == ',')
			++bp;
		else if(*bp == '=')
			*(--base) = *(bp);
		if(!strcmp(base, "=="))
			return ".eq.";
		if(!strcmp(base, "="))
			return "-eq";
		if(!strncmp(base, "!=", 2))
			return ".ne.";
		if(!strncmp(base, "<>", 2))
			return "-ne";
		if(!strcmp(base, "<"))
			return "-lt";
		if(!strcmp(base, "<="))
			return "-le";
		if(!strcmp(base, ">"))
			return "-gt";
		if(!strcmp(base, ">="))
			return "-ge";
		return base;
	}

	if(isalnum(*bp) || strchr("~/:,. \t\'", *bp))
	{
		while(isalnum(*bp) || strchr("~=/:,. \t\'", *bp))
			*(cp++) = *(bp++);
	}
	else while(!isspace(*bp) && *bp && *bp != '\"')
		*(cp++) = *(bp++);

	if(*bp == '\n' || !*bp || *bp == '\"')
		quote = false;

	if(*bp == '\n' || *bp == '\"')
		++bp;

	*cp = 0;
	return base;
}

unsigned ScriptImage::gather(const char *suffix, Name **array, unsigned max)
{
	unsigned count = 0;
	unsigned sort = 0;
	unsigned key = 0;
	Name *scr;
	char *ext;
	int ins;

	while(count < max && key < KEYWORD_INDEX_SIZE)
	{
		scr = index[key];
		while(scr && count < max)
		{
			ext = strstr(scr->name, "::");
			if(!ext)
			{
				scr = scr->next;
				continue;
			}
			ext += 2;
			if(stricmp(ext, suffix))
			{
				scr = scr->next;
				continue;
			}
			sort = 0;
			while(sort < count)
			{
				if(stricmp(scr->name, array[sort]->name) < 0)
					break;
				++sort;
			}
			ins = count;
			while(ins > sort)
			{
				array[ins] = array[ins - 1];
				--ins;
			}
			array[sort] = scr;
			++count;
			scr = scr->next;
		}
		++key;
	}
	return count;
}

Script::Name *ScriptImage::dupScript(const char *src, const char *dest)
{
	Name *scr = getScript(dest);
	Name *nscr;
	unsigned key;
	if(scr)
		return scr;

	scr = getScript(src);
	if(!scr)
		return NULL;

	if(scr->mode == Name::COPY)
		return NULL;

	key = keyindex(dest);
	duplock.enterMutex();
	nscr = (Name *)alloc(sizeof(Name));
	memcpy(nscr, scr, sizeof(Name));
	nscr->name = alloc((char *)dest);
	nscr->mode = Name::COPY;
	scr->mode = Name::COPIED;
	nscr->next = index[key];
	index[key] = nscr;
	duplock.leaveMutex();
	return nscr;
}

Script::Name *ScriptImage::getScript(const char *name)
{
	int key = keyindex(name);
	Name *scr = index[key];

	while(scr)
	{
		if(!stricmp(scr->name, name))
			break;

		scr = scr->next;
	}
	return scr;
}

int ScriptImage::compile(const char *scrname)
{
	char buffer[strlen(scrname) + 1];
	char *token;
	char *ext;

	strcpy(buffer, scrname);
	token = strrchr(buffer, '/');
	if(!token)
		token = buffer;
	else
		++token;

	ext = strrchr(token, '.');
	if(ext)
		*ext = 0;

	return compile(scrname, token);
}

Script::Name *ScriptImage::include(const char *scrname)
{
	char buffer[strlen(scrname) + 1];
	char *token;
	char *ext;
	Name *inc;

	strcpy(buffer, scrname);
	token = strrchr(buffer, '/');
	if(!token)
		token = buffer;
	else
		++token;

	ext = strrchr(token, '.');
	if(ext)
		*ext = 0;

	inc = getScript(token);
	if(!inc)
	{
		compile(scrname, token);
		inc = getScript(token);
	}
	return inc;
}

int ScriptImage::compile(const char *scrname, char *name)
{
	scrSource.open(scrname);
	if(!scrSource.is_open())
		return 0;

	compile((istream *)&scrSource, name, scrname);
	scrSource.close();
	scrSource.clear();
}

int ScriptImage::compile(istream *str, char *name, const char *scrname)
{
	const char *basename = name;
	char namebuf[256];
	char *command, *token;
	char *args[33];
	char *err, *cp;
	bool trapflag, errflag;
	int argc, i, key, tlen;
	unsigned short count, number, total = 0;
	Name *script;
	Line *line, *last;
	unsigned long addmask, submask, trapmask, mask, cmask;
	unsigned branch;
	Method handler;
	unsigned char loopid, looplevel;
	Name *tmpl = NULL, *base = NULL;
	bool sub = false;
	streampos pos;

	scrStream = str;

compile:
	trapflag = false;
	count = number = 0;
	last = NULL;
	bool req = false;
	addmask = submask = trapmask = 0;
	branch = 0;
	handler = NULL;
	loopid = looplevel = 0;
	bool then = false;

	key = keyindex(name);
	script = (Name *)alloc(sizeof(Name));
	memset(script, 0, sizeof(Name));
	script->name = alloc(name);
	script->mask = 0;
	script->next = index[key];
	script->mode = Name::ORIGINAL;
	index[key] = script;

	if(!base)
		base = script;

	if(sub)
	{
		sub = false;
		memcpy(script->trap, base->trap, sizeof(base->trap));
	}

	for(;;)
	{
		quote = false;
		if(!then)
		{
			cmask = 0;
			errflag = false;
			scrStream->getline(buffer, sizeof(buffer) - 1);
			if(scrStream->eof())
				break;

			bp = buffer;
		}
		else
			then = false;

		++number;
		while(NULL != (token = getToken()))
		{
			if(!strncmp(token, "::", 2))
			{
				name = token + 2;
				break;
			}
			tlen = strlen(token);
			if(token[tlen - 1] == ':')
			{
				token[tlen - 1] = 0;
				name = token;
				token = "::";
				break;
			}
			
			if(*token == '^')
			{
				if(!trapflag)
				{
					trapmask = 0;
					trapflag = true;
				}
			}
			if(*token != '^' && *token != '+' && *token != '-' && *token != '?')
			{
				if(*token >= '0' && *token <= '9')
				{
					branch |= 1 << (*token - '0');
				}
				else
					break;
			}

			if(*token == '^')
				mask = cmds->getTrapMask(token + 1);
			else
				mask = cmds->getTrapModifier(token + 1);

			if(!mask)
			{
				slog(Slog::levelError) << token + 1 << ": unknown trap id; " << script->name << "(" << number << ")" << endl;
				continue;
			}

			switch(*token)
			{
			case '^':
				last = NULL;
				script->mask |= mask | cmds->getTrapDefault();
				trapmask |= mask;
				break;
			case '+':
				addmask |= mask;
				break;
			case '-':
				submask |= mask;
				break;
			case '?':
				cmask |= mask;
			}
		}
		if(!token)
			continue;


		if(!strncmp(token, "::", 2))
			break;

		if(!stricmp(token, ".module"))
		{
			name = getToken();
			if(name)
				break;
			continue;
		}

		if(!stricmp(token, ".sub"))
		{
			name = getToken();
			if(name)
			{
				sub = true;
				break;
			}
			continue;
		}

		if(!stricmp(token, ".requires"))
		{
			while(NULL != (token = getToken()));
			{
				if(getHandler(token))
					continue;
				req = true;
				slog(Slog::levelCritical) << token << ": required keyword missing" << endl;
			}
			if(req)
				exit(-1);
			continue;
		}	

		if(!stricmp(token, ".include") && scrStream == (istream *)&scrSource)
		{
			token = getToken();
			if(!token)
				continue;
			
			if(*token != '/')
			{
				strcpy(namebuf, scrname);
				cp = strrchr(namebuf, '/');
				if(cp)
				{
					strcpy(cp + 1, token);
					token = namebuf;
				}
			}
			pos = scrSource.tellg();
			scrSource.close();
			scrSource.clear();
			include(token);
			scrSource.open(scrname, ios::in); 
			scrSource.seekg(pos);
			continue;
		}

		if(!stricmp(token, ".template") && scrStream == (istream *)&scrSource)
		{
			token = getToken();
			if(*token != '/')
			{
				strcpy(namebuf, scrname);
				cp = strrchr(namebuf, '/');
				if(cp)
				{
					strcpy(cp + 1, token);
					token = namebuf;
				}
			}
			pos = scrSource.tellg();
			scrSource.close();
			scrSource.clear();
			tmpl = include(token);
			scrSource.open(scrname, ios::in);
			scrSource.seekg(pos);
			if(!tmpl)
				continue;

			for(i = 0; i < TRAP_BITS; ++i)
			{
				if(!script->trap[i])
					script->trap[i] = tmpl->trap[i];
			}	
			continue;
		}

		if(!stricmp(token, ".define"))
		{
			while(NULL != (token = getToken()))
			{
				if(!cmds->getLast(token))
					cmds->setValue(token, getToken());
				else
					getToken();
			}
			continue;
		}

		if(*token == '!')
		{
			errflag = true;
			++token;
		}
		trapflag = false;
		handler = cmds->getHandler(token);
		if(!handler)
		{
			addmask = submask = 0;
			slog(Slog::levelError) << token << ": unknown keyword; " << script->name << "(" << number << ")" << endl;
			continue;
		}

		command = alloc(token);
	
		argc = 0;
		while(argc < 32 && NULL != (token = getToken()))
		{
			if(*token == '$')
			{
				token = (char *)cmds->getLast(++token);
				if(!token)
					token = "";
			}
			else
				token = alloc(token);
			
			args[argc++] = token;
			if(!stricmp(token, "then") && !stricmp(command, "if"))
			{
				--bp;
				*bp = ' ';
				then = true;
				handler = &ScriptInterp::scrIfThen;
				break;
			}
		}

		args[argc++] = NULL;
		line = (Line *)alloc(sizeof(Line));
		line->line = number;
		line->cmask = cmask;
		line->mask = ((~0 & ~trapmask) | addmask) & ~submask;
		if(script->mask)
			line->mask &= cmds->getTrapHandler(script);
		line->next = NULL;
		line->args = (char **)alloc(sizeof(char *) * argc);
		line->argc = --argc;
		line->error = errflag;
		line->method = handler;
		line->cmd = command;
		line->loop = 0;

		addmask = submask = 0;

		if(!stricmp(command, "repeat") || !stricmp(command, "for") || !stricmp(command, "do") || !stricmp(command, "foreach"))
		{
			if(!looplevel)
				++loopid;
			++looplevel;
			line->loop = loopid * 256 + looplevel;
		}

		if(!stricmp(command, "loop"))
		{
			line->loop = loopid * 256 + looplevel;
			if(!looplevel)
			{
				slog(Slog::levelError) << "loop: nesting error" << script->name << "(" << number << ")" << endl;
				continue;
			}
			else
				--looplevel;
		}
		memcpy(line->args, &args, sizeof(char *) * argc);

		err = cmds->check(command, line, this);
		if(err)
		{
			slog(Slog::levelError) << command << ": " << err << "; " << script->name << "(" << number << ")" << endl;
			continue;
		}

		++count;
		script->mask |= trapmask;
		if(!script->first)
			script->first = line;

		if(trapmask && !last)
		{
			for(i = 0; i < TRAP_BITS; ++i)
			{
				if((1l << i) & trapmask)
				{
					if(!script->trap[i])
						script->trap[i] = line;
				}
			}
		}
		if(branch)
		{
			for(i = 0; i < 10; ++i)
			{
				if((1l << i) & branch)
				{
					if(!script->skip[i])
						script->skip[i] = line;
				}
			}
		}

		if(last)
			last->next = line;
			
		last = line;
		branch = 0;
	}
	line = script->first;
	if(!script->mask)
		script->mask = cmds->getTrapDefault();
	while(line)
	{
		line->mask &= script->mask;
		line = line->next;
	}
	total += count;
	slog(Slog::levelInfo) << script->name << ": " << count << " steps compiled" << endl;
	if(!scrStream->eof())
	{
		strcpy(namebuf, basename);
		strcat(namebuf, "::");
		strcat(namebuf, name);
		name = namebuf;
		goto compile;
	}
	scrSource.close();
	scrSource.clear();
	return total;
}

#ifdef	CCXX_NAMESPACES
};
#endif
