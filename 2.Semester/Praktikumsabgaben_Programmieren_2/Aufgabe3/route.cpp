#include "route.h"
#include <algorithm>
#include <limits>

Route::Route(const float pHeight, function<float(float, float, float, float, float)> pDist){
    height = pHeight;
    dist = pDist;
    destinations = new vector<pair<float, float>>();
}

Route::Route(const Route& route){
    height = route.height;
    dist = route.dist;
    destinations = new vector <pair<float, float>>(*route.destinations);
}                  

Route::~Route(){
    delete destinations;
}

void Route::add(const float destX, const float destY){
    pair<float, float> insert = {destX, destY};
    destinations->push_back(insert);
}             

const vector<pair<float, float>>& Route::getDestinations() const{
    return *destinations;
}              

float Route::getHeight() const {
    return height;
}

void Route::setHeight(const float pHeight){
    height = pHeight;
}

void Route::setDist(function<float(float, float, float, float, float)> pDist){
    dist = pDist;
}      

float Route::distance() const{
//function Dist verwenden
//wenn zielliste leer ist soll 0,0 rauskommen
}           

Route Route::shortestRoute() const{

}
