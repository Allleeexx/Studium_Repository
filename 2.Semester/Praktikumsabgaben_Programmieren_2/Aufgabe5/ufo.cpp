#include <cmath>
using namespace std; 

//HEADER DATEIEN
#include "ufo.h"


// Konstruktor der abstrakten Basisklasse Ufo
// Wird z. B. durch Vertical oder Ballistic aufgerufen
// - ID wird gespeichert
// - Ufosim-Objekt erzeugt (simuliert den Flug)
// - Speedup-Faktor der Simulation wird global auf 4 gesetzt (schnellere Ausführung)
Ufo::Ufo(const string& pId){
    id = pId;
    Ufosim::setSpeedup(4);// Zeitraffer-Faktor für die Simulation
    sim = new Ufosim();// Simulator-Instanz dynamisch erzeugen
}

// Destruktor: Gibt das dynamisch allozierte Ufosim-Objekt wieder frei
Ufo::~Ufo(){
    delete sim;
}

// Gibt die ID des Ufos zurück (z. B. "r2d2")
const string& Ufo::getId() const{
    return id;
}

vector<float> Ufo::getPosition() const{
    return {sim->getX(), sim->getY(), sim->getZ()};
}

float Ufo::getFtime() const{
    return sim->getFtime();
}

vector<float> Ufo::wayPoint(const float x1,const float y1,const float x2,const float y2,const float h,const float phi){
    //A(x1,y1,0) und D(x2, y2, 0), h>0 und Winkel 0<q<90    -->Punkt B(x,y,0) gesucht
    

     // Winkel in Bogenmaß umrechnen
    const float rad = phi * static_cast<float>(M_PI) / 180.0f;

    // Richtung vom Punkt A nach D
    float dx = x2 - x1;     //bsp. A = (0, 0), D = (10, 10) → dx = 10, dy = 10
    float dy = y2 - y1;
    
     // Gesamtlänge der Strecke von A nach D
     //Pythagoras: Gesamte Luftlinie zwischen Start A und Ziel D.
    float lengthAD = sqrt(dx * dx + dy * dy);

    
    // Länge der Strecke von A nach B, berechnet über Höhe und Steigungswinkel (horizontaler Anteil der Steigung):
    //Gegenkathete = h (die gewünschte Höhe)
    //Winkel = phi
    //Gesucht: Ankathete (horizontale Länge von A bis B)
    //tan(φ) = h / AB → AB = h / tan(φ)
    float lengthAB = h / tan(rad);

    // Verhältnis der Teilstrecke AB zur Gesamtstrecke AD -->  wie weit entlang der AD-Strecke man fliegen muss, um lengthAB zurückzulegen.
    float scale = lengthAB / lengthAD;

    
    // Zielkoordinaten des Wegpunkts (B), basierend auf der Skala
    float x = x1 + dx * scale;
    float y = y1 + dy * scale;

    return { x, y }; //Punkt B(x, y) auf dem Weg von A nach D, bei dem das Ufo bei Steigung phi genau die Höhe h erreichen würde.
}
