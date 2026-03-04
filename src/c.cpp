#include<iostream>
#include <map>
#include <vector>

using namespace std;

int main()

{
    map < string, vector<int>> marksheet;
    while(true)
    cout <<"1. Add new student\n";
    cout <<"2. Add marks to existing student\n";
    cout <<"3. Display all students\n";
    cout <<"4. Find average marks of a student\n";
    cout <<"5. Remove a student\n"; 
    cout <<"6. Exit\n";
    int a;
    cin >> a ;
    switch (a)
    {
    case 1:
        cout<<"add student";
        string name;
        cin>> name;
        vector<int> marks;
        int a;
        for (int i = 0; i < 5; i++)
        {
            cin >> a ; 
            marks.push_back()
        }
        marksheet[name] = marks;
        break;
    case 2:
        marksheet[name]
    case 3:
        for (auto it = marksheet.begin(); it != marksheet.end(); i++)
        {
            cout<< it->first <<endl;
            for(auto itt = it->second.begin(); itt != it->second.end(); itt++)
            cout<< *itt<<endl;
        }

    case 4:
        cout <<"display the avg student\n"
        string name;
        cin >> name;
        for ( auto marks : marksheet[mark])
        {   
            
        }
        break;
    case 5:
        cout <<"remove the f student\n"
        string name;
        cin >> name;
        marksheet.erase(name);
        break;
    case 6:
        exit(0);
        break;
    default:
    cout << "wrong value\n";
        break;
    }





}