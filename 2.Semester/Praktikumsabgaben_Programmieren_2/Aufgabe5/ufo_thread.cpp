#include "ufo_thread.h"


// Konstruktor: Initialisiert das Ufo-Thread-Objekt mit einem gegebenen Ufo-Pointer.
// Thread-Zeiger wird auf nullptr gesetzt, isFlying auf false (fliegt noch nicht).
UfoThread::UfoThread(Ufo* pUfo){
    ufo = pUfo;
    isFlying = false;
    flyThread = nullptr;
}

// Destruktor: Falls ein Thread noch aktiv ist, wird er zuerst beendet (join) und dann gelöscht.v
UfoThread::~UfoThread(){
    if(flyThread != nullptr){
        flyThread->join();
        delete flyThread;
    }
}

// Thread-Funktion (wird vom Hintergrund-Thread ausgeführt):
// Führt den Flug zum Ziel aus und setzt anschließend isFlying auf false.
void UfoThread::runner(const float x, const float y, const float height, const int speed){      //das hier ist die Thread Funktion. Fliegt das Ufo nach (x,y,0.0)
    ufo->flyToDest(x, y, height, speed);
    isFlying = false;
}

// Startet einen neuen Flug in einem separaten Thread
void UfoThread::startUfo(const float x, const float y, const float height, const int speed){        //startet den Thread. Parameter von startUfo an runner übergeben
    if(flyThread != nullptr){   //Wenn thread existiert Thread schließen
        flyThread->join();  //thread schließen    
        delete flyThread;
    }

    isFlying = true;
    flyThread = new thread(&UfoThread::runner, this, x, y, height, speed);  //startet den Thread und ruft funktion runner auf
    
}

// Getter für den Flugstatus – gibt zurück, ob gerade ein Flug aktiv ist
bool UfoThread::getIsFlying(){
    return isFlying;
}