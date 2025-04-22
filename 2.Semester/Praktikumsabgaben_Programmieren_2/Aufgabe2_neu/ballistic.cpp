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

void Ballistic::flyToDest(const float x, const float y, const float height, const int speed) const {}

vector<float> Ballistic::firstWaypoint(const float x, const float y, const float height) const{
    return Ufo::wayPoint(sim->getX(), sim->getY(), x, y, height, takeOffAngle);
}

vector<float> Ballistic::secondWaypoint(const float x, const float y, const float height) const{
    
}
