#include "pointer/Locked.hpp"
#include "testing/Testing.hpp"
#include "threading/InstanceCounter.hpp"
#include "sleep/sleep.hpp"
#include "boost/thread.hpp"
#define GUBG_LOG
#include "logging/Log.hpp"
using namespace gubg::pointer;

namespace
{
    class Data
    {
        public:
            typedef boost::shared_ptr<Data> Ptr;

            Data()
            {
                LOG_SM(Data, "constructor");
                value_ = 0;
            }
            virtual ~Data()
            {
                LOG_SM(Data, "destructor, value_: " << value_);
            }

            void increase(){++value_;}

            static int value_;
    };
    int Data::value_ = 0;
    class Derived: public Data
    {
        public:
            Derived()
            {
                LOG_SM(Derived, "constructor");
            }
            virtual ~Derived()
            {
                LOG_SM(Derived, "destructor");
            }
    };
    typedef Locked<Data, ThreadSafeInstance> IDataPtr;
    typedef Locked<Data, ThreadSafeType> TDataPtr;
    typedef Locked<Data, ThreadSafeBaseType, Data> TBDataPtr;
    typedef Locked<Derived, ThreadSafeBaseType, Data> TDDataPtr;

    //Increments value_ in a way that would fail if performed multi-threaded without proper locking
    template <typename IDataPtr>
    struct IncrementThread: gubg::InstanceCounter<IncrementThread<IDataPtr>>
    {
        IncrementThread(IDataPtr data):
            data_(data),
            thread_(boost::ref(*this)){}
        void operator()()
        {
            typename IDataPtr::Unlock unlock(data_);
            int v = unlock->value_;
            gubg::nanosleep(0, 100000000);
            unlock->value_ = v+1;
            delete this;
        }
        IDataPtr data_;
        boost::thread thread_;
    };
}

int main()
{
    TEST_TAG(main);
    LOG_S(main);
    {
        LOG_SM(constructor, "Constructor");
        {
            LOG_SM(RawPointer, "RawPointer");
            IDataPtr data(new Data);
            TEST_TRUE(data());
        }
        {
            LOG_SM(SharedPointer, "SharedPointer");
            Data::Ptr dataPtr(new Data);
            IDataPtr data(dataPtr);
            TEST_TRUE(data());
        }
    }
    {
        LOG_SM(Unlock, "Unlock");
        IDataPtr data(new Data);
        data->increase();
        IDataPtr::Unlock unlock(data);
        ++unlock->value_;
        {
            IDataPtr::Unlock unlock2(data);
            ++unlock2->value_;
        }
    }
    {
        LOG_SM(MultiThreaded, "MultiThreaded");
        const int NrThreads = 10;
        IDataPtr data(new Data);
        for (int i = 0; i < NrThreads; ++i)
            new IncrementThread<IDataPtr>(data);
        while (IncrementThread<IDataPtr>::nrInstances() > 0)
            gubg::nanosleep(0, 100000000);
        IDataPtr::Unlock unlock(data);
        TEST_EQ(NrThreads, unlock->value_);
    }
    {
        LOG_SM(Singleton, "Mutex per instance");
        IDataPtr data1(new Data);
        IDataPtr data2(new Data);
        new IncrementThread<IDataPtr>(data1);
        new IncrementThread<IDataPtr>(data2);
        while (IncrementThread<IDataPtr>::nrInstances() > 0)
            gubg::nanosleep(0, 100000000);
    }
    {
        LOG_SM(Singleton, "Mutex per type");
        TDataPtr data1(new Data);
        TDataPtr data2(new Data);
        new IncrementThread<TDataPtr>(data1);
        new IncrementThread<TDataPtr>(data2);
        while (IncrementThread<TDataPtr>::nrInstances() > 0)
            gubg::nanosleep(0, 100000000);
    }
    {
        LOG_SM(Singleton, "Mutex per base type");
        TBDataPtr base(new Data);
        TDDataPtr derived(new Derived);
        new IncrementThread<TBDataPtr>(base);
        new IncrementThread<TDDataPtr>(derived);
        while (IncrementThread<TBDataPtr>::nrInstances() > 0)
            gubg::nanosleep(0, 100000000);
        while (IncrementThread<TDDataPtr>::nrInstances() > 0)
            gubg::nanosleep(0, 100000000);
    }

    return 0;
}