#include <chrono>
#include <cmath>
#include <iostream>
#include "ballistic.h"


Ballistic::Ballistic(string& pld, float pTakeOffAngle, float pLandingAngle):Ufo(pId){
    takeOffAngle = (pTakeOffAngle > 0.0f && pTakeOffAngle <= 90.0f) ? pTakeOffAngle : 45.0f;
    landingAngle = (landingAngle> 0.0f && landingAngle <= 90.0f) ? landingAngle : 45.0f;
}

Ballistic::~Ballistic(){}

float Ballistic::getTakeOffAngle(){
    return takeOffAngle;
}

float Ballistic::getLandingAngle(){
    return landingAngle;
}

void Ballistic::flyToDest(const float x, const float y, const float height, const int speed){
    vector<float> wp1 = firstWaypoint(x, y, height);
    vector<float> wp2 = secondWaypoint(x, y, height);

    sim->flyTo(wp1[0], wp1[1], height, speed, speed);
    sim->flyTo(wp2[0], wp2[1], height, speed, speed);
    sim->flyTo(x, y, 0.0f, speed, 0);       //0.0f , da direkt in Float ohne f wäre es Doubela
}

vector<float> Ballistic::firstWaypoint(const float x, const float y, const float height) const{
    return wayPoint(sim->getX(),sim->getY(),x, y, height, takeOffAngle);
}

vector<float> Ballistic::secondWaypoint(const float x, const float y, const float height) const{
    return wayPoint(sim->getX(),sim->getY(),x, y, height, landingAngle);
}
