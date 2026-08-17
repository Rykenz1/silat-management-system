#include "sys_function.h"

using namespace std;
using namespace sql;

void DatabaseManager::adminDashboard(){
    char choice;
    bool endLoop=false;
    
    while (!endLoop)
    {
        /* code */
        cout << "===== Admin Dashboard =====" << endl;
        cout << "   [1] Register Instructor" << endl;
        cout << "   [2] View all students" << endl;
        cout << "   [3] View withdrawals" << endl;
        cout << "   [4] Monthly Summary" << endl;
        cout << "   [0] Exit" << endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "   Select an option: ";
        cin>>choice;

        switch (choice)
        {
        case '0':
            endLoop=true;
            break;
        
        case '1':
            regInstructor();
            break;
        
        case '2':
            cout<<"case 2"<<endl;
            break;

        case '3':
            /* code */
            break;
            
        case '4':
            /* code */
            break;

        case '5':
            /* code */
            break;
        
        default:
            break;
        }
    }
    
    
}   // admin dashboard