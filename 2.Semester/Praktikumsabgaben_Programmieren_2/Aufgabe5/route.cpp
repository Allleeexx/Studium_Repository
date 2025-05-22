#include "route.h"
#include <algorithm>

float error = 0.0;

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
    if(destinations->empty()){          //wenn destinations leer ist return error
        return error;
    }

    float StartX = 0.0;
    float StartY = 0.0;
    float EndX = 0.0;
    float EndY = 0.0;
    int i = 0;
    float distance = 0.0;  //Alles in einer Zeile gibt warum auch immer einen fehler deswegen nochmal einzeln
    float AnzahlDestinations = destinations->size();
    float height = getHeight();

    while(i<AnzahlDestinations){
        EndX = destinations->at(i).first;        //wenn punkt zum beispiel (1,0) dann ist hier EndX die 1 und py2 ist 0
        EndY = destinations->at(i).second;
        distance += dist(StartX, StartY, EndX, EndY, height);
        StartX = EndX;
        StartY = EndY;
        i++;
    }
    distance += dist(StartX,StartY, 0,0, height);
    return distance;
}           

Route Route::shortestRoute() const {
    Route workingcopy = Route(*this);

    // Prüfen, ob es überhaupt Destinationen gibt
    if (workingcopy.destinations->empty()) {
        return workingcopy; 
    }

    sort(workingcopy.destinations->begin(), workingcopy.destinations->end());
    float shortestDistance = workingcopy.distance();
    Route bestRoute = Route(*this);

    while (next_permutation(workingcopy.destinations->begin(), workingcopy.destinations->end())) {
        float currentDistance = workingcopy.distance();
        if (currentDistance < shortestDistance) {
            shortestDistance = currentDistance;
            bestRoute.setHeight(workingcopy.getHeight());
            bestRoute.setDist(workingcopy.dist);
            *bestRoute.destinations = *workingcopy.destinations;
        }
    }
    return bestRoute;
}
