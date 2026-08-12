#ifndef SYS_FUNCTION_H
#define SYS_FUNCITON_H

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
using namespace std;
using namespace sql;

class DatabaseManager{
private:
    Connection* con; // Connection stored once as a class attribute
    string currentUser; //account ID
    string userRole;    //admin, instructor, parent, student
public:
    DatabaseManager();
    ~DatabaseManager();
    bool connect(const string& host, const string& user, const string& pass, const string& db);

    // Function Declarations
    void login();
    void registration();      //register menu
    void createAcc();  //create account
    void regStudent();  //register student
    void regParent();   //register parent
    string getCurUsr(string username, string password); //get current user
    string getNextID(string tableName,int digitCount); //find biggest account id, and increment by 1
};


#endif // SYS_FUNCTION_H