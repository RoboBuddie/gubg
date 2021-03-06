#include "gubg/Platform.hpp"
#include "boost/filesystem.hpp"
#define GUBG_MODULE "Platform"
#define LOG_LEVEL Debug
#include "gubg/logging/Log.hpp"
#include <fstream>
#include "boost/thread/mutex.hpp"

#ifdef GUBG_LINUX
#include <stdlib.h>
#include <unistd.h>
#endif
#ifdef GUBG_WIN32
#include <unistd.h>
#endif

using namespace std;

namespace
{
    //4k is the intel page size
    const size_t MaxPath = 4096;
    char page[MaxPath];
    boost::mutex pageMutex;
}

namespace gubg
{
    bool spawn(const string &command)
    {
#ifdef GUBG_LINUX
        switch (fork())
        {
            case -1:
                //Something went wrong
                return false;
                break;
            case 0:
                //Child process
                execl("/bin/sh", "sh", "-c", command.c_str(), (char *)0);
                break;
            default:
                //Parent process, continue without waiting
                return true;
                break;
        }
#endif
        return false;
    }

    string escapeForCLI(const string &str, Quotes quotes)
    {
        string res;
        switch (quotes)
        {
            case Quotes::Add:
                res = "\"" + str + "\"";
                break;
        }
        return res;
    }

    bool createDirectory(const string &p)
    {
        return boost::filesystem::create_directory(p);
    }
    bool createRegular(const string &p)
    {
        std::ofstream of(p.c_str());
        bool ret = of.is_open();
        of.close();
        return ret;
    }
    bool deleteFile(const std::string &name, bool recursive)
    {
        LOG_S_(Debug, deleteFile, "Deleting " << name << " recursive: " << recursive);
        try
        {
            if (recursive)
                boost::filesystem::remove_all(name);
            else
                boost::filesystem::remove(name);
        }
        catch (boost::filesystem::filesystem_error &exc)
        {
            LOG_M_(Warning, "Failed to delete " << name << ": " << exc.what());
            return false;
        }
        return true;
    }

    string getCurrentWorkingDirectory()
    {
        boost::mutex::scoped_lock lock(pageMutex);
        page[0] = '\0';
        auto tmp = ::getcwd(page, MaxPath);
        return page;
    }
    string getHomeDirectory()
    {
        auto home = ::getenv("HOME");
        return (home ? home : "");
    }
}
