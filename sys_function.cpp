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

    for (int i = 3; i > 0; i--)
    {
        cout << "=====LOGIN PAGE=====" << endl;
        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;

        getCurUsr(username,password);

        if (userRole=="admin")
        {
            adminDashboard();
            break;
        }
        else if (userRole=="student")
        {
            studenDashboard();
            break;
        }
        else if (userRole=="parent")
        {
            parentDashboard();
            break;
        }
        else if (userRole=="instructor")
        {
            instructorDashboard();
            break;
        }else{
            cout<<"\nIncorrect username or password.."<<endl;
            cout<<"Try again"<<endl;
            cout<<i-1<<" retries left"<<endl;
        }
    }
}

void DatabaseManager::registration(){
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
            regStudent(0,"NULL");
            break;
            
        
        case '2':
            cout<<"register parent"<<endl;
            regParent();
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


void DatabaseManager::createAcc(int option){
    string username;    
    string password;
    string cfmPwd;      //confirm password
    string accType;

    //set register account type
    switch (option)
        {
        case 1:
            accType="student";
            break;

        case 2:
            accType="parent";
            break;
        
        case 3:
            accType="instructor";
            break;
        
        default:
            break;
    }

    cout<<"enter username: ";
    cin>>username;

    for (int i = 0; i < 3; i++)
    {
        cout<<"enter password: ";
        cin>>password;
        cout<<"confirm password: ";
        cin>>cfmPwd;

        if(password==cfmPwd){
            password = cfmPwd;
            break;
        }else{
            cout<<"try again"<<endl;
        }
    }

    
    string sqlStatement = "insert into account(accountID,username, password,acc_type)"
        "value(?,?,?,?)";

    PreparedStatement* pstmt= con->prepareStatement(sqlStatement);

    pstmt->setString(1,getNextID("account",3));
    pstmt->setString(2,username);
    pstmt->setString(3,password);
    pstmt->setString(4,accType);

    ResultSet* res= pstmt->executeQuery();

    getCurUsr(username,password);

    delete pstmt;
}       //create account


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
    
    string sqlStatement = "insert into instructor(instructorID, fullName, accountID, homeAdd, phoneNum, joinDate, classID)"
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

    cout<<"\nSuccessfully registered instructor :D"<<endl;
    cout<<instructorID<<endl;
    cout<<fName<<endl;
    cout<<currentUser<<endl;
    cout<<homeAdd<<endl;
    cout<<phoneNum<<endl;
    cout<<classSlot<<endl;

}   //register instructor


void DatabaseManager::getCurUsr(string username, string password){
    string userID;

    string sqlStatement="select accountID, acc_type from account where username=? and password=?";
    PreparedStatement* pstmt = con->prepareStatement(sqlStatement);

    pstmt->setString(1,username);
    pstmt->setString(2,password);

    ResultSet* res=pstmt->executeQuery();
    
    if (res->next())
    {   
        currentUser= res->getString("accountID");
        userRole= res->getString("acc_type");
    }else{
        currentUser="NULL";
        userRole="NULL";
        // cout<<"Incorrect username or password.."<<endl;
        // cout<<"Try again"<<endl;
    }
    
    cout<<currentUser<<endl;
    cout<<userRole<<endl;
    
    delete pstmt;
    delete res;
}   //get current user

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
            /* code */
            break;

        case '5':
            /* code */
            break;
        
        default:
            break;
        }
    }
    
    
}   // admin dashboard

void DatabaseManager::studenDashboard(){
    cout << "===== Student Dashboard =====" << endl;
    cout << "\n[ USER PROFILE ]" << endl;
    cout << "  • Student Name : " << endl;
    cout << "  • Class Slot   : " << endl;
    cout << "  • Current Rank : " << endl;
    cout << "  • Status       : " << endl;
    cout << "  • Fee Status   : " << endl;
    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    cout << "[ RANK PROMOTION HISTORY ]" << endl;
    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    cout << "[ AVAILABLE ACTIONS ]" << endl;
    cout << "  [1] Pay Monthly Fees" << endl;
    cout << "  [2] Withdraw" << endl;
    cout << "  [0] Exit" << endl;
    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    cout << "   Select an option: ";
}   // student dashboard

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

void DatabaseManager::instructorDashboard(){
    clearScreen();
    
    cout << "===== Instructor Dashboard =====" << endl;
    cout << "\nHi, (instructor name)" << endl;
    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    cout << "[ AVAILABLE ACTIONS ]" << endl;
    cout << "  [1] Pending Approval (0)" << endl;
    cout << "  [2] View Students" << endl;
    cout << "  [3] Promote Students" << endl;
    cout << "  [4] Withdrawal Requests (0)" << endl;
    cout << "  [0] Exit" << endl;
    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    cout << "   Select an option: ";
}   // instructor dashboard








void DatabaseManager::clearScreen(){
    #if defined(_WIN32) || defined(_WIN64)
        std::system("cls");   // Windows command
    #else
        std::system("clear"); // Linux / macOS / Unix command
    #endif
}   //clear screen