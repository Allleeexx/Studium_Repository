#ifndef VERTICAL_H
#define VERTICAL_H

#include <string>
#include "ufo.h"
using namespace std; 

class Vertical : public Ufo{
    public:
        Vertical (const string& pId);
        ~Vertical();
        virtual void flyToDest(const float x, const float y, const float height, const int speed) const override;
        static float distance(const float x1, const float y1, const float x2, const float y2, const float h);
};

#endif