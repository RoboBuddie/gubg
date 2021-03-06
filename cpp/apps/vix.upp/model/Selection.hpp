#ifndef vix_model_Selection_hpp
#define vix_model_Selection_hpp

#include "vix/model/FileSystem.hpp"
#include "gubg/threading/Queue.hpp"
#include "boost/regex.hpp"
#include "boost/signals2.hpp"
#include "boost/thread/mutex.hpp"
#include <vector>
#include <string>

namespace vix
{
    namespace model
    {
        enum class Action {View, Edit, Open, Delete, Copy, Move};
        enum class Activation {Error, Directory, Regular, SomethingElse};
        enum class ChangedItem
        {
            Nothing = 0x00,
            Selections = 0x01,
            Files = 0x02,
            Preview = 0x04,
        };

        class Selection;

        class Selections
        {
            public:
                typedef boost::mutex::scoped_lock Lock;
                Lock lock() const {return Lock(mutex_);}

                Selections();
                virtual ~Selections();

                typedef boost::signals2::signal<void (ChangedItem)> UpdateSignal;
                boost::signals2::connection connect(const UpdateSignal::slot_type &subscriber);

                bool empty() const;
                Selection *current() const;
                int currentIX() const;
                int nrModels() const;
                void setCurrent(int ix);
                void addSelection(const std::string &path);
                void deleteSelection(int ix);

                void fileSystemChanged();

            private:
                friend class Selection;
                UpdateSignal updated_;
                typedef std::vector<Selection*> Selections_;
                Selections_ selections_;
                int current_;
                mutable boost::mutex mutex_;
        };

        enum Format {Unknown, Html};
        class Selection
        {
            private:
                friend class Selections;
                Selection(Selections &selections, const std::string &path);

            public:
                ~Selection();
                Path path() const {return path_;}
                File selectedFile() const;
                void setPath(Path);
                void setSelected(const std::string &selected);
                std::string getSelected() const;

                std::vector<Selection*> selections();

                static const int InvalidIX = -2;
                void getFiles(Files &, int &selectedIX) const;
                Activation activateSelected(Action);
                enum class Direction {Up, Down};
                void move(Direction);
                void refresh();

                void setNameFilter(const std::string &);
                std::string getNameFilter() const;

                void setContentFilter(const std::string &);
                std::string getContentFilter() const;

                void setRecursiveMode(bool);
                bool getRecursiveMode() const;

                bool getContent(std::string &content, Format);

            private:
                Selections &selections_;

                //This is the primary data for the set of files_
                Path path_;
                std::string nameFilter_;
                typedef boost::scoped_ptr<boost::regex> Regex;
                Regex reNameFilter_;
                std::string contentFilter_;
                Regex reContentFilter_;
                Files files_;
                bool recursive_;
                mutable boost::mutex filesMutex_;
                void updateFiles_();

                //We first try to match selected_. If that fails, we take something as close as possible to selectedIX_
                std::string selected_;
                int selectedIX_;
                //Keeps track the last selected per path, when the same path is revisited later, the same selected is preferred
                std::map<std::string, std::string> selectedPerPath_;
                mutable boost::mutex selectedMutex_;
                void updateSelected_();

                boost::shared_ptr<std::string> content_;
                Format format_;
                mutable boost::mutex contentMutex_;
                void prepareContent_(gubg::file::Regular, Format, boost::weak_ptr<std::string> wp);

                //All updating of internal data goes via a message queue
                struct Message
                {
                    typedef std::unique_ptr<Message> Ptr;
                    //Only the fields that have a value set are of course valid
                    Path path;
                    std::unique_ptr<std::string> nameFilter;
                    std::unique_ptr<std::string> contentFilter;
                    std::unique_ptr<std::string> selected;
                    std::unique_ptr<bool> recursive;
                    std::unique_ptr<Direction> direction;
                    std::unique_ptr<bool> refresh;
                };
                typedef gubg::threading::Queue<Message> QueueT;
                QueueT queue_;
                void consumer_();
                boost::thread consumerThread_;
        };
    }
}
inline vix::model::ChangedItem operator|(vix::model::ChangedItem lhs, vix::model::ChangedItem rhs)
{
    return static_cast<vix::model::ChangedItem>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
}
inline vix::model::ChangedItem operator&(vix::model::ChangedItem lhs, vix::model::ChangedItem rhs)
{
    return static_cast<vix::model::ChangedItem>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
}
inline vix::model::ChangedItem &operator|=(vix::model::ChangedItem &lhs, vix::model::ChangedItem rhs)
{
    lhs = static_cast<vix::model::ChangedItem>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
    return lhs;
}
inline std::ostream &operator<<(std::ostream &os, vix::model::ChangedItem ci)
{
    return os << std::hex << static_cast<unsigned int>(ci);
}

#endif
