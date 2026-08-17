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
    cout << "===== Parent Dashboard =====" << endl;
    cout << "\n[ PARENT PROFILE ]" << endl;
    cout << "  • Name       : " << endl;
    cout << "  • Phone No.  : " << endl;
    cout << "  • Fee Status : " << endl;
    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    cout << "[ CHILD(REN) ]" << endl;
    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    cout << "[ AVAILABLE ACTIONS ]" << endl;
    cout << "  [1] Pay Monthly Fees" << endl;
    cout << "  [2] Manage Children" << endl;
    cout << "  [0] Exit" << endl;
    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    cout << "   Select an option: ";
}   // parent dashboard