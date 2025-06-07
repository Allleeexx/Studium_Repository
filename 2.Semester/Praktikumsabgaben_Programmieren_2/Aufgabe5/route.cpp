#include "route.h"
#include <algorithm>

float error = 0.0;

// Konstruktor: Initialisiert Höhe, Distanzfunktion und erstellt neuen Vektor für Ziele
Route::Route(const float pHeight, function<float(float, float, float, float, float)> pDist){
    height = pHeight;
    dist = pDist;
    destinations = new vector<pair<float, float>>();
}

// Kopierkonstruktor: Erstellt eine tiefe Kopie der Destinationen
Route::Route(const Route& route){
    height = route.height;
    dist = route.dist;
    destinations = new vector <pair<float, float>>(*route.destinations);
}                  

// Destruktor: Gibt den allokierten Speicher für Destinationen frei
Route::~Route(){
    delete destinations;
}

// Move-Konstruktor: Übernimmt Besitz des Zeigerinhalts, Quelle wird auf nullptr gesetzt
//Der neue Route übernimmt den Inhalt, und das alte Route ist danach leer bzw. nicht mehr zuständig für das Löschen des Vektors. --> dadurch teure Kopie des Vektors vermieden
Route::Route(Route&& route){
    height = route.height;
    dist = route.dist;
    destinations = route.destinations;// Besitz übernehmen
    route.destinations = nullptr;// Quelle invalidieren
}

// Fügt ein Zielkoordinatenpaar zur Route hinzu
void Route::add(const float destX, const float destY){
    pair<float, float> insert = {destX, destY};
    destinations->push_back(insert);
}             

// Gibt eine konstante Referenz auf den Zielvektor zurück
const vector<pair<float, float>>& Route::getDestinations() const{
    return *destinations;
}              


// Gibt die Flughöhe zurück
float Route::getHeight() const {
    return height;
}

// Setzt die Flughöhe
void Route::setHeight(const float pHeight){
    height = pHeight;
}

// Setzt die Distanzfunktion neu
void Route::setDist(function<float(float, float, float, float, float)> pDist){
    dist = pDist;
}      


// Berechnet die Gesamtdistanz der Route inkl. Rückflug zum Start (0,0)
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

    // Schleife über alle Ziele: Von einem Punkt zum nächsten
    while(i<AnzahlDestinations){
        EndX = destinations->at(i).first;        //wenn punkt zum beispiel (1,0) dann ist hier EndX die 1 und py2 ist 0
        EndY = destinations->at(i).second;
        distance += dist(StartX, StartY, EndX, EndY, height);// Distanz vom aktuellen Startpunkt zum Zielpunkt berechnen und addieren
         
        // Neuer Startpunkt ist das aktuelle Ziel
        StartX = EndX;
        StartY = EndY;
        i++;
    }

    // Rückflug zum Startpunkt (0,0) einrechnen
    distance += dist(StartX,StartY, 0,0, height);
    return distance;
}           


// Bestimmt die kürzeste mögliche Route durch Permutieren aller Ziele
Route Route::shortestRoute() const {
    Route workingcopy = Route(*this);// Kopie der aktuellen Route

    // Prüfen, ob es überhaupt Destinationen gibt
    if (workingcopy.destinations->empty()) {
        return workingcopy; 
    }

    // Startsortierung für Permutationsfunktion
    sort(workingcopy.destinations->begin(), workingcopy.destinations->end());
    float shortestDistance = workingcopy.distance();
    Route bestRoute = Route(*this);// Beste Route wird hier gespeichert

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
