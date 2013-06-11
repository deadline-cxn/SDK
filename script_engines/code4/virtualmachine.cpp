// virtualmachine.cpp
// Gregory Rosenblatt

#include "virtualmachine.h"
using namespace std;
using namespace MetaLang;

void VirtualMachine::Execution::Execute(ScriptRef scriptPtr)
{
    _scriptPtr = scriptPtr;
    SetDataSize(_scriptPtr->VarCount());    // initialize variable data
    _instrPtr = _scriptPtr->InstrPtr();     // initialize root pointer
    _instrEnd = _scriptPtr->End();          // initialize end marker
    _instr = _instrPtr; // set our iterator to the beginning
    while (_instr < _instrEnd)  // ensure pointer stays in-bounds
    {
        switch(_instr->Code())
        {
        // output
        case op_output:
            cout << static_cast<int>(_stack.Top()) << endl;
            _stack.Pop();
            ++_instr;
            break;
        // stack
        case op_push_const:
            _stack.Push(_instr->Data()[0]);
            ++_instr;
            break;
        case op_push_var:
            //assert(_instr->Data()[0] < _scriptPtr->VarCount());
            _stack.Push(_stack[_instr->Data()[0]]);
            ++_instr;
            break;
        case op_assign:
            //assert(_instr->Data()[0] < _scriptPtr->VarCount());
            _stack[_instr->Data()[0]] = _stack.Top();
            _stack.Pop();
            ++_instr;
            break;
        // arithmetic
        case op_add:
        {
            char top = _stack.Top();
            _stack.Pop();
            _stack.Top() += top;
        }
            ++_instr;
            break;
        case op_subtract:
        {
            char top = _stack.Top();
            _stack.Pop();
            _stack.Top() -= top;
        }
            ++_instr;
            break;
        case op_multiply:
        {
            char top = _stack.Top();
            _stack.Pop();
            _stack.Top() *= top;
        }
            ++_instr;
            break;
        case op_divide:
        {
            char top = _stack.Top();
            _stack.Pop();
            _stack.Top() /= top;
        }
            ++_instr;
            break;
        // logical
        case op_not:
            _stack.Top() = !_stack.Top();
            ++_instr;
            break;
        case op_and:
        {
            char top = _stack.Top();
            _stack.Pop();
            _stack.Top() = (_stack.Top() && top);
        }
            ++_instr;
            break;
        case op_or:
        {
            char top = _stack.Top();
            _stack.Pop();
            _stack.Top() = (_stack.Top() || top);
        }
            ++_instr;
            break;        
        // comparison
        case op_equal:
        {
            char top = _stack.Top();
            _stack.Pop();
            _stack.Top() = (_stack.Top() == top);
        }
            ++_instr;
            break;
        case op_not_equal:
        {
            char top = _stack.Top();
            _stack.Pop();
            _stack.Top() = (_stack.Top() != top);
        }
            ++_instr;
            break;
        case op_greater:
        {
            char top = _stack.Top();
            _stack.Pop();
            _stack.Top() = (_stack.Top() > top);
        }
            ++_instr;
            break;
        case op_greater_equal:
        {
            char top = _stack.Top();
            _stack.Pop();
            _stack.Top() = (_stack.Top() >= top);
        }
            ++_instr;
            break;
        case op_less:
        {
            char top = _stack.Top();
            _stack.Pop();
            _stack.Top() = (_stack.Top() < top);
        }
            ++_instr;
            break;
        case op_less_equal:
        {
            char top = _stack.Top();
            _stack.Pop();
            _stack.Top() = (_stack.Top() <= top);
        }
            ++_instr;
            break;
        // flow control
        case op_jump:
            _instr = _instrPtr + _instr->Data()[0];
            break;
        case op_jump_if_true:
            if (_stack.Top() != 0)
                _instr = _instrPtr + _instr->Data()[0];
            else
                ++_instr;
            _stack.Pop();                
            break;
        case op_jump_if_false:
            if (_stack.Top() == 0)
                _instr = _instrPtr + _instr->Data()[0];
            else
                ++_instr;
            _stack.Pop();
            break;           
        // end
        case op_end:
            _instr = _instrEnd;  // discontinue the loop
            break;
        }
    }
}

void VirtualMachine::Execution::ExposeStackState(ostream& out) const
{
    size_t numScopes = _stack.ScopeDepth();
    size_t stackSize = _stack.Size();
    size_t index, label, curDepth = 0, curEnd = 0;
    for (index = 0, label = 0; index < stackSize; ++index, ++label) // for the entire stack
    {
        if (index == curEnd)    // for every new scope in the stack
        {
            label = 0;
            out << "Scope Depth: " << curDepth << endl; // output the scope depth
            if (curDepth < numScopes)
            {
                curEnd = _stack.GetScopeIndex(curDepth);    // get end of this scope
                ++curDepth; // advance to next scope depth
            }
            else
                curEnd = 0;
        }
        out << "    " << label << ": " << static_cast<int>(_stack[index]) << endl;  // output each value
    }
}

void VirtualMachine::Execute(size_t scriptId)
{
    _execution.Execute(SelectScript(scriptId)); // select our script by ID
}

