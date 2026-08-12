#include "sys_function.h"
// #include <iostream>

using namespace std;
using namespace sql;

DatabaseManager::DatabaseManager() : con(nullptr) {}

DatabaseManager::~DatabaseManager() {
    if (con) {
        delete con; // Handles cleanup automatically when class destroys
        
    }
}

bool DatabaseManager::connect(const string& host, const string& user, const string& pass, const string& db) {
    try {
        mysql::MySQL_Driver* driver = mysql::get_mysql_driver_instance();
        con = driver->connect(host, user, pass);
        con->setSchema(db);
        return true;
    } catch (SQLException& e) {
        cerr << "Connection failed: " << e.what() << endl;
        return false;
    }
}


void DatabaseManager::login() {
    string username, password;

    cout << "=====LOGIN PAGE=====" << endl;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    // Use ? placeholders for variables
    PreparedStatement* pstmt = con->prepareStatement(
        "SELECT acc_type FROM account WHERE username = ? AND password = ?"
    );
    pstmt->setString(1, username); 
    pstmt->setString(2, password);

    ResultSet* res = pstmt->executeQuery();

    if (res->next()) {
        string accType = res->getString("acc_type");
        cout << "Login successful! Account type: " << accType << endl;
    } else {
        cout << "Invalid credentials." << endl;
    }

    // Clean up dynamic memory
    delete pstmt;
    delete res;
}

void DatabaseManager::regAcc(){
    char choice;
    bool running=true;

    while (running)
    {
        cout<<"=====REGISTER MENU====="<<endl<<endl;
        cout<<"Select registration caterogy:"<<endl;
        cout<<"  [1] Individual Student (self register)"<<endl;
        cout<<"  [2] Parent / Guardian (registering children <13 years old)"<<endl;
        cout<<"  [0] Back to Main Menu"<<endl;

        cout<<"Select an option [0-2]: ";
        cin>>choice;

        switch (choice)
        {
        case '1':
            cout<<"register student"<<endl;
            regStudent();
            break;
            
        
        case '2':
            regParent();
            cout<<"register parent"<<endl;
            break;

        case '0':
            running = false;
            break;
            
        default:
            cout<<"invalid input"<<endl;
            break;
        }
    }
    
    
}      //register menu


void DatabaseManager::regStudent(){
    string currAcc; //current user
    string usr;     //username
    string pwd;     //password  
    string cfmPwd;  //confirm password
    string fName;
    string ic;
    string homeAdd;
    string phoneNum;
    string classSlot="";
    char choice;

    

    cout<<"=====REGISTERING AS A STUDENT====="<<endl;
    cout<<"enter username: ";
    cin>>usr;

    for (int i = 0; i < 3; i++)
    {
        cout<<"enter password: ";
        cin>>pwd;
        cout<<"confirm password: ";
        cin>>cfmPwd;

        if(pwd==cfmPwd){
            pwd = cfmPwd;
            break;
        }else{
            cout<<"try again"<<endl;
        }
    }
    
    string sqlStatement = "insert into account(accountID,username, password,acc_type)"
        "value(?,?,?,?)";

    PreparedStatement* pstmt= con->prepareStatement(sqlStatement);

    pstmt->setString(1,getNextID("account",3));
    pstmt->setString(2,usr);
    pstmt->setString(3,pwd);
    pstmt->setString(4,"student");

    ResultSet* res= pstmt->executeQuery();
    
    cin.ignore();
    cout<<"Enter full name: ";
    getline(cin, fName);
    cout<<"enter IC: ";
    getline(cin, ic);
    cout<<"enter home address";
    getline(cin, homeAdd);
    cout<<"enter phone number: ";
    getline(cin, phoneNum);

    cout<<"choose class slot:"<<endl;
    cout<<"  [1] Thursday"<<endl;
    cout<<"  [2] Friday"<<endl;
    cout<<"  [3] Saturday"<<endl;
    cout<<"  [4] Sunday"<<endl;
    cout<<"───────────────────────────────────────────────────────────────"<<endl;
    cout<<"  Select an option [1-4]: ";
    cin>>choice;

    switch (choice)
    {
    case '1':
        classSlot = "Thursday";
        break;
    
    case '2':
        classSlot = "Friday";
        break;

    case '3':
        classSlot = "Saturday";
        break;

    case '4':
        classSlot = "Sunday";
        break;

    default:
        cout<<"invalid input"<<endl;
        break;
    }

    cin.ignore();
    cout<<"confirm registration? y/n"<<endl;

    if(choice=='y' || choice== 'Y'){
        cout<<"registration success... waiting for instructor approval"<<endl;
    }else if(choice== 'n' || choice == 'N'){
        cout<<"registration cancelled"<<endl;
    }else{
        cout<<"invalid input"<<endl;
    }

    cout<<"username: "<<usr<<endl;     //username
    cout<<"password: "<<pwd<<endl;     //password  
    cout<<"confirm password: "<<cfmPwd<<endl;  //confirm password
    cout<<"full name: "<<fName<<endl;
    cout<<"ic: "<<ic;
    cout<<"home addrese: "<<homeAdd<<endl;
    cout<<"phone number: "<<phoneNum<<endl;
    cout<<"class slot: "<<classSlot<<endl;

    // PreparedStatement* pstmt = con->prepareStatement(
    //     ""
    //     ""
    //     ""
    // );
    

}  //register student


void DatabaseManager::regParent(){

}   //register parent

string DatabaseManager::getNextID(string tableName, int digitCount){
    int num=0;
    char prefix;

    //combine add "ID" to tablename to get columnID
    //student+ID = studentID
    string ID_column = tableName+"ID";

    string sqlStatement="select max("+ ID_column+") from "+tableName;
    Statement* stmt= con->createStatement();

    ResultSet* res= stmt->executeQuery(sqlStatement);
    
    string maxAcc;
    if (res->next()){

        if(res->isNull(1)){
            // Assign default prefix based on table name (using if-else for std::string)
            if (tableName == "student") {
                prefix = 's';
            } else if (tableName == "parent") {
                prefix = 'p';
            } else if (tableName == "instructor") {
                prefix = 'i';
            } else if (tableName == "account") {
                prefix = 'a';
            }
            num = 1; // First ID starts at 1 (e.g., s001)
        }else{

            maxAcc = res->getString(1); //get the result of first column
            prefix= maxAcc[0]; //get the first letter
    
            //substring extract number, stoi convert string to int
            num=stoi(maxAcc.substr(1))+1;
        }
        
    }
    
    ostringstream oss;
    //combine prefix and number
    oss << prefix << setfill('0') << setw(digitCount) << num;
    
    string nextID = oss.str(); // combine prefix and value "a002"

    delete res;
    delete stmt;

    return nextID;
}