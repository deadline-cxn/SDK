// main.cpp
// Gregory Rosenblatt

#include "virtualmachine.h"
using namespace MetaLang;
using std::vector;

int main()
{
	VirtualMachine vm;

	// build the script
	vector<Instruction> InstrList;
	InstrList.push_back(Instruction(op_talk));	// talk twice
	InstrList.push_back(Instruction(op_talk));
	InstrList.push_back(Instruction(op_end));	// then end
	Script script(InstrList);

	// load the script and save the id
	size_t scriptID = vm.Load(script);

	// execute the script by its id
	vm.Execute(scriptID);

	// pause for input to prevent app from closing before getting a look
	std::cin.get();
	return 0;
}
