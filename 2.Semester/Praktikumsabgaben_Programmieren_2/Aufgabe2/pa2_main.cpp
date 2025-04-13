#include <iostream>
#include <cmath>
using namespace std;

#include "vertical.h"
#include "ballistic.h"
#include "ufo.h"
#include "ufosim.h"

int main(){
    vert = new Vertical();
    ball = new Ballistic();

    int eingabe = 0;
    cout << "Gebe eine Zahl ein. Entweder 0 oder 1" <<"\n";
    cin >> eingabe;
    return 0;
}