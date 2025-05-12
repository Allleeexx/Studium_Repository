#ifndef UFO_THREAD_H
#define UFO_THREAD_H

#include <string>
#include <vector>
#include <functional>
#include <utility>

#include "ufo.h"
using namespace std; 

class UfoThread : public Ufo{
    private:
        thread* flyThread;
        Ufo* ufo;
        bool isFlying;      //soll standardmäßig false sein. nur true wenn fliegt

    public:
        UfoThread(Ufo* pUfo);
        ~UfoThread();
        void runner(const float x, const float y, const float height, const int speed);
        void startUfo(const float x, const float y, const float height, const int speed);
        bool getIsFlying();
};

#endif