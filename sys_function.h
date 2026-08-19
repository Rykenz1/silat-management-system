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
#include <mysql_connection.h>
#include <mysql_driver.h>
using namespace std;
using namespace sql;

class DatabaseManager{
private:
    Connection* con; // Connection stored once as a class attribute
    string currentUser; //account ID
    string userRole;    //admin, instructor, parent, student
    string userName;    //current user's name
    string entityID;    //s00x , p00x, i00x
    
    //color variables
    const string RED   = "\033[1;31m";
    const string GREEN = "\033[1;32m";
    const string YELLOW = "\033[1;33m";
    const string RESET = "\033[0m";

public:
    DatabaseManager();
    ~DatabaseManager();
    bool connect(const string& host, const string& user, const string& pass, const string& db);

    // Function Declarations
    void login();
    void registration();      //register menu
    void createAcc(int option);  //1: student, 2: parent, 3: instructor
    void getCurUsr(string username, string password); //get current user
    string getNextID(string tableName,int digitCount); //find biggest account id, and increment by 1
    bool getFeeStatus(); //get fee status
    set<int> parseSelections(const string& input, int maxCount); //to parse input
    void payFees();
    void donate();

    void PETC(); // press enter to continue
    void clearScreen();

    //=====STUDENT FUNCTIONS=====
    void regStudent(int option, string parentID);  //0 self register, 1 under parent
    void studenDashboard();
    void withdrawRequest(int option); //0 self-request, 1 parent request

    string studentStatus(string status); //add color to status

    //=====PARENT FUNCTIONS=====
    void regParent();   //register parent
    void parentDashboard();
    void dispChildren(); //display children list
    void editInfo(string& phoneNum, string& h);
    void mngChild();
    
    //=====INSTURCTOR FUNCTIONS=====
    void instructorDashboard();
    void studentApproval(string instructorID, string classSlot);
    
    //=====ADMIN FUNCTIONS=====
    void adminDashboard();
    void regInstructor(); //register instructor


};

#endif // SYS_FUNCTION_H