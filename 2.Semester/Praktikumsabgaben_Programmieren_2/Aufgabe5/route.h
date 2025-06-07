#ifndef ROUTE_H
#define ROUTE_H

#include <string>
#include <vector>
#include <functional>
#include <utility>
using namespace std; 

class Route{
    private:
        vector<pair<float, float>>* destinations;   /// Zeiger auf Vektor von Zielkoordinaten (x, y) –  z ist immer 0,0
        float height;   //Flughöhe
        function<float(float, float, float, float, float)>dist; //5 float Parameter und einem float rückgabewert        ----> Klassentemplate function verwenden
    public:
        Route(const float pHeight, function<float(float, float, float, float, float)> pDist);
        Route(const Route& route);                  //Copy Konstruktor: Erzeugt eine tiefe Kopie (eigene Kopie der Ziele)
        Route(Route&& route); // Move-Konstruktor: Übernimmt Ressourcen von temporärem Objekt (besitzübertragend)
        ~Route();
        void add(const float destX, const float destY);             //Fügt Ziel hinten an destinations an
        const vector<pair<float, float>>& getDestinations() const;   // Gibt konstante Referenz auf alle Zielkoordinaten zurück
        float getHeight() const ;
        void setHeight(const float pHeight);
        void setDist(function<float(float, float, float, float, float)> pDist);      //Setter für dist
        float distance() const;           //gesamte zu fliegende Distanz zurückgeben
        Route shortestRoute() const;  // Sucht per vollständiger Permutation die kürzeste mögliche Route und gibt sie zurück
};

#endif
