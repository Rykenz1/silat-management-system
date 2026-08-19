#include "sys_function.h"

using namespace std;
using namespace sql;


void DatabaseManager::regStudent(int option,string parentID){
    string userID;
    string fName;
    string ic;
    string homeAdd;
    string phoneNum;
    string classSlot="";
    char choice;
    
    //only create account if self register (option 0)
    if (option == 0)
    {
        cout<<"=====REGISTERING AS A STUDENT====="<<endl;
        createAcc(1);
    }
    
    cin.ignore();
    cout<<"Enter full name: ";
    getline(cin, fName);
    cout<<"enter IC: ";
    getline(cin, ic);

    if (option==0)
    {
        cout<<"enter home address: ";
        getline(cin, homeAdd);
        cout<<"enter phone number: ";
        getline(cin, phoneNum);
    }
    
    cout<<"choose class slot:"<<endl;
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
    
    

    cin.ignore();
    cout<<"confirm registration? y/n: ";
    cin>>choice;
    if(choice=='y' || choice== 'Y'){
        string sqlStatement = "insert into student(studentID, fullName, ic, accountID, homeAdd, phoneNum, joinDate, slotID,parentID)"
        "value(?,?,?,?,?,?,CURDATE(),?,?)";

        PreparedStatement* pstmt= con->prepareStatement(sqlStatement);
        
        userID=getNextID("student",3);
        pstmt->setString(1,userID); //studentID
        pstmt->setString(2,fName);  //full name
        pstmt->setString(3,ic);     //ic
        
        if (option==0) //for self register student
        {
        
            pstmt->setString(4,currentUser); //connect accountID
            pstmt->setString(5,homeAdd);
            pstmt->setString(6,phoneNum);
            pstmt->setNull(8, DataType::VARCHAR);

            

        }else if(option==1){ //for parent under parent
            pstmt->setNull(4, DataType::VARCHAR);
            pstmt->setNull(5, DataType::VARCHAR);
            pstmt->setNull(6, DataType::VARCHAR);
            pstmt->setString(8,parentID);
        }
        pstmt->setString(7,classSlot);

        ResultSet* res= pstmt->executeQuery();

        cout<<"registration success... waiting for instructor approval"<<endl;

        
    }else if(choice== 'n' || choice == 'N'){
        cout<<"registration cancelled"<<endl;
    }else{
        cout<<"invalid input"<<endl;
    }

    // cout<<"full name: "<<fName<<endl;
    // cout<<"ic: "<<ic;
    // cout<<"home addrese: "<<homeAdd<<endl;
    // cout<<"phone number: "<<phoneNum<<endl;
    // cout<<"class slot: "<<classSlot<<endl;

}  //register student


void DatabaseManager::studenDashboard(){
    // string studentID;
    string fName;
    string classSlot;
    string curRank;
    string stdStatus;
    string feeStatus; //paid, unpaid, etc
    char choice;
    
    bool endLoop=false;

    while (!endLoop)
    {
        string sqlStmt = "select st.*, sl.classDay, r.color"
         " from student st"
         " join slot sl on st.slotID = sl.slotID"
         " join rank_history rh on st.studentId = rh.studentID"
         " and rh.date_achieved = (select max(rh2.date_achieved) from rank_history rh2 where rh2.studentID = st.studentID) join rank r on rh.rankID = r.rankID where accountID=?";

        PreparedStatement* pstmt=con->prepareStatement(sqlStmt);

        pstmt->setString(1,currentUser);

        ResultSet* res=pstmt->executeQuery();

        if(res->next()){
            fName=res->getString("fullName");
            classSlot=res->getString("classDay");
            stdStatus=res->getString("stdStatus");
            curRank=res->getString("color");

            userName=fName; // set global variable user Name to student full name
        }

        //get student rank
        string rankStmt="select r.color, rh.date_achieved from rank_history rh"
        " join rank r on rh.rankID = r.rankID"
        " where rh.studentID= (select s.studentID from student s where accountID=?)"
        " order by r.rankID desc";

        PreparedStatement* rstmt=con->prepareStatement(rankStmt);

        rstmt->setString(1,currentUser);

        ResultSet* rankRes=rstmt->executeQuery();

        cout << "===== Student Dashboard =====" << endl;
        cout << "\n[ USER PROFILE ]" << endl;
        cout << "  • Student Name : "<< fName << endl;
        cout << "  • Class Slot   : "<< classSlot << endl;
        cout << "  • Current Rank : "<< curRank << endl;
        cout << "  • Status       : "<< stdStatus << endl;
        cout << "  • Fee Status   : "<< (getFeeStatus() ? (GREEN + "[ PAID ]" + RESET) : (RED + "[ UNPAID ]" + RESET)) << endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "[ RANK PROMOTION HISTORY ]" << endl;

        cout << "    "<<left<<setw(13)<<"color"<<"  Date Achieved"<<endl;
        while (rankRes->next())
        {
            cout<<"  • "<<left<<setw(13)<<rankRes->getString("color")<<": "<<rankRes->getString("date_achieved")<<endl;
        }
        
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "[ AVAILABLE ACTIONS ]" << endl;
        cout << "  [1] Pay Monthly Fees" << endl;
        cout << "  [2] Donate :)" << endl;
        cout << "  [3] Withdraw" << endl;
        cout << "  [0] Exit" << endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "   Select an option: ";
        cin>>choice;

        switch (choice)
        {
        case '0':
            endLoop=true;
            break;

        case '1':
            //payfees;
            cout<<"payfee"<<endl;
            payFees();
            break;
        
        case '2':
            //donate
            cout<<"donate"<<endl;
            donate();
            break;
        
        case '3':
            //withdraw;
            // cout<<"withdraw"<<endl;
            withdrawRequest();
            break;
        
        default:
            cout<<"invalid input"<<endl;
            break;
        }

        delete res;
        delete pstmt;
        delete rankRes;
        delete rstmt;
    }
    

    
}   // student dashboard

void DatabaseManager::withdrawRequest(){
    string studentID;
    string studentName;
    string instructorID;
    string stdStatus;
    string reason;
    char choice;
    bool endLoop=false;

    string profileStmt="select studentID, fullName, stdStatus, instructorID from student where accountID=?";

    PreparedStatement* pstmt=con->prepareStatement(profileStmt);

    pstmt->setString(1,currentUser);

    ResultSet* res=pstmt->executeQuery();

    if(res->next()){
        studentID=res->getString("studentID");
        studentName=res->getString("fullName");
        stdStatus=res->getString("stdStatus");
        instructorID=res->getString("instructorID");
    }

    while (!endLoop)
    {
        clearScreen();

        cout<<RED<<"┌─────────────────────────────────────────────────────────────┐"<<endl;
        cout<<"│                 STUDENT WITHDRAWAL REQUEST                  │"<<endl;
        cout<<"└─────────────────────────────────────────────────────────────┘"<<RESET<<endl;
        cout<<"[ STUDENT DETAILS ]"<<endl;
        cout<<"  • Student ID  : "<<studentID<<endl;
        cout<<"  • Full Name   : "<<studentName<<endl;
        cout<<"  • Status      : "<<stdStatus<<endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout<<"[ NOTICE ]"<<endl;
        cout<<"  • Submitting this form sends a withdrawal request to your instructor for formal review and processing."<<endl;
        cout<<"  • Type '0' or cancel at any time to abort."<<endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "  Please State your reason for withdrawing:\n  >> ";
        
        cin.ignore();
        getline(cin,reason);

        if (reason == "0" || reason == "cancel" || reason == "CANCEL") {
            cout << "\n  " << YELLOW << "[CANCELLED]" << RESET << " Withdrawal request was cancelled.\n" << endl;
            return;
        }
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "  Confirm withdrawal request? (y/n): ";
        cin>>choice;

        if (choice == 'Y' || choice=='y')
        {
            clearScreen();

            cout<<GREEN<<" [ SUCCESS ]"<<RESET<<" Your withdrawal request has been submitted for review!"<<endl;
            endLoop=true;
        }
        
    }

    string insertStmt="insert into withdraw(withdrawID, studentID, instructorID, reason, wthDate)"
    " values(?,?,?,?,CURDATE())";

    PreparedStatement* inStmt=con->prepareStatement(insertStmt);

    inStmt->setString(1,getNextID("withdraw",4));
    inStmt->setString(2,studentID);
    inStmt->setString(3,instructorID);
    inStmt->setString(4,reason);

    inStmt->executeUpdate();

    delete pstmt;
    delete inStmt;
    delete res;
    
}   //withdraw request

string DatabaseManager::studentStatus(string status){

    if ( status =="active"){
        return GREEN + "[ ACTIVE ]" + RESET; 
    } else if (status=="pending"){
        return YELLOW + "[ PENDING ]" + RESET;
    } else if (status == "withdraw" || status == "inactive" || status == "rejected") {
        return RED + "[ WITHDRAWN ]" + RESET;
    }

    return "[ " + status + " ]";
}   //student status
