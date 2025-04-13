#ifndef VERTICAL_H
#define VERTICAL_H

#include <string>
#include <vector>
#include "ufosim.h"
#include "ufo.h"
using namespace std; 

//Klassen
class Vertical : public Ufo{
    private:
        static const string type;
        string id;
        Ufosim* sim;

    public:
        Vertical(const string& pld);
        ~Vertical();
        void flyToDest(const float x, const float y, const float height, const int speed) const override;
};

#endif