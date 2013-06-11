// virtualmachine.h
// Gregory Rosenblatt

#ifndef VirtualMachine_H__
#define VirtualMachine_H__

#include <iostream>
#include <vector>

namespace MetaLang
{
    // enumerations describing the bytecode instructions the VM is able to process
    enum opcode
    {
        op_talk,
        op_end
    };

    // the basic instruction, currently just encapsulating an opcode
    class Instruction
    {
    public:
        Instruction(opcode code) : _code(code)	{}
        opcode Code() const         { return _code; }
        //const char* Data() const    { return _data; }
    private:
        opcode  _code;
        //char*   _data;  // additional data, currently not necessary
    };

    // the basic script, currently just encapsulating an arrayed list of instructions
    class Script
    {
    public:
        Script(const std::vector<Instruction>& instrList)
            : _instrList(instrList)	{}
        const Instruction* InstrPtr() const { return &_instrList[0]; }
    private:
        std::vector<Instruction>    _instrList;
    };

    // rudimentary virtual machine with methods inlined for convenience
    class VirtualMachine
    {
    public:
        VirtualMachine()
            : _scriptPtr(0), _instrPtr(0), _instr(0),
            _scriptCount(0) {}
        // a very basic interface
        inline void Execute(size_t scriptId);
        size_t Load(const Script& script)   { return AddScript(script); }
    private:	// useful abstractions
	// pointers used as non-modifying dynamic references
        typedef const Script*       ScriptRef;
        typedef const Instruction*  InstrRef;
    private:	// utilities
        size_t AddScript(const Script& script) // add script to list and retrieve id
        {_scriptList.push_back(script); return _scriptCount++;}
        void SelectScript(size_t index) // set current script by id
        {assert(index < _scriptCount);  // make sure the id is valid
        _scriptPtr = &_scriptList[index];
        _instrPtr = _scriptPtr->InstrPtr();}
    private:	// data members
        std::vector<Script> _scriptList;
        ScriptRef           _scriptPtr; // current script
        InstrRef            _instrPtr;  // root instruction
        InstrRef            _instr;     // current instruction
        size_t              _scriptCount;	// track the loaded scripts
    };

    void VirtualMachine::Execute(size_t scriptId)
    {
        SelectScript(scriptId);	// select our _instrPtr by script ID
        _instr = _instrPtr;	// set our iterator to the beginning
        while (_instr)
        {
            switch(_instr->Code())
            {
            case op_talk:
                std::cout << "I am talking." << std::endl;
                ++_instr;	// iterate
                break;
            case op_end:
                _instr = 0;	// discontinue the loop
                break;
            }
        }
    }
}

#endif
