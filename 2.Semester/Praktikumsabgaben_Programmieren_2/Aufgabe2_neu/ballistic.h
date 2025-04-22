#ifndef BALLISTIC_H
#define BALLISTIC_H

#include <string>
#include <vector>
#include "ufosim.h"
#include "ufo.h"
using namespace std; 

class Ballistic : public Ufo{
    private:
        float takeOffAngle;
        float landingAngle;

    public:
        Ballistic(const string& pId, const float pTakeOffAngle, const float pLandingAngle);
        ~Ballistic();
        float getTakeOffAngle() const;
        float getLandingAngle() const;
        virtual void flyToDest(const float x, const float y, const float height, const int speed) const override;
        vector<float> firstWaypoint(const float x, const float y, const float height) const;
        vector<float> secondWaypoint(const float x, const float y, const float height) const;
};

#endif 