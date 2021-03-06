#include "da/Codes.hpp"
#include "da/Tasks.hpp"
#include "da/Finalize.hpp"
#include "gubg/OptionParser.hpp"
using namespace da;
using gubg::OptionParser;
using namespace std;

namespace 
{
    void addFixTask(Tasks &tasks, const string &name)
    {
        if (name == "guards")
            tasks.push_back(FixIncludeGuards::create());
        else
            DA_FINALIZE_ERROR("Unknown fix task " << STREAM(name));
    }

    ReturnCode main_(int argc, char **argv)
    {
        MSS_BEGIN(ReturnCode);

        Tasks tasks;
        Options options;
        {
            OptionParser optionParser("Develop assistent");
            optionParser.addSwitch("-h", "--help", "Displays this help", [&optionParser](){DA_FINALIZE_OK(optionParser.help());});
            optionParser.addMandatory("-f", "--fix TYPE", "Fix something (e.g., guards)", [&tasks](string v){addFixTask(tasks, v);});
            optionParser.addSwitch("-r", "--real", "Fix for real", [&options](){options.doFix = true;});
            optionParser.addMandatory("-e", "--exe SOURCE", "Compile SOURCE into an executable", [&tasks](string source){tasks.push_back(CompileExe::create(source));});

            OptionParser::Args args;
            MSS(OptionParser::createArgs(args, argc, argv));
            MSS(optionParser.parse(args));
        }

        for (auto task: tasks)
            MSS(task->execute(options));

        MSS_END();
    }
}

int main(int argc, char **argv)
{
    if (!gubg::mss::isOK(main_(argc, argv)))
        DA_FINALIZE_ERROR("Problems were encountered...");
    DA_FINALIZE_OK("Everything went OK");
    return 0;
}
