#ifndef BPLUS_TREE_H
#define BPLUS_TREE_H

#include <cstdio>
#include <vector>

/*
    Node hierarchy (Inheritance + Polymorphism)

            Node            <- abstract base: what every node has (sorted keys)
           /    \
    LeafNode    InternalNode

    - LeafNode     : keys + one data pointer (FILE*) per key + link to next leaf
    - InternalNode : keys + (keys.size() + 1) child pointers

    Generally the size of a node should be equal to the disk block size, which
    limits the number of disk accesses. Internal nodes only hold tree pointers
    (small), so they can hold more of them; only leaf nodes hold data pointers.

    IMPORTANT := All the data has to be present in the leaf nodes.

    Encapsulation: node internals are not public. Only BPTree may touch them,
    which is granted through `friend class BPTree`.
*/

class BPTree;

class Node {
   protected:
    std::vector<int> keys;  // sorted keys stored in this node

   public:
    virtual ~Node() {}
    virtual bool isLeaf() const = 0;  // pure virtual -> Node is abstract

    friend class BPTree;
};

class LeafNode : public Node {
   private:
    std::vector<FILE*> dataPtr;  // dataPtr[i] points to the record of keys[i]
    LeafNode* ptr2next;          // next leaf, so all leaves form a sorted linked list

   public:
    LeafNode() : ptr2next(nullptr) {}
    bool isLeaf() const override { return true; }

    friend class BPTree;
};

class InternalNode : public Node {
   private:
    std::vector<Node*> ptr2Tree;  // children; always keys.size() + 1 of them

   public:
    bool isLeaf() const override { return false; }

    friend class BPTree;
};

class BPTree {
    /*
        ::For Root Node :=
            The root node has at least two tree pointers (or is a single leaf)
        ::For Internal Nodes :=
            1. ceil(maxIntChildLimit/2)     <=  #of children <= maxIntChildLimit
            2. ceil(maxIntChildLimit/2)-1   <=  #of keys     <= maxIntChildLimit - 1
        ::For Leaf Nodes :=
            1. ceil(maxLeafNodeLimit/2)     <=  #of keys     <= maxLeafNodeLimit

        Single Responsibility: BPTree only maintains the index (key -> data pointer).
        It never opens, reads or deletes record files; that is RecordStore's job.
    */
   private:
    int maxIntChildLimit;  // max #children of an internal node (fan-out)
    int maxLeafNodeLimit;  // max #keys in a leaf node
    Node* root;            // pointer to the B+ Tree root

    static LeafNode* asLeaf(Node* node);
    static InternalNode* asInternal(Node* node);

    LeafNode* findLeaf(int key, InternalNode** parent = nullptr, int* childIdx = nullptr) const;
    void insertInternal(int x, InternalNode* cursor, Node* child);  // insert x and child into cursor (parent)
    void removeInternal(InternalNode* cursor, Node* child);         // remove child and its separator key from cursor
    InternalNode* findParent(InternalNode* cursor, Node* child) const;
    LeafNode* firstLeftNode(Node* cursor) const;
    void destroy(Node* cursor);

   public:
    BPTree();
    BPTree(int degreeInternal, int degreeLeaf);
    ~BPTree();
    BPTree(const BPTree&) = delete;             // the tree owns its nodes,
    BPTree& operator=(const BPTree&) = delete;  // so copying is disabled

    Node* getRoot() const;
    int getMaxIntChildLimit() const;
    int getMaxLeafNodeLimit() const;

    void display(Node* cursor) const;
    void seqDisplay(Node* cursor) const;
    FILE* search(int key) const;            // data pointer of key, or nullptr if absent
    bool insert(int key, FILE* filePtr);    // false if key already exists
    FILE* removeKey(int key);               // data pointer of the removed key, or nullptr if absent
};

#endif
