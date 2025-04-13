#ifndef BALLISTIC_H
#define BALLISTIC_H

#include <string>
#include <vector>

#include "ufo.h"

class Ballistic:Ufo{
    private:
        float takeOffAngle;
        float landingAngle;
    
    public:
        Ballistic(string& pld, float pTakeOffAngle, float pLandingAngle);
        ~Ballistic();

        float getTakeOffAngle();        
        float getLandingAngle();
        void flyToDest(const float x, const float y, const float height, const int speed);
        vector<float> firstWaypoint(const float x, const float y, const float height);
        vector<float> secondWaypoint(const float x, const float y, const float height);

};

#endif