#include <iostream>
#include <queue>
#include "B+ Tree.h"

using namespace std;

void BPTree::display() const {
    /*
        Level Order Display (BFS): one tree level per line, nodes separated by ||
    */
    if (root == nullptr) {
        cout << "No Data in the Database yet!" << endl;
        return;
    }
    queue<Node*> q;
    q.push(root);

    while (!q.empty()) {
        int sz = q.size();
        for (int i = 0; i < sz; i++) {
            Node* u = q.front();
            q.pop();

            //printing keys in self
            for (int val : u->keys)
                cout << val << " ";

            cout << "|| ";  //to seperate next adjacent nodes

            if (!u->isLeaf()) {
                for (Node* v : asInternal(u)->ptr2Tree)
                    q.push(v);
            }
        }
        cout << endl;
    }
}

void BPTree::seqDisplay() const {
    /*
        Sequential Display: walk the linked list of leaves (all keys in sorted order)
    */
    LeafNode* firstLeft = firstLeftNode(root);

    if (firstLeft == nullptr) {
        cout << "No Data in the Database yet!" << endl;
        return;
    }
    while (firstLeft != nullptr) {
        for (int key : firstLeft->keys)
            cout << key << " ";
        firstLeft = firstLeft->ptr2next;
    }
    cout << endl;
}
