module ulbu.primitive;

import ulbu.element;

class Integer: Element
{
    char[] name(){return "int";}
    uint size(){return 4;}
    Element[] variables(){return [];}
}

version (Test)
{
    void main()
    {
        auto i = new Integer;
    }
}