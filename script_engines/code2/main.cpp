// main.cpp
// Gregory Rosenblatt

#include "virtualmachine.h"
using namespace MetaLang;
using namespace std;

int main()
{
    VirtualMachine vm;

    // simulate some external data
    char* buffer = "this is printed";

    // build the talk script
    vector<Instruction> talkInstrList;
    talkInstrList.push_back(Instruction(op_talk));	// talk still works the same way
    talkInstrList.push_back(Instruction(op_print, buffer, strlen(buffer)+1));  // print
    talkInstrList.push_back(Instruction(op_end));	// then end
    Script talkScript(talkInstrList, 0);

    // create variable manipulation data
    char setData1[] = {0, 7}; char setData2[] = {1, 7}; char setData3[] = {2, 7};
    char incData = 1;
    char decData = 2;
    char addData[] = {3, 0, 2};// add 1st and 3rd var, and store in 4th

    // proper instruction data size constants (temporary for safety)
    const int SET_SIZE  = 2*sizeof(char);
    const int INC_SIZE  = sizeof(char);
    const int DEC_SIZE  = sizeof(char);
    const int ADD_SIZE  = 3*sizeof(char);

    // build the variable manipulation script
    vector<Instruction> varInstrList;
    varInstrList.push_back(Instruction(op_set, setData1, SET_SIZE));   // set first 3 vars to 7
    varInstrList.push_back(Instruction(op_set, setData2, SET_SIZE));
    varInstrList.push_back(Instruction(op_set, setData3, SET_SIZE));
    varInstrList.push_back(Instruction(op_inc, &incData, INC_SIZE));   // inc 2nd var
    varInstrList.push_back(Instruction(op_dec, &decData, DEC_SIZE));   // dec 3rd var
    varInstrList.push_back(Instruction(op_add, addData, ADD_SIZE));
    varInstrList.push_back(Instruction(op_end));	// then end
    Script varScript(varInstrList, 4);  // we need 4 variables

    // load the scripts and save the ids
    size_t talkID = vm.Load(talkScript);
    size_t varManipID = vm.Load(varScript);

    // execute each script by its id
    cout << "***TALK SCRIPT***" << endl;
    vm.Execute(talkID);
    cout << "\n***VARIABLE MANIPULATION***" << endl;
    vm.Execute(varManipID);
    // check out the variable states
    vm.ShowVariableState();

    // pause for input to prevent app from closing before getting a look
    cin.get();
    return 0;
}
