#ifndef VERTICAL_H
#define VERTICAL_H

#include <string>
#include <vector>

//Klassen
class Vertical{
    private:        //Da Minus davor
        string type;                    //statisch
        string id;
        //Ufosim* sim;                  //In Aufgabe 1 steht das noch nicht einfügen, das kommt erst später

    public:         //Da Plus davor
        Vertical(string& pId);          // Konstruktor der Klasse
        ~Vertical();                    // Destruktor der Klasse

        //Getter    
        string getType();               //statisch
        string& getId();
        vector <float> getPosition();
        float getFtime();
        void flfyToDest(float x; float y1; float heigth; int speed);
        vector<float> waypoint(float x1; float x2; float y2; float h; float phi);       //statisch -> da im Klassendiagramm unterschtrichen

}

#endif