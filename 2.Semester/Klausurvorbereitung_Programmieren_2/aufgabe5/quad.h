
#ifndef QUAD_H
#define QUAD_H

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
using namespace std;

class Quadrat{
    private:
        float xLO;
        float yLO;
        float length;
        string name = "no game";

    public:
        Quadrat(const float pXLO, const float pYLO, const float length);
        Quadrat(const Quadrat& quadrat);
        float getXLO() const;
        float getYLO() const;
        float getLength() const;
        const string& getString() const;

        void setString(string text);
};

#endif