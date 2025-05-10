#include "vertical.h"
#include <cmath>

Vertical::Vertical (const string& pId) : Ufo(pId){}

Vertical::~Vertical(){}

 void Vertical::flyToDest(const float x, const float y, const float height,const int speed) const {
    //mit flyto von Ufosim wird von aktuelller  Pos der Drohne in gerader Linie zu Punkt            -> Fluggeschw. ist vFlight
    //Nach Flug geschw. vPost       -> wen vPost 0 steht die DrWohne 
    //Wenn vPost = vFlight      --> Dann fliegt Drohne mit gleicher Geschwindigkeit

    //Abfolge       -> Nach oben    -->dann nach (x,y,height) und dann (x,y,0.0)    speed als Par. setzen

    sim->flyTo(sim->getX(), sim->getY(), height, speed, 0);
    sim->flyTo(x, y, height, speed, 0);
    sim->flyTo(x, y, 0.0, speed, 0);
 }

  float Vertical::distance(const float x1, const float y1, const float x2, const float y2, const float h){   
   return 2+sqrt(h*h)+sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));        //2 + vertikakler Abstand + horizontaler Abstand          --> vertikaler Abstand ist der Betrag der höhe quadrat      --> horizontaler Abstand ist die Distanz zwischen den einzelnen Punkten
 }
