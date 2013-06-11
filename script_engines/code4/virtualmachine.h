// virtualmachine.h
// Gregory Rosenblatt

#ifndef VirtualMachine_H_
#define VirtualMachine_H_

#include <iostream>
#include <vector>
#include <assert.h>
#include "utilities.h"

namespace MetaLang
{
    // enumerations describing the bytecode instructions the VM is able to process
    enum opcode
    {
        op_output=0,
        
        // stack
        op_push_const,
        op_push_var,
        op_assign,

        // arithmetic
        op_add,
        op_subtract,
        op_multiply,
        op_divide,
        
        // logical
        op_not,
        op_and,
        op_or,
                
        // comparison
        op_equal,
        op_not_equal,
        op_greater,
        op_greater_equal,
        op_less,
        op_less_equal,
        
        // flow control
        op_jump,
        op_jump_if_true,
        op_jump_if_false,

        op_end,

        // not an opcode
        num_codes   // value is the number of opcodes if first opcode has value of 0
    };

    // the basic instruction
    class Instruction
    {
    public:
        Instruction(opcode code) : _code(code)  {}
        Instruction(opcode code, const char* data, size_t dataSize)
            : _code(code), _data(data, dataSize)  {}
        opcode Code() const         { return _code; }
        const char* Data() const    { return _data.Data(); }
    private:
        opcode  _code;
        Buffer  _data;  // additional data
    };

    // the basic script
    class Script
    {
    public:
        Script(const std::vector<Instruction>& instrList, size_t varCount)
            : _instrList(instrList), _varCount(varCount)	{}
        const Instruction* InstrPtr() const { return &_instrList[0]; }
        const Instruction* End() const      { return _instrList.end(); }
        size_t VarCount() const             { return _varCount; }
    private:
        std::vector<Instruction>    _instrList;
        size_t                      _varCount;
    };

    // our virtual machine thus far
    class VirtualMachine
    {
    public:
        // a very basic interface
        void Execute(size_t scriptId);
        size_t Load(const Script& script)   { return AddScript(script); }
        // debugging
        void ShowVariableState() const    { _execution.ExposeStackState(std::cout); }
    private:
        // pointers used as non-modifying dynamic references
        typedef const Script*       ScriptRef;
        typedef const Instruction*  InstrRef;
        // a script's executional state
        class Execution
        {
        public:
            Execution() : _scriptPtr(0), _instrPtr(0), _instr(0)    {}
            void Execute(ScriptRef scriptPtr);
            // initialization
            void SetDataSize(size_t varCount)   { _stack.SetInitialScope(varCount); }
            // data access
            void SetVar(size_t i, char val) { _stack[i] = val; }
            char GetVar(size_t i) const     { return _stack[i]; }
            void ExposeStackState(std::ostream& out) const;
        private:
            ScopeStack<int>     _stack; // initially of type ScopeStack<char>
            ScriptRef           _scriptPtr; // executing script
            InstrRef            _instrPtr;  // root instruction
            InstrRef            _instrEnd;  // end of instruction list
            InstrRef            _instr;     // current instruction
        };
    private:    // utilities
        size_t AddScript(const Script& script) // add script to list and retrieve id
        { _scriptList.push_back(script); return (_scriptList.size()-1); }
        ScriptRef SelectScript(size_t index) const  // set current script by id
        {
            assert(index < _scriptList.size());  // make sure the id is valid
            return &_scriptList[index];
        }
    private:    // data members
        Execution           _execution; // since we're single-threaded, we only need one
        std::vector<Script> _scriptList;
    };
}

#endif

