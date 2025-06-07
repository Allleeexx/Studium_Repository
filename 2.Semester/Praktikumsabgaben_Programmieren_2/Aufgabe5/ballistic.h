#ifndef BALLISTIC_H
#define BALLISTIC_H

#include <string>
#include <vector>
#include "ufo.h"
using namespace std; 

// Die Klasse Ballistic erbt von der Basisklasse Ufo und modelliert ein Ufo, 
// das in einer ballistischen Flugbahn (mit Start- und Landewinkel) fliegt.
class Ballistic : public Ufo{
    private:
        float takeOffAngle;
        float landingAngle;

    public:
        Ballistic(const string& pId, const float pTakeOffAngle, const float pLandingAngle);// Konstruktor: Erzeugt ein Ballistic-Ufo mit ID, Startwinkel und Landewinkel. Ungültige Winkel werden auf den Default-Wert 45° gesetzt.
        ~Ballistic();
        float getTakeOffAngle() const;
        float getLandingAngle() const;
        virtual void flyToDest(const float x, const float y, const float height, const int speed) const override;  // Überschreibt die virtuelle Methode aus der Ufo-Basisklasse: Führt einen Flug in drei Etappen aus (2 Zwischenziele + Landung)
        vector<float> firstWaypoint(const float x, const float y, const float height) const;
        vector<float> secondWaypoint(const float x, const float y, const float height) const;
};

#endif 