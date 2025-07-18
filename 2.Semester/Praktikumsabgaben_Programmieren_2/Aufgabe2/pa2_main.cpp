#include <iostream>
#include <cmath>
using namespace std;

#include "vertical.h"
#include "ballistic.h"
#include "ufo.h"
#include "ufosim.h"

int main(){
    int usereingabe;

    //Vertical vert("1");
    Vertical* vert = new Vertical("1");
    //Ballistic ball("1", 20.0, 20.0);
    Ballistic* ball = new Ballistic("1", 30.0, 60.0);
    
    vector<Ufo*> ufos {vert, ball};



    cout <<"Triff deine Auswahl: \nVertical ----> 0\nBallistic ----> 1" << "\n";
    cin >> usereingabe;

    switch (usereingabe){
    case 0:
        // fly from (0.0, 0.0, 0.0) to (5.0, -1.5, 0.0)
        // at altitude 4.0 with 10 km/h
        ufos[0]->flyToDest(5.0, -1.5, 4.0, 10);

        // fly from (5.0, -1.5, 0.0) to (-3.0, 0.0, 0.0)
        // at altitude 8.0 with 5 km/h
        ufos[0]->flyToDest(-3.0, 0.0, 8.0, 5);
        break;
    case 1:
        // fly from (0.0, 0.0, 0.0) to (5.0, -1.5, 0.0)
        // at altitude 4.0 with 10 km/h
        ufos[1]->flyToDest(-15.0, 1.5, 8.0, 10);

        // fly from (5.0, -1.5, 0.0) to (-3.0, 0.0, 0.0)
        // at altitude 8.0 with 5 km/h
        ufos[1]->flyToDest(-3.0, 0.0, 8.0, 5);
        break;
    default:
        cout << "Falsche Eingabe \n";
        break;
    }


    delete vert;
    delete ball;

    return 0;
}