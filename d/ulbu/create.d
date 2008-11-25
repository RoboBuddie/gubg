module ulbu.create;

template Create(TS)
{
    static char[] indent()
        {
            char[] id;
            id.length = 2*level;
            foreach (ix, inout ch; id)
                ch = (ix%2 == 0 ? '|' : ' ');
            return id;
        }

    static void fail(TS ts)
        {
            puts("Failing on the following code:");
            for (uint i = 0; i < 10 && !ts.empty; ++i, ts.pop)
                puts("Element = {}", ts.peep.str);
            throw new Exception("Mmh, some unknown things that start with the above.");
        }

    static char[] createString(TS ts)
        {
            char[] res;
            scope sp = ts.savePoint;
            for (uint i = 0; i < 10 && !ts.empty; ++i, ts.pop)
            {
                if (i > 0)
                    res ~= " ";
                res ~= ts.peep.str;
            }
            sp.restore;
            return res;
        }

    // Create a node based on TS input
    static typeof(this) create(inout TS ts)
	{
	    static TS.TokenT head;

	    ++level;
	    scope (exit) --level;
	    puts("{}Trying {} \"{}\"", indent, this.stringof, createString(ts));
		
	    typeof(this) res;

	    if (head !is ts.peep)
	    {
		head = ts.peep;

		scope sp = ts.savePoint;
		    
		typeof(this).createI(res, ts);
		    
		if (res is null)
		    sp.restore;
		else
		    puts("{}OK {}", indent, this.stringof);
		head = null;
	    }
		
	    return res;
	}
}
