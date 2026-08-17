#include "sys_function.h"

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
            } else if (tableName == "rank") {
                prefix = 'r';
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


set<int> DatabaseManager::parseSelections(const string& input, int maxCount) {
    set<int> indices;
    stringstream ss(input);
    string token;

    while (getline(ss, token, ',')) {
        // Trim spaces if any
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);

        if (token.empty()) continue;

        size_t dashPos = token.find('-');
        if (dashPos != string::npos) {
            // Handle range (e.g. 2-5)
            try {
                int start = stoi(token.substr(0, dashPos));
                int end = stoi(token.substr(dashPos + 1));
                if (start > end) swap(start, end);
                for (int i = start; i <= end; ++i) {
                    if (i >= 1 && i <= maxCount) indices.insert(i);
                }
            } catch (...) {}
        } else {
            // Handle single number (e.g. 1)
            try {
                int val = stoi(token);
                if (val >= 1 && val <= maxCount) {
                    indices.insert(val);
                }
            } catch (...) {}
        }
    }
    return indices;
}


void DatabaseManager::clearScreen(){
    #if defined(_WIN32) || defined(_WIN64)
        system("cls");   // Windows command
    #else
        system("clear"); // Linux / macOS / Unix command
    #endif
}   //clear screen