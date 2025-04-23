#include <chrono>
#include <cmath>
#include <iostream>
#include "ballistic.h"
#include "vertical.h"

Ballistic::Ballistic(const string& pId, const float pTakeOffAngle, const float pLandingAngle){
    id = pId;
    if((0<pTakeOffAngle<=90)){
        takeOffAngle = pTakeOffAngle;
    }else{  //Hier die Default-Werte
        takeOffAngle = 45.0;
    }

    if(0<pLandingAngle<=90){
        landingAngle = pLandingAngle;
    }else{
        landingAngle = 45.0;
    }

}

Ballistic::~Ballistic(){}

float Ballistic::getTakeOffAngle() const{
    return takeOffAngle;
}

float Ballistic::getLandingAngle() const{
    return landingAngle;
}

void Ballistic::flyToDest(const float x, const float y, const float height, const int speed) const {
    vector<float> erstesZiel = firstWaypoint(x, y, height);
    vector<float> zweitesZiel = secondWaypoint(x, y, height); 

    //Testausgabe, ob ich die richtigen Werte habe
    cout << "Koordinaten von Ziel 1: \n X:" << erstesZiel[0] << "Y: " << erstesZiel[1] << "\n"; 
    cout << "Koordinaten von Ziel 2: \n X:" << zweitesZiel[0] << "Y: " << zweitesZiel[1] << "\n"; 
    //3 mal Fly to aufrufen
    sim->flyTo(erstesZiel[0], erstesZiel[1], height, speed, speed);       //Fliegen schräg zu x1,y1
    sim->flyTo(zweitesZiel[0], zweitesZiel[1], height, speed, speed);       //Fliegen von x1, y1 weiter nach x1,y2
    sim->flyTo(x,y, 0, speed, 0);       //Fliegen weiter von x2,y2 nach x,y,0,0
}

vector<float> Ballistic::firstWaypoint(const float x, const float y, const float height) const{
    return Ufo::wayPoint(sim->getX(), sim->getY(), x, y, height, takeOffAngle);
}

vector<float> Ballistic::secondWaypoint(const float x, const float y, const float height) const{
    return Ufo::wayPoint(x,y, sim->getX(), sim->getY(),height,landingAngle);
}
