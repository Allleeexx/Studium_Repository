
#include "quad.h"

Quadrat::Quadrat(const float pXLO, const float pYLO, const float pLength){
    xLO = pXLO;
    yLO = pYLO;
    if(pLength<=0.0 ){
        length = 1.0;
    }else{
        length  = pLength;
    }
}
Quadrat::Quadrat(const Quadrat& quadrat){

}

float Quadrat::getXLO() const{
    return xLO;
}

float Quadrat::getYLO() const{
    return yLO;
}

float Quadrat::getLength() const{
    return length;
}

const string& Quadrat::getString() const{
    return name;
}

void Quadrat::setString(const string text){
    name = text;
}

int main(){
    
    return 0;
}