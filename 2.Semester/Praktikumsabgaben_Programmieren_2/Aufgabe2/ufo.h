#ifndef UFO_H
#define UFO_H

#include <string>
#include <vector>
#include "ufosim.h"
using namespace std; 

class Ufo{
    private:
        string id;
    
    protected:      //bedeutet, dass sim nicht von anderen Klassen zugreifbar ist, aber von abgeleiteten Klassen verwendet werden darf
        Ufosim* sim;

    public:
        Ufo(const string& pId);
        virtual ~Ufo();
        const string& getId() const;  //const hinten das Funktion keine Attribute ändern kann
        vector<float> getPosition() const;
        float getFtime() const;
        virtual void flyToDest(const float x, const float y, const float height, const int speed) const = 0;        //rein Virtual für abstrakte Klasse
        static vector<float> wayPoint(const float x1,const float y1,const float x2,const float y2,const float h,const float phi);       //static, damit ich sie aufrufen kann, ohne ein extra Ufo Objekt zu benötigen
};

#endif