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

vector<pair<int, FILE*>> BPTree::rangeSearch(int low, int high) const {
    /*
        Range query: O(log N + K), K = number of results.
        This is why B+ Tree leaves are linked: descend once, then walk right.
    */
    vector<pair<int, FILE*>> result;
    if (root == nullptr || low > high)
        return result;

    // 1. Descend once to the leaf where `low` would be: O(log N)
    LeafNode* leaf = findLeaf(low);
    int idx = lower_bound(leaf->keys.begin(), leaf->keys.end(), low) - leaf->keys.begin();

    // 2. Walk right along the leaf links until a key exceeds `high`: O(K)
    while (leaf != nullptr) {
        for (; idx < (int)leaf->keys.size(); idx++) {
            if (leaf->keys[idx] > high)
                return result;
            result.push_back({leaf->keys[idx], leaf->dataPtr[idx]});
        }
        leaf = leaf->ptr2next;
        idx = 0;
    }
    return result;
}
