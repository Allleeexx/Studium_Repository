#include <cctype>
#include <iostream>
#include <fstream>
#include <cmath>
using namespace std; 

//HEADER DATEIEN
#include "vertical.h"


const string Vertical::type = "vertical";

//Das hier ist der Konstruktor  -> Das hier wird aufgerufen, wenn ich in der main am anfang Vertical vert("r2d2") eingebe. Dann wird id mit r2d2 initialisiert
Vertical::Vertical(const string& pId){
    id = pId;
    Ufosim::setSpeedup(4);
    sim = new Ufosim();
}

//Das hier ist der Destruktor 
Vertical::~Vertical(){
    delete sim;
}

string Vertical::getType(){
    return type;
}

const string& Vertical::getId() const{
    return id;
}

vector<float> Vertical::getPosition() const{
    return {(sim->getX(), sim->getY(), sim->getZ())};
}

float Vertical::getFtime() const{
    return sim->getFtime();
}

void Vertical::flyToDest(const float x, const float y, const float height, const int speed) const{
    //mit flyto von Ufosim wird von aktuelller  Pos der Drohne in gerader Linie zu Punkt            -> Fluggeschw. ist vFlight
    //Nach Flug geschw. vPost       -> wen vPost 0 steht die DrWohne 
    //Wenn vPost = vFlight      --> Dann fliegt Drohne mit gleicher Geschwindigkeit

    //Abfolge       -> Nach oben    -->dann nach (x,y,height) und dann (x,y,0.0)    speed als Par. setzen

    sim->flyTo(sim->getX(), sim->getY(), height, speed, 0);
    sim->flyTo(x, y, height, speed, 0);
    sim->flyTo(sim->getX(), sim->getY(), 0.0, speed, 0);
}

vector<float> Vertical::wayPoint(const float x1,const float y1,const float x2,const float y2,const float h,const float phi){
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