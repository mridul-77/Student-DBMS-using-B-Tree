#include <algorithm>
#include "B+ Tree.h"

using namespace std;

FILE* BPTree::search(int key) const {
    if (root == nullptr)
        return nullptr;

    // Descend to the leaf: binary search (upper_bound) in every internal node
    LeafNode* cursor = findLeaf(key);

    // Binary search inside the leaf
    int idx = lower_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();
    if (idx == (int)cursor->keys.size() || cursor->keys[idx] != key)
        return nullptr;

    /*
        The data pointer lets us fetch the record from the disk into main memory
    */
    return cursor->dataPtr[idx];
}
