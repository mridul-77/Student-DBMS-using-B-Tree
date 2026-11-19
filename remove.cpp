#include <iostream>
#include <algorithm>
#include "B+ Tree.h"

using namespace std;

FILE* BPTree::removeKey(int x) {
    // If tree is empty
    if (root == nullptr) {
        cout << "B+ Tree is Empty" << endl;
        return nullptr;
    }

    // Going to the Leaf Node which may contain the *key*, remembering the parent
    // and which child of the parent we went into (siblings are childIdx-1 and childIdx+1)
    InternalNode* parent = nullptr;
    int childIdx = -1;
    LeafNode* cursor = findLeaf(x, &parent, &childIdx);

    // Check if the value exists in this leaf node (binary search)
    int pos = lower_bound(cursor->keys.begin(), cursor->keys.end(), x) - cursor->keys.begin();
    if (pos == (int)cursor->keys.size() || cursor->keys[pos] != x) {
        cout << "Key Not Found in the Tree" << endl;
        return nullptr;
    }

    // Remove key & its data pointer from the leaf (vector::erase shifts the rest left)
    FILE* removedData = cursor->dataPtr[pos];
    cursor->keys.erase(cursor->keys.begin() + pos);
    cursor->dataPtr.erase(cursor->dataPtr.begin() + pos);
    cout << "Deleted " << x << " From Leaf Node successfully" << endl;

    // If it is leaf as well as the root node
    if (cursor == root) {
        if (cursor->keys.empty()) {
            // Tree becomes empty
            delete cursor;
            root = nullptr;
            cout << "Ohh!! Our Tree is Empty Now :(" << endl;
        }
        return removedData;
    }

    int minLeafKeys = (maxLeafNodeLimit + 1) / 2;  // ceil(maxLeafNodeLimit / 2)
    if ((int)cursor->keys.size() >= minLeafKeys) {
        //Sufficient keys available for invariant to hold
        return removedData;
    }

    cout << "UnderFlow in the leaf Node Happended" << endl;
    cout << "Starting Redistribution..." << endl;

    int leftSibling = childIdx - 1;
    int rightSibling = childIdx + 1;

    //1. Try to borrow a key from leftSibling
    if (leftSibling >= 0) {
        LeafNode* leftNode = asLeaf(parent->ptr2Tree[leftSibling]);

        //Check if LeftSibling has extra Key to transfer
        if ((int)leftNode->keys.size() > minLeafKeys) {
            //Transfer the maximum key from the left Sibling
            cursor->keys.insert(cursor->keys.begin(), leftNode->keys.back());
            cursor->dataPtr.insert(cursor->dataPtr.begin(), leftNode->dataPtr.back());
            leftNode->keys.pop_back();
            leftNode->dataPtr.pop_back();

            //Update Parent: separator between leftNode and cursor
            parent->keys[leftSibling] = cursor->keys[0];
            cout << "Transferred from left sibling of leaf node" << endl;
            return removedData;
        }
    }

    //2. Try to borrow a key from rightSibling
    if (rightSibling < (int)parent->ptr2Tree.size()) {
        LeafNode* rightNode = asLeaf(parent->ptr2Tree[rightSibling]);

        //Check if RightSibling has extra Key to transfer
        if ((int)rightNode->keys.size() > minLeafKeys) {
            //Transfer the minimum key from the right Sibling
            cursor->keys.push_back(rightNode->keys.front());
            cursor->dataPtr.push_back(rightNode->dataPtr.front());
            rightNode->keys.erase(rightNode->keys.begin());
            rightNode->dataPtr.erase(rightNode->dataPtr.begin());

            //Update Parent: separator between cursor and rightNode
            parent->keys[childIdx] = rightNode->keys[0];
            cout << "Transferred from right sibling of leaf node" << endl;
            return removedData;
        }
    }

    //3. Merge and Delete Node (the right node of the pair is always the one removed)
    if (leftSibling >= 0) {
        // leftNode <- cursor
        LeafNode* leftNode = asLeaf(parent->ptr2Tree[leftSibling]);
        leftNode->keys.insert(leftNode->keys.end(), cursor->keys.begin(), cursor->keys.end());
        leftNode->dataPtr.insert(leftNode->dataPtr.end(), cursor->dataPtr.begin(), cursor->dataPtr.end());
        leftNode->ptr2next = cursor->ptr2next;
        cout << "Merging two leaf Nodes" << endl;
        removeInternal(parent, cursor);  //delete parent Node Key
        delete cursor;
    } else {
        // cursor <- rightNode
        LeafNode* rightNode = asLeaf(parent->ptr2Tree[rightSibling]);
        cursor->keys.insert(cursor->keys.end(), rightNode->keys.begin(), rightNode->keys.end());
        cursor->dataPtr.insert(cursor->dataPtr.end(), rightNode->dataPtr.begin(), rightNode->dataPtr.end());
        cursor->ptr2next = rightNode->ptr2next;
        cout << "Merging two leaf Nodes" << endl;
        removeInternal(parent, rightNode);  //delete parent Node Key
        delete rightNode;
    }
    return removedData;
}

void BPTree::removeInternal(InternalNode* cursor, Node* child) {
    /*
        Removes `child` from cursor's children together with the separator key
        just left of it (child is always the right node of a merged pair).
    */
    int pos = find(cursor->ptr2Tree.begin(), cursor->ptr2Tree.end(), child) - cursor->ptr2Tree.begin();
    cursor->keys.erase(cursor->keys.begin() + pos - 1);
    cursor->ptr2Tree.erase(cursor->ptr2Tree.begin() + pos);

    // Root is allowed to go below the minimum; if it has no keys left,
    // its only child becomes the new root (tree height shrinks by one)
    if (cursor == root) {
        if (cursor->keys.empty()) {
            root = cursor->ptr2Tree[0];
            delete cursor;
            cout << "Wow! New Changed Root" << endl;
        }
        return;
    }

    int minIntKeys = (maxIntChildLimit + 1) / 2 - 1;  // ceil(maxIntChildLimit / 2) - 1

    // If there is No underflow. Phew!!
    if ((int)cursor->keys.size() >= minIntKeys) {
        cout << "Deleted key from internal node successfully" << endl;
        return;
    }

    cout << "UnderFlow in internal Node! What did you do :/" << endl;

    InternalNode* parent = findParent(asInternal(root), cursor);

    // Finding Left and Right Siblings as we did earlier
    int idx = find(parent->ptr2Tree.begin(), parent->ptr2Tree.end(), cursor) - parent->ptr2Tree.begin();
    int leftSibling = idx - 1;
    int rightSibling = idx + 1;

    // If possible borrow from leftSibling (rotate through the parent)
    if (leftSibling >= 0) {
        InternalNode* leftNode = asInternal(parent->ptr2Tree[leftSibling]);

        if ((int)leftNode->keys.size() > minIntKeys) {
            // parent key comes down into cursor, left's max key goes up into parent
            cursor->keys.insert(cursor->keys.begin(), parent->keys[leftSibling]);
            parent->keys[leftSibling] = leftNode->keys.back();
            leftNode->keys.pop_back();

            // left's last child moves over to cursor
            cursor->ptr2Tree.insert(cursor->ptr2Tree.begin(), leftNode->ptr2Tree.back());
            leftNode->ptr2Tree.pop_back();

            cout << "Transferred from left sibling of internal node" << endl;
            return;
        }
    }

    // If possible borrow from rightSibling (rotate through the parent)
    if (rightSibling < (int)parent->ptr2Tree.size()) {
        InternalNode* rightNode = asInternal(parent->ptr2Tree[rightSibling]);

        if ((int)rightNode->keys.size() > minIntKeys) {
            // parent key comes down into cursor, right's min key goes up into parent
            cursor->keys.push_back(parent->keys[idx]);
            parent->keys[idx] = rightNode->keys.front();
            rightNode->keys.erase(rightNode->keys.begin());

            // right's first child moves over to cursor
            cursor->ptr2Tree.push_back(rightNode->ptr2Tree.front());
            rightNode->ptr2Tree.erase(rightNode->ptr2Tree.begin());

            cout << "Transferred from right sibling of internal node" << endl;
            return;
        }
    }

    //Start to Merge Now, if None of the above cases applied
    if (leftSibling >= 0) {
        //leftNode + parent key + cursor
        InternalNode* leftNode = asInternal(parent->ptr2Tree[leftSibling]);
        leftNode->keys.push_back(parent->keys[leftSibling]);
        leftNode->keys.insert(leftNode->keys.end(), cursor->keys.begin(), cursor->keys.end());
        leftNode->ptr2Tree.insert(leftNode->ptr2Tree.end(), cursor->ptr2Tree.begin(), cursor->ptr2Tree.end());

        cout << "Merged with left sibling" << endl;
        removeInternal(parent, cursor);
        delete cursor;  // its children now belong to leftNode
    } else {
        //cursor + parent key + rightNode
        InternalNode* rightNode = asInternal(parent->ptr2Tree[rightSibling]);
        cursor->keys.push_back(parent->keys[idx]);
        cursor->keys.insert(cursor->keys.end(), rightNode->keys.begin(), rightNode->keys.end());
        cursor->ptr2Tree.insert(cursor->ptr2Tree.end(), rightNode->ptr2Tree.begin(), rightNode->ptr2Tree.end());

        cout << "Merged with right sibling" << endl;
        removeInternal(parent, rightNode);
        delete rightNode;  // its children now belong to cursor
    }
}
