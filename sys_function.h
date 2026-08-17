#ifndef SYS_FUNCTION_H
#define SYS_FUNCITON_H

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <set>
#include <vector>
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
    void createAcc(int option);  //1: student, 2: parent, 3: instructor
    void regStudent(int option, string parentID);  //0 self register, 1 under parent
    void regParent();   //register parent
    void regInstructor(); //register instructor
    void getCurUsr(string username, string password); //get current user
    string getNextID(string tableName,int digitCount); //find biggest account id, and increment by 1
    void adminDashboard();
    void studenDashboard();
    void parentDashboard();
    void instructorDashboard();
    void studentApproval(string instructorID, string classSlot);
    set<int> parseSelections(const string& input, int maxCount); //to parse input
    
    void clearScreen();

};


#endif // SYS_FUNCTION_H