#define _CRT_SECURE_NO_WARNINGS  //for VS 2019 (must come before any include)

#include <iostream>
#include <string>
#include "B+ Tree.h"
#include "RecordStore.h"
#include "StudentDatabase.h"

using namespace std;

/*
    main only handles the user interface (menu + input).
    Indexing lives in BPTree, storage in FileRecordStore, and
    StudentDatabase connects the two.
*/

void insertionMethod(StudentDatabase& db) {
    Student s;
    cout << "Please provide the rollNo: ";
    cin >> s.rollNo;

    cout << "\nWhat's the Name, Age and Marks acquired?: ";
    cin >> s.name >> s.age >> s.marks;

    db.insert(s);
}

void searchMethod(const StudentDatabase& db) {
    int rollNo;
    cout << "What's the RollNo to Search? ";
    cin >> rollNo;

    db.search(rollNo);
}

void printMethod(const StudentDatabase& db) {
    int opt;
    cout << "Press \n\t1.Hierarical-Display \n\t2.Sequential-Display\n";
    cin >> opt;

    cout << "\nHere is your File Structure" << endl;
    if (opt == 1)
        db.display();
    else
        db.seqDisplay();
}

void deleteMethod(StudentDatabase& db) {
    cout << "Showing you the Tree, Choose a key from here: " << endl;
    db.display();

    int tmp;
    cout << "Enter a key to delete: " << endl;
    cin >> tmp;
    db.remove(tmp);

    //Displaying
    db.display();
}

int main() {
    /*
        Please have a look at the default schema to get to know about the table
        Reference - img/database.jpg
    */

    cout << "\n***Welcome to DATABASE SERVER**\n"
         << endl;

    int maxChildInt = 4, maxNodeLeaf = 3;
    cout << "Please provide the value to limit maximum child Internal Nodes can have: ";
    cin >> maxChildInt;
    cout << "\nAnd Now Limit the value to limit maximum Nodes Leaf Nodes can have: ";
    cin >> maxNodeLeaf;

    if (maxChildInt < 3 || maxNodeLeaf < 2) {
        cout << "\nInternal nodes need at least 3 children and leaves at least 2 keys. Using defaults 4 and 3." << endl;
        maxChildInt = 4;
        maxNodeLeaf = 3;
    }

    BPTree bPTree(maxChildInt, maxNodeLeaf);
    FileRecordStore store("DBFiles");
    StudentDatabase db(bPTree, store);

    bool flag = true;
    int option;

    do {
        cout << "\nPlease provide the queries with respective keys : " << endl;
        cout << "\tPress 1: Insertion \n\tPress 2: Search \n\tPress 3: Print Tree\n\tPress 4: Delete Key In Tree\n\tPress 5: ABORT!" << endl;
        if (!(cin >> option))
            break;  // input closed or not a number

        switch (option) {
            case 1:
                insertionMethod(db);
                break;
            case 2:
                searchMethod(db);
                break;
            case 3:
                printMethod(db);
                break;
            case 4:
                deleteMethod(db);
                break;
            default:
                flag = false;
                break;
        }
    } while (flag);

    return 0;
}
