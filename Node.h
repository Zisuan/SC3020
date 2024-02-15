// Node.h
#ifndef NODE_H
#define NODE_H

#include <vector>
#include <memory>

struct Record {
    char tconst[10];     // IMDb identifier
    float averageRating; // Movie rating
    int32_t numVotes;    // Number of votes
};

// B+ Tree Node
struct Node {
    bool isLeaf;
    std::vector<float> keys; // Store keys
    // Pointers to child nodes for internal nodes
    std::vector<std::shared_ptr<Node>> children; 
    // Pointers to records for leaf nodes, allowing for duplicates
    std::vector<std::vector<std::shared_ptr<Record>>> records; 
    std::shared_ptr<Node> next; // Pointer to the next leaf node for range queries

    explicit Node(bool leaf) : isLeaf(leaf) {}
};

#endif // NODE_H
