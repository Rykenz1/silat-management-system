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
            } else if (tableName == "withdraw"){
                prefix = 'w';
            } else if (tableName == "payment"){
                prefix = 'p';
            }
            num = 0; // First ID starts at 0 (e.g., s000)
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

bool DatabaseManager::getFeeStatus(){
    bool isPaid=false;

    string checkSql=" select count(*) from payment where accountID = ?"
        " and month(paymentDate) = month(curdate())"
        " and year(paymentDate) = year(curdate())";

    PreparedStatement* checkStmt=con->prepareStatement(checkSql);

    checkStmt->setString(1,currentUser);

    ResultSet* checkRes=checkStmt->executeQuery();

    if(checkRes->next() && checkRes->getInt(1) > 0){
        isPaid= true;
    }
    
    return isPaid;
}   //get fee status


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

void DatabaseManager::payFees(){
    double totalFee=0.0;
    int childcount=0;
    string entityID;


    if (userRole == "student")
    {
        PreparedStatement* sStmt=con->prepareStatement(
            "select studentID from student where accountID = ?");

        sStmt->setString(1,currentUser);

        ResultSet* sRes=sStmt->executeQuery();

        if(sRes->next()){
            entityID=sRes->getString("studentID");
        }

        totalFee=20;

        delete sStmt;
        delete sRes;

    } else if (userRole == "parent")
    {
        PreparedStatement* pStmt=con->prepareStatement(
            "select parentID from parent where accountID = ?");

        pStmt->setString(1,currentUser);

        ResultSet* sRes=pStmt->executeQuery();

        if(sRes->next()){
            entityID=sRes->getString("parentID");
        }

        // count active children under this parent
        PreparedStatement* cStmt=con->prepareStatement(
            "select count(*) from student where parentID = ? and stdStatus = 'active'");
        
        cStmt->setString(1, entityID);

        ResultSet* cRes=cStmt->executeQuery();

        if(cRes->next()){
            childcount=cRes->getInt(1);
        }

        if (childcount <=0){
            totalFee=0;
        } else if( childcount <=2 ){
            totalFee = childcount * 20;
        } else {
            totalFee = (2 * 20) + ((childcount-2)*10);
        }
        
        delete pStmt;
        delete sRes;
        delete cStmt;
        delete cRes;
    }
    
    

    cout << "┌─────────────────────────────────────────────────────────────┐" << endl;
    cout << "│                    MONTHLY FEE PAYMENT                      │" << endl;
    cout << "└─────────────────────────────────────────────────────────────┘" << endl;

    //overview
    cout << "\n[ BILLING DETAILS ]"<<endl;
    cout << "  • Account Type : "<< (userRole == "student" ? "Student (Personal)" : "Parent / Guardian") <<endl;
    cout << "  • " << (userRole == "student" ? "Student ID   : " : "Parent ID    : ") << entityID << endl;
    cout << "  • Billing Cycle: Current Month" << endl;
    cout << "  • Payment Stat : " << (getFeeStatus() ? (GREEN + "[ PAID ]" + RESET) : (RED + "[ UNPAID ]" + RESET)) << endl;

    //if already paid
    if (getFeeStatus()) {
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "  " << GREEN << "[NOTICE]" << RESET << " Your monthly fee has already been settled." << endl;
        cout << "           No further payment is required for this billing cycle." << endl;
        cout << "\n  Press Enter to return...";
        cin.ignore();
        cin.get();
        return;
    }

    //if parent has 0 active children
    if (userRole == "parent" && childcount == 0) {
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "  " << YELLOW << "[NOTICE]" << RESET << " You have 0 active children enrolled." << endl;
        cout << "           No tuition fees due at this moment." << endl;
        cout << "\n  Press Enter to return...";
        cin.ignore();
        cin.get();
        return;
    }

    // 6. Display Breakdown & Pricing
    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    cout << "  [ FEE BREAKDOWN ]" << endl;
    cout << fixed << setprecision(2);

    if (userRole == "student") {
        cout << "  • Base Monthly Fee : RM 20.00" << endl;
    } else {
        int tier1Count = min(childcount, 2);
        int tier2Count = max(0, childcount - 2);

        cout << "  • Active Children  : " << childcount << endl;
        cout << "  • 1st & 2nd Child  : " << tier1Count << " x RM 20.00  = RM " << (tier1Count * 20.00) << endl;
        if (tier2Count > 0) {
            cout << "  • Additional (" << tier2Count << ")   : " << tier2Count << " x RM 10.00  = RM " << (tier2Count * 10.00) << endl;
        }
    }
    cout << "  -------------------------------------------------------------" << endl;
    cout << "  • Total Amount Due : RM " << totalFee << endl;

    // 7. Payment Confirmation
    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    char confirm;
    cout << "  Proceed with payment of RM " << totalFee << "? (Y/N): ";
    cin >> confirm;

    if (toupper(confirm) != 'Y') {
        cout << "\n  " << YELLOW << "[CANCELLED]" << RESET << " Payment transaction cancelled.\n" << endl;
        return;
    }

    // 8. Insert Record into SQL Payment Table
    try {
        string paySql = "INSERT INTO payment (paymentID, paymentDate, amount, type, accountID) VALUES (?, CURDATE(), ?,'fees', ?)";

        PreparedStatement* payStmt=con->prepareStatement(paySql);

        payStmt->setString(1, getNextID("payment",4));
        payStmt->setDouble(2, totalFee);
        payStmt->setString(3, currentUser);
        payStmt->executeUpdate();

        cout << "\n  " << GREEN << "[SUCCESS]" << RESET << " Payment of RM " << totalFee << " recorded successfully!\n" << endl;

    } catch (SQLException& e) {
        cerr << "\n  " << RED << "[ERROR]" << RESET << " Failed to record payment: " << e.what() << endl;
    }
}   //pay fees

void DatabaseManager::donate(){
    double amount=0.0;
    string choice;

    //display page
    cout <<GREEN<< "┌─────────────────────────────────────────────────────────────┐" << endl;
    cout << "│                 GELANGGANG DONATION / INFAQ                 │" << endl;
    cout << "└─────────────────────────────────────────────────────────────┘" <<RESET<< endl;
    
    cout << "\n  [ DONOR INFORMATION ]" << endl;
    cout << "  • Contributor  : " << userName << endl;
    cout << "  • Account Type : " << userRole << endl;

    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    cout << "  [ SELECT DONATION AMOUNT ]" << endl;
    cout << "   [1] RM 10.00" << endl;
    cout << "   [2] RM 30.00" << endl;
    cout << "   [3] RM 50.00" << endl;
    cout << "   [4] Custom Amount" << endl;
    cout << "   [0] Cancel / Return" << endl;
    cout << "───────────────────────────────────────────────────────────────" << endl;
    cout << "  Select an option [0-4]: ";

    getline(cin,choice);

    if (choice =="0")
    {
        PETC();
        return;
    }else if (choice=="1"){
        amount=10.0;
    }else if (choice=="2"){
        amount=30.0;
    }else if (choice=="3"){
        amount=50.0;
    }else if (choice=="4"){
        // cin.ignore();
        cout<<"Enter custom amount to donate:\nRM";
        cin>>amount;
    }else{
        cout << "Invalid choice"<<endl;
    }


    string insertStmt="insert into payment(paymentID, paymentDate, amount, type, accountID)"
     "values(?,curdate(),?,'donation',?)";
    
    PreparedStatement* dStmt=con->prepareStatement(insertStmt);

    dStmt->setString(1,getNextID("payment",4));
    dStmt->setDouble(2,amount);
    dStmt->setString(3,currentUser);
    
    dStmt->executeUpdate();

    PETC();
    delete dStmt;
} //Donate

void DatabaseManager::PETC(){
    cout << "\n  Press Enter to return...";
    // cin.ignore(10000, '\n');
    cin.get();
    return;
}

void DatabaseManager::clearScreen(){
    #if defined(_WIN32) || defined(_WIN64)
        system("cls");   // Windows command
    #else
        system("clear"); // Linux / macOS / Unix command
    #endif
}   //clear screen