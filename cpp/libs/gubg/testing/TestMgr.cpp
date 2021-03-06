#include "testing/TestMgr.hpp"
#include "OnlyOnce.hpp"
#include "boost/thread/tss.hpp"
#include "boost/thread/mutex.hpp"
#include <memory>
#include "nullptr.hpp"
using namespace gubg::testing;
using namespace std;
using namespace boost;

//#define L_ENABLE_DEBUG
#include "debug.hpp"

namespace
{
    unique_ptr<TestMaster> testMaster;
    mutex testMasterMutex;

    //This will hold the thread-specific root tag
    void doNothing(gubg::testing::TestTag*){}
    boost::thread_specific_ptr<TestTag> rootTag(doNothing);
}

//Statistics methods
Statistics::Statistics():
    nrOK(0),
    nrFailure(0){}
void Statistics::addResult(TestResult result)
{
    switch (result)
    {
        case TestResult::OK: ++nrOK; break;
        case TestResult::Failure: ++nrFailure; break;
    }
}
Statistics &Statistics::operator+=(const Statistics &rhs)
{
    nrFailure += rhs.nrFailure;
    nrOK += rhs.nrOK;
    return *this;
}
unsigned int Statistics::nrTotal() const
{
    return nrOK + nrFailure;
}

//TestTag methods
TestTag::TestTag(const string &tag):
    tag_(tag),
    root_(nullptr)
{
    DEBUG_PRINT("ctor " << tag_);
    if (nullptr == rootTag.get())
    {
        DEBUG_PRINT("We are the root tag");
        rootTag.reset(this);
        root_ = new ThreadStats;
    }
    else
    {
        root_ = rootTag.get()->root_;
    }
    root_->addTag(tag_);
}
TestTag::~TestTag()
{
    DEBUG_PRINT("dtor " << tag_);
    root_->popTag();
    if (this == rootTag.get())
    {
        DEBUG_PRINT("We were the root tag");
        TestMaster::instance().report(*(const ThreadStats *)root_);
        delete root_;
        rootTag.reset(nullptr);
    }
}
void TestTag::addResult(TestResult result)
{
    root_->addResult(result);
}
//TestTag::ThreadStats methods
TestTag::ThreadStats::ThreadStats():
    currentStats_(nullptr){}
void TestTag::ThreadStats::addTag(const std::string &tag)
{
    tags_.push_back(tag);
    currentStats_ = &statsPerTags_[tags_];
}
void TestTag::ThreadStats::popTag()
{
    tags_.pop_back();
    currentStats_ = (tags_.empty() ? nullptr : &statsPerTags_[tags_]);
}
void TestTag::ThreadStats::addResult(TestResult result)
{
    currentStats_->addResult(result);
}

//TestMaster methods
TestMaster::TestMaster():
    reportType_(defaultReportType){}
TestMaster::~TestMaster()
{
    cout << *this << endl;
}
TestMaster &TestMaster::instance()
{
    lock_guard<mutex> lock(testMasterMutex);
    if (!testMaster)
        testMaster.reset(new TestMaster);
    return *testMaster;
}
void TestMaster::report(const TestTag::ThreadStats &threadStats)
{
    DEBUG_PRINT("Someone is reporting thread statistics");
    lock_guard<mutex> lock(testMasterMutex);
    threadStatss_.push_back(threadStats);
}
std::ostream &operator<<(std::ostream &os, const TestTag::ThreadStats::Tags &tags)
{
    gubg::OnlyOnce skipDelimiter;
    for (auto it = tags.begin(); it != tags.end(); ++it)
    {
        const auto &v = *it;
        if (!skipDelimiter())
            os << "|";
        os << v;
    }
    return os;
}
std::ostream &operator<<(std::ostream &os, const Statistics &stats)
{
    if (stats.nrFailure > 0)
        os << stats.nrFailure << " out of " << stats.nrTotal() << " test(s) FAILED";
    else
        os << "All " << stats.nrTotal() << " test(s) SUCCEEDED";
    return os;
}
std::ostream &operator<<(std::ostream &os, const TestMaster &testMaster)
{
    os << "I received results from " << testMaster.threadStatss_.size() << " thread(s)" << endl;
    Statistics stats;
    for (auto it = testMaster.threadStatss_.begin(); it != testMaster.threadStatss_.end(); ++it)
    {
        const auto &threadStats = *it;
        for (auto it2 = threadStats.statsPerTags_.begin(); it2 != threadStats.statsPerTags_.end(); ++it2)
        {
            const auto &v = *it2;
            const TestTag::ThreadStats::Tags &t =  v.first;
            const Statistics &s = v.second;
            if (s.nrTotal() > 0)
            {
                switch (testMaster.reportType_)
                {
                    case TestMaster::ReportType::Full:
                        os << t << ": " << s << endl;
                        break;
                    case TestMaster::ReportType::OnlyErrors:
                        if (s.nrFailure > 0)
                            os << t << ": " << s << endl;
                        break;
                }
            }
            stats += s;
        }
    }
    os << "Global: " << stats;
    return os;
}

#ifdef UnitTest
int main()
{
    {
        TestTag root("root");
        TestTag level1("level1");
        level1.addResult(TestResult::OK);
        TestTag level2("level2");
        level2.addResult(TestResult::OK);
    }
    cout << TestMaster::instance() << endl;
    {
        TestTag root("root");
        TestTag level1("level1");
        level1.addResult(TestResult::Failure);
        TestTag level2("level2");
        level2.addResult(TestResult::OK);
        level2.addResult(TestResult::Failure);
    }
    cout << TestMaster::instance() << endl;
}
#endif
