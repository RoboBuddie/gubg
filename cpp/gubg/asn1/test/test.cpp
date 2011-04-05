#include "asn1/Decoder.hpp"
#include "asn1/Encoder.hpp"
#include "Testing.hpp"
#include <iostream>
using namespace gubg;
using namespace std;

//#define L_ENABLE_DEBUG
#include "debug.hpp"

int main(int argc, char **argv)
{
    TEST_TAG(asn1);
    asn1::Encoder encoder;
    {
        TEST_TAG(encoder);

        encoder.clear();
        encoder.append(0);
        TEST_EQ(string({0x02, 0x01, 0}), encoder.encode());

        encoder.clear();
        encoder.append(123);
        TEST_EQ(string({0x02, 0x01, 123}), encoder.encode());

        encoder.clear();
        encoder.append(0x1234);
        TEST_EQ(string({0x02, 0x02, 0x12, 0x34}), encoder.encode());

        encoder.clear();
        encoder.append("12345", asn1::StringType::Octet);
        TEST_EQ(string({0x04, 0x05, 0x31, 0x32, 0x33, 0x34, 0x35}), encoder.encode());

        encoder.clear();
        encoder.append("12345", asn1::StringType::IA5);
        TEST_EQ(string({0x16, 0x05, 0x31, 0x32, 0x33, 0x34, 0x35}), encoder.encode());
    }

    asn1::Decoder decoder;
    {
        TEST_TAG(integer);
        int i;
        {
            TEST_TAG(success);

            //Read 0
            decoder.reset({0x02, 0x00});
            TEST_TRUE(decoder.extract(i));
            TEST_EQ(0, i);

            //Read 34, single byte
            decoder.reset({0x02, 0x01, 34});
            TEST_TRUE(decoder.extract(i));
            TEST_EQ(34, i);

            //Read -128, single byte but negative
            decoder.reset({0x02, 0x01, -128});
            TEST_TRUE(decoder.extract(i));
            TEST_EQ(-128, i);

            //Read 4-byte integer, positive
            decoder.reset({0x02, 0x04, 0x12, 0x34, 0x56, 0x78});
            TEST_TRUE(decoder.extract(i));
            TEST_EQ(0x12345678, i);

            //Read 4-byte integer, negative
            decoder.reset({0x02, 0x02, -128, 0x00});
            TEST_TRUE(decoder.extract(i));
            TEST_EQ(-32768, i);

            //Read 2 4-byte integers
            decoder.reset({0x02, 0x04, 0x12, 0x34, 0x56, 0x78,    0x02, 0x04, 0x12, 0x34, 0x56, 0x79});
            TEST_TRUE(decoder.extract(i));
            TEST_EQ(0x12345678, i);
            TEST_TRUE(decoder.extract(i));
            TEST_EQ(0x12345679, i);
        }
        {
            TEST_TAG(failure);

            //No length
            decoder.reset({0x02});
            TEST_FALSE(decoder.extract(i));

            //Not enough content
            decoder.reset({0x02, 0x01});
            TEST_FALSE(decoder.extract(i));

            //Not an integer
            decoder.reset({0x03, 0x01, 34});
            TEST_FALSE(decoder.extract(i));

            //Integer to long to fit in 4 bytes
            decoder.reset({0x02, 0x05, 1, 2, 3, 4, 5});
            TEST_FALSE(decoder.extract(i));
        }
    }

    {
        TEST_TAG(string);
        string str;
        {
            TEST_TAG(success);

            //Read empty octet string
            decoder.reset({0x04, 0x00});
            TEST_TRUE(decoder.extract(str));
            TEST_EQ("", str);

            //Read 5-byte octet string
            decoder.reset({0x04, 0x05, 0x31, 0x32, 0x33, 0x34, 0x35});
            TEST_TRUE(decoder.extract(str));
            TEST_EQ("12345", str);

            //Read 5-byte IA5String
            decoder.reset({0x16, 0x05, 0x31, 0x32, 0x33, 0x34, 0x35});
            TEST_TRUE(decoder.extract(str));
            TEST_EQ("12345", str);

            //Read 5-byte zero-filled octet string
            decoder.reset({0x04, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00});
            TEST_TRUE(decoder.extract(str));
            string wanted = {0, 0, 0, 0, 0};
            TEST_EQ(wanted, str);

            //Read 2 octet strings
            decoder.reset({0x04, 0x05, 0x31, 0x32, 0x33, 0x34, 0x35,         0x04, 0x04, 0x34, 0x33, 0x32, 0x31});
            TEST_TRUE(decoder.extract(str));
            TEST_EQ("12345", str);
            TEST_TRUE(decoder.extract(str));
            TEST_EQ("4321", str);
        }
        {
            TEST_TAG(failure);

            //Not an octet string nor IA5String
            decoder.reset({0x02, 0x01, 34});
            TEST_FALSE(decoder.extract(str));
        }
    }

    {
        TEST_TAG(sequence_simple);
        vector<int> vec;

        {
            TEST_TAG(success);

            //Read sequence of 3 integers
            decoder.reset({0x30, 0x09,   0x02, 0x01, 1,   0x02, 0x01, 2,   0x02, 0x01, 3});
            TEST_TRUE(decoder.extract(vec));
            vector<int> wanted = {1, 2, 3};
            TEST_EQ(wanted, vec);
        }
        {
            TEST_TAG(failure);

            //Not all integers
            decoder.reset({0x30, 0x09,   0x02, 0x01, 1,   0x02, 0x01, 2,   0x04, 0x01, 3});
            TEST_FALSE(decoder.extract(vec));
        }
    }

    {
        TEST_TAG(subdecoder);

        //A sequence of 2 int (1, 2), followed by another int (99)
        const string der = {0x30, 0x06,   0x02, 0x01, 1,   0x02, 0x01, 2,         0x02, 0x01, 99};
        int i;
        {
            TEST_TAG(success);

            decoder.reset(der);
            {
                //We read the first 2 ints using a subdecoder
                asn1::Decoder subdecoder;
                TEST_TRUE(decoder.extract(subdecoder));
                TEST_TRUE(subdecoder.extract(i));
                TEST_EQ(1, i);
                TEST_TRUE(subdecoder.extract(i));
                TEST_EQ(2, i);
                TEST_TRUE(subdecoder.empty());
                subdecoder.indicateDecodingOK();
            }
            //We read the last int using the toplevel decoder
            TEST_TRUE(decoder.extract(i));
            TEST_EQ(99, i);
            TEST_TRUE(decoder.empty());
        }
        {
            TEST_TAG(failure);

            //We do the same as above, but try to make as many mistakes as possible
            decoder.reset(der);
            string str;
            {
                //We read the first 2 ints using a subdecoder
                asn1::Decoder subdecoder;
                TEST_FALSE(decoder.extract(i));
                TEST_TRUE(decoder.extract(subdecoder));
                TEST_FALSE(subdecoder.empty());
                TEST_FALSE(subdecoder.extract(str));
                TEST_TRUE(subdecoder.extract(i));
                TEST_FALSE(subdecoder.empty());
                TEST_EQ(1, i);
                TEST_FALSE(subdecoder.extract(str));
                TEST_TRUE(subdecoder.extract(i));
                TEST_EQ(2, i);
                TEST_FALSE(subdecoder.extract(str));
                TEST_FALSE(subdecoder.extract(i));
                TEST_TRUE(subdecoder.empty());
                TEST_THROW(asn1::Decoder::Blocked, decoder.extract(i));
                subdecoder.indicateDecodingOK();
            }
            //We read the last int using the toplevel decoder
            TEST_FALSE(decoder.extract(str));
            TEST_FALSE(decoder.empty());
            TEST_TRUE(decoder.extract(i));
            TEST_EQ(99, i);
            TEST_TRUE(decoder.empty());
        }
    }

    {
        TEST_TAG(general);

        struct S
        {
            int i;
            string str;

            bool operator==(const S &rhs) const
            {
                return rhs.i == i && rhs.str == str;
            }

            private:
            friend class asn1::Decoder;
            bool construct(asn1::Decoder &decoder)
            {
                return decoder.extract(i) && decoder.extract(str);
            }
        };
        S s;
        vector<S> vec;

        {
            TEST_TAG(success_single);

            //Read a single S from an integer and a string
            decoder.reset({0x02, 0x01, 34,    0x04, 0x05, 0x31, 0x32, 0x33, 0x34, 0x35});
            TEST_TRUE(decoder.extract(s));
            TEST_EQ(34, s.i);
            TEST_EQ("12345", s.str);
        }
        {
            TEST_TAG(success_sequence);

            //Read a vector of S from a sequence of alternating integers and strings
            decoder.reset({0x30, 0x0c,   0x02, 0x01, 1,  0x04, 0x01, 0x31,   0x02, 0x01, 2,  0x04, 0x01, 0x32});
            TEST_TRUE(decoder.extract(vec));
            vector<S> wanted = {{1, "1"}, {2, "2"}};
            TEST_EQ(wanted, vec);
        }
        {
            TEST_TAG(failure);

            //Cannot construct S from 2 integers
            decoder.reset({0x02, 0x01, 34,    0x02, 0x01, 35});
            TEST_FALSE(decoder.extract(s));
            //Check that decoder remained unchanged and that we still can read the 2 integer directly
            int i;
            TEST_TRUE(decoder.extract(i));
            TEST_EQ(34, i);
            TEST_TRUE(decoder.extract(i));
            TEST_EQ(35, i);
        }
    }
    return 0;
}
