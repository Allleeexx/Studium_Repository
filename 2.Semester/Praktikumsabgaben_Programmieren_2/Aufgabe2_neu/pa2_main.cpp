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
    Vertical vert = new Vertical("1");
    //Ballistic ball("1", 20.0, 20.0);
    Ballistic ball = new Ballistic("1", 20.0, 20.0);
    
    vector<Ufo*> ufos {vert, ball};



    cout <<"Triff deine Auswahl: \nVertical ----> 0\nBallistic ----> 1" << "\n";
    cin >> usereingabe;

        if(usereingabe == 0 || usereingabe == 1){
            if(usereingabe == 0){            //Dann mit Vert
                ufos[0].flyToDest(5.0, -1.5, 4.0, 10);
                ufos[0].flyToDest(-3.0, 0.0, 8.0, 5);
            }else{
                ufos[1].flyToDest(5.0, -1.5, 4.0, 10);
                ufos[1].flyToDest(-3.0, 0.0, 8.0, 5);
            }
        }else{
            cout << "Falsche Eingabe";
        }
    
/*
    switch (usereingabe)
    {
    case 0:
        // fly from (0.0, 0.0, 0.0) to (5.0, -1.5, 0.0)
        // at altitude 4.0 with 10 km/h
        vert.flyToDest(5.0, -1.5, 4.0, 10);

        // fly from (5.0, -1.5, 0.0) to (-3.0, 0.0, 0.0)
        // at altitude 8.0 with 5 km/h
        vert.flyToDest(-3.0, 0.0, 8.0, 5);
        break;
    case 1:
        // fly from (0.0, 0.0, 0.0) to (5.0, -1.5, 0.0)
        // at altitude 4.0 with 10 km/h
        ball.flyToDest(5.0, -1.5, 4.0, 10);

        // fly from (5.0, -1.5, 0.0) to (-3.0, 0.0, 0.0)
        // at altitude 8.0 with 5 km/h
        ball.flyToDest(-3.0, 0.0, 8.0, 5);
        break;
    default:
        break;
    }
*/

    return 0;
}