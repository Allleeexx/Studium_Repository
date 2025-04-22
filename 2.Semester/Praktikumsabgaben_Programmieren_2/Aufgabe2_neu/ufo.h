#ifndef UFO_H
#define UFO_H

#include <string>
#include <vector>
#include "ufosim.h"
using namespace std; 

class Ufo{
    public:
        string id;
    
    protected:
        Ufosim* sim;

    private:
        Ufo(const string& pId);
        virtual ~Ufo();
        const string& getId() const;  //const hinten das Funktion keine Attribute ändern kann
        vector<float> getPosition() const;
        float getFtime() const;
        virtual void flyToDest(const float x, const float y, const float height, const int speed) const = 0;        //rein Virtual für abstrakte Klasse
        static vector<float> wayPoint(const float x1,const float y1,const float x2,const float y2,const float h,const float phi);
};

#endif