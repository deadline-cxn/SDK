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

#ifndef	CCXX_SCRIPT_H_
#define	CCXX_SCRIPT_H_

#ifndef CCXX_MISC_H_
#include <cc++/misc.h>
#endif

#ifndef	CCXX_FILE_H_
#include <cc++/file.h>
#endif

#include <iostream>
#include <fstream>

#ifdef	CCXX_NAMESPACES
namespace ost {
#endif

class CCXX_CLASS_EXPORT ScriptCommand;
class CCXX_CLASS_EXPORT ScriptImage;
class CCXX_CLASS_EXPORT ScriptInterp;

#define	MAX_LOCKS 8
#define	TRAP_BITS (sizeof(unsigned long) * 8)
#define SCRIPT_STACK_SIZE 20
#define	SCRIPT_TEMP_SPACE 16
#define	KEYWORD_INDEX_SIZE 37
#define SYMBOL_INDEX_SIZE 187
#define SCRIPT_INDEX_SIZE KEYWORD_INDEX_SIZE

class Script
{
protected:
	class Line;

	typedef bool (ScriptInterp::*Method)(void);
	typedef char *(ScriptCommand::*Check)(Line *line, ScriptImage *img);

	enum	SymType
	{
		NORMAL = 0,
		ALIAS,
		FIFO,
		INDEX,
		SEQUENCE,
		STACK,
		COUNTER,
		TRIGGER,
		POINTER,
		REF,
		CACHE
	};
	typedef enum SymType SymType;

#pragma pack(1)
	typedef struct _symbol
	{
		struct _symbol *next;
		char *id;
		struct
		{
			unsigned size : 16;
			bool initial : 1;
			bool system : 1;
			bool readonly : 1;
			bool commit : 1;
			SymType type : 6;
		} flags;
		char data[1];
	}	Symbol;

	class Line
	{
	public:
		Line *next;
		unsigned long cmask;
		unsigned long mask;
		unsigned short loop;
		unsigned short line;
		unsigned char argc;
		bool error : 1;
		Method method;
		char *cmd;
		char **args;
	};

	class Name
	{
	public:
		Name *next;
		Line *first;
		Line *trap[TRAP_BITS];
		Line *skip[10];
		unsigned long mask;
		char *name;
		enum Mode
		{
			ORIGINAL,
			COPIED,
			COPY
		} mode;
		typedef enum Mode Mode;
	};

	class Define
	{
	public:
		const char *keyword;
		Method method;
		Check check;
	};

#pragma pack()

	/**
 	 * This class holds a distributed lock set used for global script locks
 	 * and the new lock/unlock commands.
 	 *
 	 * @author David Sugar <dyfet@ostel.com>
	 * @short global lock table.
	 */
	class Locks : private Mutex
	{
	public:

		ScriptInterp *locks[MAX_LOCKS];

		void release(ScriptInterp *interp);
		bool lock(ScriptInterp *interp, unsigned id);
		bool unlock(ScriptInterp *interp, unsigned id);

		Locks();
	};

	/**
	 * A class to represent internal ccScript loaded modules.  These are
	 * collected either from "use" statements embedded in ccScript programs
	 * or from implicitly calling Script::use() directly.  If
	 * the module is not already loaded, then it is created and linked.
	 *
	 * @author David Sugar <dyfet@ostel.com>
	 * @short loaded dso modules.
	 */
	class Package : protected DSO
	{
	public:
		static Package *first;
		Package *next;
		char *filename;

		Package(char *name);
	};
public:
	CCXX_MEMBER_EXPORT(static bool) use(const char *name);

	/**
	 * This class holds an "active" session handler that has been created
	 * by a "slow" module handler.  This provides a mechanism for active
	 * sessions to impliment thread-based callback and thread based
	 * session handlers in a generic fashion.
	 *
	 * @author David Sugar <dyfet@ostel.com>
	 * @short active session base class.
	 */
	class CCXX_CLASS_EXPORT Session
	{
	private:
		friend class ScriptInterp;
		ScriptInterp *interp;

	protected:
		/**
		 * Notify interpreter callback that the session is complete
		 * and pass an optional signal handler for stepping.
		 *
	 	 * @param optional signal handler.
		 */
		void stepScheduler(const char *sighandler = NULL);

		/**
		 * Notify interpreter callback that a timed operation is
		 * in progress.
		 *
		 * @param time delay of operation.
		 */
		void sleepScheduler(timeout_t delay);

		/**
		 * Base constructor.
		 */
		Session(ScriptInterp *interp);

		virtual ~Session()
			{return;};

	public:
		/**
		 * Used to wait for operation completion.
		 */
		virtual void waitHandler(void) = 0;
	};

	/**
	 * This class is used for DSO modules that impliment property
	 * extensions for scripting objects.
	 *
	 * @author David Sugar <dyfet@ostel.com>
	 * @short ccScript property module
	 */
	class CCXX_CLASS_EXPORT Property
	{
	private:
		friend class ScriptInterp;

		static Property *first;
		Property *next;
		const char *id;

	protected:
		/**
		 * Set property method.  Performs set.xxx and init.xxx methods.
		 *
		 * @param data buffer to work from.
		 * @param temp workspace buffer to use.
		 * @param size of data area.
		 * @param size of temp area.
		 */
		virtual void setProperty(char *data, char *temp, size_t size) = 0;

		/**
		 * Get property method.  Performs var.xxx conversions.
		 *
		 * @param data buffer to copy from.
		 * @param temp workspace to save to.
		 * @param size of temp workspace.
		 */
		virtual void getProperty(char *data, char *temp, size_t size) = 0;

		/**
		 * adjust value method.  Performs inc.xxx conversions.
		 *
		 * @param data buffer to work from.
		 * @param size of data buffer.
		 * @param accumulated offset.
		 */
		virtual void adjProperty(char *data, size_t size, int adjust)
			{return;};

		/**
		 * Get default size of creation if override.
		 *
		 * @return size to make.
		 */
		virtual size_t getPropertySize(void)
			{return 0;};

		Property(const char *name);

	public:
		static Property* find(const char *name);
	};
};

/**
 * This class is used for generic ccScript DSO plugins.  This allows
 * plugins to become part of the base ccScript engine.
 *
 * @author David Sugar <dyfet@ostel.com>
 * @short ccScript loadable module
 */
class CCXX_CLASS_EXPORT ScriptModule : public Script
{
private:
	friend class ScriptInterp;
	friend class ScriptCommand;
	static ScriptModule *first;
	ScriptModule *next;
	const char *cmd;

protected:
	/**
	 * Method to call for handling script attach events.
	 *
	 * @param interpreter being attached.
	 */
	virtual void moduleAttach(ScriptInterp *interp)
		{return;};

	/**
	 * Method to call for handling script detach events.
	 *
	 * @param interpreter being detached.
	 */
	virtual void moduleDetach(ScriptInterp *interp)
		{return;};

	/**
	 * Check for script handler.
	 *
 	 * @return error string.
	 * @param interpreter image.
	 * @param line object parsed.
	 * @param pointer to session handler.
	 */
	virtual char *getSession(ScriptInterp *interp, Line *line, Session **session)
		{return NULL;};

	/**
	 * Validate script command for parser.
	 *
	 * @return error string or NULL if ok.
	 * @param script line evaluated.
	 * @param image set analyzed.
	 */
	virtual char *checkScript(Line *line, ScriptImage *img)
		{return NULL;};

	/**
	 * Construct and link-in a command handler.
	 *
	 * @param name of command being created.
	 */
	ScriptModule(const char *name);

	/**
	 * Find a given module.
	 *
	 * @return module found.
	 * @param module name.
	 */
	static ScriptModule *find(const char *name);
};

/**
 * This class holds the bound keyword set for a given Bayonne style
 * script interpreter.  Application specific dialects are created
 * by deriving a application specific version of ScriptCommand which
 * then binds application specific keywords and associated methods
 * in an application derived ScriptInterp which are typecast to
 * (scriptmethod_t).
 *
 * @author David Sugar <dyfet@ostel.com>
 * @short Bayonne script keyword binding tables and compiler constants.
 */
class CCXX_CLASS_EXPORT ScriptCommand : public Keydata, public Mutex, public Script
{
private:
	friend class ScriptImage;
	friend class ScriptInterp;
	friend class ScriptModule;

#pragma pack(1)
	typedef struct _keyword
	{
		struct _keyword *next;
		Method method;
		Check check;
		char keyword[1];
	}	Keyword;
#pragma pack()


	Keyword *keywords[KEYWORD_INDEX_SIZE];
	char *traps[TRAP_BITS];
	ScriptImage *active;
	int keyword_count;
	int trap_count;

protected:
	/**
	 * Get the method handler associated with a given keyword.  This
	 * is used by ScriptImage when compiling.
	 *
	 * @param keyword to search for.
	 * @return method handler to execute for this keyword.
	 */
	Method getHandler(const char *keyword);

	/**
	 * Check keyword syntax.
	 *
	 * @return syntax error string or NULL.
	 * @param name of keyword to check.
	 * @param hook to compiler source.
	 */
	char *check(char *command, Line *line, ScriptImage *img);

	/**
	 * Get the trap id number associated with a trap name.
	 *
	 * @return trap id number, 0 (exit) if invalid.
	 * @param name of trap identifier.
	 */
	virtual unsigned getTrapId(const char *trap);

	/**
	 * Get the mask bits for the default script.
	 *
	 * @return trap mask to use.
	 */
	virtual unsigned long getTrapDefault(void)
		{return 0x00000003;};

	/**
	 * Get the mask bits for a trap "handler".
	 *
	 * @return trap mask to use.
	 */
	virtual unsigned long getTrapHandler(Name *scr)
		{return getTrapDefault();}

	/**
	 * Get a trap mask for a given identifer.  This is a virtual
	 * since some derived handlers may manipulate mask bits.
	 *
	 * @return signal mask.
	 * @param identifier.
	 */
	virtual unsigned long getTrapMask(unsigned id);

	/**
	 * A helper method for the compiler.  Converts a named
	 * trap into it's bit shifted mask.  By making it a virtual,
	 * derived dialects can add "aliases" to default trap names.
	 *
	 * @param name of trap identifier.
	 * @return bit shifted mask or 0 if invalid.
	 */
	virtual unsigned long getTrapModifier(const char *trapname)
		{return getTrapMask(trapname);};

	/**
	 * A helper method for the compiler used specifically for
	 * "^" trap subsection requests.  These will occasionally
	 * carry different attribute settings.
	 *
	 * @param name of trap identifier.
	 * @return bit shifted mask or 0 if invalid.
	 */
	virtual unsigned long getTrapMask(const char *trapname);

	/**
	 * Default compiler syntax to accept any syntax.
	 */
	char *chkIgnore(Line *line, ScriptImage *img);

	/**
	 * Module based script checker.
	 */
	char *chkModule(Line *line, ScriptImage *img);

	/**
	 * Performs DSO load phase for USE modules.
	 */
	char *chkUse(Line *line, ScriptImage *img);

	/**
	 * A check used by "inc" and "dec".
	 *
	 * @return synxtax error message string or NULL.
	 * @param line statement.
	 */
	char *chkHasModify(Line *line, ScriptImage *img);

	/**
	 * Script compiler syntax check for certain variable using
	 * statements such as "clear".  Assumes list of valid variable
	 * arguments.
	 */
	char *chkHasVars(Line *line, ScriptImage *img);

	/**
	 * Script compiler syntax check for assignment statements
	 * such as "set", "for", etc.
	 *
	 * @return syntax error message string or NULL.
	 * @param line statement.
	 */
	char *chkHasList(Line *line, ScriptImage *img);

	/**
	 * Script compiler syntax check for commands that require
	 * no arguments to be present.
	 *
	 * @return syntax error message string or NULL.
	 * @param line statement.
	 */
	char *chkNoArgs(Line *line, ScriptImage *img);

	/**
	 * Script compiler syntax check for commands that require
	 * one or more arguments to be present.
	 *
	 * @return syntax error message string or NULL.
	 * @param line statement.
	 */
	char *chkHasArgs(Line *line, ScriptImage *img);

	/**
	 * Load a set of keywords into the system keyword table.  This
	 * provides a convenient method of initializing and adding to
	 * the keyword indexes.
	 *
	 * @param keyword entries to load.
	 */
	void load(Script::Define *keywords);

	/**
	 * Add a trap handler symbolic identity to the keyword table.
	 * These are used to handle signal mask coercion and event
	 * branch points in the compiler.
	 *
	 * @param requested trap name.
	 * @return assigned id number for the trap.
	 */
	int trap(const char *name);

	/**
	 * Get count of active traps.
	 *
	 * @return count of active trap identifiers.
	 */
	inline int getCount(void)
		{return trap_count;};

	/**
	 * Perform compile time check of a specified symbol.
	 *
	 * @return syntax error message string.
	 * @param line pointer.
	 */
	virtual char *check(Check chk, Line *line, ScriptImage *img)
		{return (this->*(chk))(line, img);};

	/**
	 * Load a keydata entry for compile-time constants and
	 * bind the default ScriptInterp base class methods.  This
	 * class is never used alone, so no public constructor is
	 * provided.
	 *
	 * @param keydata entry for constants.
	 */
	ScriptCommand(const char *cfgfile);
};

/**
 * This class manages symbol tables for the scripting engine.  A symbol
 * is a variable with a reserved memory space.  Some derived systems, like
 * Bayonne IVR, may make use of a channel specific and global symbol
 * tables.
 *
 * @author David Sugar <dyfet@ostel.com>
 * @short Symbol table management for scripting engine.
 */
class CCXX_CLASS_EXPORT ScriptSymbol : public SharedMemPager, public Script
{
private:
	friend class ScriptInterp;

	int symsize;
	Symbol *index[SYMBOL_INDEX_SIZE];
	Symbol *trigger;

	unsigned getIndex(const char *symbol);

public:
	/**
	 * Fetch a symbol entry.  Normally this is presumed to be from
	 * the native symbol table.  However, the use of a virtual allows
	 * one to derive special "system" symbols as needed for Bayonne
	 * dialects, as well as specify special attributes and properties
	 * for system symbols.  For example, in the Bayonne IVR, symbols
	 * might be used to retrieve the current DTMF digit buffer and
	 * count, but these are stored as and manipulated in a channel
	 * structure rather than in "symbol space".
	 *
	 * @return symbol entry or NULL if not found.
	 * @param symbol name to find or create.
	 * @param size of symbol space to create if not found.
	 */
	virtual Symbol *getEntry(const char *symbol, int size = 0);

protected:
	/**
	 * A virtual method used when "committing" symbol changes.
	 * Normally this is a non-operation, but when system specific
	 * dialects are created, special symbols that must be converted
	 * from strings into other kinds of internal data types, or that
	 * might perform operations can be represented with the commit.
	 *
	 * @param symbol to commit.
	 */
	virtual void commit(Symbol *sym);

public:
	/**
	 * Fetch a pending trigger value and clear it.
	 *
	 * @return symbol trigger that is active.
	 */
	Symbol *getTrigger(void);

	/**
	 * Fetch default symbol size.
	 *
	 * @return default symbol size.
	 */
	inline int getSymbolSize(void)
		{return symsize;};

	ScriptSymbol(int size, int pgsize = 1024);

	/**
	 * Fetch an embedded pointer held in a symbol identifier.
	 *
	 * @param symbol name.
	 * @return pointer held in symbol or NULL.
	 */
	void *getPointer(const char *symbol);

	/**
	 * Set an embedded pointer object as a symbol.
	 *
	 * @param symbol name.
	 * @param data pointer to save.
	 * @return true if successful.
	 */
	bool setPointer(const char *symbol, void *data);

	/**
	 * Fetch the address of the value of a given symbol identifier.
	 *
	 * @param symbol name.
	 * @return symbol buffer address, or NULL if not found.
	 */
	char *getSymbol(const char *symbol);

	/**
	 * Set a given symbol to a new value.
	 *
 	 * @param symbol name to find.
	 * @param value string to set.
	 * @return NULL if not found, else value.
	 */
	char *setSymbol(const char *symbol, const char *value = "");

	/**
	 * Set a constant value.
	 *
	 * @param symbol name to find.
	 * @param value string to set.
	 * @return NULL if not found, else value.
	 */
	char *setConst(const char *symbol, const char *value = "");

	/**
	 * Create a sequence object (looping fifo)
	 *
	 * @return false if fail.
	 * @param name of sequence.
	 * @param max. number of sequences.
	 * @param size of sequence data record.
	 */
	bool makeSequence(const char *id, unsigned char count, unsigned char recsize);

	/*
	 * Creating a cache object (looping lifo)
	 *
	 * @return false if fail.
	 * @param name of cache.
	 * @param max. number of sequences.
	 * @param size of cache data record.
	 */
	bool makeCache(const char *id, unsigned char count, unsigned char recsize);

	/**
	 * Create a stack object.
	 *
	 * @return false if fail.
	 * @param name of stack.
	 * @param number of entries.
	 * @param size of record.
	 */
	bool makeStack(const char *id, unsigned char count, unsigned char recsize);

	/**
	 * Create a fifo object.
	 *
	 * @return false if fail.
	 * @param name of fifo.
	 * @param number of records.
	 * @param size of record.
	 */
	bool makeFifo(const char *id, unsigned char count, unsigned char recsize);

	/**
	 * Create an automatic counter object.
	 *
	 * @return false if fail.
	 * @param name of counter.
	 */
	bool makeCounter(const char *id);

	/**
	 * Post data into a fifo.
	 *
	 * @return false if full.
	 * @param symbol object to post.
	 * @param data value to post.
	 */
	bool postSymbol(Symbol *sym, const char *value);


	/**
	 * Remove entry from a queue.
	 *
	 * @return false if invalid.
	 * @param symbol object to remove from.
	 * @param data value to find and remove.
	 */
	bool removeSymbol(Symbol *sym, const char *value);

	/**
	 * Read symbol handles special symbol types.
	 *
	 * @return data of symbol.
	 * @param symbol.
	 */
	char *readSymbol(Symbol *sym);

	/**
	 * Set an alias.
	 *
	 * @param symbol name to alias.
	 * @param value of alias to set.
	 * @return bool if not set, else value.
	 */
	bool setAlias(const char *symbol, const char *source);

	/**
	 * Get an alias.
	 *
	 * @param symbol name to find.
	 * @return script symbol object or NULL.
	 */
	Symbol *getAlias(const char *symbol);

	/**
	 * Create a new symbol entry of a specified size.
	 *
	 * @param symbol name to create.
	 * @param size of symbol, otherwise default size used.
	 * @param buffer or NULL if exists.
	 */
	char *setSymbol(const char *symbol, int size = 0);

	/**
	 * Clear a symbol definition.  This does not remove the
	 * entry if it already exists.  It mearly clears the initial
	 * flag so it still can appear undefined.
	 *
	 * @param symbol name to clear.
	 */
	void clrSymbol(const char *id);

	/**
	 * Purge user defined symbols and memory space.
	 */
	void purge(void);

	/**
	 * Gather a listing of common script symbols.
	 */
	unsigned gather(Symbol **index, unsigned max, const char *prefrix, const char *suffix = "");
};

/**
 * A derivable class to hold compiled script images for active processes.
 * This includes the script image compiler itself.  Typically, a script is
 * compiled one file at a time from a directory, and the committed, during
 * the constructor in a derived class.
 *
 * @author David Sugar <dyfet@ostel.com>.
 * @short Script compiler image set.
 */
class CCXX_CLASS_EXPORT ScriptImage : public MemPager, public Script
{
protected:
	std::ifstream scrSource;
	std::istream *scrStream;
	ScriptCommand *cmds;
	int refcount;
	Name *index[SCRIPT_INDEX_SIZE];
	char buffer[512];
	char *bp;
	bool quote;
	Mutex duplock;

	friend class ScriptInterp;
	friend class ScriptModule;

	char *getToken(void);

	/**
	 * Get the interpreter method pointer for a given keyword.
	 *
	 * @return method handler.
	 * @param keyword to search.
	 */
	Method getHandler(const char *keyword)
		{return cmds->getHandler(keyword);};

	/**
	 * Construct a new working image.  This must be derived to an
	 * application specific compiler that can scan directories and
	 * invoke the compiler as needed.
	 *
	 * @param cmdset of keyword table object used.
	 */
	ScriptImage(ScriptCommand *cmdset);

	/**
	 * Purge and reload the script image workspace.
	 */
	void purge(void);

	/**
	 * A method to invoke the script compiler to include a script
	 * only if it has not been included already.
	 *
	 * @return named script object.
	 * @param name of script file to compile.
	 */
	Name *include(const char *scrfile);

	/**
	 * The script compiler itself.  This linearly compiles a Bayonne
	 * script file that is specified.  Normally used along with a dir
	 * scanner in the constructor.
	 *
	 * @return lines of script compiled.
	 * @param name of script file to compile.
	 */
	int compile(const char *scrfile);

	/**
	 * Compile a script from disk and give it a different internal
	 * "name" as passed.
	 *
	 * @return lines of script compiled.
	 * @param name of script file to compile.
	 * @param name of script to save under.
	 * @param test if script already loaded.
	 */
	int compile(const char *scrfile, char *name);

	/**
	 * Compile an open stream object into a script.
	 *
	 * @return lines of script compiled.
	 * @param stream object to use.
	 * @param name of script save under.
	 */
	int compile(std::istream *str, char *name, const char *scrname = NULL);

	/**
	 * Used in the derived constructor to "commit" the current image
         * for new processes.  This is usually the last statement in the
	 * derived constructor.
	 */
	void commit(void);

public:
	/**
	 * Fetch named script.
	 *
	 * @param script name to find.
	 * @return script or NULL.
	 */
	virtual Name *getScript(const char *name);

	/**
	 * Duplicate script if needed.
	 *
	 * @param script name to dup.
	 * @param target name to dup as.
	 * @return script or NULL.
	 */
	virtual Name *dupScript(const char *name, const char *target);

	/**
	 * Fetch list of relational scripts.
	 *
	 * @param suffix.
	 * @param index.
	 * @param max entries.
	 * @return count of entries found.
	 */
	unsigned gather(const char *suffix, Name **array, unsigned size);

	/**
	 * Used by embedded interpreters to fetch script from the current
	 * source file.
	 *
	 * @return reference to source file stream.
	 */
	inline std::istream *getSource(void)
		{return (std::istream *)&scrSource;};
};

/**
 * System script interpreter core engine class.  This class is further
 * derived to impliment application specific language dialects.
 *
 * @author David Sugar <dyfet@ostel.com>
 * @short Extensible Bayonne Scripting Engine.
 */
class CCXX_CLASS_EXPORT ScriptInterp : public ScriptSymbol
{
private:
	friend class ScriptImage;
	friend class Script::Session;
	friend class ScriptModule;

#pragma	pack(1)
	class Context
	{
	public:
		Name *script;
		Line *line, *read;
		unsigned char index;
		ScriptSymbol *local;
		bool caseflag;
	};
#pragma pack()

	static Locks locks;
	ScriptCommand *cmd;
	ScriptImage *image;
	Session *session;
	Context	script[SCRIPT_STACK_SIZE + 1];
	char *temps[SCRIPT_TEMP_SPACE];
	int tempidx;
	int stack;
	size_t symsize, pgsize;
	bool once, loop;
	char packtoken;
	unsigned long signalmask;

	bool scrTemplate(void);
	bool scrEnable(void);
	bool scrDisable(void);
	bool scrUse(void);
	bool scrLoadable(void);
	bool scrPacked(void);
	bool scrPack(void);
	bool scrUnpack(void);
	bool scrOn(void);
	bool scrSlog(void);
	bool scrBasename(void);
	bool scrDirname(void);
	bool scrFullpath(void);
	bool scrGather(void);
	bool scrDump(void);
	bool scrInc(void);
	bool scrDec(void);
	bool scrFifo(void);
	bool scrMin(void);
	bool scrMax(void);
	bool scrCounter(void);
	bool scrReset(void);
	bool scrRemove(void);
	bool scrPost(void);
	bool scrStack(void);
	bool scrCache(void);
	bool scrSequence(void);
	bool scrDup(void);
	bool scrArray(void);
	bool scrList(void);
	bool scrArm(void);
	bool scrDisarm(void);
	bool scrSet(void);
	bool scrAlias(void);
	bool scrRef(void);
	bool scrConst(void);
	bool scrSize(void);
	bool scrInit(void);
	bool scrClear(void);
	bool scrCall(void);
	bool scrHas(void);
	bool scrMissing(void);
	bool scrLabel(void);
	bool scrCase(void);
	bool scrEndcase(void);
	bool scrIf(void);
	bool scrIfThen(void);
	bool scrThen(void);
	bool scrElse(void);
	bool scrEndif(void);
	bool scrFor(void);
	bool scrRead(void);
	bool scrRepeat(void);
	bool scrForeach(void);
	bool scrTryeach(void);
	bool scrSwap(void);
	bool scrDo(void);
	bool scrLoop(void);
	bool scrBreak(void);
	bool scrContinue(void);
	bool scrReturn(void);
	bool scrPop(void);
	bool scrSelect(void);
	bool scrOnce(void);
	bool scrLock(void);
	bool scrTry(void);
	bool scrSkip(void);
	bool expConditional(void);

	friend class ScriptCommand;

protected:
	/**
	 * Derive a new instance of the system script interpreter.
	 *
	 * @param symsize for default symbol sizing.
	 * @param pgsize for memory fragmentation.
	 */
	ScriptInterp(ScriptCommand *cmd, size_t symsize, size_t pgsize = 1024);

	~ScriptInterp();

	/**
	 * Get a local and/or global trigger.
	 *
	 * @param use or clear.
	 */
	void getTrigger(bool use);

	/**
	 * Fetch unique "once" flag and set.
	 *
	 * @return true if once.
	 */
	bool getOnce(void);

	/**
	 * Notify signal mask for "on" handlers.
	 *
	 * @param mask value.
	 */
	inline void Notify(unsigned long mask)
		{signalmask |= mask;};

	/**
	 * Notify signal by symbolic name.
	 *
	 * @param mask name.
	 */
	inline void Notify(const char *str)
		{signalmask |= cmd->getTrapMask(str);};

	/**
	 * Fetch the current interpreter mask bits directly.
	 *
	 * @return interpreter mask.
	 */
	unsigned long getMask(void);

	/**
	 * Fetch the script mask identifer.
	 *
	 * @return script mask.
	 */
	inline unsigned long getScriptMask(const char *id)
		{return cmd->getTrapMask(id);};

	/**
	 * Fetch the active command interpreter subsystem.
	 *
	 * @return script interpreter.
	 */
	inline ScriptCommand *getCommand(void)
		{return cmd;};

	/**
	 * Used to process "conditional" arguments such as for IF/DO/LOOP
	 * statements.  The index is kept at the next logical argument
	 * so a goto can be performed if needed.
	 *
	 * @return true if conditional test is true.
	 */
	bool conditional(void);

	/**
	 * Some systems can derive alternative "exit" commands
	 * which can call this routine after performing other
	 * operations.
	 */
	bool scrExit(void);

	/**
	 * This is often called to process branching requests.
	 */
	bool scrGoto(void);

	/**
	 * Method used to encode "data" lines.
	 */
	bool scrData(void);

	/**
	 * Used to return channel identifiers in some systems.
	 *
	 * @return channel id or 0 for none.
	 */
	virtual unsigned getId(void)
		{return 0;};


	/**
	 * Used to fetch a global trap execution context branch.
	 *
	 * @return true if external global trap handler found.
	 * @param trap identifier.
	 */
	virtual bool getGlobalTrap(unsigned id)
		{return false;};

	/**
	 * Fetch a variable (symbol) that will be used to store data.
	 * This advances the index.
	 *
	 * @return symbol found if any.
	 * @param optional size to allocate if new symbol.
	 */
	Symbol *getVariable(size_t size = 0);


	/**
	 * May be used to override processing of indirect content
	 * requests.  This is used in the Bayonne IVR to fetch
	 * content constants based on language and country settings.
	 *
	 * @return symbol for indirect content.
	 * @param indirect fetch request.
	 */
	virtual Symbol *getIndirect(char *sym)
		{return NULL;};

	/**
	 * Advance program to the next script statement.
	 */
	void advance(void);

	/**
	 * Set error variable and advance to either the error handler
	 * or next script statement.
	 *
	 * @param error message.
	 */
	void error(const char *error);

	/**
	 * Set the execution interpreter to a trap identifier.  If no
	 * trap id exists, then advance to next script statement (unless
	 * exit trap).
	 *
	 * @param id of trap to select numerically.
	 */
	void trap(unsigned id);

	/**
	 * Select trap by symbolic name and execute if found, else advance
	 * to next script step (unless exit trap).
	 *
	 * @param name of trap to select.
	 */
	void trap(const char *trapname);

	/**
	 * Attempt to push a value onto the stack.
	 *
	 * @return false if stack overflow.
	 */
	bool push(void);

	/**
	 * Attempt to recall a previous stack level.
	 *
	 * @return false if stack underflow.
	 */
	bool pull(void);

	/**
	 * Signals are used during "delayed" execution steps when a
	 * signal event has occured aynchronously with the execution
	 * of a script controlled state event handler.  This mechanism
	 * can be used in place of calling implicit "Step" traps.
	 *
	 * @return true if signal handler is not blocked.
	 * @param name of signal identifier.
	 */
	bool signal(const char *trapname);

	/**
	 * Signals can be referenced by numeric id as well as by symbolic
	 * name.
	 *
	 * @return true if signal handler is not blocked.
	 * @param trap number of handler.
	 */
	bool signal(unsigned trapid);

	/**
	 * Runtime execution of script handler.  This can be called in
	 * the current or derived class to invoke extensible methods.
	 *
	 * @return true if immediately ready for next step.
	 * @param derived method to call.
	 */
	virtual bool execute(Method method)
		{return (this->*(method))();};

	/**
	 * Stop request handler.  Used for handling external trap
	 * requests during a "Signal".  This is needed in some state
	 * engines when the current state is being abandoned because
	 * of a signaled trap.
	 *
	 * @param signal mask forcing stop.
	 */
	virtual void stop(unsigned long mask)
		{return;};

	/**
	 * Exit request handler.  This is called when no script line
	 * exists.  No default behavior is known.
	 */
	virtual void exit(void) = 0;

	/**
 	 * Runtime branch point for label selection.
	 *
	 * @return script found.
	 * @param label to locate.
	 * @param script line associated.
	 */
	virtual Name *getScriptImage(const char *label);

	/**
	 * Fetch duplicative script image as needed.
	 *
	 * @return script object.
	 * @param label to dup.
	 */
	Name *getScriptCopy(const char *src);

	/**
	 * Patch point for sleep delay notification.
	 *
	 * @param sleep timeout expected.
	 */
	virtual void sleepScheduler(timeout_t timeout)
		{return;};

	/**
	 * Step scheduler callback to force step execution.
	 *
	 * @param trap name to use.
	 */
	virtual void stepScheduler(const char *trapname)
		{trap(trapname);};

public:
	/**
	 * Search for symbols either thru the virtual global space
	 * or thru local space if a local space has been allocated
	 * on the heap.
	 *
	 * @return symbol found.
	 * @param script symbol to look for or create.
	 * @param size of symbol to allocate.
	 */
	Symbol *getLocal(const char *name, size_t size = 0);

	/**
	 * Attempt to attach script engine to active session and
	 * specify a starting script.
	 *
	 * @return false if failed to attach.
	 * @param name of script entry.
	 */
	bool attach(const char *scrname);

	/**
	 * Detach the current script image.  If it is the last
	 * ref count and the exit flag is set, then delete it.
	 */
	void detach(void);

	/**
	 * Script redirection support.
	 *
	 * @return false if redirect failed.
	 * @param name of script entry.
	 */
	bool redirect(const char *scrname);

	/**
	 * Execute the next pending script statement.  If no statement
	 * is selected, then we execute the exit handler instead.  We
	 * can also force feed a trap identifier.
	 *
	 * @return true if advanced to next script statement already.
	 * @param optional trap branch point (also checks mask).
	 */
	bool step(const char *trapname = NULL);

	/**
	 * Test if script interpreter is currently active.
	 *
	 * @return true if active.
	 */
	inline bool isActive(void)
		{return script[stack].line;};

	/**
	 * Fetch next logical option but do not evaluate.  The index
	 * is advanced.
	 *
	 * @return option or NULL if end of list.
	 * @param optional default.
	 */
	char *getOption(const char *def = NULL);

	/**
	 * Fetch an embedded attribute keyword argument.  This allows
	 * embedded tag=value keywords to be extracted.
	 *
	 * @return option or NULL if not found.
	 * @param keyword to find.
	 */
	char *getKeyword(const char *keyword);

	/**
	 * Initialize symbols from the keyword list.
	 */
	int initKeywords(int size);

	/**
	 * Fetch and evaluate next logical option. If a symbol ref.
	 * then the symbol is also expressed.  The index is advanced.
	 *
	 * @return option value or NULL if end of list.
	 * @param optional default value.
	 */
	char *getValue(const char *def = NULL);

	/**
	 * Evaluate the content of an option retrieved with getOption.
	 *
	 * @return value of evaluation.
	 * @param option string.
	 */
	char *getContent(char *sym);

	/**
	 * Return the current script line object so it may be debugged.
	 *
	 * @return script image record being executed.
	 */
	inline Line *getScript(void)
		{return script[stack].line;};

	/**
	 * Return the member id of a script command.
	 *
	 * @return member id or NULL if none.
	 */
	const char *getMember(void);

	/**
	 * Return the master script object for resource context.
	 *
	 * @return script object being executed.
	 */
	inline Name *getObject(void)
		{return script[stack].script;};

	/**
	 * Return the script image holding this application's
	 * context.
	 *
	 * @return script image.
	 */
	inline ScriptImage *getImage(void)
		{return image;};

	/**
	 * Enable or disable autolooping of statements.
	 *
	 * @param true to enable autoloop (default).
	 */
	inline void autoloop(bool enable)
		{loop = enable;};
};

#ifdef	CCXX_NAMESPACES
};
#endif

#endif

