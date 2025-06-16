#include "person.h"

Person::Person(const string& name, const string& nachname, const vector<int>& dateofBirth):name(name), firstName(nachname), dateOfBirth(dateofBirth){}

int Person::age() const{
    assert(dateOfBirth.size() == 3);
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    int year = t->tm_year + 1900;
    int month = t->tm_mon + 1;
    int day = t->tm_mday;

    int res = year - dateOfBirth[0] - 1;
    if (month > dateOfBirth[1] || (month == dateOfBirth[1] && day >= dateOfBirth[2]))
        res = res + 1;
    return res;
}

void Person::print() const{
    cout << "Name: " <<name <<"\n";
    cout << "NachN: " <<firstName <<"\n";
    cout << "Date: " <<dateOfBirth.at(0)<<"."<<dateOfBirth.at(1)<<"."<<dateOfBirth.at(2) <<"\n";
    cout << "Salary: " <<salary <<"\n\n\n";
}