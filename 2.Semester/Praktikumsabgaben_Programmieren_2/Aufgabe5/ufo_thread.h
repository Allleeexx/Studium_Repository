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


// Steuert ein Ufo-Objekt in einem separaten Thread und nutzt Qt-Signale zur Benachrichtigung,
// wenn das Ufo seinen Flug beendet hat.
class UfoThread : public QObject{

    Q_OBJECT
    private:
        std::thread* flyThread;      // Zeiger auf den laufenden std::thread (Threadobjekt zum Ausführen des Flugs)
        Ufo* ufo;           //Pointer auf ein Objekt von Ballistic oder Vertical
        bool isFlying;      //soll standardmäßig false sein. nur true wenn fliegt
        void runner(const float x, const float y, const float height, const int speed){      //das hier ist die Thread Funktion. Fliegt das Ufo nach (x,y,0.0)
            ufo->flyToDest(x, y, height, speed);
            isFlying = false;
            emit stopped(ufo->getPosition());   // Qt-Signal senden: Ufo hat Flug beendet
        }


    public:
        UfoThread(Ufo* pUfo){
            ufo = pUfo;
            isFlying = false;
            flyThread = nullptr;
        }

        // Destruktor: Falls ein Thread aktiv ist, wird gewartet und anschließend Speicher freigegeben
        ~UfoThread(){
            if(flyThread != nullptr){
                flyThread->join();
                delete flyThread;
            }
        }

        // Startet das Ufo in einem neuen Thread
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
     // Qt-Signal, das ausgelöst wird, wenn das Ufo seinen Flug beendet hat.
    // Übergibt die aktuelle Position als Vektor (x, y, z)
        void stopped(vector<float>);
};

#endif
