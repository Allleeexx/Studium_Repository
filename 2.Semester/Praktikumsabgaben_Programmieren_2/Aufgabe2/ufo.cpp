#include <chrono>
#include <cmath>
#include <iostream>
#include "ufo.h"
#include <cmath>

Ufo::Ufo(const string& pId){
    id=pId;
    Ufosim::setSpeedup(4);
    sim = new Ufosim();
}

Ufo::~Ufo(){
    delete sim;
}

const string& Ufo::getId() const{
    return id;
}

vector<float> Ufo::getPosition() const{
    return {sim->getX(), sim->getY(), sim->getZ()};
}

float Ufo::getFtime() const{
    return sim->getFtime();
}

static vector<float> Ufo::wayPoint(const float x1,const float y1,const float x2,const float y2,const float h,const float phi){
    //A(x1,y1,0) und D(x2, y2, 0), h>0 und Winkel 0<q<90    -->Punkt B(x,y,0) gesucht
    const float rad = phi * static_cast<float>(M_PI) / 180.0f;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float lengthAD = sqrt(dx * dx + dy * dy);

    float lengthAB = h / tan(rad);
    float scale = lengthAB / lengthAD;

    float x = x1 + dx * scale;
    float y = y1 + dy * scale;

    return { x, y };
}