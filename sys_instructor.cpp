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
    string slotID;
    int pendingCount=0;
    int withdrawCount=0;
    char choice;
    bool endLoop=false;

    while(!endLoop){
        //get instructor info
        string sqlStatement=
            "SELECT "
            "i.*, sl.slotID, "
            "COUNT(s.studentID) AS pendingCount, "
            "COUNT(w.studentID) AS withdrawCount "
            "FROM instructor i "
            "LEFT JOIN student s ON i.slotID = s.slotID "
            "   AND s.stdStatus = 'pending' "
            "join slot sl on sl.slotID = i.slotID "
            "left join withdraw w on w.instructorID = i.instructorID "
            "   AND w.wthStatus = 'pending' "
            "WHERE i.accountID = ?";

        PreparedStatement* pstmt=con->prepareStatement(sqlStatement);

        pstmt->setString(1,currentUser);

        ResultSet* res=pstmt->executeQuery();

        if(res->next()){
            instructorID = res->getString("instructorID");
            entityID=instructorID;
            fName = res->getString("fullName");
            userName=fName;
            homeAdd = res->getString("homeAdd");
            phoneNum = res->getString("phoneNum");
            joinDate = res->getString("joinDate");
            slotID = res->getString("slotID");
            pendingCount = stoi(res->getString("pendingCount"));
            withdrawCount = stoi(res->getString("withdrawCount"));
        }

        //rendering
        clearScreen();
        cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
        cout << "│                             INSTRUCTOR DASHBOARD                            │" << endl;
        cout << "╰─────────────────────────────────────────────────────────────────────────────╯" << endl;
        cout << "\nHi, "<<fName << endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "[ AVAILABLE ACTIONS ]" << endl;
        cout << "  [1] Pending Approval ("<<pendingCount<<")"<< endl;
        cout << "  [2] View Students" << endl;
        cout << "  [3] Withdrawal Requests ("<<(withdrawCount!=0? (RED):(RESET))<<withdrawCount<<RESET<<")"<< endl;
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
            studentApproval(instructorID, slotID);
            break;
        
        
        case '2':
            //view students
            viewStudents(entityID, slotID);
            break;
        
        case '3':
            studentWithrawal(instructorID);
            break;

        default:
            cout<<"invalid input"<<endl;
            break;
        }
    }
}   // instructor dashboard


void DatabaseManager::studentApproval(string instructorID, string slotID){
    clearScreen();
    cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
    cout << "│                               STUDENT APPROVAL                              │" << endl;
    cout << "╰─────────────────────────────────────────────────────────────────────────────╯" << endl;
    struct pendingStudent
    {
        int digit;
        string studentID;
        string studentName;
        string phoneNum; 
        int age;
    };
    
    vector<pendingStudent> pendingList; //to store studentID of to-be-approve student
    

    string sqlStatement="select * from student where slotID=? and stdStatus = 'pending'";

    PreparedStatement* pstmt=con->prepareStatement(sqlStatement);
    
    pstmt->setString(1,slotID);

    ResultSet* res=pstmt->executeQuery();

    //display student list in table view
    cout<<right<<setw(4)<<"No "<<left<<setw(30)<<"Name"<<setw(15)<<"Contact"<<setw(5)<<"Age"<<endl;
    cout<<"───────────────────────────────────────────────────────────────"<<endl;
    int pendingCount=0;
    while(res->next()){
        pendingStudent s;

        ++pendingCount;
        s.digit=pendingCount;
        s.studentID=res->getString("studentID");
        s.studentName=res->getString("fullName");
        s.phoneNum=res->getString("phoneNum");
        s.age=calcAge(res->getString("ic"));

        pendingList.push_back(s);

        cout<<right<<setw(3)<<pendingCount<<" "<<left<<setw(30)<<s.studentName<<setw(15)<<s.phoneNum<<left<<setw(5)<<s.age<<endl;
    }

    //check if list is empty
    if (pendingList.empty())
    {   
        cout<<YELLOW<<"\n[ NOTICE ]"<<RESET<<"No pending student registration for the class slot: "<<getSlotDay(slotID)<<endl;
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

    cout <<GREEN<<"\n[ SUCCESS ] "<<RESET<< " Approved " << approvedCount << " student(s)!\n";
    
    delete pstmt;
    delete istmt;
    delete updStmt;
    delete res;

    PETC();
}   //student Approval

void DatabaseManager::viewStudents(string instructorID, string slotID){
    
    struct student {
        string studentID;
        string fullName;
        string contactNum;
        int age;
        string rank;
    };
    string choice;
    bool endLoop=false;    

    string getStudentSql=
        "select"
        "   s.studentID, s.fullName, s.ic, "
        "   COALESCE(r.color, 'N/A') AS rankColor, "
        "   COALESCE(s.phoneNum,p.phoneNum) AS contactNum, "
        "   sl.classDay "
        "from student s "
        "left join parent p on s.parentID = p.parentID "
        "left join rank_history rh on rh.studentID = s.studentID"
        "   and rh.rankID ="
        "   (select "
        "       rh2.rankID from rank_history rh2 "
        "           where rh2.studentID=s.studentID "
        "           order by rh2.date_achieved desc, "
        "                    rh2.rankID desc limit 1) " 
        "left join rank r on rh.rankID = r.rankID "
        "join slot sl on sl.slotID = s.slotID "
        "where s.instructorID = ? and s.stdStatus = 'active' "
        "   and sl.slotID=? "
        "order by r.value desc, s.ic asc"
    ;

    while (!endLoop)
    {
        clearScreen();
        vector<student> studentList;

        PreparedStatement* sStmt=con->prepareStatement(getStudentSql);

        sStmt->setString(1,instructorID);
        sStmt->setString(2,slotID);

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
        cout << "\n╭─────────────────────────────────────────────────────────────────────────────╮" << endl;
        cout << "│                                  STUDENTS                                   │" << endl;
        cout << "╰─────────────────────────────────────────────────────────────────────────────╯" << endl;

        cout<<"  • Instructor Name : "<<GREEN<<userName<<RESET<<endl;
        cout<<"  • Class Day       : "<<GREEN<<getSlotDay(slotID)<<RESET<<endl;
        cout<<"  • Total Students  : "<<GREEN<<studentList.size()<<" Students"<<RESET<<endl;

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
                cout<<"│ "<<WHITE<<left<<setw(76)<<("[ "+toUpperCase(tableRank)+" ("+to_string(studentThisRankCount)+" STUDENTS)"+" ]")<<RESET<<"│"<<endl;
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
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "[ AVAILABLE ACTIONS ]" << endl;
        cout << "  [1] Promote Student" << endl;
        cout << "  [0] Exit" << endl;
        cout << "\n───────────────────────────────────────────────────────────────" << endl;
        cout << "   Select an option: ";
        getline(cin>>ws, choice);

        if(choice=="0"){
            endLoop=true;
        }
        else if (choice =="1"){
            promoteStudents(instructorID);
        }else{
            invalidInput();
        }
    }

    return;

} //view students

void DatabaseManager::promoteStudents(string instructorID){
    string name;
    string choice;
    bool endLoop=false;
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
        "   and rh.rankID = "
        "       (select rankID from rank_history where studentID = s.studentID "
        "       order by date_achieved desc, rankID desc limit 1) "
        "join rank r on r.rankID = rh.rankID "
        "where s.fullname like ? and s.instructorID=? " 
        "order by r.rankID desc, s.ic asc";
    
    while (!endLoop)
    {
        //clear the found list vector
        foundList.clear();

        // Display selected student list
        if (!selectedList.empty()) {
            cout << "\n╭────────────────────────────────────────────────────────────────╮" << endl;
            cout << "│                SELECTED STUDENTS FOR PROMOTION                 │" << endl;
            cout << "├──────┬─────────────────────────────────────┬──────────┬────────┤" << endl;
            cout << "│ ID   │ NAME                                │ CURRENT  │ AFTER  │" << endl;
            cout << "├──────┼─────────────────────────────────────┼──────────┼────────┤" << endl;

            for (size_t i = 0; i < selectedList.size(); ++i) {
                const selectedStudent& sl = selectedList[i];
                cout <<"│ "<< left  << setw(4)  << sl.studentID << " │ "
                    << left  << setw(35) << sl.fullName  << " │ "
                    << left  << setw(8)  << getRankColor(sl.curRank)  << " │ "
                    << left  << setw(6)  << getRankColor(sl.nextRank) << " │" 
                    << endl;
            }

            cout << "╰──────┴─────────────────────────────────────┴──────────┴────────╯" << endl;
        }

        cout<<"\nEnter 0 or cancel to cancel"<<endl;
        cout<<"Enter name to find"<<endl;
        cout<<" >> "<<GREEN;
        getline(cin >> ws, name);
        cout<<RESET;

        if(name=="0" ||name=="cancel"){
            endLoop=true;
        }

        PreparedStatement* fsStmt=con->prepareStatement(findStudentSQL);

        fsStmt->setString(1, ("%"+name+"%"));
        fsStmt->setString(2, instructorID);

        ResultSet* fsRes=fsStmt->executeQuery();
        
        //insert found students into the found list
        while (fsRes->next())   
        {
            selectedStudent s;
            s.studentID   = fsRes->getString("studentID");
            s.fullName    = fsRes->getString("fullName");
            s.age         = calcAge(fsRes->getString("ic"));
            s.curRank     = fsRes->getString("rankID");
            s.nextRank    = getNextRank(s.curRank); // Map to next rankID according to your belt hierarchy
            foundList.push_back(s);
        }

        delete fsRes;
        delete fsStmt;

        if(foundList.empty()){
            if(name!="0" && name!="cancel"){
                cout<<YELLOW<<"[ ERROR ]"<<" Did not found any student named "<<name<<RESET<<endl;
            }

        }else if (foundList.size()==1){ //if only 1 student found
            cout<<GREEN<<"\nFound 1 student"<<RESET<<endl;
            cout << "    " << left << setw(40) << "NAME" << setw(15) << "RANK" << "AGE" << endl;
            cout << "─────────────────────────────────────────────────────────────" << endl;

            cout<< left <<"  • "<< setw(40) << foundList[0].fullName
                << left << setw(15) << getRankColor(foundList[0].curRank)
                << right << setw(3) << foundList[0].age
                << endl;

            cout<<BLUE<<"\nSelect this student? (y/n): ";
            getline(cin>>ws,choice);
            cout<<RESET;
            if(choice=="y" || choice=="Y"){
                selectedList.push_back(foundList[0]);
                cout <<GREEN<< "Student added to selection."<<RESET << endl;
            }else {
                cout <<RED<< "Selection cancelled." <<RESET << endl;
            }
        }else{ //found more than 1 students
            cout<<GREEN<<"\nFound "<<foundList.size()<<" student"<<RESET<<endl;
            cout << "    " << left << setw(40) << "NAME" << setw(15) << "RANK" << "AGE" << endl;
            cout << "─────────────────────────────────────────────────────────────" << endl;
            
            for (size_t i = 0; i < foundList.size(); ++i) {
                cout << "[" << (i + 1) << "] "
                    << left << setw(40) << foundList[i].fullName
                    << left << setw(15) << getRankColor(foundList[i].curRank)
                    << right << setw(3) << foundList[i].age
                    << endl;
            }
            int choiceIndex=0;
            cout << "\nEnter student number to select (1-" << foundList.size() << ") or 0 to cancel: ";
            cin>>choiceIndex;
                
                

            if(choiceIndex >=1 && choiceIndex <= foundList.size()){
                string confirm;

                cout<<BLUE<<"Confirm selection of "<<foundList[choiceIndex-1].fullName<<"? (y/n): "<<RESET;
                getline(cin>>ws,confirm);

                if (confirm == "y" || confirm == "Y"){
                    selectedList.push_back(foundList[choiceIndex - 1]);
                    cout <<GREEN<< "Student added to selection." <<RESET<< endl;
                }else {
                    cout <<RED<< "Selection aborted." <<RESET<< endl;

                }
                
            }else if(choiceIndex == 0) {
                cout <<YELLOW<< "Operation cancelled" <<RESET<< endl;
                
            }else{
                invalidInput();
            }
        }
    }

    if (selectedList.empty()) {
        cout<<YELLOW<<"[ WARNING ] No selected student. Aborting promotion"<<RESET<<endl;
        PETC();
        return;
    }

    // Final Promotion Confirmation
    bool validOption=false;
    while (!validOption)
    {
        string finalConfirm;
        cout << "\nConfirm promotion of all selected student(s)? (y/n): ";     
        getline(cin>>ws, finalConfirm);

        if (finalConfirm=="y" || finalConfirm=="Y"){
            string promoteSql = 
                "insert into rank_history(rankID, studentID, date_achieved, instructorID) "
                "values(?,?,curdate(),?)";

            PreparedStatement* pStmt=con->prepareStatement(promoteSql);

            for (size_t i=0; i<selectedList.size();++i){
                selectedStudent sl=selectedList[i];
                pStmt->setString(1,sl.nextRank);
                pStmt->setString(2,sl.studentID);
                pStmt->setString(3,instructorID);
                pStmt->executeUpdate();
                cout <<GREEN<< "Successfully promoted " << sl.fullName << "!" <<RESET << endl;

            }
            delete pStmt;
            PETC();
            return;

        }else if(finalConfirm=="n" || finalConfirm=="N"){
            cout <<RED<< "Promotion cancelled." <<RESET<< endl;
            PETC();
            return;

        }else{
            invalidInput();
        }
    }
    
    
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

string DatabaseManager::getRankColor(string rankID){
    string getColor="select color from rank where rankID = ?";
    
    PreparedStatement* gcStmt=con->prepareStatement(getColor);
    gcStmt->setString(1,rankID);
    
    ResultSet* gcRes=gcStmt->executeQuery();
    
    string color="";
    if(gcRes->next()){
        color=gcRes->getString("color");
    }

    delete gcStmt;
    delete gcRes;
    return color;
    
}   //get rank color

string DatabaseManager::getSlotDay(string slotID){
    string getDaySql="select classDay from slot where slotID=?";
    string day="";
    
    PreparedStatement* gdStmt=con->prepareStatement(getDaySql);
    gdStmt->setString(1,slotID);
    
    ResultSet* gdRes=gdStmt->executeQuery();

    if(gdRes->next()){
        day=gdRes->getString("classDay");
    }
    return day;
}

void DatabaseManager::studentWithrawal(string instructorID){

    struct studentRequest{
        string studentID;
        string withdrawID;
        string studentName;
        int age;
        string curRank;
        string reason;
    };

    bool endLoop=false;

    //get all student under this instructor from withdraw table
    string getStdSql = 
        "select "
        "s.studentID, s.fullName, s.ic, rh.rankID,w.withdrawID, w.reason "
        "from student s "
        "join withdraw w on w.studentID = s.studentID"
        "   and w.wthStatus ='pending' "
        "join rank_history rh on rh.studentID = s.studentID "
        "   and rh.rankID = (select "
        "       rankID from rank_history where studentID = s.studentID "
        "       order by rankID desc limit 1) "
        "where s.instructorID = ? "
        "order by w.wthDate asc, w.withdrawID asc";
      
    vector<studentRequest> requestList;

    while (!endLoop)
    {
        
        requestList.clear();

        PreparedStatement* wdrwStmt = con->prepareStatement(getStdSql);
        wdrwStmt->setString(1,instructorID);

        ResultSet* wdrwRes=wdrwStmt->executeQuery();

        while (wdrwRes->next()) 
        {
            studentRequest sr;

            sr.withdrawID=wdrwRes->getString("withdrawID");
            sr.studentID=wdrwRes->getString("studentID");
            sr.studentName=wdrwRes->getString("fullName");
            sr.age=calcAge(wdrwRes->getString("ic"));
            sr.curRank=getRankColor(wdrwRes->getString("rankID"));
            sr.reason=wdrwRes->getString("reason");

            requestList.push_back(sr);
        }
        
        delete wdrwRes;
        delete wdrwStmt;

        
        //diplay in table view (studentID, name, currank, age, reason) sory by oldest date
        cout << "\n╭────────────────────────────────────────────────────────────────────────────────────────╮" << endl;
        cout << "│                              PENDING WITHDRAWAL REQUESTS                               │" << endl;
        cout << "╰────────────────────────────────────────────────────────────────────────────────────────╯" << endl;

        //if no request, returnws
        if (requestList.empty()) {
            cout <<YELLOW<< "\n[ NOTICE ] "<<RESET<<"No pending withdrawal requests.";
            PETC();
            return;
        }
        
        //print header
        cout << "\n╭──────┬─────────────────────────────────────┬─────┬─────────────────────────────────────╮" << endl;

        cout << "│ " << left << setw(4) << " ID" 
            << " │ " << setw(35) << "NAME" 
            << " │ " << right << setw(3)  << "AGE" 
            << " │ " << left << setw(10) << "RANK" 
            << right << setw(29)<< " │" << endl;
        
        for (size_t i = 0; i < requestList.size(); ++i) {
            studentRequest rl = requestList[i];
            string color =(i%2 == 0 ? BLUE:CYAN);

            if (i==0)
            {
                cout << "├──────┼─────────────────────────────────────┼─────┼─────────────────────────────────────┤"<<endl;

            }else{
                cout << "├──────┬─┴───────────────────────────────────┬─────┬─────────────────────────────────────┤"<<endl;

            }
            
            //cout << "├──────┼─────────────────────────────────────┼─────┼─────────────────────────────────────┤"<<endl;
            cout << RESET << "│ " << color << left << setw(4)  << rl.studentID 
                << RESET << " │ " << color << setw(35)<< rl.studentName 
                << RESET << " │ " << color << right << setw(3) << rl.age 
                << RESET << " │ " << color << left << setw(10) << rl.curRank 
                << RESET << right << setw(29)<< " │" << endl;;
            cout << "├──────┴─┬───────────────────────────────────┴─────┴─────────────────────────────────────┤"<<endl;
            cout << "│ " << color << "Reason" << RESET <<" │ " << color << left <<setw(77) << rl.reason << RESET <<" │"<< endl;
        }
        cout << "╰────────┴───────────────────────────────────────────────────────────────────────────────╯" << endl;


        //user enter studentID
        string input;
        string choice;
        cout << "──────────────────────────────────────────────────────────────────────────────────────────" << endl;
        cout << "Enter studentID to select (or '0' / 'cancel' to abort): ";
        getline(cin>>ws, input);

        if(input == "0" || input == "cancel"){
            cout<<"\nAborting process..."<<endl;
            PETC();
            return;
        }

        //approve or reject withdraw request
        //if approved, update wthStatus on withdraw table to 'approved' and update stdStatus to 'withdrawn'
        //if rejected, update wthStatus on withdraw table to 'rejected'
        int selectedIndex = -1;
        for (size_t i = 0; i < requestList.size(); ++i) {
            if (requestList[i].studentID == input) {
                selectedIndex = i;
                break;
            }
        }

        if (selectedIndex == -1) {
            cout <<YELLOW<< "\n[ ERROR ]"<<RESET<<" Student ID '" << input << "' not found in the pending list." << endl;
            PETC();
            continue;
        }

        studentRequest target = requestList[selectedIndex];

        // 5. Review & Decision
        cout << "\n[ SELECTED STUDENT ]" << endl;
        cout << "  • ID     : " << target.studentID << endl;
        cout << "  • Name   : " << target.studentName << endl;
        cout << "  • Rank   : " << target.curRank << endl;
        cout << "  • Reason : " << target.reason << endl;

        cout << "\nApprove " << target.studentName << "'s withdrawal? (y/n): ";
        getline(cin >> ws, choice);

        string updateWithdrawSql = "UPDATE withdraw SET wthStatus = ? WHERE withdrawID = ?";

        PreparedStatement* updWthStmt = con->prepareStatement(updateWithdrawSql);
        if (choice == "y" || choice == "Y") {
            
            // Update withdraw table to 'approved'
            updWthStmt->setString(1, "approved");
            updWthStmt->setString(2, target.withdrawID);
            updWthStmt->executeUpdate();
            delete updWthStmt;

            // Update student table stdStatus to withdrawn
            string updateStudentSql = "UPDATE student SET stdStatus = 'withdrawn' WHERE studentID = ?";
            PreparedStatement* updStdStmt = con->prepareStatement(updateStudentSql);
            updStdStmt->setString(1, target.studentID);
            updStdStmt->executeUpdate();
            delete updStdStmt;

            clearScreen();
            cout << GREEN << "\n[ SUCCESS ]"<<RESET<<" Withdrawal approved. Student status updated to 'withdraw'." << endl;
        } else if (choice == "n" || choice == "N") {
            
            // Update withdraw table to 'rejected'
            updWthStmt->setString(1, "rejected");
            updWthStmt->setString(2, target.withdrawID);
            updWthStmt->executeUpdate();
            delete updWthStmt;

            cout <<YELLOW<< "\n[ NOTICE ]"<<RESET<<" Withdrawal request rejected." << endl;
        } else {
            invalidInput();
        }
    }
    
    PETC();
}   //student withdrawal