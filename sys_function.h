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
#if defined(_WIN32) || defined(_WIN64)
        #include <conio.h>
    #else
        #include <termios.h>
        #include <unistd.h>
#endif
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
    const string BLACK   = "\033[1;30m";
    const string RED   = "\033[1;31m";
    const string GREEN = "\033[1;32m";
    const string YELLOW = "\033[1;33m";
    const string BLUE = "\033[1;34m";
    const string PURPLE = "\033[1;35m";
    const string CYAN = "\033[1;36m";
    const string WHITE = "\033[1;37m";
    const string RESET = "\033[0m";

public:
    DatabaseManager();
    ~DatabaseManager();
    bool connect(const string& host, const string& user, const string& pass, const string& db);

    // Function Declarations
    void login();
    void registration();      //register menu
    bool createAcc(int option);  //1: student, 2: parent, 3: instructor
    void getCurUsr(string username, string password); //get current user
    void payFees();
    void donate();

    void PETC(); // press enter to continue
    void invalidInput();
    void clearScreen();
    void testColor(string str);

    //function with RETURN value
    string getNextID(string tableName,int digitCount); //find biggest account id, and increment by 1
    bool getFeeStatus(string payerAccID); //get fee status
    set<int> parseSelections(const string& input, int maxCount); //to parse input
    int calcAge(string IC); //calculate age based on IC
    string toUpperCase(string str); //convert string to uppercase
    string getHiddenPassword(const string& prompt);

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
    void viewStudents(string instructorID, string classDay);
    void promoteStudents(string instructorID);
    string getNextRank(string rankID);
    string getRankColor(string rankID);
    void studentWithrawal(string instructorID);
    
    //=====ADMIN FUNCTIONS=====
    void adminDashboard();
    void regInstructor(); //register instructor
    void viewSummary();
    void allStudents();
    void allInstructors();
    void nvwReport(); //new vs withdraw report
    void viewWithdrawals(); 
    void performanceOverview();
    string numToMonth(int monthInt);

    //41 functions daaamnnn
};

#endif // SYS_FUNCTION_H