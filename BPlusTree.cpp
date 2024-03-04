// BPlusTree.cpp - Insertion Part
#include "BPlusTree.h"
#include <iostream>
#include <limits>
#include "Record.h"
#include "Storage.h"

// Constructor with block size parameter to estimate max keys per node
BPTree::BPTree(size_t blkSize) {
    this->maxKeys = (blkSize / sizeof(Record)) - 1; // Estimate based on block size and record size
    this->root = std::make_shared<Node>(true); // Initially, root is a leaf node
}

// Destructor to clean up resources
BPTree::~BPTree() {
    // Implement cleanup if needed
}

// Function to insert a new record into the B+ Tree
void BPTree::insert(float key, const std::shared_ptr<Record>& recordPtr) {
    if (!root) {
        root = std::make_shared<Node>(true); // true indicates that the node is a leaf
        root->keys.push_back(key);
        root->records.push_back({recordPtr});
        return;
    }

    std::shared_ptr<Node> cursor = root;
    std::shared_ptr<Node> parent = nullptr; // Initialize parent as nullptr

    // Navigate down to the leaf node where the key should be inserted
    while (!cursor->isLeaf) {
        parent = cursor; // Keep track of the parent
        bool found = false;
        for (size_t i = 0; i < cursor->keys.size(); ++i) {
            if (key < cursor->keys[i]) {
                cursor = cursor->children[i];
                found = true;
                break;
            }
        }
        if (!found) {
            // If the key is greater than all keys in the current node, go to the rightmost child
            cursor = cursor->children[cursor->keys.size()];
        }
    }

    // Insert the key and record in the leaf node
    auto it = std::lower_bound(cursor->keys.begin(), cursor->keys.end(), key);
    int index = it - cursor->keys.begin();
    cursor->keys.insert(it, key);
    cursor->records.insert(cursor->records.begin() + index, {recordPtr});

    // Split the leaf node if it overflows
    if (cursor->keys.size() > maxKeys) {
        splitLeafNode(cursor);
    }
}


// Function to split a leaf node that overflows after insertion
void BPTree::splitLeafNode(std::shared_ptr<Node>& leaf) {
    auto newLeaf = std::make_shared<Node>(true); // Create a new leaf node
    int median = leaf->keys.size() / 2;

    // Distribute the keys and records to the new leaf
    newLeaf->keys.assign(leaf->keys.begin() + median, leaf->keys.end());
    newLeaf->records.assign(leaf->records.begin() + median, leaf->records.end());

    // Resize the original leaf to hold only the left half
    leaf->keys.resize(median);
    leaf->records.resize(median);

    // Update linked list pointers
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    // Handling parent
    std::shared_ptr<Node> parent = leaf->parent.lock();
    if (!parent) { // This means leaf was the root
        auto newRoot = std::make_shared<Node>(false); // false indicates it's an internal node
        newRoot->keys.push_back(newLeaf->keys.front());
        newRoot->children.push_back(leaf);
        newRoot->children.push_back(newLeaf);

        // Update parent pointers
        leaf->parent = newRoot;
        newLeaf->parent = newRoot;

        // Update the tree's root
        root = newRoot;
    } else {
        // If there's a parent, insert the new key into it
        insertInternal(newLeaf->keys.front(), parent, newLeaf);
    }
}

// Function to insert a new key into an internal node during node splitting
void BPTree::insertInternal(float key, std::shared_ptr<Node> parent, std::shared_ptr<Node> child) {
    auto it = std::lower_bound(parent->keys.begin(), parent->keys.end(), key);
    int index = it - parent->keys.begin();
    parent->keys.insert(it, key);
    parent->children.insert(parent->children.begin() + index + 1, child);

    if (parent->keys.size() > maxKeys) {
        // Split the internal node
        auto newInternal = std::make_shared<Node>(false);
        int median = parent->keys.size() / 2;

        newInternal->keys.assign(parent->keys.begin() + median + 1, parent->keys.end());
        newInternal->children.assign(parent->children.begin() + median + 1, parent->children.end());

        parent->keys.resize(median);
        parent->children.resize(median + 1);

        if (parent == root) {
            auto newRoot = std::make_shared<Node>(false);
            newRoot->keys.push_back(parent->keys[median]);
            newRoot->children.push_back(parent);
            newRoot->children.push_back(newInternal);
            root = newRoot;
        } else {
            insertInternal(parent->keys[median], findParent(root, parent), newInternal);
        }
    }
}

// Function to find the leaf node where a given key should be inserted
std::shared_ptr<Node> BPTree::findLeafNode(float key) const {
    std::shared_ptr<Node> cursor = root;
    while (!cursor->isLeaf) {
        // Traverse the tree down to the leaf level
        for (int i = 0; i < cursor->keys.size(); i++) {
            if (key < cursor->keys[i]) {
                cursor = cursor->children[i];
                break;
            }
            if (i == cursor->keys.size() - 1) {
                cursor = cursor->children[i + 1];
                break;
            }
        }
    }
    return cursor; // Return the leaf node where the key fits
}

// Function to find the parent node of a given child node
std::shared_ptr<Node> BPTree::findParent(const std::shared_ptr<Node>& cursor, const std::shared_ptr<Node>& child) {
    if (cursor->isLeaf || cursor->children[0]->isLeaf) {
        return nullptr; // If cursor is a leaf or children are leaves, then there's no parent to find
    }

    for (const auto& childNode : cursor->children) {
        if (childNode == child) {
            return cursor; // If the child is directly connected to the cursor, return cursor as the parent
        }
        auto potentialParent = findParent(childNode, child); // Recursively search for the parent
        if (potentialParent) return potentialParent; // If a parent is found in a deeper level, return it
    }

    return nullptr; // If no parent is found, return nullptr (should not happen if tree structure is correct)
}

// Function to search for a record by key
std::vector<std::shared_ptr<Record>> BPTree::search(float key) const {
    std::shared_ptr<Node> cursor = root;
    while (!cursor->isLeaf) {
        // Traverse down to the leaf level
        bool found = false;
        for (int i = 0; i < cursor->keys.size(); i++) {
            if (key < cursor->keys[i]) {
                cursor = cursor->children[i];
                found = true;
                break;
            }
        }
        if (!found) {
            cursor = cursor->children[cursor->children.size() - 1]; // If key is greater, go to the rightmost child
        }
    }

    // Now cursor is at the leaf level, search for the key in the leaf
    for (int i = 0; i < cursor->keys.size(); i++) {
        if (cursor->keys[i] == key) {
            return cursor->records[i]; // If key is found, return the associated records
        }
    }

    return std::vector<std::shared_ptr<Record>>(); // If key is not found, return an empty vector
}

// Function to delete a key (and its associated records) from the B+ Tree
void BPTree::deleteKey(float key) {
    std::shared_ptr<Node> leaf = findLeafNode(key);
    bool found = false;
    int idx; // Index of the key to be deleted

    // Search for the key in the leaf node
    for (idx = 0; idx < leaf->keys.size(); idx++) {
        if (leaf->keys[idx] == key) {
            found = true;
            break;
        }
    }

    if (!found) {
        std::cout << "Key " << key << " not found." << std::endl;
        return; // If the key is not found, there's nothing to delete
    }

    // Remove the key and its associated records from the leaf node
    leaf->keys.erase(leaf->keys.begin() + idx);
    leaf->records.erase(leaf->records.begin() + idx);

    // If the leaf node underflows after deletion, handle the underflow
    if (leaf->keys.size() < minKeys) {
        auto parent = findParent(root, leaf);
        int childIndex = findChildIndex(parent, leaf);
        std::shared_ptr<Node> prevNode = (childIndex > 0) ? parent->children[childIndex - 1] : nullptr;
        std::shared_ptr<Node> nextNode = (childIndex < parent->children.size() - 1) ? parent->children[childIndex + 1] : nullptr;

        if (prevNode && prevNode->keys.size() > minKeys) {
            // Borrow a key from the previous sibling
            borrowFromPrev(leaf, prevNode, parent, childIndex);
        } else if (nextNode && nextNode->keys.size() > minKeys) {
            // Borrow a key from the next sibling
            borrowFromNext(leaf, nextNode, parent, childIndex);
        } else if (prevNode) {
            // Merge with the previous sibling
            mergeWithNext(prevNode, leaf, parent, childIndex - 1);
        } else if (nextNode) {
            // Merge with the next sibling
            mergeWithNext(leaf, nextNode, parent, childIndex);
        }

        // If the parent node underflows after a merge, handle the underflow recursively
        if (parent && parent->keys.size() < minKeys && parent != root) {
            rebalanceAfterDeletion(parent);
        }
    }
}

// Function to perform a range query on the B+ Tree, returning all records with keys within the specified range
std::vector<std::shared_ptr<Record>> BPTree::rangeQuery(float lowerBound, float upperBound) const {
    std::vector<std::shared_ptr<Record>> results;
    std::shared_ptr<Node> cursor = findLeafNode(lowerBound);

    // Traverse the leaf nodes starting from the leaf node that contains the lower bound
    while (cursor != nullptr && !cursor->keys.empty() && cursor->keys.front() <= upperBound) {
        // Iterate through the keys in the current leaf node
        for (int i = 0; i < cursor->keys.size(); i++) {
            if (cursor->keys[i] >= lowerBound && cursor->keys[i] <= upperBound) {
                // If the key is within the range, add all associated records to the results
                results.insert(results.end(), cursor->records[i].begin(), cursor->records[i].end());
            } else if (cursor->keys[i] > upperBound) {
                // If the current key exceeds the upper bound, stop the search
                return results;
            }
        }

        // Move to the next leaf node in the linked list
        cursor = cursor->next;
    }

    return results; // Return all records found within the specified range
}

// Function to borrow a key from the next sibling during deletion handling
void BPTree::borrowFromNext(std::shared_ptr<Node>& node, std::shared_ptr<Node>& nextNode, std::shared_ptr<Node>& parent, int parentIndex) {
    // Borrow the first key from the next sibling node
    node->keys.push_back(nextNode->keys.front());
    nextNode->keys.erase(nextNode->keys.begin());

    // If the nodes are internal nodes, also borrow the first child from the next sibling
    if (!node->isLeaf) {
        node->children.push_back(nextNode->children.front());
        nextNode->children.erase(nextNode->children.begin());
    }

    // Update the key in the parent node that separates the two siblings
    parent->keys[parentIndex] = nextNode->keys.front();
}

// Function to borrow a key from the previous sibling during deletion handling
void BPTree::borrowFromPrev(std::shared_ptr<Node>& node, std::shared_ptr<Node>& prevNode, std::shared_ptr<Node>& parent, int parentIndex) {
    // Borrow the last key from the previous sibling node
    node->keys.insert(node->keys.begin(), prevNode->keys.back());
    prevNode->keys.pop_back();

    // If the nodes are internal nodes, also borrow the last child from the previous sibling
    if (!node->isLeaf) {
        node->children.insert(node->children.begin(), prevNode->children.back());
        prevNode->children.pop_back();
    }

    // Update the key in the parent node that separates the two siblings
    parent->keys[parentIndex - 1] = node->keys.front();
}

// Function to merge a node with its next sibling during deletion handling
void BPTree::mergeWithNext(std::shared_ptr<Node>& node, std::shared_ptr<Node>& nextNode, std::shared_ptr<Node>& parent, int parentIndex) {
    // Move all keys and children (if any) from the next sibling node to the current node
    node->keys.insert(node->keys.end(), nextNode->keys.begin(), nextNode->keys.end());
    if (!node->isLeaf) {
        node->children.insert(node->children.end(), nextNode->children.begin(), nextNode->children.end());
    }

    // Remove the key and child pointer for the next sibling from the parent node
    parent->keys.erase(parent->keys.begin() + parentIndex);
    parent->children.erase(parent->children.begin() + parentIndex + 1);
}

// Function to find the index of a child node in its parent's children vector
int BPTree::findChildIndex(std::shared_ptr<Node>& parent, std::shared_ptr<Node>& child) {
    for (int i = 0; i < parent->children.size(); i++) {
        if (parent->children[i] == child) {
            return i; // Return the index of the child in the parent's children vector
        }
    }
    return -1; // This should not happen if the tree structure is correct
}

// Function to handle underflow in internal nodes after deletion
void BPTree::rebalanceAfterDeletion(std::shared_ptr<Node>& node) {
    auto parent = findParent(root, node);
    int childIndex = findChildIndex(parent, node);
    std::shared_ptr<Node> prevNode = (childIndex > 0) ? parent->children[childIndex - 1] : nullptr;
    std::shared_ptr<Node> nextNode = (childIndex < parent->children.size() - 1) ? parent->children[childIndex + 1] : nullptr;

    if (node->keys.size() < minKeys) {
        if (prevNode && prevNode->keys.size() > minKeys) {
            // Borrow a key from the previous sibling
            borrowFromPrev(node, prevNode, parent, childIndex);
        } else if (nextNode && nextNode->keys.size() > minKeys) {
            // Borrow a key from the next sibling
            borrowFromNext(node, nextNode, parent, childIndex);
        } else if (prevNode) {
            // Merge with the previous sibling if borrowing is not possible
            mergeWithNext(prevNode, node, parent, childIndex - 1);
        } else if (nextNode) {
            // Merge with the next sibling if borrowing is not possible
            mergeWithNext(node, nextNode, parent, childIndex);
        }

        // If the parent node underflows after a merge, handle the underflow recursively
        if (parent && parent->keys.size() < minKeys && parent != root) {
            rebalanceAfterDeletion(parent);
        }
    }
}

// Function to return the total number of nodes in the B+ Tree
int BPTree::getTotalNodes() const {
    return countNodes(root); // Count the nodes starting from the root
}

// Recursive function to count the number of nodes in the subtree rooted at a given node
int BPTree::countNodes(std::shared_ptr<Node> node) const {
    if (!node) return 0; // Base case: if the node is null, return 0
    int count = 1; // Start with counting the current node
    if (!node->isLeaf) {
        // If the node is not a leaf, count the number of nodes in each child's subtree
        for (const auto& child : node->children) {
            count += countNodes(child);
        }
    }
    return count; // Return the total count
}

// Function to return the number of levels (depth) of the B+ Tree
int BPTree::getTreeLevels() const {
    return calculateDepth(root); // Calculate the depth starting from the root
}

// Recursive function to calculate the depth of the subtree rooted at a given node
int BPTree::calculateDepth(std::shared_ptr<Node> node) const {
    if (!node || node->isLeaf) return 1; // Base case: if the node is null or a leaf, the depth is 1
    // Recursively calculate the depth of the first child's subtree and add 1 for the current node
    return 1 + calculateDepth(node->children.front());
}

// Function to return the maximum number of keys per node (n parameter of the B+ Tree)
int BPTree::getMaxKeys() const {
    return maxKeys; // Return the maximum number of keys per node
}

// Function to return the total number of records stored in the B+ Tree
int BPTree::getTotalRecords() const {
    return countRecords(root); // Count the records starting from the root
}

// Recursive function to count the number of records in the subtree rooted at a given node
int BPTree::countRecords(std::shared_ptr<Node> node) const {
    if (!node) return 0; // Base case: if the node is null, return 0
    int totalRecords = 0;
    if (node->isLeaf) {
        // If the node is a leaf, count the records in each key's associated record list
        for (const auto& recordGroup : node->records) {
            totalRecords += recordGroup.size();
        }
    } else {
        // If the node is not a leaf, recursively count the records in each child's subtree
        for (const auto& child : node->children) {
            totalRecords += countRecords(child);
        }
    }
    return totalRecords; // Return the total count of records
}

// Function to return the total number of blocks (leaf nodes) in the B+ Tree
int BPTree::getTotalBlocks() const {
    return countBlocks(root); // Count the blocks starting from the root
}

// Recursive function to count the number of leaf nodes in the subtree rooted at a given node
int BPTree::countBlocks(std::shared_ptr<Node> node) const {
    if (!node) return 0; // Base case: if the node is null, return 0
    if (node->isLeaf) return 1; // If the node is a leaf, it represents a block, so return 1
    int totalBlocks = 0;
    // If the node is not a leaf, recursively count the number of blocks in each child's subtree
    for (const auto& child : node->children) {
        totalBlocks += countBlocks(child);
    }
    return totalBlocks; // Return the total count of blocks
}

// Function to print the structure of the B+ Tree for debugging purposes
void BPTree::printTree() const {
    std::cout << "B+ Tree Structure:" << std::endl;
    printTree(root, 0); // Call the recursive utility function to print the tree starting from the root
    std::cout << "---------------------------------\n";
}

// Recursive utility function to print the structure of the subtree rooted at a given node
void BPTree::printTree(std::shared_ptr<Node> node, int space) const {
    if (!node) return; // Base case: if the node is null, there's nothing to print

    // Start from the rightmost key and recursively print the subtree rooted at the right child
    for (int i = node->keys.size() - 1; i >= 0; i--) {
        if (!node->isLeaf) {
            printTree(node->children[i + 1], space + 5); // Recursively print the right child's subtree with increased indentation
        }

        // Print the current key with the appropriate indentation
        for (int j = 0; j < space; j++) {
            std::cout << ' ';
        }
        std::cout << node->keys[i] << "\n";
    }

    // Recursively print the subtree rooted at the leftmost child
    if (!node->isLeaf) {
        printTree(node->children[0], space + 5);
    }
}

// Function to be called in Experiment 2 to print the B+ Tree statistics
void BPTree::experiment2Statistics() const {
    std::cout << "Experiment 2: B+ Tree Statistics" << std::endl;
    std::cout << "Parameter n (max keys per node): " << maxKeys << std::endl;
    std::cout << "Number of nodes in the B+ tree: " << getTotalNodes() << std::endl;
    std::cout << "Number of levels in the B+ tree: " << getTreeLevels() << std::endl;
    std::cout << "Content of the root node (only keys): ";
    // Print the keys in the root node
    for (const auto& key : (root ? root->keys : std::vector<float>())) {
        std::cout << key << " ";
    }
    std::cout << std::endl;
    std::cout << "---------------------------------\n";
    //printTree(); // Print the structure of the B+ Tree for visual inspection
}

void BPTree::breadthFirstTraversal() const {
    if (!root) return; // Check if the tree is empty

    std::deque<std::shared_ptr<Node>> queue;
    queue.push_back(root);

    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop_front();

        // Process the current node, e.g., print its keys
        for (const auto& key : current->keys) {
            std::cout << key << " ";
        }
        std::cout << "| "; // Separator for nodes at the same level

        // Enqueue child nodes
        if (!current->isLeaf) {
            for (const auto& child : current->children) {
                queue.push_back(child);
            }
        }
    }
    std::cout << std::endl;
}