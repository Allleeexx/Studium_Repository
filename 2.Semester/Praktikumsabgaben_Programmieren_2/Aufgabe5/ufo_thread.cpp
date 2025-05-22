#include "ufo_thread.h"


UfoThread::UfoThread(Ufo* pUfo){
    ufo = pUfo;
    isFlying = false;
    flyThread = nullptr;
}

UfoThread::~UfoThread(){
    if(flyThread != nullptr){
        flyThread->join();
        delete flyThread;
    }
}

void UfoThread::runner(const float x, const float y, const float height, const int speed){      //das hier ist die Thread Funktion. Fliegt das Ufo nach (x,y,0.0)
    ufo->flyToDest(x, y, height, speed);
    isFlying = false;
}

void UfoThread::startUfo(const float x, const float y, const float height, const int speed){        //startet den Thread. Parameter von startUfo an runner übergeben
    if(flyThread != nullptr){   //Wenn thread existiert Thread schließen
        flyThread->join();  //thread schließen    
        delete flyThread;
    }

    isFlying = true;
    flyThread = new thread(&UfoThread::runner, this, x, y, height, speed);  //startet den Thread und ruft funktion runner auf
    
}

bool UfoThread::getIsFlying(){
    return isFlying;
}