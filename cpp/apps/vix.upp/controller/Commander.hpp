#ifndef vix_controller_Commander_hpp
#define vix_controller_Commander_hpp

#include "vix/controller/Command.hpp"
#include "vix/controller/Instruction.hpp"
#include "vix/controller/StateMachine.hpp"
#include "vix/model/Selection.hpp"
#include "boost/signals2.hpp"
#include <deque>
#include <string>

namespace vix
{
    namespace controller
    {
        namespace command
        {
            class Open;
            class NewTab;
            class CloseTab;
        }

        class Commander: public MetaMachine
        {
            public:
                typedef MetaMachine Base;
                typedef boost::signals2::signal<void (int, const std::string *)> UpdateSignal;
                Commander(model::Selections &);

                //Each subscriber receives updates of the different fields:
                // * 0: Filter
                // * 1: Content
                // * 2: Command
                boost::signals2::connection connect(const UpdateSignal::slot_type &subscriber);

                void add(ICommand::Ptr);
                void add(Special);
                void add(char ch);
                void clear();

                void activate(Special);
                void move(Special);
                void removeSelected(Special);

                void changeTab(int ix);

                int currentMode() const;

            private:
                void nameFilterChanged_(const std::string *);
                void contentFilterChanged_(const std::string *);
                void commandChanged_(const std::string *);
                void update_();
                UpdateSignal updated_;

                void executeCommands_();

                friend class vix::controller::command::Open;
                friend class vix::controller::command::NewTab;
                friend class vix::controller::command::CloseTab;
                friend class vix::controller::command::ToggleRecursiveMode;
                friend class vix::controller::command::CreateFile;
                model::Selections &selections_;
                Instruction getInstruction_() const;

                typedef std::deque<ICommand::Ptr> Commands;
                Commands pendingCommands_;
                Commands executedCommands_;

                //The submachines
                void connect_(Control, const vix::EditableString::Slot &);
                NameFilterStateMachine nameFilter_;
                ContentFilterStateMachine contentFilter_;
                CommandStateMachine command_;
        };
    }
}

#endif
