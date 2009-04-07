module gubg.patterns.composite;
// The composite pattern

public import tango.core.Exception;

// Interfaces
enum ReplaceMode
{
    Create,
        Remove,
        Set,
	Get,
}
interface IComponent(I): I
{
    uint nrComponents();
    void setNrComponents(uint nr);
    IComponent!(I) replaceComponent(ReplaceMode mode, uint ix, IComponent!(I) newComponent);
    IComposite!(I) parent();
    IComposite!(I) parent(IComposite!(I) p);
    bool isLeaf();
}
interface IComposite(I): IComponent!(I)
{
}
interface ILeaf(I): IComponent!(I)
{
}

// Templates
// Provide parent()
template TParent(I)
{
    // NOTE::Do not forget to set the parent in replaceComponent()
    IComposite!(I) parent(){return mParent;}
    IComposite!(I) parent(IComposite!(I) p)
        {
            mParent = p;
            return mParent;
        }
private:
    IComposite!(I) mParent;
}
// Composite
template TComposite(I)
{
    mixin TParent!(I);
    bool isLeaf(){return false;}
}
// Provide replaceComponent() and parent() for classes that specify:
//  * opIndex and opIndexAssign
//  * nrComponents and setNrComponents
template TIndexComposite(I)
{
    IComponent!(I) replaceComponent(ReplaceMode mode, uint ix, IComponent!(I) newComponent)
        {
            IComponent!(I) origComponent = null;
            
            uint nrComp = nrComponents;

            switch (mode)
            {
            case ReplaceMode.Create:
                if (ix >= nrComp)
                {
                    nrComp = ix + 1;
                    setNrComponents(nrComp);
                }
                // We fall-through to ReplaceMode.Set
            case ReplaceMode.Set:
                origComponent = this[ix];
                if (origComponent !is null)
                    origComponent.parent = null;
                if (newComponent !is null)
                    newComponent.parent(this);
                this[ix] = newComponent;
                break;

            case ReplaceMode.Get:
                origComponent = this[ix];
                break;

            case ReplaceMode.Remove:
                origComponent = this[ix];
                if (origComponent !is null)
                    origComponent.parent = null;
                for (uint i = ix + 1; ix < nrComp; ++ix)
                    this[i-1] = this[i];
                --nrComp;
                this[nrComp] = null;
                setNrComponents(nrComp);
                break;
            }

            return origComponent;
        }
    mixin TComposite!(I);
}
// Provide parent() and disabled replaceComponent() functionality
template TLeaf(I)
{
    uint nrComponents(){return 0;}
    void setNrComponents(uint nr){throw new ArrayBoundsException(__FILE__, __LINE__);}
    IComponent!(I) replaceComponent(ReplaceMode mode, uint ix, IComponent!(I) newComponent)
        {
            throw new ArrayBoundsException(__FILE__, __LINE__);
            return null;
        }
    bool isLeaf(){return true;}
    mixin TParent!(I);
}


version (UnitTest)
{
    // We want all our components to provide draw()
    interface ComponentMethods
    {
        void draw();
    }
    // Component is our ComponentMethods interface, extended with the typical component methods
    alias IComponent!(ComponentMethods) Component;

    // A leaf component
    class L: ILeaf!(ComponentMethods)
    {
        mixin TLeaf!(ComponentMethods);

	// ComponentMethods
        void draw(){}
    }

    // A general composite component
    class C: IComposite!(ComponentMethods)
    {
        uint nrComponents(){return 0;}
        void setNrComponents(uint nr){}
        Component replaceComponent(ReplaceMode mode, uint ix, Component newComponent)
        {
            throw new ArrayBoundsException(__FILE__, __LINE__);
            return null;
        }
        mixin TComposite!(ComponentMethods);

	// ComponentMethods
        void draw(){}
    }

    // A composite component which provides opIndex and opIndexAssign
    class IC: IComposite!(ComponentMethods)
    {
        uint nrComponents(){return mArray.length;}
        void setNrComponents(uint nr){mArray.length = nr;}
        Component opIndex(uint ix){return mArray[ix];}
        Component opIndexAssign(Component rhs, uint ix){return (mArray[ix] = rhs);}
        mixin TIndexComposite!(ComponentMethods);

	// ComponentMethods
        void draw(){}
    private:
        Component[] mArray;
    }

    void main()
    {
        auto l = new L();
        auto c = new C();
        auto ic = new IC();
	ic.replaceComponent(ReplaceMode.Create, 1, l);
	ic.replaceComponent(ReplaceMode.Set, 0, c);
    }
}
