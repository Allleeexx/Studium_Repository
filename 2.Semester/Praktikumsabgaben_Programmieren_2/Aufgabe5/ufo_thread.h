#ifndef UFO_THREAD_H
#define UFO_THREAD_H

#include <string>
#include <vector>
#include <functional>
#include <utility>
#include <thread>
#include <QObject>

#include "ufo.h"
using namespace std;

class UfoThread : public QObject{

    Q_OBJECT
    private:
        thread* flyThread;      //Attribut flyThread ist pointer auf einen Thread
        Ufo* ufo;           //Pointer auf ein Objekt von Ballistic oder Vertical
        bool isFlying;      //soll standardmäßig false sein. nur true wenn fliegt
        void runner(const float x, const float y, const float height, const int speed);

    public:
        UfoThread(Ufo* pUfo);
        ~UfoThread();
        void startUfo(const float x, const float y, const float height, const int speed);
        bool getIsFlying();

    signals:
        void stopped(vector<float>);
};

#endif
