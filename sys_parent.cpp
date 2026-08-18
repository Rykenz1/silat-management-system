#include "sys_function.h"

using namespace std;
using namespace sql;

void DatabaseManager::regParent(){
    string parentID;
    string fName;
    string ic;
    string homeAdd;
    string phoneNum;

    cout<<"=====REGISTERING AS A PARENT====="<<endl;
    
    //create account
    createAcc(2);

    cin.ignore();
    cout<<"Enter full name: ";
    getline(cin, fName);
    cout<<"enter IC: ";
    getline(cin, ic);
    cout<<"enter home address: ";
    getline(cin, homeAdd);
    cout<<"enter phone number: ";
    getline(cin, phoneNum);

    string sqlStatement = "insert into parent(parentID, fullName,accountID, homeAdd, phoneNum)"
        "value(?,?,?,?,?)";

    PreparedStatement* pstmt= con->prepareStatement(sqlStatement);
    
    parentID=getNextID("parent",3);
    pstmt->setString(1,parentID); //parentID
    pstmt->setString(2,fName);  //full name
    pstmt->setString(3,currentUser);     //accountID
    pstmt->setString(4,homeAdd); //connect accountID
    pstmt->setString(5,phoneNum);

    ResultSet* res= pstmt->executeQuery();

    
    int childCount;
    char input;
    cout << "How many child to register? (enter number): " ;
    cin>>input;

    childCount = input - '0';

    for (int i = 0; i < childCount; i++)
    {
        cout<<"\nregister child "<<i+1<<endl;
        regStudent(1,parentID);
    }
    

}   //register parent


void DatabaseManager::parentDashboard(){
    string choice;
    string phoneNum;
    string homeAdd;
    bool endLoop=false;

    while (!endLoop)
    {
        //get parent info
        string parentSql="select * from parent where accountID=?";
        PreparedStatement* pStmt=con->prepareStatement(parentSql);

        pStmt->setString(1,currentUser);

        ResultSet* pRes=pStmt->executeQuery();

        if(pRes->next()){
            userName=pRes->getString("fullName");
            phoneNum=pRes->getString("phoneNum");
            homeAdd=pRes->getString("homeAdd");
        }

        //get children list
        string getChildSql = "select s.fullName, s.stdStatus, r.color from student s"
        " join rank_history rh on s.studentID =  rh.studentID"
        " join rank r on rh.rankID =  r.rankID"
        " where s.parentID = (select parentID from parent where accountID = ?)";

        PreparedStatement* gcStmt=con->prepareStatement(getChildSql);

        gcStmt->setString(1,currentUser);

        ResultSet* gcRes=gcStmt->executeQuery();


        //display page
        cout << "===== Parent Dashboard =====" << endl;
        cout << "\n[ PARENT PROFILE ]" << endl;
        cout << "  • Name         : "<<userName << endl;
        cout << "  • Phone No.    : "<<phoneNum << endl;
        cout << "  • Home Address : "<<homeAdd << endl;
        cout << "  • Fee Status   : "<< (getFeeStatus() ? (GREEN + "[ PAID ]" + RESET) : (RED + "[ UNPAID ]" + RESET)) << endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "[ CHILD(REN) ]" << endl;
        cout <<left<<"  "<<setw(40)<<"Name"<<setw(10)<<"  Rank"<<setw(10)<<"  Status"<<endl;
        //display children
        while (gcRes->next())
        {
            cout <<left <<"  • "<<setw(40)<< gcRes->getString("fullName");
            cout<<setw(10)<<gcRes->getString("color");
            cout<<setw(10)<<gcRes->getString("stdStatus")<<endl;
        }
        
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "[ AVAILABLE ACTIONS ]" << endl;
        cout << "  [1] Pay Monthly Fees" << endl;
        cout << "  [2] Manage Children" << endl;
        cout << "  [3] Edit Information" << endl;
        cout << "  [4] Donation" << endl;
        cout << "  [0] Exit" << endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "   Select an option: ";
        getline(cin,choice);

        if (choice == "0"){
            endLoop=true;
        } else if ( choice == "1"){
            //pay fee
            clearScreen();
            payFees();
        } else if ( choice == "2"){
            //manage children
        } else if ( choice == "3"){
            //edit info
            clearScreen();
            cout<<"edit Info"<<endl;
            editInfo(phoneNum, homeAdd);

        } else if ( choice == "4"){
            //donation
            clearScreen();
            donate();
        } else {
            clearScreen();
            cout<< "Invalid Input"<<endl;
        }
    }

    clearScreen();   
    return;

}   // parent dashboard

void DatabaseManager::editInfo(string& phoneNum, string& homeAdd){
    string choice;
    string updateSql="";
    bool endLoop=false;

    while (!endLoop)
    {
        
        PreparedStatement* updStmt = nullptr;
        
        cout<<"=====EDIT INFO====="<<endl;

        cout << "\n[ PARENT PROFILE ]" << endl;
        cout << "  • Name         : "<<userName << endl;
        cout << "  • Phone No.    : "<<phoneNum << endl;
        cout << "  • Home Address : "<<homeAdd << endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "[ EDIT ]" << endl;
        cout << "  [1] Phone Number" << endl;
        cout << "  [2] Home Address" << endl;
        cout << "  [0] Exit" << endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "   Select an option: ";

        getline(cin,choice);

        if ( choice == "0"){
            endLoop=true;
        } else if( choice == "1"){ //update phone num
            string newPhoneNum;

            cout<<"\nEnter new phone number: ";
            getline(cin,newPhoneNum);

            if(!newPhoneNum.empty()){
                updateSql = "update parent set phoneNum = ? where accountID =?";

                updStmt=con->prepareStatement(updateSql);

                updStmt->setString(1,newPhoneNum);
                updStmt->setString(2,currentUser);

                updStmt->executeUpdate();

                phoneNum=newPhoneNum;

                clearScreen();
                cout<<GREEN<<" [ SUCCESS ]"<<RESET<<" Update phone number success!!"<<endl;
            } else {
                clearScreen();
                cout<<RED<<" [ ERROR ]"<<RESET<<" Empty value"<<endl;
            }
            

        } else if( choice == "2"){ //update home address
            string newHomeAdd;

            cout<<"\nEnter new home address: ";
            getline(cin,newHomeAdd);

            if(!newHomeAdd.empty()){
                updateSql = "update parent set homeAdd = ? where accountID =?";

                updStmt=con->prepareStatement(updateSql);

                updStmt->setString(1,newHomeAdd);
                updStmt->setString(2,currentUser);

                updStmt->executeUpdate();

                homeAdd=newHomeAdd;
                
                clearScreen();
                cout<<GREEN<<" [ SUCCESS ]"<<RESET<<" Update home address success!!"<<endl;
            } else {
                clearScreen();
                cout<<RED<<" [ ERROR ]"<<RESET<<" Empty value"<<endl;
            }
            


        } else {
            clearScreen();
            cout<<YELLOW<<"Invalid choice"<<RESET<<endl;
        }


        if (updStmt != nullptr){
            delete updStmt;
            updStmt = nullptr;
        }
    }
     
    clearScreen();
}