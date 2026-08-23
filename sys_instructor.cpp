#include "sys_function.h"

using namespace std;
using namespace sql;

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
        string sqlStatement="SELECT i.*, sl.classDay, COUNT(s.studentID) AS pendingCount FROM instructor i LEFT JOIN student s ON i.slotID = s.slotID AND s.stdStatus = 'pending' join slot sl on sl.slotID = i.slotID WHERE i.accountID = ?";

        PreparedStatement* pstmt=con->prepareStatement(sqlStatement);

        pstmt->setString(1,currentUser);

        ResultSet* res=pstmt->executeQuery();

        if(res->next()){
            instructorID = res->getString("instructorID");
            entityID=instructorID;
            fName = res->getString("fullName");
            userName=fName;
            entityID=instructorID;
            homeAdd = res->getString("homeAdd");
            phoneNum = res->getString("phoneNum");
            joinDate = res->getString("joinDate");
            classSlot = res->getString("classDay");
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
            viewStudents(entityID, classSlot);
            break;
        
        case '3':
            promoteStudents(entityID);
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

void DatabaseManager::viewStudents(string instructorID, string classDay){
    struct student {
        string studentID;
        string fullName;
        string contactNum;
        int age;
        string rank;
    };

    vector<student> studentList;
    

    string getStudentSql=
        "select"
        "   s.studentID, s.fullName, s.ic, "
        "   COALESCE(r.color, 'N/A') AS rankColor, "
        "   COALESCE(s.phoneNum,p.phoneNum) AS contactNum, "
        "   sl.classDay "
        "from student s "
        "left join parent p on p.parentID = p.parentID "
        "left join rank_history rh on rh.studentID = s.studentID"
        "   and rh.date_achieved ="
        "   (select "
        "       rh2.date_achieved from rank_history rh2 "
        "           where rh2.studentID=s.studentID "
        "           order by rh2.date_achieved desc limit 1)" 
        "left join rank r on rh.rankID = r.rankID "
        "join slot sl on sl.slotID = s.slotID "
        "where s.instructorID = ? and s.stdStatus = 'active' "
        "   and sl.classDay=? "
        "order by r.value desc, s.ic asc";

    PreparedStatement* sStmt=con->prepareStatement(getStudentSql);

    sStmt->setString(1,instructorID);
    sStmt->setString(2,classDay);

    ResultSet* sRes=sStmt->executeQuery();

    if (sRes->rowsCount()<=0)
    {
       cout << "No active students found for this instructor." << endl;
    }else{
        

        while (sRes->next())
        {
            student st;
            
            st.studentID=sRes->getString("studentID");
            st.fullName=sRes->getString("fullName");
            st.age=calcAge(sRes->getString("ic"));
            st.rank=sRes->getString("rankColor");
            st.contactNum=sRes->getString("contactNum");

            studentList.push_back(st);

            
        }
    }

    delete sStmt;
    delete sRes;


    //rendering
    // clearScreen();

    cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
    cout << "│                                  STUDENTS                                   │" << endl;
    cout << "╰─────────────────────────────────────────────────────────────────────────────╯" << endl;

    cout<<"  • Instructor Name : "<<userName<<endl;
    cout<<"  • Class Day : "<<classDay<<endl;

    string tableRank="";
    
    for(size_t i=0;i<studentList.size(); i++){
        student sl=studentList[i];
        //print header
        if (studentList[i].rank != tableRank){
            tableRank=sl.rank;

            int studentThisRankCount =0;
            for(int j =0;j<studentList.size(); j++){
                if(studentList[j].rank == tableRank){
                    ++studentThisRankCount;
                }
            }

            cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
            cout<<"│ "<<left<<setw(76)<<("[ "+toUpperCase(tableRank)+" ("+to_string(studentThisRankCount)+" STUDENTS)"+" ]")<<"│"<<endl;
            cout << "├──────┬────────────────────────────────────────────────┬─────┬───────────────┤" << endl;
            cout<<left<<"│ ID  "<<" │ "<<setw(46)<<" STUDENT NAME"<<" │ "<<"AGE "<<"│ "<<setw(13)<<"CONTACT"<<" │"<<endl;
            cout << "├──────┼────────────────────────────────────────────────┼─────┼───────────────┤" << endl;

        }
        
        cout << "│ " << left << studentList[i].studentID
             << " │ " << left <<setw(46)<< studentList[i].fullName
             << " │ " << right<<setw(3) << studentList[i].age
             << " │ " << left<<setw(13) << studentList[i].contactNum
             << " │"<< endl;

        if (i == studentList.size() - 1 || studentList[i + 1].rank != tableRank){
            cout << "╰──────┴────────────────────────────────────────────────┴─────┴───────────────╯" << endl;
        }
    }

    string choice;
    bool endLoop=false;
    cout << "\n───────────────────────────────────────────────────────────────" << endl;
    cout << "[ AVAILABLE ACTIONS ]" << endl;
    cout << "  [1] Promote Student" << endl;
    cout << "  [0] Exit" << endl;
    cin.ignore();
    getline(cin, choice);

    while (!endLoop)
    {
        if(choice=="0") endLoop=true;
        else if (choice =="1"){

        }
    }
    

    PETC();
    return;

} //view students

void DatabaseManager::promoteStudents(string instructorID){
    string name;
    string choice;

    cout<<"Enter name "<<endl;
    cout<<" >> ";
    cin.ignore();
    getline(cin, name);

    struct selectedStudent{
        string studentID;
        string fullName;
        int age;
        string curRank;
        string nextRank;
    };

    vector<selectedStudent> foundList;
    vector<selectedStudent> selectedList;
    
    //find student from name
    string findStudentSQL = 
        "select "
        "   s.studentID, s.fullName, s.ic, "
        "   rh.rankID, r.color "
        "from student s "
        "join rank_history rh on rh.studentID = s.studentID "
        "   and rh.date_achieved = "
        "       (select date_achieved from rank_history where studentID = s.studentID "
        "       order by date_achieved desc limit 1) "
        "join rank r on r.rankID = rh.rankID "
        "where s.fullname like ?";

    PreparedStatement* fsStmt=con->prepareStatement(findStudentSQL);

    fsStmt->setString(1, ("%"+name+"%"));

    ResultSet* fsRes=fsStmt->executeQuery();
    
    while (fsRes->next())   
    {
        selectedStudent s;
        s.studentID   = fsRes->getString("studentID");
        s.fullName    = fsRes->getString("fullName");
        s.age         = calcAge(fsRes->getString("ic"));
        s.curRank     = fsRes->getString("rankID");
        s.nextRank    = ""; // Map to next rankID according to your belt hierarchy
        foundList.push_back(s);
    }
    

    selectedStudent ss;
    if (fsRes->rowsCount()==0)
    {
        cout<<"No student found :("<<endl;
        return;

    }else if(fsRes->rowsCount()==1){
        // cout<<"Found "<<fsRes->rowsCount()<<" student"<<endl;
        while (fsRes->next())
        {
            
            cout << fsRes->getString("fullName")<<endl;
        }

        cout<<"Select this student? (y/n)";
        getline(cin,choice);

        if(choice=="y"||choice=="Y"){
            cout<<"Student selected"<<endl;
            ss.studentID=fsRes->getString("studentID");
            ss.fullName=fsRes->getString("fullName");
            ss.age=calcAge(fsRes->getString("ic"));
            ss.curRank=fsRes->getString("rankID");
        }
        
    }else{
        cout<<"Found "<<fsRes->rowsCount()<<" student"<<endl;
        cout<<WHITE<<"    "<<left<<setw(40)<<"NAME"<<setw(10)<<"RANK"<<"AGE"<<RESET<<endl;
        int foundCount=0;
        while (fsRes->next())
        {
            ++foundCount;
            
            cout <<"["<<foundCount<<"] "
                <<left<<setw(40)<< fsRes->getString("fullName")
                <<left<<setw(10)<<fsRes->getString("color")
                <<right<<setw(3)<<calcAge(fsRes->getString("ic"))
                <<endl;
        }
    }
    string promoteSql = 
        "insert into rank_history(rankID, studentID, date_achieved, instructorID) "
        "values(?,?,curdate(),?)";
}   //promote students

string DatabaseManager::getNextRank(string rankID){
    ostringstream oss;
    int curNum=stoi(rankID.substr(1));
    int nextNum=curNum+1;
    
    if (curNum < 1 || curNum > 7)
    {
        return "what?";
    }
    
    if (curNum != 7 )
    {
        //combine char r and next number
        oss<<'r'<<nextNum;
        return oss.str();
        
    }else if(curNum == 7){
        cout<<"Reached highest rank!!"<<endl;
        return rankID;
    }
    

    return "how you reach here?";
}   //get next rank