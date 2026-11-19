#include <iostream>
#include <algorithm>
#include <stdexcept>
#include "B+ Tree.h"

using namespace std;

BPTree::BPTree() : BPTree(4, 3) {
    /*
        By Default it will take the maxIntChildLimit as 4. And
        maxLeafNodeLimit as 3.

        ::REASON FOR TWO SEPERATE VARIABLES maxIntChildLimit & maxLeafNodeLimit !!
        We are keeping the two seperate Orders
        because Internal Nodes can hold more values in one disc block
        as the size of the Tree pointer is small but the size of the
        data pointer in the leaf nodes is large so we can only put less
        nodes in the leafs as compared to the internal Nodes. Thats the
        reson to reperate out these to variables.
    */
}

BPTree::BPTree(int degreeInternal, int degreeLeaf) {
    // An internal node needs >= 3 children and a leaf >= 2 keys, otherwise
    // splits and merges cannot keep every node at least half full.
    if (degreeInternal < 3 || degreeLeaf < 2)
        throw invalid_argument("BPTree needs degreeInternal >= 3 and degreeLeaf >= 2");
    this->maxIntChildLimit = degreeInternal;
    this->maxLeafNodeLimit = degreeLeaf;
    this->root = nullptr;
}

BPTree::~BPTree() {
    destroy(root);  // free every node; no memory leaks
}

void BPTree::destroy(Node* cursor) {
    if (cursor == nullptr) return;
    if (!cursor->isLeaf()) {
        for (Node* child : asInternal(cursor)->ptr2Tree)
            destroy(child);
    }
    delete cursor;  // virtual destructor -> correct subclass destructor runs
}

LeafNode* BPTree::asLeaf(Node* node) {
    return static_cast<LeafNode*>(node);
}

InternalNode* BPTree::asInternal(Node* node) {
    return static_cast<InternalNode*>(node);
}

int BPTree::getMaxIntChildLimit() const {
    return maxIntChildLimit;
}

int BPTree::getMaxLeafNodeLimit() const {
    return maxLeafNodeLimit;
}

bool BPTree::isEmpty() const {
    return root == nullptr;
}

LeafNode* BPTree::findLeaf(int key, InternalNode** parent, int* childIdx) const {
    /*
        Walks from the root down to the leaf where `key` is (or would be).
        Optionally reports that leaf's parent and its index among the parent's children.

        upper_bound returns the first key strictly greater than `key`. Equal keys go
        right because when a leaf splits, the separator copied up stays in the right leaf.
    */
    Node* cursor = root;
    if (parent) *parent = nullptr;
    if (childIdx) *childIdx = -1;

    while (!cursor->isLeaf()) {
        InternalNode* internal = asInternal(cursor);
        int idx = upper_bound(internal->keys.begin(), internal->keys.end(), key) - internal->keys.begin();
        if (parent) *parent = internal;
        if (childIdx) *childIdx = idx;
        cursor = internal->ptr2Tree[idx];
    }
    return asLeaf(cursor);
}

LeafNode* BPTree::firstLeftNode(Node* cursor) const {
    if (cursor == nullptr) return nullptr;
    while (!cursor->isLeaf())
        cursor = asInternal(cursor)->ptr2Tree[0];
    return asLeaf(cursor);
}

InternalNode* BPTree::findParent(InternalNode* cursor, Node* child) const {
    /*
        Finds the parent of `child` using depth first traversal starting at `cursor`.
        Leaf nodes are skipped as they cannot be parents.
    */
    for (Node* next : cursor->ptr2Tree) {
        if (next == child)
            return cursor;
        if (!next->isLeaf()) {
            InternalNode* found = findParent(asInternal(next), child);
            if (found) return found;
        }
    }
    return nullptr;
}
