#include "pemploy.h"

int main(){
    Employer e("Alex", "Koch", {2002,06,25}, 4000.00);
    
    
    e.print();
    cout<<"Gehalt Erhoeht um 10% \n";
    e.riseSalary(10.0);
    e.print();

    return 0;
}