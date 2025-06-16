#ifndef PEMPLOY_H
#define PEMPLOY_H

#include <string>
#include <vector>
#include <iostream>
#include <cassert>
#include <ctime>

using namespace std;
class Employer{
    private:
        string name;
        string firstName;
        vector <int> dateOfBirth;
        float salary;
        static const string company;

    public:
        Employer(const string& name, const string& nachname, const vector<int>& dateofBirth, const float salar);
        ~Employer();
        void print() const;
        void riseSalary( const float prozent);
        static string getCompany();
        float getSalary() const;
        int age() const;
};
#endif