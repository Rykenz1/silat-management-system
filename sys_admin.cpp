#include "sys_function.h"

using namespace std;
using namespace sql;

void DatabaseManager::adminDashboard(){
    char choice;
    bool endLoop=false;
    
    while (!endLoop)
    {
        clearScreen();
        cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
        cout << "│                               ADMIN DASHBOARD                               │" << endl;
        cout << "╰─────────────────────────────────────────────────────────────────────────────╯" << endl;
        cout << "\n[ AVAILABLE ACTIONS ]" << endl;
        cout << "   [1] Register Instructor" << endl;
        cout << "   [2] View All Students" << endl;
        cout << "   [3] View All Instructors" << endl;
        cout << "   [4] View Withdrawals" << endl;
        cout << "   [5] Monthly Summary" << endl;
        cout << "   [6] Anual Summary" << endl;
        cout << "   [7] View Performance Overview" << endl;
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
            //register instructor
            regInstructor();
            break;
        
        case '2':
            //view all students
            allStudents();
            break;

        case '3':
            //view all instructors
            allInstructors();
            break;
            
        case '4':
            //view all withdrawals
            viewWithdrawals();
            break;

        case '5':
            //view monthly summary
            viewSummary();
            break;
        
        case '6':
            //view Anually summary
            nvwReport();
            break;
        
        case '7':
            //view student performance
            performanceOverview();
            break;
        
        default:
            invalidInput();
            break;
        }
    }
    
    
}   // admin dashboard

void DatabaseManager::regInstructor(){
    string instructorID;
    string fName;
    string homeAdd;
    string phoneNum;
    string classSlot;
    char choice;
    cout<<"=====REGISTERING INSTRUCTOR====="<<endl;
    
    //create account
    if(!createAcc(3)){
        return;
    };

    cin.ignore();
    cout<<"Enter full name: ";
    getline(cin >> ws, fName);
    cout<<"enter home address: ";
    getline(cin >> ws, homeAdd);
    cout<<"enter phone number: ";
    getline(cin >> ws, phoneNum);


    cout<<"Assign this Instructor to which class?"<<endl;
    cout<<"  [1] Monday (9pm - 11pm)"<<endl;
    cout<<"  [2] Tuesday (9pm - 11pm)"<<endl;
    cout<<"  [3] Wednesday (9pm - 11pm)"<<endl;
    cout<<"  [4] Thursday (9pm - 11pm)"<<endl;
    cout<<"  [5] Friday (9pm - 11pm)"<<endl;
    cout<<"  [6] Saturday (9am - 11am)"<<endl;
    cout<<"  [7] Sunday (9am - 11am)"<<endl;
    cout<<"───────────────────────────────────────────────────────────────"<<endl;
    cout<<"  Select an option [1-7]: ";
    cin>>choice;
    bool validInput=true;

    do
    {
        switch (choice)
        {
        case '1':
            classSlot = "s1";
            validInput=true;
            break;
        
        case '2':
            classSlot = "s2";
            validInput=true;
            break;

        case '3':
            classSlot = "s3";
            validInput=true;
            break;

        case '4':
            classSlot = "s4";
            validInput=true;
            break;

        case '5':
            classSlot = "s5";
            validInput=true;
            break;

        case '6':
            classSlot = "s6";
            validInput=true;
            break;

        case '7':
            classSlot = "s7";
            validInput=true;
            break;

        default:
            cout<<"invalid input"<<endl;
            validInput=false;
            break;
        }
        
    } while (validInput==false);
    
    string sqlStatement = "insert into instructor(instructorID, fullName, accountID, homeAdd, phoneNum, joinDate, slotID)"
    "value(?,?,?,?,?,CURDATE(),?)";

    PreparedStatement* pstmt=con->prepareStatement(sqlStatement);

    instructorID=getNextID("instructor",3);
    pstmt->setString(1,instructorID);
    pstmt->setString(2,fName);
    pstmt->setString(3,currentUser);
    pstmt->setString(4,homeAdd);
    pstmt->setString(5,phoneNum);
    pstmt->setString(6,classSlot);

    ResultSet* res= pstmt->executeQuery();

}   //register instructor

void DatabaseManager::viewSummary(){
    int actvInstructor=0;
    int actvStudent=0;
    int newStudent=0;
    int wdrwCount=0;
    double revenue=0;
    double donation=0;
    int month=9, year=2026;

    // //get month, year
    // string getMY = "select month(curdate()), year(curdate())";

    // PreparedStatement* myStmt=con->prepareStatement(getMY);

    // ResultSet* myRes=myStmt->executeQuery();

    // if (myRes->next())  
    // {
    //     month=myRes->getInt(1);
    //     year=myRes->getInt(2);
    // }
    
    bool endLoop = false;
    do
    {
         //get active instructor count
        string getInstructor="select count(*) from account where acc_type='instructor' and approval='approved'";

        PreparedStatement* iStmt=con->prepareStatement(getInstructor);

        ResultSet* iRes=iStmt->executeQuery();

        if(iRes->next()){
            actvInstructor=iRes->getInt(1); //get the first column of result
        }

        delete iStmt;
        delete iRes;

        //get active student and new student count
        string getStudent="select count(*) as totalStudent, count(case when month(joindate)= ? AND YEAR(joinDate) = ? then studentID else null end) as NewStudent from student where stdStatus='active'";

        PreparedStatement* sStmt=con->prepareStatement(getStudent);
        sStmt->setInt(1,month);
        sStmt->setInt(2,year);

        ResultSet* sRes=sStmt->executeQuery();

        if(sRes->next()){
            actvStudent=sRes->getInt("totalStudent");
            newStudent=sRes->getInt("NewStudent");
        }
        delete sStmt;
        delete sRes;

        //get revenue and donation
        string getRevenue= "SELECT SUM(amount) AS total_amount, SUM(CASE WHEN type = 'donation' THEN amount ELSE 0 END) AS total_donation FROM payment WHERE MONTH(paymentDate) = ? AND YEAR(paymentDate) = ?";

        PreparedStatement* rStmt=con->prepareStatement(getRevenue);
        rStmt->setInt(1,month);
        rStmt->setInt(2,year);

        ResultSet* rRes=rStmt->executeQuery();

        if(rRes->next()){
            revenue=rRes->getDouble("total_amount");
            donation=rRes->getDouble("total_donation");
        }
        delete rStmt;
        delete rRes;

        //get withdrawal count
        string getWdrw="select count(*) from withdraw where month(wthDate) = ? and year(wthDate) = ? and wthStatus ='approved'";

        PreparedStatement* wStmt=con->prepareStatement(getWdrw);
        wStmt->setInt(1,month);
        wStmt->setInt(2,year);
        
        ResultSet* wRes=wStmt->executeQuery();

        if(wRes->next()){
            wdrwCount=wRes->getInt(1);
        }

        delete wStmt;
        delete wRes;

        clearScreen();

        cout << "\n╭─────────────────────────────────────────────────────────────╮" << endl;
        cout << "│                        MONTHLY SUMMARY                      │" << endl;
        cout << "╰─────────────────────────────────────────────────────────────╯" << endl;
        
        cout << "[ "<<numToMonth(month)<<" "<<year<<" ]"<< endl;
        cout << "  • Revenue  : "<<GREEN<<"RM"<< revenue <<RESET<< endl;
        cout << "  • Donation : "<<GREEN<<"RM"<< donation<<RESET << endl;
        cout << "\n  • Total Active Instructors : "<< actvInstructor <<" Instructors"<< endl;
        cout << "  • Total Active Students    : "<< actvStudent<<" Students"<< endl;
        cout << "  • New Student Count        : "<<GREEN<< newStudent<<" New Students"<< RESET<<endl;
        cout << "  • Withdrawal Count         : "<<RED<< wdrwCount <<" Withdrawn"<<RESET<< endl;
        cout<<"\n───────────────────────────────────────────────────────────────" << endl;
        cout<<"    [1] Change date"<<endl;
        cout<<"    [0] Exit"<<endl;
        cout<<"\n───────────────────────────────────────────────────────────────" << endl;
        cout << "   Select an option: ";

        string choice;
        getline(cin>>ws,choice);

        if(choice == "0") endLoop=true;
        else if(choice =="1"){
            cout<<"Enter month: ";
            cin>>month;
            cout<<"Enter year : ";
            cin>>year;
        }
    } while (!endLoop);
    

   

    PETC();

}   //view summary

void DatabaseManager::allStudents() {
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

    // Main Header
    clearScreen();

    cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
    cout << "│                                ALL STUDENTS                                 │" << endl;
    cout << "╰─────────────────────────────────────────────────────────────────────────────╯" << endl;
    cout << "  • Total Students : " << studentList.size() << endl;
    cout << "  • Grouped by Rank (Highest → Lowest) | Sorted by Age (Oldest → Youngest)\n" << endl;

    string currentRank = "";

    for (size_t i = 0; i < studentList.size(); ++i) {
        student st = studentList[i];

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

        if(st.fullName.length()>35){
            st.fullName=st.fullName.substr(0, 32)+"...";
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

}   //all students

void DatabaseManager::allInstructors(){
    //need name, class day, student count
    struct instructor{
        string fullName;
        string classSlot;
        int studentCount=0;
    };

    vector<instructor> instructorList;

    string instructorSql = 
        "select i.fullName, sl.classDay, "
        "count(s.instructorID) as studentCount "
        "from instructor i "
        "left join student s on i.instructorID = s.instructorID "
        "left join slot sl on sl.slotID = i.slotID "
        "group by i.fullName "
        "order by sl.slotID asc, studentCount desc "
    ;

    PreparedStatement* iStmt=con->prepareStatement(instructorSql);

    ResultSet* iRes=iStmt->executeQuery();

    if (iRes->rowsCount() == 0)
    {
        cout<<YELLOW<<"\n[ NOTICE ] "<<RESET<<"No instructor at all. "<<endl;
        PETC();
        return;
    }
    
    while (iRes->next())
    {
        instructor ins;

        ins.fullName=iRes->getString("fullName");
        ins.classSlot=iRes->getString("classDay");
        ins.studentCount=iRes->getInt("studentCount");

        instructorList.push_back(ins);
    }

    delete iStmt;
    delete iRes;
    
    //rendering
    clearScreen();
    cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
    cout << "│                               ALL INSTRUCTORS                               │" << endl;
    cout << "╰─────────────────────────────────────────────────────────────────────────────╯" << endl;
    cout << "  • Total Instructors: " <<GREEN<< instructorList.size()<<RESET<<endl;

    string currentDay="";
    
    //display in table list
    for (size_t i = 0; i < instructorList.size(); i++)
    {
        instructor il = instructorList[i];

        //print header
        if(il.classSlot != currentDay){
            currentDay = il.classSlot;

            // Count instructor in this day group
            int instructorsInDay =0;
            int studentsInDay =0;

            string getCountSql = 
                "select "
                "sl.classDay, "
                "count(distinct i.instructorID) as instructorCount, "
                "count(distinct s.studentID) as studentCount "
                "from slot sl "
                "left join instructor i on i.slotID = sl.slotID "
                "left join student s on s.slotID = sl.slotID "
                "where sl.classDay = ? "
                "group by sl.classDay"
            ;
            PreparedStatement* cStmt=con->prepareStatement(getCountSql);
            cStmt->setString(1,currentDay);

            ResultSet* cRes=cStmt->executeQuery();

            if (cRes->next())
            {
                instructorsInDay = cRes->getInt("instructorCount");
                studentsInDay = cRes->getInt("studentCount");
            }
            

            cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
            cout << "│ "<< WHITE <<"[ DAY: " << left << setw(10) << (toUpperCase(currentDay) + " ]") << RESET
                 << left << setw(10)<<YELLOW<<"[ " << instructorsInDay << "/3 INSTRUCTOR(S) ]"<<RESET 
                 << left << setw(10)<<BLUE<<"[ " << studentsInDay << "/30 STUDENT(S) ]"<<RESET 
                 << right << setw(16) << "│" << endl;
            cout << "├─────────────────────────────────────────────────────────────┬───────────────┤" << endl;
            cout << "│ " << left << setw(59) << "Name"
                 << " │ " << left << setw(12) << "Student Count"
                 << " │" << endl;
            cout << "├─────────────────────────────────────────────────────────────┼───────────────┤" << endl;

        }
        string textColor = (il.studentCount >= 10) ? RED : 
                (il.studentCount >= 6)  ? YELLOW : GREEN;

        cout << "│ " << left << setw(59) << il.fullName
             << " │ " << textColor << right << setw(13) << (to_string(il.studentCount) + " / 10")
             << RESET << " │" << endl;

        if( i == instructorList.size() - 1 || instructorList[i+1].classSlot != currentDay){
            cout << "╰─────────────────────────────────────────────────────────────┴───────────────╯" << endl;
        }

    }

    PETC();
    

}   //all instructors

void DatabaseManager::nvwReport(){

    int curYear=0;
    int curMonth=0;
    int selectedYear=2026;
    
    struct month{
        int year;
        int month;
        int newCount=0;
        int withdrawCount=0;
    };

    string nwSql = 
        "select "
        "year(curdate()) as curYear, "
        "month(curdate()) as curMonth, "
        "? as year, "
        "? as month, "
        "(select count(studentID) "
        "   from student where month(joinDate) = month "
        "   and year(joinDate)=year "
        ") as newCount, "
        "(select count(studentID) "
        "   from withdraw where month(wthDate) = month "
        "   and year(wthDate)=year "
        "   and wthStatus = 'approved' "
        ") as withdrawCount "
    ;
    
    vector<month> monthList;

    do
    {
        int totalNew=0;
        int totalWithdraw=0;

        monthList.clear();
        //insert data into vector

        for(size_t i=1;i<=12;++i){
            int thisMonth=i;
            int thisYear=selectedYear;

            
            PreparedStatement* nwStmt=con->prepareStatement(nwSql);
            nwStmt->setInt(1,thisYear);
            nwStmt->setInt(2,thisMonth);

            ResultSet* nwRes=nwStmt->executeQuery();

            if(nwRes->next()){
                month m;

                curYear = nwRes->getInt("curYear");
                curMonth = nwRes->getInt("curMonth");
                m.month = nwRes->getInt("month");
                m.year = nwRes->getInt("year");
                m.newCount = nwRes->getInt("newCount");
                totalNew += m.newCount;
                m.withdrawCount= nwRes->getInt("withdrawCount");
                totalWithdraw += m.withdrawCount;
                monthList.push_back(m);
            }

            delete nwStmt;
            delete nwRes;

        }

        int loopCount=0;
        if(selectedYear == curYear){
            loopCount = curMonth; //display up to current month
        }else{
            loopCount =12; //display all month
        }

        //rendering
        clearScreen();
        cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
        cout << "│                          ANNUAL NEW STUDENT REPORT                          │" << endl;
        cout << "╰─────────────────────────────────────────────────────────────────────────────╯" << endl;
        
        cout<<WHITE<<"\nYEAR: [ "<<selectedYear<<" ]"<<RESET<<endl;
        cout<<"       Total New Students: "<<GREEN<<totalNew<<RESET<<endl;
        cout<<"Total Withdrawed Students: "<<RED<<totalWithdraw<<RESET<<endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        
        
        for(int i=0;i<loopCount;++i){
            month ml = monthList[i];
            cout<<endl;
            cout<<BLUE<<"[ "<<numToMonth(ml.month)<<" ]"<<RESET<<endl;
            cout<<"       New Student : ";
            for (int j=0;j<ml.newCount;++j) cout<<GREEN<<"█"<<RESET;
            cout<<" ["<<ml.newCount<<"]"<<endl;

            cout<<"Withdrawed Student : ";
            for (int j=0;j<ml.withdrawCount;++j) cout<<RED<<"█"<<RESET;
            cout<<" ["<<ml.withdrawCount<<"]"<<endl;
        }
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "    [1] Change Year"<<endl;
        cout << "    [0] Exit"<<endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "   Select an option: ";
        string choice;
        getline(cin>>ws,choice);

        if(choice == "0") return;
        else if(choice == "1"){
            cout<<"Enter year: ";
            cin>>selectedYear;
        } 
    } while (true);
    
    PETC();
    
    

}   //new vs withdraw Report

void DatabaseManager::viewWithdrawals() { //made with gemini
    struct withdrawalRecord {
        string name;
        string date;
        string reason;
    };

    vector<withdrawalRecord> records;

    string query = 
        "SELECT "
        "    s.fullName AS name, "
        "    w.wthDate AS date, "
        "    w.reason AS reason "
        "FROM withdraw w "
        "JOIN student s ON w.studentID = s.studentID "
        "where wthStatus = 'approved'"
        "ORDER BY w.wthDate DESC";

    
        PreparedStatement* pstmt=con->prepareStatement(query);
        ResultSet* res=pstmt->executeQuery();

    while (res->next()) {
        withdrawalRecord wr;

        wr.name = res->getString("name");
        wr.date = res->getString("date");
        wr.reason = res->getString("reason");
        records.push_back(wr);
    }
   

    // Header Banner
    clearScreen();
    cout << "\n╭────────────────────────────────────────────────────────────────────────────────────────╮" << endl;
    cout << "│                                   STUDENT WITHDRAWALS                                  │" << endl;
    cout << "╰────────────────────────────────────────────────────────────────────────────────────────╯" << endl;

    if (records.empty()) {
        cout << "\n  No withdrawal records found.\n" << endl;
        return;
    }

    // Table View
    cout << "╭─────────────────────────────────────┬────────────┬─────────────────────────────────────╮" << endl;
    cout << "│ " << left << setw(35) << "STUDENT NAME" 
         << " │ " << setw(10) << "DATE" 
         << " │ " << setw(35) << "REASON" << " │" << endl;
    cout << "├─────────────────────────────────────┼────────────┼─────────────────────────────────────┤" << endl;

    for (int i = 0 ; i<records.size(); i++) {
        withdrawalRecord row = records[i];
        
        // Truncate long reasons if they exceed column width to keep table borders aligned
        string displayReason = row.reason;
        if (displayReason.length() > 35) {
            displayReason = displayReason.substr(0, 32) + "...";
        }

        cout << "│ " << left << setw(35) << row.name 
             << " │ " << left << setw(10) << row.date 
             << " │ " << left << setw(35) << displayReason << " │" << endl;
    }

    cout << "╰─────────────────────────────────────┴────────────┴─────────────────────────────────────╯\n" << endl;
    PETC();
}

void DatabaseManager::performanceOverview() {
    //get student name, rank1 date achieve, rank2 date achieve, daysTaken
    string getPerfSql=
    "select "
    "s.fullName, "
    "rh1.rankID as rank1, "
    "rh2.rankID as rank2, "
    "rh1.date_achieved as startDate, "
    "rh2.date_achieved as endDate, "
    "datediff(rh2.date_achieved, rh1.date_achieved) as daysTaken "
    "from student s "
    "join rank_history rh1 on rh1.studentID= s.studentID "
    "and rh1.rankID=? "
    "join rank_history rh2 on rh2.studentID= s.studentID "
    "and rh2.rankID=? "
    "order by daysTaken asc"
    ;
    struct student{
        string fullName;
        string r1; //rank1
        string r2; //rank2
        string startDate;
        string endDate;
        int daysTaken;
    };
    
    string rank1="r1";
    string rank2="r2";

    while(true){

        PreparedStatement* gpStmt=con->prepareStatement(getPerfSql);
        gpStmt->setString(1,rank1);
        gpStmt->setString(2,rank2);

        ResultSet* gpRes=gpStmt->executeQuery();

        //store in vector
    
        vector<student> studentList;
        studentList.clear();
        int avgDay=0;
        while(gpRes->next()){
            student s;

            s.fullName=gpRes->getString("fullName");
            s.r1=gpRes->getString("rank1");
            s.r2=gpRes->getString("rank2");
            s.startDate=gpRes->getString("startDate");
            s.endDate=gpRes->getString("endDate");
            s.daysTaken=gpRes->getInt("daysTaken");
            avgDay+=s.daysTaken;

            studentList.push_back(s);
        }
        

        delete gpStmt;
        delete gpRes;

        if(studentList.empty()){
            cout<<YELLOW<<"[ NOTICE ] "<<RESET<<"No student"<<endl;
        }else{
            avgDay /= studentList.size();
        }


        //display in table (eg: white->blue)
        clearScreen();
        cout << "\n╭────────────────────────────────────────────────────────────────────────────────────────╮" << endl;
        
        cout<<"│ "<<WHITE<<left<<setw(40)<<toUpperCase("[ "+getRankColor(rank1) + " --> " +getRankColor(rank2) + " ]")<<RESET
            <<BLUE<<left<<setw(46)<<("[ AVERAGE DAYS: "+ to_string(avgDay) +" ]")<<RESET
            <<" │"<<endl;
        cout << "├──────────────────────────────────────────────────┬──────────────┬──────────────┬───────┤" << endl;
        cout<<"│ "<<left<<setw(48) <<"Name"
            <<" │ "<<right<<setw(12)<<"Start"
            <<" │ "<<right<<setw(12)<<"End"
            <<" │ "<<right<<setw(5)<<"Days"
            <<" │"<< endl;
        cout << "├──────────────────────────────────────────────────┼──────────────┼──────────────┼───────┤" << endl;
        
        for(int i=0; i<studentList.size();++i){
            student sl = studentList[i];

            cout <<"│ "<<left<<setw(48)<< sl.fullName
                 <<" │ "<<right<<setw(12)<<sl.startDate
                 <<" │ "<<right<<setw(12)<<sl.endDate
                 <<" │ "<<right<<setw(5)<<sl.daysTaken
                 <<" │"<< endl;
        }
        cout << "╰──────────────────────────────────────────────────┴──────────────┴──────────────┴───────╯" << endl;

        //allow to change viewed ranks
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout<<"[ AVAILABLE OPTION ]"<<endl;
        cout<<"   [1] White --> Blue"<<endl;
        cout<<"   [2] Blue --> Green"<<endl;
        cout<<"   [3] Green --> Yellow"<<endl;
        cout<<"   [4] Yellow --> Orange"<<endl;
        cout<<"   [5] Orange --> Red"<<endl;
        cout<<"   [6] Red --> Black"<<endl;
        cout<<"   [0] Exit"<<endl;
        
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout<<"Select an option [0-6]: ";
        string choice;
        getline(cin>>ws,choice);

        if(choice=="0") return;
        else if(choice == "1"){
            rank1="r1";
            rank2="r2";
        }else if(choice == "2"){
            rank1="r2";
            rank2="r3";
        }else if(choice == "3"){
            rank1="r3";
            rank2="r4";
        }else if(choice == "4"){
            rank1="r4";
            rank2="r5";
        }else if(choice == "5"){
            rank1="r5";
            rank2="r6";
        }else if(choice == "6"){
            rank1="r6";
            rank2="r7";
        }
    }
    
}   //performance overview

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