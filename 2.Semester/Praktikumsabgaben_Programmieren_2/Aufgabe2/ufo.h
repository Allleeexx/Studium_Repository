#ifndef UFO_H
#define UFO_H

#include <string>
#include <vector>
#include "ufosim.h"

/*Abstrakte Klasse UFO*/
class Ufo:Ufosim{
    private:
        string id;
        Ufosim* sim;

    public:
        Ufo(const string& pld);
        virtual ~Ufo();        //soll virtuell sein
        const string& getId() const;  //const hinten das Funktion keine Attribute ändern kann
        vector<float> getPosition() const;
        float getFtime() const;
        virtual flyToDest(const float x, const float y, const float height, const int speed) const;        //soll virtuell sein
        static vector<float> wayPoint(const float x1,const float y1,const float x2,const float y2,const float h,const float phi);
};

#endif