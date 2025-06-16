#include "pemploy.h"

const string Employer::company = "AUDI AG";

Employer::Employer(const string& name, const string& nachname, const vector<int>& dateofBirth, const float salar):name(name), firstName(nachname), dateOfBirth(dateofBirth), salary(salar){}

Employer::~Employer(){}

void Employer::printEmployee() const{
    cout << "Name: " <<name <<"\n";
    cout << "NachN: " <<firstName <<"\n";
    cout << "Date: " <<dateOfBirth.at(0)<<"."<<dateOfBirth.at(1)<<"."<<dateOfBirth.at(2) <<"\n";
    cout << "Salary: " <<salary <<"\n\n\n";
}

void Employer::riseSalary(const float prozent){
    float erhoehung = (salary*prozent)/100;
    cout <<"Erhoeht um: "<< erhoehung <<" €" <<"\n\n\n";

    salary += erhoehung;
}

string Employer::getCompany(){
    return company;
}

float Employer::getSalary() const{
    return salary;
}