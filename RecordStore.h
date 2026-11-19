#ifndef RECORD_STORE_H
#define RECORD_STORE_H

#include <cstdio>
#include <string>
#include <unordered_set>

#ifdef _WIN32
#include <direct.h>
#define MAKE_DIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MAKE_DIR(path) mkdir(path, 0755)
#endif

/*
    Storage layer.

    RecordStore is an abstract interface (Interface Segregation: only the three
    operations the database needs). StudentDatabase depends on this abstraction,
    not on files directly (Dependency Inversion), so a new storage backend can be
    added as another subclass without touching the B+ Tree or the database code
    (Open/Closed, Liskov Substitution).
*/

struct Student {
    int rollNo;
    std::string name;
    int age;
    int marks;
};

class RecordStore {
   public:
    virtual ~RecordStore() {}
    virtual FILE* write(const Student& s) = 0;                         // persist a record, return its data pointer
    virtual bool read(FILE* dataPtr, std::string& tuple) const = 0;    // load a record through its data pointer
    virtual bool remove(int rollNo, FILE* dataPtr) = 0;                // delete a record from storage
};

/*
    Mimics disk blocks: every tuple is stored in its own file DBFiles/<rollNo>.txt,
    and the FILE* to that file is what the B+ Tree leaf stores as the data pointer.
*/
class FileRecordStore : public RecordStore {
   private:
    std::string directory;
    std::unordered_set<FILE*> openFiles;  // handles owned by this store

    std::string pathFor(int rollNo) const {
        return directory + "/" + std::to_string(rollNo) + ".txt";
    }

   public:
    explicit FileRecordStore(const std::string& dir) : directory(dir) {
        MAKE_DIR(directory.c_str());  // create DBFiles if missing (no-op if it exists)
    }

    ~FileRecordStore() override {
        for (FILE* fp : openFiles)
            fclose(fp);
    }

    FileRecordStore(const FileRecordStore&) = delete;
    FileRecordStore& operator=(const FileRecordStore&) = delete;

    FILE* write(const Student& s) override {
        FILE* fp = fopen(pathFor(s.rollNo).c_str(), "w+");  // w+ : write now, read later
        if (fp == nullptr)
            return nullptr;
        fprintf(fp, "%s %d %d\n", s.name.c_str(), s.age, s.marks);
        fflush(fp);  // make sure the tuple actually reaches the disk
        openFiles.insert(fp);
        return fp;
    }

    bool read(FILE* dataPtr, std::string& tuple) const override {
        if (dataPtr == nullptr)
            return false;
        rewind(dataPtr);  // go back to the start of the record
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), dataPtr) == nullptr)
            return false;
        tuple = buffer;
        if (!tuple.empty() && tuple.back() == '\n')
            tuple.pop_back();
        return true;
    }

    bool remove(int rollNo, FILE* dataPtr) override {
        if (dataPtr != nullptr) {
            fclose(dataPtr);  // must close before deleting on Windows
            openFiles.erase(dataPtr);
        }
        return std::remove(pathFor(rollNo).c_str()) == 0;
    }
};

#endif
