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
    string choice;
    struct slot{
        string slotID;
        string day;
        int freeSlot=0;
    };
    vector<slot> slotList;
    slotList.clear();
    
    //only create account if self register (option 0)
    if (option == 0)
    {
        cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
        cout << "│                          REGISTERING AS A STUDENT                           │" << endl;
        cout << "╰─────────────────────────────────────────────────────────────────────────────╯" << endl;
        if(!createAcc(1)) return;
    }
    
    
    //check for not full slot
    string checkSlotSql=
        "select "
        "sl.slotID, "
        "sl.classDay, "
        "count(distinct s.studentID) as studentCount "
        "from slot sl "
        "left join student s on s.slotID = sl.slotID "
        "group by classDay "
        "having studentCount<30 "
        "order by sl.slotID asc"
    ;
    PreparedStatement* slotStmt=con->prepareStatement(checkSlotSql);
    ResultSet* slotRes=slotStmt->executeQuery();

    while (slotRes->next())
    {
        slot sl;

        sl.slotID= slotRes->getString("slotID");
        sl.day= slotRes->getString("classDay");
        sl.freeSlot= 30 - slotRes->getInt("studentCount");
        slotList.push_back(sl);
    }

    //
    if(slotList.empty()){
        cout<<YELLOW<<"[ NOTICE ] "<<RESET<<"No empty slot currently. Sorry"<<endl;
        PETC();
        return;
    }

    bool isValid=false;
    while (!isValid)
    {
        cout<<"Enter full name : ";
        getline(cin>>ws, fName);

        if(isValidFullName(fName)) isValid=true;
        else{
            cout<<RED<<"[ ERROR ] "<<RESET<<"Please enter valid name.\n"<<endl;
        }
    }
    
    isValid=false;
    while (!isValid)
    {
        cout<<"Enter IC (without hyphen '-'): ";
        getline(cin>>ws, ic);

        if(isValidIC(ic)) isValid=true;
        else{
            cout<<RED<<"[ ERROR ] "<<RESET<<"Please enter valid IC format.\n"<<endl;
        }
    }
    
    

    if (option==0)
    {
        cout<<"Enter home address: ";
        getline(cin>>ws, homeAdd);
        cout<<"Enter phone number: ";
        getline(cin>>ws, phoneNum);
    }
    
    cout<<"\nChoose class slot:"<<endl;
    for(int i=0;i<slotList.size();++i){
        slot sl = slotList[i];

        cout <<"  ["<<i+1<<"] "
             <<left<<setw(10)<<sl.day<<" (9pm - 11pm)"
             <<BLUE<<" [ "<<sl.freeSlot<<" slot(s) ]"
             <<RESET<<endl;
    }

    bool validInput=false;
    int selectedIndex = -1;
    
    do
    {
        cout << "───────────────────────────────────────────────────────────────" << endl;
        cout << "  Select an option [1-" << slotList.size() << "]: ";
        getline(cin >> ws, choice);
        try {
            size_t pos;
            int choiceNum = stoi(choice, &pos);

            // Ensure entire string was numeric and within valid range [1, slotList.size() - 1]
            if (pos == choice.length() && choiceNum >= 1 && choiceNum <= static_cast<int>(slotList.size() - 1)) {
                selectedIndex = choiceNum; // or choiceNum - 1 depending on whether your list is 0-indexed
                classSlot = slotList[selectedIndex].slotID;
                validInput = true;
            } else {
                cout << RED<<"[ERROR] "<<RESET<<"Invalid option. Please enter a number between 1 and " 
                    << (slotList.size()) << "." << endl;
            }
        } catch (...) {
            cout <<RED<< "[ERROR] "<<RESET<<"Please enter a valid numeric digit." << endl;
        }
    } while (validInput==false);
    
    

    cout<<"\nConfirm registration? "<<GREEN<<"(y/n)"<<RESET<<": ";
    getline(cin>>ws,choice);
    if(choice=="y" || choice== "Y"){
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

        cout<<GREEN<<"[ SUCCESS ] "<<RESET<<"Waiting for instructor approval"<<endl;

        
    }else if(choice== "n" || choice == "N"){
        //delete account
        string deleteAccSql="delete from account where accountID = ?";
        PreparedStatement* dStmt=con->prepareStatement(deleteAccSql);
        dStmt->setString(1,currentUser);
        dStmt->executeUpdate();
        cout<<YELLOW<<"[ NOTICE ] "<<"Registration cancelled"<<endl;
    }else{
        invalidInput();
    }


}  //register student


void DatabaseManager::studenDashboard(){

    
    // string studentID;
    string fName;
    string classSlot;
    string curRank;
    string stdStatus;
    string feeStatus; //paid, unpaid, etc
    int withdrawRequestCount=0;
    string choice;
    
    bool endLoop=false;

    while (!endLoop)
    {
        clearScreen();
        cout<<"Current account ID: "<<currentUser<<endl;
        string sqlStmt = "select st.*, sl.classDay, "
            "COALESCE(r.color, 'Pending Approval') AS color,"
            "count(w.studentID) as requestCount, "
            "w.wthStatus "
            "from student st "
            "left join slot sl on st.slotID = sl.slotID "
            "left join rank_history rh on st.studentId = rh.studentID "
            "left join withdraw w on w.studentID = st.studentID "
            "     and w.wthStatus = (select wthStatus "
            "         from withdraw where wthStatus = 'pending' "
            "         order by withdrawID desc limit 1) "
            "and rh.date_achieved = (select max(rh2.date_achieved) from rank_history rh2 "
            "     where rh2.studentID = st.studentID) "
            "left join rank r on rh.rankID = r.rankID "
            "where accountID=? "
            "group by st.studentID, sl.classDay, r.color, w.wthStatus ";

        PreparedStatement* pstmt=con->prepareStatement(sqlStmt);

        pstmt->setString(1,currentUser);

        ResultSet* res=pstmt->executeQuery();

        if(res->next()){
            fName=res->getString("fullName");
            classSlot=res->getString("classDay");
            stdStatus=res->getString("stdStatus");
            curRank=res->getString("color");
            withdrawRequestCount=res->getInt("requestCount");

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

        string status = (stdStatus == "withdrawn" ? (RED + stdStatus + RESET) : (stdStatus == "pending" ? (YELLOW + stdStatus + RESET):(GREEN + stdStatus + RESET)));
        cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
        cout << "│                              STUDENT DASHBOARD                              │" << endl;
        cout << "╰─────────────────────────────────────────────────────────────────────────────╯" << endl;
        cout << "\n[ USER PROFILE ]" << endl;
        cout << "  • Student Name          : "<< fName << endl;
        cout << "  • Class Slot            : "<< classSlot << endl;
        cout << "  • Current Rank          : "<< curRank << endl;
        cout << "  • Status                : "<< status << endl;
        cout << "  • Fee Status            : "<< (getFeeStatus(currentUser) ? (GREEN + "[ PAID ]" + RESET) : (RED + "[ UNPAID ]" + RESET)) << endl;
        
        //if have withdraw request
        if(withdrawRequestCount>0){
            cout << "  • Withdraw Status       : "<<YELLOW<< res->getString("wthStatus") <<RESET<< endl;
        }
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "[ RANK PROMOTION HISTORY ]" << endl;

        cout << "    "<<left<<setw(13)<<"Color"<<"  Date Achieved"<<endl;
        while (rankRes->next())
        {
            cout<<"  • "<<left<<setw(13)<<rankRes->getString("color")<<": "<<rankRes->getString("date_achieved")<<endl;
        }
        
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "[ AVAILABLE ACTIONS ]" << endl;
        if(stdStatus == "active"){
            cout << "  [1] Pay Monthly Fees" << endl;
            cout << "  [2] Donate :)" << endl;
            cout << "  [3] Withdraw" << endl;
        }
        
        cout << "  [0] Exit" << endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "   Select an option: ";
        getline(cin>>ws, choice);

        if(choice == "0"){
            endLoop=true;
        }else if (choice == "1" && stdStatus == "active"){
            payFees();
        }else if (choice == "2" && stdStatus == "active"){
            donate();
        }else if (choice == "3" && stdStatus == "active"){
            withdrawRequest(0);
        }else{
            invalidInput();
        }

        delete res;
        delete pstmt;
        delete rankRes;
        delete rstmt;
    }
 
}   // student dashboard

void DatabaseManager::withdrawRequest(int option){
    string studentID;
    string studentName;
    string instructorID;
    string stdStatus;
    string reason;
    string profileStmt;
    string childName;
    string choice;
    int requestCount=0;
    bool endLoop=false;


    while (!endLoop)
    {
        if (option == 0){
            profileStmt="SELECT s.studentID, s.fullName, s.stdStatus, s.instructorID, count(w.studentID) as requestCount FROM student s left join withdraw w on w.studentID = s.studentID and w.wthStatus = 'pending' WHERE accountID =? ";

        }else if (option == 1){
            profileStmt="SELECT s.studentID, s.fullName, s.stdStatus, s.instructorID, count(w.studentID) as requestCount FROM student s left join withdraw w on w.studentID = s.studentID and w.wthStatus = 'pending' WHERE parentID = (select parentID from parent where accountID =?) and fullName like ?";
        }

        PreparedStatement* pstmt=con->prepareStatement(profileStmt);

        if(option == 0){
            pstmt->setString(1,currentUser);
        } else if (option ==1){
            cout<<" Enter child name: ";
            getline(cin,childName);
            pstmt->setString(1,currentUser);
            pstmt->setString(2,"%"+childName+"%");
        }

        ResultSet* res=pstmt->executeQuery();

        if(res->next()){
            studentID=res->getString("studentID");
            studentName=res->getString("fullName");
            stdStatus=res->getString("stdStatus");
            instructorID=res->getString("instructorID");
            requestCount=res->getInt("requestCount");
        } else{
            clearScreen();
            cout<<RED<<"[ ERROR ] "<<RESET<<"Account did not found"<<endl;
            delete res;
            delete pstmt;
        }

        
        clearScreen();

        cout<<RED<<"╭─────────────────────────────────────────────────────────────╮"<<endl;
        cout<<"│                 STUDENT WITHDRAWAL REQUEST                  │"<<endl;
        cout<<"╰─────────────────────────────────────────────────────────────╯"<<RESET<<endl;
        cout<<"\n[ STUDENT DETAILS ]"<<endl;
        cout<<"  • Student ID  : "<<studentID<<endl;
        cout<<"  • Full Name   : "<<studentName<<endl;
        cout<<"  • Status      : "<<stdStatus<<endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;

        if(requestCount == 0){
            cout<<YELLOW<<"[ NOTICE ]"<<RESET<<endl;
            cout<<"  • Submitting this form sends a withdrawal request to your instructor for formal review and processing."<<endl;
            cout<<"  • Type '0' or cancel at any time to abort."<<endl;
            cout << "\n───────────────────────────────────────────────────────────────" << endl;
            cout << "  Please State your reason for withdrawing:\n  >> ";
            
            // cin.ignore();
            getline(cin>>ws,reason);

            if (reason == "0" || reason == "cancel" || reason == "CANCEL") {
                clearScreen();
                cout << "\n  " << YELLOW << "[CANCELLED]" << RESET << " Withdrawal request was cancelled.\n" << endl;
                delete res;
                delete pstmt;
                PETC();
                return;
            }
            
            cout << "\n───────────────────────────────────────────────────────────────" << endl;
            cout << "  Confirm withdrawal request? (y/n): ";
            getline(cin>>ws,choice);

            if (choice == "Y" || choice=="y")
            {
                clearScreen();

                cout<<GREEN<<" [ SUCCESS ]"<<RESET<<" Your withdrawal request has been submitted for review!"<<endl;
                endLoop=true;

                PETC();
            }
        }else{
            cout<<YELLOW<<"[ NOTICE ] "<<RESET<<"You have a withdrawal request pending already lah."<<endl;
            cout<<"           wait ah"<<endl;
            PETC();
            return;
        }
        
        delete pstmt;
        
        delete res;
        
    }

    string insertStmt="insert into withdraw(withdrawID, studentID, instructorID, reason, wthDate)"
    " values(?,?,?,?,CURDATE())";

    PreparedStatement* inStmt=con->prepareStatement(insertStmt);

    inStmt->setString(1,getNextID("withdraw",4));
    inStmt->setString(2,studentID);
    inStmt->setString(3,instructorID);
    inStmt->setString(4,reason);

    inStmt->executeUpdate();

    delete inStmt;
    
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
