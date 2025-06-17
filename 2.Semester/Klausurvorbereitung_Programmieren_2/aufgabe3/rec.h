#ifndef REC_H
#define REC_H

#include <string>
#include <vector>
#include <iostream>
using namespace std;

#include "graph.h"

class Rectangle : public GraphObj{
    private:
        int height;
        int width;

    public:
        Rectangle(const int pXCoord, const int pYCoord, const int pHeight, const int pWidth);
        int getHeight() const;
        int getWidth() const;
        virtual double area();
        void draw();
};
#endif