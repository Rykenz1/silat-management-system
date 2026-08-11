#include <iostream>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include "sys_function.h"

using namespace std;
using namespace sql;

void login(Connection* con);
int main()
{
    
    try {
        DatabaseManager dbm;
    
        //test connection to database
        if (!dbm.connect("tcp://127.0.0.1:3306", "root", "", "silat_db"))
        {
            cerr << "[CRITICAL ERROR] Failed to initialize database connection. Exiting..." << endl;
            return 1;
        }

        char choice;
        bool running=true;
        while (running)
        {
            //display main menu
            cout<<"=====SILAT MANAGEMENT SYSTEM====="<<endl;
            cout<<"[ MAIN MENU ]"<<endl;
            cout<<"  [1] Login Account"<<endl;
            cout<<"  [2] Register New Account"<<endl;
            cout<<"  [3] test function"<<endl;
            cout<<"  [0] Exit"<<endl;
            cout<<"\nSelect an option [0-2]: ";
            cin>>choice;

            switch (choice)
            {
            case '1':
                dbm.login();
                break;
            
            case '2':
                dbm.regAcc();
                break;
                
            case '3':
                cout<<dbm.getNextID("accountID", "account",3)<<endl;
                break;

            case '0':
                cout<<"Adios amigos :)"<<endl;
                running=false;
                break;
            default:
                cout<<"invalid input"<<endl;
            }
        }
        
        

        
        
    }
    catch (const exception& e) {
        // Catches unexpected general C++ exceptions
        cerr << "[FATAL ERROR] An unhandled exception occurred: " << e.what() << endl;
        return 1;
    }
    catch (...) {
        // Catches any non-standard exception
        cerr << "[FATAL ERROR] An unknown error occurred." << endl;
        return 1;
    }

    
    return 0;
}


