#include "gubg/testing/Testing.hpp"
#include "gubg/tmp/If.hpp"
#include "gubg/l.hpp"
#include <string>
using namespace gubg::tmp;

struct One {enum: int {Value = 1};};
struct Two {enum: int {Value = 2};};

int main()
{
    TEST_TAG(If);
    TEST_EQ(1, (If<true, One, Two>::Type::Value));
    TEST_EQ(2, (If<false, One, Two>::Type::Value));
    return 0;
}
