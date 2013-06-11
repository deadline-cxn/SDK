// virtualmachine.h
// Gregory Rosenblatt

#ifndef VirtualMachine_H__
#define VirtualMachine_H__

#include <iostream>
#include <vector>
//#include <assert.h>
// may have to include such a header file if the asserts are problematic when compiling

namespace MetaLang
{
    // enumerations describing the bytecode instructions the VM is able to process
    enum opcode
    {
        op_talk,
        op_print,   // our new printing code

        // variable manipulators
        op_set, // char, char : destination index, value to set
        op_inc, // char : index to increment
        op_dec, // char : index to decrement
        op_add, // char, char, char : dest index, srce index1, srce index2

        op_end
    };

    // the basic instruction with copy-on-write
    class Instruction
    {
    public:
        Instruction(opcode code) : _code(code), _datsize(0), _data(0)	{}
        Instruction(opcode code, const char* data, size_t dataSize)
            : _code(code), _datsize(dataSize), _data(new char[dataSize])
        { memcpy(_data, data, dataSize); }	// we must store our own internal copy
        Instruction(const Instruction& instr)
            : _code(instr._code), _datsize(instr._datsize), _data(new char[_datsize])
        { memcpy(_data, instr._data, _datsize); }
        Instruction(const Instruction& instr)
	    : _code(instr._code), _datsize(instr._datsize), _data(new char[_datsize])
	{ memcpy(_data, instr._data, _datsize); }
	void operator=(const Instruction& instr)
	{ delete[] _data;
	  _size = instr._size;
	  _data = new char[_size];
	  memcpy(_data, instr._data, _datsize); }
        ~Instruction()  { delete[] _data; }	// and we must then clean up after it

        opcode Code() const         { return _code; }
        const char* Data() const    { return _data; }
    private:
        opcode  _code;
	size_t  _datsize;
        char*   _data;  // additional data
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

    // a script's executional state
    class ScriptState
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

    // rudimentary virtual machine with methods inlined for convenience
    class VirtualMachine
    {
    public:
        VirtualMachine() : _scriptPtr(0), _instrPtr(0), _instr(0)	{}

        // a very basic interface
        inline void Execute(size_t scriptId);
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

        void ExposeVariableState(const ScriptState& state) const
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
        ScriptState         _curState;	// since we're single-threaded, only need one
        std::vector<Script> _scriptList;
        ScriptRef           _scriptPtr; // current script
        InstrRef            _instrPtr;  // root instruction
        InstrRef            _instr;     // current instruction
    };

    void VirtualMachine::Execute(size_t scriptId)
    {
        SelectScript(scriptId);	// select our _instrPtr by script ID
        _curState.SetDataSize(_scriptPtr->VarCount());  // initialize variable data
        _instr = _instrPtr;	// set our iterator to the beginning
        while (_instr)
        {
            switch(_instr->Code())
            {
            // message functionality
            case op_talk:
                std::cout << "I am talking." << std::endl;
                ++_instr;	// iterate
                break;
            case op_print:
                std::cout << _instr->Data() << std::endl;    // print data
                ++_instr;   // iterate
                break;
            // new semi-math functionality
            case op_set:
                assert(_instr->Data()[0] < _scriptPtr->VarCount());
                _curState.SetVar(_instr->Data()[0], _instr->Data()[1]);
                ++_instr;
                break;
            case op_inc:
                assert(_instr->Data()[0] < _scriptPtr->VarCount());
                _curState.SetVar(_instr->Data()[0], _curState.GetVar(_instr->Data()[0])+1);
                ++_instr;
                break;
            case op_dec:
                assert(_instr->Data()[0] < _scriptPtr->VarCount());
                _curState.SetVar(_instr->Data()[0], _curState.GetVar(_instr->Data()[0])-1);
                ++_instr;
                break;
            case op_add:
                assert(_instr->Data()[0] < _scriptPtr->VarCount());
                assert(_instr->Data()[1] < _scriptPtr->VarCount());
                assert(_instr->Data()[2] < _scriptPtr->VarCount());
                _curState.SetVar(_instr->Data()[0],
                                _curState.GetVar(_instr->Data()[1])
                                + _curState.GetVar(_instr->Data()[2]));
                ++_instr;
                break;
            // end
            case op_end:
                _instr = 0;	// discontinue the loop
                break;
            }
        }
    }
}

#endif
