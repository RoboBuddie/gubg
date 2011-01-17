#ifndef visualization_h
#define visualization_h

#include <iostream>
#include <vector>
#include <ctime>
#include <sys/time.h> 

#include "canvas.hpp"
#include "drawable.hpp"
#include "thread.hpp"

using namespace std;

namespace gubg
{
    class Visualization: public Thread
    {
    private:
        // This is the function that is called before drawing each frame and should be overloaded to insert your own specific dynamics. Return false if you want to exit.
        virtual bool dynamics(){cout << "This is the base Visualization::dynamics(), please overload me" << endl; return true;}
        // This function is called when start is issued, and allows you to set things last-minute, like time (mCurrentTick is valid)
        virtual bool onStart(){cout << "This is the base Visualization::onStart(), please overload me" << endl; return true;}

        class Drawables
        {
        public:
            Drawables():
                mFirstIsCurrent(true){}
            void flip(){mFirstIsCurrent = !mFirstIsCurrent;}
            vector<Drawable*> &getCurrent(){return (mFirstIsCurrent ? mFirst : mSecond);}
            vector<Drawable*> &get(bool current){return (current == mFirstIsCurrent ? mFirst : mSecond);}

            vector<Drawable*> mFirst;
            vector<Drawable*> mSecond;
            bool mFirstIsCurrent;
        };

    public:
        Visualization(int width, int height, bool bShow = true, unsigned int bgColor = 0x123456):
            _canvas(width,height),
            _show(bShow),
            _quit(true),
            _backgroundColor(bgColor),
            _drawablesM(&_drawables){};
        ~Visualization()
            {
                finishThread();
            }

        int createBasis(double xO, double yO, double xW, double yW){return _canvas.createBasis(xO,yO,xW,yW);}
        bool addDrawable(int basis, Drawable *drawable, bool current = true)
            {
                if (_show)
                {
                    drawable->setBasis(basis);
                    _drawablesM.lock();
                    _drawables.get(current).push_back(drawable);
                    _drawablesM.unlock();
                }
                return true;
            }

        void initialize();
        void start();
        void handleInput();
        void draw();
        void flip()
            {
                if (_show)
                {
                    _drawablesM.lock();
                    _drawables.flip();
                    _drawablesM.unlock();
                }
            }
        int width(){return _canvas.width();}
        int height(){return _canvas.height();}
        bool clear(bool deleteMemory = false)
            {
                if (_show)
                {
                    _drawablesM.lock();
                    vector<Drawable*> &drawables = _drawables.getCurrent();
                    if (deleteMemory)
                        for (int i=0;i<drawables.size();++i)
                            delete drawables[i];
                    drawables.resize(0);
                    _canvas.clear(_backgroundColor);
                    _drawablesM.unlock();
                }
                return true;
            }

        bool startThread(){return Thread::start();}
        void finishThread(){Thread::finish();}

    protected:
        virtual bool execute(){start(); return true;}

    protected:
        void updateTime(bool start = false)
            {
                if (start)
                {
                    gettimeofday(&mStartTV,0);
                    mCurrentTV = mStartTV;
                    mCurrentTick = mStartTick = (1000000*long(mStartTV.tv_sec)+long(mStartTV.tv_usec));
                    mElapsedTime = mTotalElapsedTime = 0;
                    return;
                }
                long previousTick = mCurrentTick;
                gettimeofday(&mCurrentTV,0);
                mCurrentTick = (1000000*long(mCurrentTV.tv_sec)+long(mCurrentTV.tv_usec));
                mElapsedTime = mCurrentTick - previousTick;
                mTotalElapsedTime = mCurrentTick - mStartTick;
            }
        struct timeval mStartTV;
        struct timeval mCurrentTV;
        long mStartTick;
        long mCurrentTick;
        long mElapsedTime;                // Measured in microseconds
        long mTotalElapsedTime;        // Measured in microseconds

    private:
        Canvas _canvas;
        bool _show;
        bool _quit;
        Drawables _drawables;
        Mutex<Drawables> _drawablesM;
        Cairo::Matrix _FPSBasis;
        unsigned int _backgroundColor;
    };
};

#endif
