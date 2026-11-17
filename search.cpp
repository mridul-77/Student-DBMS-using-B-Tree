#include <iostream>
#include<algorithm>
#include<string>
#include "B+ Tree.h"


void BPTree::search(int key) {
    if (root == NULL) {
        cout << "NO Tuples Inserted yet" << endl;
        return;
    } else {
        Node* cursor = root;
        while (cursor->isLeaf == false) {
            /*
				upper_bound returns an iterator pointing to the first element in the range
				[first,last) which has a value greater than �val�.(Because we are storing the
				same value in the right node;(STL is doing Binary search at back end)
			*/
            int idx = std::upper_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();
            cursor = cursor->ptr2TreeOrData.ptr2Tree[idx];  //upper_bound takes care of all the edge cases
        }

        int idx = std::lower_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();  //Binary search

        if (idx == cursor->keys.size() || cursor->keys[idx] != key) {
            cout << "HUH!! Key NOT FOUND" << endl;
            return;
        }

        /*
			We can fetch the data from the disc in main memory using data-ptr
			using cursor->dataPtr[idx]
		*/

        string fileName = "DBFiles/";
        string data;
        fileName += to_string(key) + ".txt";
        ifstream file(fileName);
        cout << "Hurray!! Key FOUND" << endl;
        if (!file) {
            cout << "But could not open " << fileName << endl;
            return;
        }
        getline(file, data);
        cout << "Corresponding Tuple Data is: " << data << endl;
    }
}