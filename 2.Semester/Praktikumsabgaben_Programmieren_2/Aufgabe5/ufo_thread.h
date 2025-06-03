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
        std::thread* flyThread;      //Attribut flyThread ist pointer auf einen Thread
        Ufo* ufo;           //Pointer auf ein Objekt von Ballistic oder Vertical
        bool isFlying;      //soll standardmäßig false sein. nur true wenn fliegt
        void runner(const float x, const float y, const float height, const int speed){      //das hier ist die Thread Funktion. Fliegt das Ufo nach (x,y,0.0)
            ufo->flyToDest(x, y, height, speed);
            isFlying = false;
            emit stopped(ufo->getPosition());
        }


    public:
        UfoThread(Ufo* pUfo){
            ufo = pUfo;
            isFlying = false;
            flyThread = nullptr;
        }
        ~UfoThread(){
            if(flyThread != nullptr){
                flyThread->join();
                delete flyThread;
            }
        }
        void startUfo(const float x, const float y, const float height, const int speed){        //startet den Thread. Parameter von startUfo an runner übergeben
            if(flyThread != nullptr){   //Wenn thread existiert Thread schließen
                flyThread->join();  //thread schließen
                delete flyThread;
            }

            isFlying = true;
            flyThread = new std::thread(&UfoThread::runner, this, x, y, height, speed);  //startet den Thread und ruft funktion runner auf

        }
        bool getIsFlying(){
            return isFlying;
        }

    signals:
        void stopped(vector<float>);
};

#endif
