#include "sys_function.h"

using namespace std;
using namespace sql;

void DatabaseManager::regInstructor(){
    string instructorID;
    string fName;
    string homeAdd;
    string phoneNum;
    string classSlot;
    char choice;
    cout<<"=====REGISTERING INSTRUCTOR====="<<endl;
    
    //create account
    createAcc(3);

    cin.ignore();
    cout<<"Enter full name: ";
    getline(cin, fName);
    cout<<"enter home address: ";
    getline(cin, homeAdd);
    cout<<"enter phone number: ";
    getline(cin, phoneNum);


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

    // cout<<"\nSuccessfully registered instructor :D"<<endl;
    // cout<<instructorID<<endl;
    // cout<<fName<<endl;
    // cout<<currentUser<<endl;
    // cout<<homeAdd<<endl;
    // cout<<phoneNum<<endl;
    // cout<<classSlot<<endl;

}   //register instructor


void DatabaseManager::instructorDashboard(){
    // clearScreen();
    string instructorID;
    string fName;
    string accountID;
    string homeAdd;
    string phoneNum;
    string joinDate;
    string classSlot;
    int pendingCount=0;
    char choice;
    bool endLoop=false;

    while(!endLoop){
        //get instructor info
        string sqlStatement="SELECT i.*, COUNT(s.studentID) AS pendingCount FROM instructor i LEFT JOIN student s ON i.slotID = s.slotID AND s.stdStatus = 'pending' WHERE i.accountID = ?";

        PreparedStatement* pstmt=con->prepareStatement(sqlStatement);

        pstmt->setString(1,currentUser);

        ResultSet* res=pstmt->executeQuery();

        if(res->next()){
            instructorID = res->getString("instructorID");
            entityID=instructorID;
            fName = res->getString("fullName");
            homeAdd = res->getString("homeAdd");
            phoneNum = res->getString("phoneNum");
            joinDate = res->getString("joinDate");
            classSlot = res->getString("slotID");
            pendingCount = stoi(res->getString("pendingCount"));
        }


        
        cout << "===== Instructor Dashboard =====" << endl;
        cout << "\nHi, "<<fName << endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "[ AVAILABLE ACTIONS ]" << endl;
        cout << "  [1] Pending Approval ("<<pendingCount<<")"<< endl;
        cout << "  [2] View Students" << endl;
        cout << "  [3] Promote Students" << endl;
        cout << "  [4] Withdrawal Requests (0)" << endl;
        cout << "  [0] Exit" << endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "   Select an option: ";
        cin>>choice;

        switch (choice)
        {
        case '0':
            //exit
            endLoop=true;
            break;
        
        case '1':
            //approval
            studentApproval(instructorID, classSlot);
            break;
        
        
        case '2':
            //view students
            break;
        
        case '3':
            //promote
            break;
        
        case '4':
            //withdrawal
            break;
        
        
        default:
            cout<<"invalid input"<<endl;
            break;
        }
    }

    
}   // instructor dashboard


void DatabaseManager::studentApproval(string instructorID, string classSlot){
    cout<<"=====STUDENT APPROVAL====="<<endl;

    struct pendingStudent
    {
        int digit;
        string studentID;
        string studentName;
        string phoneNum; 
    };
    

    
    
    vector<pendingStudent> pendingList; //to store studentID of to-be-approve student
    

    string sqlStatement="select * from student where slotID=? and stdStatus = 'pending'";

    PreparedStatement* pstmt=con->prepareStatement(sqlStatement);
    
    pstmt->setString(1,classSlot);

    ResultSet* res=pstmt->executeQuery();

    //display student list in table view
    cout<<right<<setw(4)<<"No "<<left<<setw(30)<<"Name"<<setw(15)<<"Contact"<<endl;
    cout<<"───────────────────────────────────────────────────────────────"<<endl;
    int pendingCount=0;
    while(res->next()){
        pendingStudent s;

        ++pendingCount;
        s.digit=pendingCount;
        s.studentID=res->getString("studentID");
        s.studentName=res->getString("fullName");
        s.phoneNum=res->getString("phoneNum");

        pendingList.push_back(s);

        cout<<right<<setw(3)<<pendingCount<<" "<<left<<setw(30)<<s.studentName<<setw(15)<<s.phoneNum<<endl;
    }

    //check if list is empty
    if (pendingList.empty())
    {
        cout<<"\nNo pending student registration for the class slot: "<<classSlot<<endl;
    }

    cout<<"───────────────────────────────────────────────────────────────" << endl;
    cout << "Select student(s) to approve (e.g. 1 / 1,3 / 1-3) or '0' to cancel: ";
    
    string input;
    cin >> input;

    if (input == "0") {
        cout << "Approval cancelled.\n";
        return;
    }

    set<int> selectedIndices = parseSelections(input, pendingList.size());

    if (selectedIndices.empty()) {
        cout << "Invalid selection.\n";
        return;
    }

    //update statement
    string updateSql="UPDATE student SET stdStatus = 'active', instructorID=?, joinDate = CURDATE() WHERE studentID = ?";

    //insert statement
    string insertStmt="insert into rank_history(rankID, studentID, date_achieved, instructorID)"
    "value ('r1', ?, CURDATE(),?)";

    PreparedStatement* updStmt=con->prepareStatement(updateSql);

    PreparedStatement* istmt = con->prepareStatement(insertStmt);

    //update student instructorID and status
    int approvedCount =0;
    for (int index : selectedIndices){
        string targetStudentID = pendingList[index-1].studentID;

        //update statement
        updStmt->setString(1,instructorID);
        updStmt->setString(2,targetStudentID);

        updStmt->executeUpdate();

        //insert into rank_history
        istmt->setString(1,targetStudentID);
        istmt->setString(2,instructorID);

        istmt->executeUpdate();

        cout<<"Approved: "<<pendingList[index-1].studentName<<"\n";
        approvedCount++;
    }

    cout << "\nSuccessfully approved " << approvedCount << " student(s)!\n";
    
    delete pstmt;
    delete istmt;
    delete updStmt;
    delete res;

    PETC();
}   //student Approval

void DatabaseManager::viewStudents(){
    struct student{
        string fullName;
        int age;
        string classSlot;
        string feeStatus;
        string rankID;
    };

    vector<student> studentList;

    string getInfoSql= "SELECT s.fullName, s.ic, COALESCE(s.accountID, p.accountID) AS payer_accountID, CASE WHEN COUNT(pay.paymentID) > 0 THEN 'PAID' ELSE 'UNPAID' END AS fee_status"
    " from student s left join parent p on s.parentID = p.parentID"
    " left join payment pay on"
    " pay.accountID = COALESCE(s.accountID, p.accountID) AND pay.type = 'fee' AND MONTH(pay.paymentDate) = MONTH(CURRENT_DATE()) AND YEAR(pay.paymentDate) = YEAR(CURRENT_DATE())"
    " group by s.studentID, s.fullName, coalesce(s.accountID, p.accountID)";

    PreparedStatement* infoStmt=con->prepareStatement(getInfoSql);
    ResultSet* infoRes=infoStmt->executeQuery();

    while (infoRes->next())
    {
        student st;

    }
    

    cout<<"╭─────────────────────────────────────────────────────────────╮"<<endl;
    cout<<"│                         STUDENT LIST                        │"<<endl;
    cout<<"╰─────────────────────────────────────────────────────────────╯"<<endl;
    
    cout<<"\nClass Day   : "<<endl;
    cout<<"Instuctor Name: "<<endl;
    
}   //view students