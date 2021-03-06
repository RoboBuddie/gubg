#include "gubg/file/basic.hpp"
#include "gubg/parse/basic.hpp"
#include "gubg/Platform.hpp"
#ifdef GUBG_LINUX
#include <dirent.h>
#endif
using namespace std;

#define LOG(msg) cout << msg << endl

namespace
{
    typedef gubg::file::basic::ReturnCode RC;
    template <typename Parts>
        RC tokenizeAbsolutePath(Parts &parts, const string &path)
        {
            MSS_BEGIN(RC);
            MSS(!path.empty(), PathIsEmpty);
            parts = gubg::parse::tokenize(path, "/");
            MSS(!parts.empty(), PartsIsEmpty);
            MSS(parts[0] == "", PathIsNotAbsolute);
            parts.pop_front();
            MSS_END();
        }
}

gubg::file::basic::ReturnCode gubg::file::basic::expandPath(string &pathE, const string &path)
{
    MSS_BEGIN(ReturnCode);

    //Split path into a vector of parts
    auto parts = gubg::parse::tokenize(path, "/");
    MSS(!parts.empty(), InvalidPath);

    //We construct the expanded path by gradually changing the working directory based on parts from path
    deque<string> partsE;
    MSS(tokenizeAbsolutePath(partsE, getCurrentWorkingDirectory()));
    bool firstTime = true;
    while (!parts.empty())
    {
        auto head = parts[0];
        if (head == "")
        {
            if (firstTime)
                partsE.clear();
            //We ignore "//" or trailing "/" that are not leading
        }
        else if (head == ".")
        {
            //Nothing to be done, we started from the current dir and "." means no change
        }
        else if (head == "..")
        {
            //Go up one dir, we don't fail if partsE is empty (alread at root)
            if (!partsE.empty())
                partsE.pop_back();
        }
        else if (head == "~")
        {
            if (firstTime)
            {
                //We change to the $HOME directory
                MSS(tokenizeAbsolutePath(partsE, getHomeDirectory()));
            }
        }
        else
            partsE.push_back(head);

        parts.pop_front();
        firstTime = false;
    }

    //Convert the parts into an absolute path
    {
        ostringstream oss;
        if (partsE.empty())
            oss << "/";
        else
            for (auto partE = partsE.begin(); partE != partsE.end(); ++partE)
                oss << "/" << *partE;
        pathE = oss.str();
    }

    MSS_END();
}

namespace 
{
    struct Dir
    {
        DIR *h;
        Dir(DIR *d):h(d){}
        ~Dir()
        {
            if (h)
                ::closedir(h);
        }
    };
}
gubg::file::basic::ReturnCode gubg::file::basic::getDirectoryContent(Directories &dirs, Files &files, const string &path)
{
    MSS_BEGIN(ReturnCode);
#ifdef GUBG_LINUX
    //Expand the provided path
    string pathE;
    MSS(expandPath(pathE, path));
    //Open the directory
    Dir dir(::opendir(file.name().c_str()));
    MSS(dir.h, CouldNotOpenDir);
    //Create enough space for the dirent struct
    string entrypBuffer(offsetof(struct dirent, d_name)+NAME_MAX+1, '0');
    struct dirent *entryp = (struct dirent *)entrypBuffer.data();
    //Iterate over the directory entries
    struct dirent *tmp;
    for (;;)
    {
        MSS(0 == ::readdir_r(dir.h, entryp, &tmp), CouldNotReadEntry);
        if (!tmp)
            //If this is set to 0, we are done with iterating
            break;
        if (::strcmp(entryp->d_name, ".") == 0 || ::strcmp(entryp->d_name, "..") == 0)
            //We skip "." and ".."
            continue;
        switch (entryp->d_type)
        {
            case DT_DIR: dirs.push_back(entryp->d_name); break;
            case DT_REG: files.push_back(entryp->d_name); break;
        }
    }
#else
    MSS_L(NotImplemented);
#endif
    MSS_END();
}
