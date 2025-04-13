#include <cctype>
#include <iostream>
#include <fstream>
#include <cmath>
using namespace std; 

//HEADER DATEIEN
#include "vertical.h"

void Vertical::flyToDest(const float x, const float y, const float height, const int speed) const{
    //mit flyto von Ufosim wird von aktuelller  Pos der Drohne in gerader Linie zu Punkt            -> Fluggeschw. ist vFlight
    //Nach Flug geschw. vPost       -> wen vPost 0 steht die DrWohne 
    //Wenn vPost = vFlight      --> Dann fliegt Drohne mit gleicher Geschwindigkeit

    //Abfolge       -> Nach oben    -->dann nach (x,y,height) und dann (x,y,0.0)    speed als Par. setzen

    sim->flyTo(sim->getX(), sim->getY(), height, speed, 0);
    sim->flyTo(x, y, height, speed, 0);
    sim->flyTo(sim->getX(), sim->getY(), 0.0, speed, 0);
}



/*
int main(){
    Vertical vert("r2d2");  //Hier rufe ich Konstruktor auf 

    // fly from (0.0, 0.0, 0.0) to (5.0, -1.5, 0.0)
    // at altitude 4.0 with 10 km/h
    vert.flyToDest(5.0, -1.5, 4.0, 10);
    
    // fly from (5.0, -1.5, 0.0) to (-3.0, 0.0, 0.0)
    // at altitude 8.0 with 5 km/h
    vert.flyToDest(-3.0, 0.0, 8.0, 5);

    return 0;

} // <-- Hier wird am Ende automatisch der Destruktor aufgerufen
*/