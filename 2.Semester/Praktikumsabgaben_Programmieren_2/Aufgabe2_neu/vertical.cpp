#include <chrono>
#include <cmath>
#include <iostream>
#include "vertical.h"

Vertical::Vertical (const string& pId) : Ufo(pId){}

Vertical::~Vertical(){}

 void Vertical::flyToDest(const float x, const float y, const float height, int speed) const {
    //mit flyto von Ufosim wird von aktuelller  Pos der Drohne in gerader Linie zu Punkt            -> Fluggeschw. ist vFlight
    //Nach Flug geschw. vPost       -> wen vPost 0 steht die DrWohne 
    //Wenn vPost = vFlight      --> Dann fliegt Drohne mit gleicher Geschwindigkeit

    //Abfolge       -> Nach oben    -->dann nach (x,y,height) und dann (x,y,0.0)    speed als Par. setzen

    sim->flyTo(sim->getX(), sim->getY(), height, speed, 0);
    sim->flyTo(x, y, height, speed, 0);
    sim->flyTo(x, y, 0.0, speed, 0);
 }