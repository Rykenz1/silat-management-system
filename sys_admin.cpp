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

void DatabaseManager::viewStudents() {
    struct student {
        string studentID;
        string fullName;
        int age;
        string classSlot;
        string feeStatus;
        string rank;
        int rankLevel;
    };

    vector<student> studentList;

    // Single query joining Student Info, Latest Rank, and Current Month Fee Status
    string getInfoSql = 
        "SELECT "
        "    s.studentID, s.fullName, s.ic, sl.classDay, r.value, "
        "    COALESCE(r.color, 'N/A') AS rankColor, "
        "    COALESCE(s.accountID, p.accountID) AS payer_accountID, "
        "    CASE WHEN COUNT(pay.paymentID) > 0 THEN 'PAID' ELSE 'UNPAID' END AS fee_status "
        "FROM student s "
        "LEFT JOIN parent p ON s.parentID = p.parentID "
        "LEFT JOIN payment pay ON pay.accountID = COALESCE(s.accountID, p.accountID) "
        "    AND pay.type = 'fee' "
        "    AND MONTH(pay.paymentDate) = MONTH(CURRENT_DATE()) "
        "    AND YEAR(pay.paymentDate) = YEAR(CURRENT_DATE()) "
        "LEFT JOIN rank_history rh ON s.studentID = rh.studentID "
        "    AND rh.date_achieved = ( "
        "        SELECT MAX(rh2.date_achieved) "
        "        FROM rank_history rh2 "
        "        WHERE rh2.studentID = s.studentID "
        "    ) "
        "LEFT JOIN rank r ON rh.rankID = r.rankID "
        "LEFT JOIN slot sl ON s.slotID = sl.slotID "
        "    WHERE s.stdStatus='active' "
        "GROUP BY s.studentID, s.fullName, s.ic, sl.classDay, r.color, COALESCE(s.accountID, p.accountID)"
        "ORDER BY r.rankID desc, s.ic asc";

    PreparedStatement* infoStmt = con->prepareStatement(getInfoSql);
    ResultSet* infoRes = infoStmt->executeQuery();

    while (infoRes->next()) {
        student st;

        st.studentID = infoRes->getString("studentID");
        st.fullName  = infoRes->getString("fullName");
        st.age       = calcAge(infoRes->getString("ic"));
        st.classSlot = infoRes->getString("classDay");
        st.rank      = infoRes->getString("rankColor");
        st.feeStatus=(getFeeStatus(infoRes->getString("payer_accountID")) ? (GREEN + "[ PAID ]" + RESET) : (RED + "[ UNPAID ]" + RESET));
        st.rankLevel = infoRes->getInt("value");

        studentList.push_back(st);

        cout << "\n" << st.fullName << endl;
        cout << st.age << endl;
        cout << st.rank << endl;
        cout << st.classSlot << endl;
        cout << st.feeStatus << endl;
    }

    // Clean up memory
    delete infoRes;
    delete infoStmt;

    if (studentList.empty()) {
        cout << "\n  No students registered.\n";
        return;
    }

    // Sort: 1st by Rank Level (Highest to Lowest), 2nd by Age (Oldest to Youngest)
    sort(studentList.begin(), studentList.end(), [](const student& a, const student& b) {
        if (a.rankLevel != b.rankLevel) {
            return a.rankLevel > b.rankLevel; // Higher rank first
        }
        return a.age > b.age; // Older student first within same rank
    });

    // Main Header
    clearScreen();

    cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
    cout << "│                                ALL STUDENTS                                 │" << endl;
    cout << "╰─────────────────────────────────────────────────────────────────────────────╯" << endl;
    cout << "  • Total Students : " << studentList.size() << endl;
    cout << "  • Grouped by Rank (Highest → Lowest) | Sorted by Age (Oldest → Youngest)\n" << endl;

    string currentRank = "";

    for (size_t i = 0; i < studentList.size(); ++i) {
        const auto& st = studentList[i];

        // If new rank group encountered, print group header & table headers
        if (st.rank != currentRank) {
            currentRank = st.rank;

            // Count students in this rank group
            int countInRank = count_if(studentList.begin(), studentList.end(), [&](const student& s) {
                return s.rank == currentRank;
            });

            cout << "╭────────────────────────────────────────────────────────────────────────────────────────╮" << endl;
            cout << "│  [ RANK: " << left << setw(15) << (currentRank + " ]") 
                 <<left << setw(10)<<YELLOW<<"[ " << countInRank << " STUDENT(S) ]"<<RESET 
                 << right << setw(47) << "│" << endl;
            cout << "├───────┬─────────────────────────────────────┬─────┬─────────────┬──────────────────────┤" << endl;
            cout << "│ ID    │ NAME                                │ AGE │ CLASS SLOT  │ FEE STATUS           │" << endl;
            cout << "├───────┼─────────────────────────────────────┼─────┼─────────────┼──────────────────────┤" << endl;
        }

        // Print student row (setw(35) for Name)
        cout << "│ " << left  << setw(5)  << st.studentID 
             << " │ " << left  << setw(35) << st.fullName 
             << " │ " << right << setw(3)  << st.age 
             << " │ " << left  << setw(11) << st.classSlot 
             << " │ " << left  << setw(32) << st.feeStatus << "│" << endl;

        // Close table card when rank changes or on the last entry
        if (i == studentList.size() - 1 || studentList[i + 1].rank != currentRank) {
            cout << "╰───────┴─────────────────────────────────────┴─────┴─────────────┴──────────────────────╯\n" << endl;
        }
    }

    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    PETC();

}

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