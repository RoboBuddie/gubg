#include "vix/controller/Instruction.hpp"
#define GUBG_MODULE "Instruction"
#define LOG_LEVEL Warning
#include "gubg/logging/Log.hpp"
using namespace std;

namespace vix
{
    Instruction::Instruction(const string &instruction)
    {
        LOG_SM_(Debug, ctor, "instruction: " << instruction);
        parseInstruction_(instruction);
    }

    bool Instruction::isValid() const
    {
        return !command_.empty();
    }

    //Private methods
    void Instruction::parseInstruction_(const string &instruction)
    {
        if (parseCommand_("q", instruction)) return;
        if (parseCommand_("t", instruction)) return;
        if (parseCommand_("r", instruction)) return;
        if (parseCommand_("n", instruction)) return;
    }
    bool Instruction::parseCommand_(const string &command, const string &instruction)
    {
        LOG_SM_(Debug, parseCommand_, "command: " << command);
        if (instruction.length() < command.length())
            return false;
        if (instruction.substr(0, command.length()) != command)
            return false;
        command_ = instruction.substr(0, command.length());
        options_ = instruction.substr(0+command.length());
        LOG_M_(Debug, "Successfully parsed command " << command);
        return true;
    }
}
