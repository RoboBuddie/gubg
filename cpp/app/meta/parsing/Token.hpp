#ifndef meta_parsing_Token_hpp
#define meta_parsing_Token_hpp

#include "parsing/Code.hpp"
#include "parsing/Component.hpp"

namespace meta
{
    class Token;
    typedef std::vector<Token*> Tokens;
    typedef boost::iterator_range<Tokens::iterator> TokenRange;

    //A Token is the leaf Component
    struct Token: Component
    {
        Token(const CodeRange &range):
            range_(range){}

        virtual bool isEnd(){return false;}
        virtual bool isSymbol(const char wanted){return false;}
        virtual bool isSymbol(char &actual, const std::string &symbols){return false;}
        bool isNewline(){return isSymbol('\n');}
        virtual bool isName(const std::string &){return false;}
        virtual bool isWhitespace() { return false; }

        static Token *tryCreate(CodeRange &range);

        CodeRange range_;
    };
    struct End: Token
    {
        End(const CodeRange &range):
            Token(range){}
        virtual bool isEnd(){return true;}
    };
    struct Symbol: Token
    {
        Symbol(const CodeRange &range):
            Token(range){}
        
        virtual bool isSymbol(const char wanted)
        {
            return wanted == range_.front();
        }
        virtual bool isSymbol(char &actual, const std::string &symbols)
        {
            size_t pos = symbols.find(range_.front());
            if (std::string::npos == pos)
                return false;
            actual = symbols[pos];
            return true;
        }
    };
    struct Newline: Symbol
    {
        Newline(const CodeRange &range):
            Symbol(range){}
    };
    struct Name: Token
    {
        Name(const CodeRange &range):
            Token(range){}

        virtual bool isName(const std::string &str)
        {
            return str == toCode(range_);
        }
    };
    struct Number: Token
    {
        Number(const CodeRange &range):
            Token(range){}
    };
    struct Whitespace: Token
    {
        Whitespace(const CodeRange &range):
            Token(range){}

        virtual bool isWhitespace() { return true; }
    };
}

#endif
