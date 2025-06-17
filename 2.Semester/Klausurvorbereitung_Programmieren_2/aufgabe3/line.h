#ifndef LINE_H
#define LINE_H

#include <string>
#include <vector>
#include <iostream>
using namespace std;

#include "graph.h"

class Line : public GraphObj{
    private:
        int endX;
        int endY;
    public:
        Line(const int pXCoord, const int pYCoord);
        vector<int> getEndPoint() const;
        double length();
        virtual double area();
        void draw();
};
#endif