#include "ufo_thread.h"

UfoThread:: UfoThread(Ufo* pUfo) : ufo(pUfo), isFlying(false), flyThread(nullptr){
    //flyThread = nullptr;        //mit flythread != nullptr prüfen ob ein Thread existiert
}

UfoThread:: ~UfoThread(){

}

void UfoThread::runner(const float x, const float y, const float height, const int speed){      //das hier ist die Thread Funktion. Fliegt das Ufo nach (x,y,0.0)
    isFlying = true;
    ufo->flyToDest(x, y, height, speed);
    isFlying = false;
}

void UfoThread::startUfo(const float x, const float y, const float height, const int speed){        //startet den Thread. Parameter von startUfo an runner übergeben
    if(flyThread != nullptr){
        flyThread->join();
        
        delete flyThread;
        runner(x, y, height, speed);
    }

    //flythread noch zuweisen
}

bool UfoThread::getIsFlying(){
    return isFlying;
}