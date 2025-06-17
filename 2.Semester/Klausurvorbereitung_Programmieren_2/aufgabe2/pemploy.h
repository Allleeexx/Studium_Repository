#ifndef PEMPLOY_H
#define PEMPLOY_H

#include <string>
#include <vector>
#include <iostream>
#include "person.h"

using namespace std;
class Employer : public Person{
    private:
       
        float salary;
        static const string company;

    public:
        Employer(const string& name, const string& nachname, const vector<int>& dateofBirth, const float salar);
        static string getCompany();
        float getSalary() const;
        void printEmployee() const;
        void riseSalary( const float prozent);
};
#endif