#ifndef gubg_Platform_hpp
#define gubg_Platform_hpp

#include <string>
#include <sstream>

#ifdef __linux
#define GUBG_LINUX
#endif
#ifdef _WIN32
#define GUBG_WIN32
#endif

#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

namespace gubg
{
    bool spawn(const std::string &command);

    template <typename FirstOption, typename ...OptionsRest>
    bool spawn(const std::string &command, FirstOption firstOption, OptionsRest... optionsRest)
    {
        std::ostringstream oss;
        oss << command << " " << firstOption;
        return spawn(oss.str(), optionsRest...);
    }

    //Basic command-line escaping
    enum class Quotes {Add};
    std::string escapeForCLI(const std::string &str, Quotes quotes);

    bool createDirectory(const std::string &name);
    bool createRegular(const std::string &name);
    //Can delete both directories and files
    bool deleteFile(const std::string &name, bool recursive = false);
}

#endif