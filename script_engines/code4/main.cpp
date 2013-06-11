// main.cpp
// Gregory Rosenblatt

#include "virtualmachine.h"
#include "parsing.h"
using namespace MetaLang;
using namespace std;

//#define LOAD_FROM_CONSOLE
#define LOAD_FROM_FILE

const int NUM_VARIABLES = 8;

int main()
{
    string codeList[num_codes] =
    {
        "output",
        "push_const",
        "push_var",
        "assign",
        "add",
        "subtract",
        "multiply",
        "divide",
        "not",
        "and",
        "or",
        "equal",
        "not_equal",
        "greater",
        "greater_equal",
        "less",
        "less_equal",
        "jump",
        "jump_if_true",
        "jump_if_false",
        "end"
    };

    VirtualMachine vm;
    Scanner scanner(codeList, num_codes);
    vector<Instruction> instrList;
    string input;

#ifdef LOAD_FROM_CONSOLE

    cout << "Input script instructions:" << endl;
    while (input != "end")
    {
        getline(cin, input);
        if (scanner.ScanLine(input))
        {
            const CharArray& tokens = scanner.GetTokens();
            opcode code = static_cast<opcode>(tokens[0]);
            instrList.push_back(Instruction(code, &tokens[1], tokens.size()-1));
        }
        else
            cout << "Invalid Instruction!" << endl;
    }
    
    // as a safety precaution, it couldn't hurt to have redundant ends
    instrList.push_back(Instruction(op_end));

    // obtain a variable count
    size_t varCount;
    cout << "Input required variables count: ";
    cin >> varCount;
#endif

#ifdef LOAD_FROM_FILE
    // prompt user to enter a filename
    cout << "Enter path/name of script file: ";
    getline(cin, input);

    // attempt to open the file
    TextReaderFile file(input);
    if (file.Bad())
    {
        cout << "Could not open file!" << endl;
        return 0;
    }
    
    // read in file data
    string fileData;
    fileData.resize(file.Length());
    file.Read(fileData, fileData.length());
    
    size_t begin = 0, end = 0, lineNum = 1, varCount = NUM_VARIABLES;
    // feed data into scanner and build instructions
    while (end != string::npos)
    {
        // grab a line from the file data
        end = fileData.find_first_of("\n", begin);
        string line(fileData, begin, end-begin);
        begin = end+1;  // move past '\n' character

        // scan the line
        if (scanner.ScanLine(line))
        {
            const CharArray& tokens = scanner.GetTokens();
            opcode code = static_cast<opcode>(tokens[0]);
            instrList.push_back(Instruction(code, &tokens[1], tokens.size()-1));
        }
        else
        {
            cout << "Invalid Instruction!" << endl;
            cout << "Line number: " << lineNum << endl;
            cout << "Line: " << line << endl;
        }               
        ++lineNum;
    }
    
    // as a safety precaution, it couldn't hurt to have redundant ends
    instrList.push_back(Instruction(op_end));
#endif

    Script script(instrList, varCount);
    
    // load the script and save the id
    size_t scriptID = vm.Load(script);

    // execute script by its id
    cout << endl << "***EXECUTING SCRIPT***" << endl;
    vm.Execute(scriptID);
    // check out the variable states
    cout << endl << "***VARIABLE STATES***" << endl;
    vm.ShowVariableState();

    // pause for input to prevent app from closing before getting a look
    getline(cin, input);
    return 0;
}
