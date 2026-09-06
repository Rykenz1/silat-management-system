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
        clearScreen();
        cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
        cout << "│                                 LOGIN PAGE                                  │" << endl;
        cout << "╰─────────────────────────────────────────────────────────────────────────────╯" << endl;
        cout << "Enter username: ";
        getline(cin >> ws, username);
        password = getHiddenPassword("Enter Password: ");

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
            cout<<YELLOW<<"\n[ NOTICE ] "<<RESET<<"Incorrect username or password.."<<endl;
            cout<<"Try again"<<endl;
            cout<<i-1<<" Retries left"<<endl;
            PETC();
        }
    }
}

void DatabaseManager::registration(){
    string choice;
    bool running=true;

    while (running)
    {
        clearScreen();
        cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
        cout << "│                                REGISTER MENU                                │" << endl;
        cout << "╰─────────────────────────────────────────────────────────────────────────────╯" << endl;
        cout<<"\nSelect registration caterogy:"<<endl;
        cout<<"  [1] Individual Student (self register)"<<endl;
        cout<<"  [2] Parent / Guardian (registering children <13 years old)"<<endl;
        cout<<"  [0] Back to Main Menu"<<endl;

        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "   Select an option [0-2]: ";
        getline(cin >>ws, choice);

        if(choice == "0") return;
        else if(choice == "1"){
            regStudent(0,"NULL");
            return;
        }
        else if(choice == "2"){
            regParent();
            return;
        }

    }
    PETC();
    
}      //register menu


bool DatabaseManager::createAcc(int option){
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
    bool validUsername=false;
    while (!validUsername)
    {
        cout<<"Enter username (or enter '0' to abort): ";
        getline(cin>>ws,username);

        if (username == "0")
        {
            cout<<YELLOW<<"[ ABORTING ] "<<RESET<<"Please wait..."<<endl;
            PETC();
            return false;
        }
        

        string checkUsernameSql= "select username from account where username = ?";

        PreparedStatement* cStmt=con->prepareStatement(checkUsernameSql);
        cStmt->setString(1,username);

        ResultSet* cRes=cStmt->executeQuery();

        if (cRes->rowsCount()==0)
        {
            cout<<GREEN<<"[ VALID ] "<<RESET<<"Username available!"<<endl;
            validUsername = true;
        }else{
            cout<<YELLOW<<"[ WARNING ] "<<RESET<<"Username already taken!"<<endl;
        }
        
    }
    
    
    bool samePassword=false;
    while (!samePassword)
    {
        cout<<"\nEnter password  : ";
        getline(cin>>ws,password);

        if (password == "0")
        {
            cout<<YELLOW<<"[ ABORTING ] "<<RESET<<"Please wait..."<<endl;
            PETC();
            return false;
        }
        
        cout<<"Confirm password: ";
        getline(cin>>ws,cfmPwd);

        if(password==cfmPwd){
            samePassword=true;
        }else{
            cout<<YELLOW<<"[ TRY AGAIN ] "<<RESET<<"Password did not match"<<endl;
        }
    }

    if(option == 3){
        string sqlStatement = "insert into account(accountID,username, password,acc_type,approval)"
        "value(?,?,?,?,'approved')";

        PreparedStatement* pstmt= con->prepareStatement(sqlStatement);

        pstmt->setString(1,getNextID("account",3));
        pstmt->setString(2,username);
        pstmt->setString(3,password);
        pstmt->setString(4,accType);

        pstmt->executeQuery();
    }else{
        string sqlStatement = "insert into account(accountID,username, password,acc_type)"
        "value(?,?,?,?)";

        PreparedStatement* pstmt= con->prepareStatement(sqlStatement);

        pstmt->setString(1,getNextID("account",3));
        pstmt->setString(2,username);
        pstmt->setString(3,password);
        pstmt->setString(4,accType);

        pstmt->executeQuery();

        delete pstmt;
    }
    

    getCurUsr(username,password);

    return true;
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

bool DatabaseManager::isValidFullName(const std::string& name){
    if (name.empty()) {
        return false;
    }

    bool hasAlpha = false;

    for (char c : name) {
        // Allow alphabetic characters (A-Z, a-z)
        if (isalpha(static_cast<unsigned char>(c))) {
            hasAlpha = true;
        } 
        // Allow spaces, hyphens, and apostrophes (e.g., "Nur 'Ain", "Abdul-Rahman")
        else if (c == ' ' || c == '-' || c == '\'') {
            continue;
        } 
        // Reject numbers, symbols, and punctuation
        else {
            return false;
        }
    }

    // Must contain at least one letter (prevents input containing only spaces/hyphens)
    return hasAlpha;
}   //is valid full name

bool DatabaseManager::isValidIC(const std::string& ic){
    
    // Must be exactly 12 characters long
    if (ic.length() != 12) {
        return false;
    }

    // Every character must be a numeric digit (0-9)
    for (char c : ic) {
        if (!isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }

    bool icExists = false;
    try {
        string findICSql = "SELECT COUNT(*) FROM student WHERE ic = ?";
        PreparedStatement* fiStmt = con->prepareStatement(findICSql);
        fiStmt->setString(1, ic);

        ResultSet* fiRes = fiStmt->executeQuery();

        if (fiRes->next() && fiRes->getInt(1) > 0) {
            cout<<YELLOW<<"[ NOTICE ] "<<RESET<<"Same IC found. Enter unique IC"<<endl;
            icExists = true;
        }

        // Clean up heap pointers to prevent memory leaks
        delete fiRes;
        delete fiStmt;

    } catch (const SQLException& e) {
        cerr << "[DATABASE ERROR] Failed to verify IC: " << e.what() << endl;
        return false;
    }

    // Return false if duplicate found, true if valid and unique
    return !icExists;
}   //check ic format

bool DatabaseManager::getFeeStatus(string payerAccID){
    bool isPaid=false;

    string checkSql=" select count(*) from payment where accountID = ? and type='fees'"
        " and month(paymentDate) = month(curdate())"
        " and year(paymentDate) = year(curdate())";

    PreparedStatement* checkStmt=con->prepareStatement(checkSql);

    checkStmt->setString(1,payerAccID);

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
    cout << "  • Payment Stat : " << (getFeeStatus(currentUser) ? (GREEN + "[ PAID ]" + RESET) : (RED + "[ UNPAID ]" + RESET)) << endl;

    //if already paid
    if (getFeeStatus(currentUser)) {
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "  " << GREEN << "[NOTICE]" << RESET << " Your monthly fee has already been settled." << endl;
        cout << "           No further payment is required for this billing cycle." << endl;
        PETC();
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
    clearScreen();
    cout <<GREEN<< "╭─────────────────────────────────────────────────────────────╮" << endl;
    cout << "│                 GELANGGANG DONATION / INFAQ                 │" << endl;
    cout << "╰─────────────────────────────────────────────────────────────╯" <<RESET<< endl;
    
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
        invalidInput();
    }


    string insertStmt="insert into payment(paymentID, paymentDate, amount, type, accountID)"
     "values(?,curdate(),?,'donation',?)";
    
    PreparedStatement* dStmt=con->prepareStatement(insertStmt);

    dStmt->setString(1,getNextID("payment",4));
    dStmt->setDouble(2,amount);
    dStmt->setString(3,currentUser);
    
    dStmt->executeUpdate();
    cout<<GREEN<<"[ THANK YOU ] "<<RESET<<"You have donated RM"<<amount<<"."<<endl;
    PETC();
    delete dStmt;
} //Donate

int DatabaseManager::calcAge(string IC){
    string curDate;
    int curYear;
    int curMonth;
    int curDay;
    
    // 1. Remove non-numeric characters (handles '-' or spaces)
    string cleanIC = "";
    for (char c : IC) {
        if (isdigit(c)) {
            cleanIC += c;
        }
    }

    // Return -1 (or 0) if the IC is invalid / too short
    if (cleanIC.length() < 6) {
        return -1; 
    }

    // 2. Extract Year, Month, and Day (First 6 digits: YYMMDD)
    int birthYearShort = stoi(cleanIC.substr(0, 2));
    int birthMonth     = stoi(cleanIC.substr(2, 2));
    int birthDay       = stoi(cleanIC.substr(4, 2));

    string getCurDate = "select curdate()";

    PreparedStatement* cdStmt=con->prepareStatement(getCurDate);

    ResultSet* cdRes=cdStmt->executeQuery();

    if(cdRes->next()){
        curDate=cdRes->getString(1);
    }

    if (curDate.length() < 10) {
        return -1; // Safeguard if query returned empty
    }

    curYear = stoi(curDate.substr(0,4));
    curMonth= stoi(curDate.substr(5,2));
    curDay=stoi(curDate.substr(8,2));

    // 4. Convert 2-digit year (YY) to 4-digit year (YYYY)
    // If 2-digit birth year <= current 2-digit year (e.g. 05 <= 26 -> 2005), else (e.g. 98 -> 1998)
    int curYearShort = curYear % 100;
    int birthYearFull = (birthYearShort <= curYearShort) ? (2000 + birthYearShort) : (1900 + birthYearShort);

    // 5. Calculate base age
    int age = curYear - birthYearFull;

    // 6. Subtract 1 if the birthday has not arrived yet this year
    if (curMonth < birthMonth || (curMonth == birthMonth && curDay < birthDay)) {
        age--;
    }

    
    delete cdStmt;
    delete cdRes;
    
    return age;

}   //calculate Age

void DatabaseManager::PETC(){
    cout << "\n  Press Enter to return...";
    cin.ignore(10000, '\n');
    cin.get();
    return;
}

void DatabaseManager::invalidInput(){
    clearScreen();
    cout<<YELLOW<<"[ WARNING ] "<<RESET<<"Invalid Input"<<endl;
}

void DatabaseManager::clearScreen(){
    #if defined(_WIN32) || defined(_WIN64)
        system("cls");   // Windows command
    #else
        system("clear"); // Linux / macOS / Unix command
    #endif
}   //clear screen

string DatabaseManager::toUpperCase(string str){
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}   //to upper case

void DatabaseManager::testColor(string str){
    cout<<BLACK<<str<<" "
        <<RED<<str<<" "
        <<GREEN<<str<<" "
        <<YELLOW<<str<<" "
        <<BLUE<<str<<" "
        <<PURPLE<<str<<" "
        <<CYAN<<str<<" "
        <<WHITE<<str<<" "
        <<RESET<<endl;
}   //test color

string DatabaseManager::getHiddenPassword(const string& prompt = "Password: "){
    //made with gemini

    cout << prompt << flush;
    string password = "";

    #if defined(_WIN32) || defined(_WIN64)
        int ch;
        while (true) {
            ch = _getch();
            if (ch == 13 || ch == 10 || ch == '\r' || ch == '\n') break;
            if (ch == 8) {
                if (!password.empty()) password.pop_back();
            } else if (ch >= 32 && ch <= 126) {
                password += static_cast<char>(ch);
            }
        }
    #else
        termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        char ch;
        while ((ch = getchar()) != '\n' && ch != '\r' && ch != EOF) {
            if (ch == 127 || ch == 8) { // Backspace
                if (!password.empty()) password.pop_back();
            } else {
                password += ch;
            }
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    #endif

    cout << "\n";
    return password;
}   //hide password

bool DatabaseManager::isValidPhoneNum(const string& phoneNum){
    if(phoneNum.empty()){
        return false;
    }

    bool isValid=false;
    int digitCount=0;

    for (char c : phoneNum){
        // reject alpabet
        if (isalpha(static_cast<unsigned char>(c))) {
            return false;
        } 

        //reject '-' or spaces
        if(c=='-' || c==' '){
            return false;
        }
        ++digitCount;
        
    }
    //accept only 9-12 digits
    if(digitCount >= 10 && digitCount <=12){
        isValid=true;
    }
    return isValid;
 } //is valid phone number
