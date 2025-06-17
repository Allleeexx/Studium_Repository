#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <iostream>
using namespace std;

class GraphObj{
    private:
        int xCoord;
        int yCoord;

    public:
        GraphObj(const int pXCoord, const int pYCoord);
        vector<int> getRefPoint() const;
        vector<int> setRefPoint(const int pXCoord, const int pYCoord);
        double length();
        virtual double area() const=0;
        virtual void draw();

};

#endif