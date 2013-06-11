// virtualmachine.cpp
// Gregory Rosenblatt

#include "virtualmachine.h"
using namespace MetaLang;

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
            _curState.SetVar(_instr->Data()[0],_curState.GetVar(_instr->Data()[1])+_curState.GetVar(_instr->Data()[2]));
            ++_instr;
            break;
        // end
        case op_end:
            _instr = 0;	// discontinue the loop
            break;
        }
    }
}

