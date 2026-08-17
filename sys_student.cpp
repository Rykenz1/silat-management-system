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
        string sqlStatement = "insert into student(studentID, fullName, ic, accountID, homeAdd, phoneNum, joinDate, classID,parentID)"
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
    string studentID;
    string fName;
    string classSlot;
    string curRank;
    string stdStatus;
    string feeStatus; //paid, unpaid, etc
    struct rankHistory{
        
    };

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
    }

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
    cout << "  • Fee Status   : " << endl;
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
    cout << "  [2] Withdraw" << endl;
    cout << "  [0] Exit" << endl;
    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    cout << "   Select an option: ";


    delete res;
    delete pstmt;
}   // student dashboard
