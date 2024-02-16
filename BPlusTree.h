// BPlusTree.h
#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "Node.h"
#include <vector>
#include <memory>

class BPTree {
private:
    std::shared_ptr<Node> root;
    int maxKeys;
    int minKeys; // Minimum keys a node must hold, determined by block size

public:
    BPTree(size_t blkSize); // Constructor based on block size
    ~BPTree(); // Destructor

    void insert(float key, const std::shared_ptr<Record>& recordPtr);
    std::vector<std::shared_ptr<Record>> search(float key) const;
    std::vector<std::shared_ptr<Record>> rangeQuery(float lowerBound, float upperBound) const;
    void deleteKey(float key);
    void printTree() const; // Utility to print tree structure for debugging

private:
    // Helper functions
    void insertInternal(float key, std::shared_ptr<Node> parent, std::shared_ptr<Node> child);
    std::shared_ptr<Node> findLeafNode(float key) const;
    void splitLeafNode(std::shared_ptr<Node>& leafNode);
    void splitInternalNode(std::shared_ptr<Node>& internalNode);
    void deleteInternal(float key, std::shared_ptr<Node>& cursor, std::shared_ptr<Node>& child);
    void removeKey(std::shared_ptr<Node>& node, float key);
    void redistributeNodes(std::shared_ptr<Node>& node);
    void mergeNodes(std::shared_ptr<Node>& node);
    void printTree(std::shared_ptr<Node> node, int space) const; // Recursive utility for printTree
    std::shared_ptr<Node> findParent(const std::shared_ptr<Node>& cursor, const std::shared_ptr<Node>& child);
    void borrowFromNext(std::shared_ptr<Node>& node, std::shared_ptr<Node>& nextNode, std::shared_ptr<Node>& parent, int parentIndex);
    void borrowFromPrev(std::shared_ptr<Node>& node, std::shared_ptr<Node>& prevNode, std::shared_ptr<Node>& parent, int parentIndex);
    void mergeWithNext(std::shared_ptr<Node>& node, std::shared_ptr<Node>& nextNode, std::shared_ptr<Node>& parent, int parentIndex);
    int findChildIndex(std::shared_ptr<Node>& parent, std::shared_ptr<Node>& child);
    void rebalanceAfterDeletion(std::shared_ptr<Node>& node);
};

#endif // BPLUSTREE_H
