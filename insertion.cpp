#include <iostream>
#include <algorithm>
#include "B+ Tree.h"

using namespace std;

bool BPTree::insert(int key, FILE* filePtr) {  //in Leaf Node
    /*
        1. If the node has an empty space, insert the key/reference pair into the node.
        2. If the node is already full, split it into two nodes, distributing the keys
        evenly between the two nodes. If the node is a leaf, take a copy of the minimum
        value in the second of these two nodes and repeat this insertion algorithm to
        insert it into the parent node. If the node is a non-leaf, exclude the middle
        value during the split and repeat this insertion algorithm to insert this excluded
        value into the parent node.
    */

    if (root == nullptr) {
        LeafNode* leaf = new LeafNode;
        leaf->keys.push_back(key);
        leaf->dataPtr.push_back(filePtr);
        root = leaf;
        cout << key << ": I AM ROOT!!" << endl;
        return true;
    }

    //searching for the possible position for the given key by doing the same procedure we did in search
    InternalNode* parent = nullptr;
    LeafNode* cursor = findLeaf(key, &parent);

    //finding the position for the key inside the leaf (binary search)
    int i = lower_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();
    if (i < (int)cursor->keys.size() && cursor->keys[i] == key) {
        cout << "Key " << key << " already exists! Primary key must be unique." << endl;
        return false;
    }

    //insert key & data pointer at their sorted position (vector::insert shifts the rest right)
    cursor->keys.insert(cursor->keys.begin() + i, key);
    cursor->dataPtr.insert(cursor->dataPtr.begin() + i, filePtr);

    if ((int)cursor->keys.size() <= maxLeafNodeLimit) {
        cout << "Inserted successfully: " << key << endl;
        return true;
    }

    /*
        DAMN!! Node Overflowed :(
        HAIYYA! Splitting the Node .
        The leaf now holds maxLeafNodeLimit + 1 keys (the "virtual node").
        Left (old) leaf keeps the first maxLeafNodeLimit/2 + 1 keys, the new leaf gets the rest.
    */
    int leftSize = maxLeafNodeLimit / 2 + 1;

    LeafNode* newLeaf = new LeafNode;
    newLeaf->keys.assign(cursor->keys.begin() + leftSize, cursor->keys.end());
    newLeaf->dataPtr.assign(cursor->dataPtr.begin() + leftSize, cursor->dataPtr.end());
    cursor->keys.resize(leftSize);
    cursor->dataPtr.resize(leftSize);

    //linking the new leaf into the leaf list: cursor -> newLeaf -> cursor's old next
    newLeaf->ptr2next = cursor->ptr2next;
    cursor->ptr2next = newLeaf;

    if (cursor == root) {
        /*
            If cursor is root node we create new node
        */
        InternalNode* newRoot = new InternalNode;
        newRoot->keys.push_back(newLeaf->keys[0]);  //COPY of the new leaf's minimum goes up
        newRoot->ptr2Tree.push_back(cursor);
        newRoot->ptr2Tree.push_back(newLeaf);
        root = newRoot;
        cout << "Created new Root!" << endl;
    } else {
        // Insert new key in the parent
        insertInternal(newLeaf->keys[0], parent, newLeaf);
    }
    return true;
}

void BPTree::insertInternal(int x, InternalNode* cursor, Node* child) {  //in Internal Nodes
    //key x goes at position i, the new child goes right after it (at i + 1)
    int i = upper_bound(cursor->keys.begin(), cursor->keys.end(), x) - cursor->keys.begin();
    cursor->keys.insert(cursor->keys.begin() + i, x);
    cursor->ptr2Tree.insert(cursor->ptr2Tree.begin() + i + 1, child);

    if ((int)cursor->keys.size() <= maxIntChildLimit - 1) {
        cout << "Inserted key in the internal node :)" << endl;
        return;
    }

    //splitting
    cout << "Overflow in internal:( HAIYAA! splitting internal nodes" << endl;

    //exclude middle element while splitting: it moves UP and stays in neither half
    int partitionIdx = cursor->keys.size() / 2;  //if the halves are uneven, the left node gets the extra key
    int partitionKey = cursor->keys[partitionIdx];

    InternalNode* newInternalNode = new InternalNode;
    newInternalNode->keys.assign(cursor->keys.begin() + partitionIdx + 1, cursor->keys.end());
    newInternalNode->ptr2Tree.assign(cursor->ptr2Tree.begin() + partitionIdx + 1, cursor->ptr2Tree.end());

    cursor->keys.resize(partitionIdx);
    cursor->ptr2Tree.resize(partitionIdx + 1);  // only the key is excluded, not the pointer

    if (cursor == root) {
        /*
            If cursor is a root we create a new Node
        */
        InternalNode* newRoot = new InternalNode;
        newRoot->keys.push_back(partitionKey);
        newRoot->ptr2Tree.push_back(cursor);
        newRoot->ptr2Tree.push_back(newInternalNode);
        root = newRoot;
        cout << "Created new ROOT!" << endl;
    } else {
        /*
            ::Recursion::
        */
        insertInternal(partitionKey, findParent(asInternal(root), cursor), newInternalNode);
    }
}
