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
        op_talk=0,
        op_print,   // our new printing code

        // variable manipulators
        op_set, // char, char : destination index, value to set
        op_inc, // char : index to increment
        op_dec, // char : index to decrement
        op_add, // char, char, char : dest index, srce index1, srce index2

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
        size_t VarCount() const             { return _varCount; }
    private:
        std::vector<Instruction>    _instrList;
        size_t                      _varCount;
    };

    // our virtual machine thus far
    class VirtualMachine
    {
    private:
        // a script's executional state
        class Execution
        {
        public:
            // initialization
            void SetDataSize(size_t varCount)   { _varData.resize(varCount); }
            // data access
            void SetVar(size_t i, char val) { assert(i < _varData.size()); _varData[i] = val; }
            char GetVar(size_t i) const     { assert(i < _varData.size()); return _varData[i]; }
            const std::vector<char>& DataArray() const  { return _varData; }
        private:
            std::vector<char>   _varData;
        };
    public:
        VirtualMachine() : _scriptPtr(0), _instrPtr(0), _instr(0)	{}
        // a very basic interface
        void Execute(size_t scriptId);
        size_t Load(const Script& script)   { return AddScript(script); }
        // debugging
        void ShowVariableState() const    { ExposeVariableState(_curState); }
    private:	// useful abstractions
        // pointers used as non-modifying dynamic references
        typedef const Script*       ScriptRef;
        typedef const Instruction*  InstrRef;
    private:	// utilities
        size_t AddScript(const Script& script) // add script to list and retrieve id
        { _scriptList.push_back(script); return (_scriptList.size()-1); }
        void SelectScript(size_t index) // set current script by id
        {
            assert(index < _scriptList.size());  // make sure the id is valid
            _scriptPtr = &_scriptList[index];
            _instrPtr = _scriptPtr->InstrPtr();
        }

        void ExposeVariableState(const Execution& state) const
        {
            std::vector<char>::const_iterator itr;
            int n = 0;  // used to denote indexed position of value
            for (itr = state.DataArray().begin(); itr != state.DataArray().end(); ++itr, ++n)
            {
                std::cout << n << ": ";
                std::cout << static_cast<int>(*itr);   // cast for numeric value
                std::cout << std::endl;
            }
        }
    private:	// data members
        Execution           _curState;	// since we're single-threaded, only need one
        std::vector<Script> _scriptList;
        ScriptRef           _scriptPtr; // current script
        InstrRef            _instrPtr;  // root instruction
        InstrRef            _instr;     // current instruction
    };
}

#endif

