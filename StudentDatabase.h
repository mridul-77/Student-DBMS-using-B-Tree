#ifndef STUDENT_DATABASE_H
#define STUDENT_DATABASE_H

#include <iostream>
#include <string>
#include "B+ Tree.h"
#include "RecordStore.h"

/*
    StudentDatabase coordinates the two components (Single Responsibility):
        - BPTree      : the index (rollNo -> data pointer), O(log N) lookups
        - RecordStore : where the tuples actually live

    It depends on the RecordStore abstraction, not a concrete class (Dependency
    Inversion), so any RecordStore subclass can be plugged in.
*/
class StudentDatabase {
   private:
    BPTree& index;
    RecordStore& store;

   public:
    StudentDatabase(BPTree& idx, RecordStore& st) : index(idx), store(st) {}

    bool insert(const Student& s) {
        if (index.search(s.rollNo) != nullptr) {
            std::cout << "Roll No " << s.rollNo << " already exists! Primary key must be unique." << std::endl;
            return false;
        }
        FILE* dataPtr = store.write(s);
        if (dataPtr == nullptr) {
            std::cout << "Could not create the record file for roll No: " << s.rollNo << std::endl;
            return false;
        }
        index.insert(s.rollNo, dataPtr);
        std::cout << "Insertion of roll No: " << s.rollNo << " Successful" << std::endl;
        return true;
    }

    void search(int rollNo) const {
        FILE* dataPtr = index.search(rollNo);
        if (dataPtr == nullptr) {
            std::cout << "HUH!! Key NOT FOUND" << std::endl;
            return;
        }
        std::string tuple;
        std::cout << "Hurray!! Key FOUND" << std::endl;
        if (store.read(dataPtr, tuple))
            std::cout << "Corresponding Tuple Data is: " << tuple << std::endl;
        else
            std::cout << "But the record could not be read" << std::endl;
    }

    void remove(int rollNo) {
        FILE* dataPtr = index.removeKey(rollNo);
        if (dataPtr == nullptr)
            return;  // key was not in the tree
        if (store.remove(rollNo, dataPtr))
            std::cout << "SuccessFully Deleted record file of roll No: " << rollNo << std::endl;
        else
            std::cout << "Unable to delete the record file of roll No: " << rollNo << std::endl;
    }

    void display() const { index.display(index.getRoot()); }
    void seqDisplay() const { index.seqDisplay(index.getRoot()); }
};

#endif
