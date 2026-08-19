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
            viewSummary();
            break;

        case '5':
            /* code */
            break;
        
        default:
            invalidInput();
            break;
        }
    }
    
    
}   // admin dashboard

void DatabaseManager::viewSummary(){
    int actvInstructor=0;
    int actvStudent=0;
    int newStudent=0;
    int wdrwCount=0;
    double revenue=0;
    double donation=0;
    string month, year;

    //get month, year
    string getMY = "select month(curdate()), year(curdate())";

    PreparedStatement* myStmt=con->prepareStatement(getMY);

    ResultSet* myRes=myStmt->executeQuery();

    if (myRes->next())  
    {
        month=numToMonth(myRes->getInt(1));
        year=myRes->getString(2);
    }
    

    //get active instructor count
    string getInstructor="select count(*) from account where acc_type='instructor' and approval='approved'";

    PreparedStatement* iStmt=con->prepareStatement(getInstructor);

    ResultSet* iRes=iStmt->executeQuery();

    if(iRes->next()){
        actvInstructor=iRes->getInt(1); //get the first column of result
    }

    //get active student and new student count
    string getStudent="select count(*) as totalStudent, count(case when month(joindate)= month(curdate()) then studentID else 0 end) as NewStudent from student where stdStatus='active'";

    PreparedStatement* sStmt=con->prepareStatement(getStudent);

    ResultSet* sRes=sStmt->executeQuery();

    if(sRes->next()){
        actvStudent=sRes->getInt("totalStudent");
        newStudent=sRes->getInt("NewStudent");
    }

    //get revenue and donation
    string getRevenue= "SELECT SUM(amount) AS total_amount, SUM(CASE WHEN type = 'donation' THEN amount ELSE 0 END) AS total_donation FROM payment WHERE MONTH(paymentDate) = MONTH(CURDATE()) AND YEAR(paymentDate) = YEAR(CURDATE())";

    PreparedStatement* rStmt=con->prepareStatement(getRevenue);

    ResultSet* rRes=rStmt->executeQuery();

    if(rRes->next()){
        revenue=rRes->getDouble("total_amount");
        donation=rRes->getDouble("total_donation");
    }

    //get withdrawal count
    string getWdrw="select count(*) from withdraw where month(wthDate) = month(curdate()) and year(wthDate) = year(curdate()) and wthStatus ='approved'";

    PreparedStatement* wStmt=con->prepareStatement(getWdrw);

    ResultSet* wRes=wStmt->executeQuery();

    if(wRes->next()){
        wdrwCount=wRes->getInt(1);
    }

    clearScreen();

    cout << "\n┌─────────────────────────────────────────────────────────────┐" << endl;
    cout << "│                        MONTHLY SUMMARY                      │" << endl;
    cout << "└─────────────────────────────────────────────────────────────┘" << endl;
    
    cout << "[ "<<month<<" "<<year<<" ]"<< endl;
    cout << "  • Revenue  : "<<GREEN<<"RM"<< revenue <<RESET<< endl;
    cout << "  • Donation : "<<GREEN<<"RM"<< donation<<RESET << endl;
    cout << "\n  • Total Active Instructors : "<< actvInstructor <<" Instructors"<< endl;
    cout << "  • Total Active Students    : "<< actvStudent<<" Students"<< endl;
    cout << "  • New Student Count        : "<<GREEN<< newStudent<<" New Students"<< RESET<<endl;
    cout << "  • Withdrawal Count         : "<<RED<< wdrwCount <<" Withdrawn"<<RESET<< endl;

    PETC();

    delete iStmt;
    delete rStmt;
    delete sStmt;
    delete wStmt;
    delete iRes;
    delete rRes;
    delete sRes;
    delete wRes;
}   //view summary

string DatabaseManager::numToMonth(int monthInt){
    switch (monthInt)
    {
    case 1: return "JANUARY"; break;
    case 2: return "FEBRUARY"; break;
    case 3: return "MARCH"; break;
    case 4: return "APRIL"; break;
    case 5: return "MAY"; break;
    case 6: return "JUNE"; break;
    case 7: return "JULY"; break;
    case 8: return "AUGUST"; break;
    case 9: return "SEPTEMBER"; break;
    case 10: return "OCTOBER"; break;
    case 11: return "NOVEMBER"; break;
    case 12: return "DECEMBER"; break;
    
    default:
        break;
    }
    
    return "";
}   //number to month