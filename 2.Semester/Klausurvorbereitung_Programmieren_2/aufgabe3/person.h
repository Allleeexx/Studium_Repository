#ifndef PERSON_H
#define PERSON_H

#include <vector>
#include <string>
#include <cassert>
#include <ctime>
#include <iostream>
using namespace std;

class Person{
    private:
        string name;
        string firstName;
        vector<int> dateOfBirth;
    public:
        Person(const string& pName, const string& pFirstName, const vector<int>& pDateOfBirth);
        void print() const;
        int age() const;
};

#endif