// BPlusTree.cpp - Insertion Part
#include "BPlusTree.h"
#include <iostream>

BPTree::BPTree(size_t blkSize) {
    this->maxKeys = (blkSize / sizeof(Record)) - 1; // Estimate max keys based on block size and record size
    this->root = std::make_shared<Node>(true); // Initially, root is a leaf node
}

BPTree::~BPTree() {
    // Destructor: Cleanup resources if needed
}

void BPTree::insert(float key, const std::shared_ptr<Record>& recordPtr) {
    std::shared_ptr<Node> leaf = findLeafNode(key);

    // Insert the record in the leaf node
    auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    int index = it - leaf->keys.begin();
    if (it != leaf->keys.end() && *it == key) {
        // Key already exists, add record to the existing vector
        leaf->records[index].push_back(recordPtr);
    } else {
        // New key, insert at the correct position
        leaf->keys.insert(it, key);
        leaf->records.insert(leaf->records.begin() + index, {recordPtr});
    }

    if (leaf->keys.size() > maxKeys) {
        splitLeafNode(leaf); // Split the leaf node if it overflows
    }
}

std::shared_ptr<Node> BPTree::findLeafNode(float key) const {
    std::shared_ptr<Node> cursor = root;
    while (!cursor->isLeaf) {
        // Find the child pointer to follow
        for (int i = 0; i < cursor->keys.size(); i++) {
            if (key < cursor->keys[i]) {
                cursor = cursor->children[i];
                break;
            }
            if (i == cursor->keys.size() - 1) { // If key is greater than all keys, follow the last pointer
                cursor = cursor->children[i + 1];
                break;
            }
        }
    }
    return cursor; // The node where the key fits
}
void BPTree::splitLeafNode(std::shared_ptr<Node>& leaf) {
    auto newLeaf = std::make_shared<Node>(true);
    int median = leaf->keys.size() / 2; // Choose the median

    // Move half the keys and records to the new leaf
    newLeaf->keys.assign(leaf->keys.begin() + median, leaf->keys.end());
    newLeaf->records.assign(leaf->records.begin() + median, leaf->records.end());

    leaf->keys.resize(median); // Keep the first half in the original leaf
    leaf->records.resize(median);

    if (leaf == root) {
        // If the leaf is the root, create a new root
        auto newRoot = std::make_shared<Node>(false);
        newRoot->keys.push_back(newLeaf->keys[0]);
        newRoot->children.push_back(leaf);
        newRoot->children.push_back(newLeaf);
        root = newRoot;
    } else {
        // Insert the first key of the new leaf to the parent
        insertInternal(newLeaf->keys[0], findParent(root, leaf), newLeaf);
    }

    newLeaf->next = leaf->next; // Link the new leaf with the next node
    leaf->next = newLeaf; // Link the original leaf with the new leaf
}

void BPTree::printTree() const {
    printTree(root, 0);
    std::cout << "---------------------------------\n";
}

void BPTree::printTree(std::shared_ptr<Node> node, int space) const {
    if (node == nullptr) return;

    int i;
    for (i = node->keys.size() - 1; i >= 0; i--) {
        if (!node->isLeaf) {
            printTree(node->children[i + 1], space + 5);
        }

        for (int j = 0; j < space; j++) {
            std::cout << ' ';
        }
        std::cout << node->keys[i] << "\n";
    }
    if (!node->isLeaf) {
        printTree(node->children[0], space + 5);
    }
}

void BPTree::insertInternal(float key, std::shared_ptr<Node> parent, std::shared_ptr<Node> child) {
    if (parent->keys.size() < maxKeys) { // Simple case: the parent has room for the new key
        auto it = std::lower_bound(parent->keys.begin(), parent->keys.end(), key);
        int index = it - parent->keys.begin();
        parent->keys.insert(it, key);
        parent->children.insert(parent->children.begin() + index + 1, child); // Insert the child pointer
    } else { // The parent needs to be split
        auto newInternal = std::make_shared<Node>(false);
        std::vector<float> tempKeys(parent->keys);
        std::vector<std::shared_ptr<Node>> tempChildren(parent->children);

        auto it = std::lower_bound(tempKeys.begin(), tempKeys.end(), key);
        int index = it - tempKeys.begin();
        tempKeys.insert(it, key); // Insert the key into the temporary vector
        tempChildren.insert(tempChildren.begin() + index + 1, child); // Insert the child pointer

        int median = tempKeys.size() / 2;
        parent->keys.assign(tempKeys.begin(), tempKeys.begin() + median); // Keep the first half in the parent
        parent->children.assign(tempChildren.begin(), tempChildren.begin() + median + 1);

        newInternal->keys.assign(tempKeys.begin() + median + 1, tempKeys.end()); // Move the second half to the new internal node
        newInternal->children.assign(tempChildren.begin() + median + 1, tempChildren.end());

        if (parent == root) { // If the parent is the root, create a new root
            auto newRoot = std::make_shared<Node>(false);
             newRoot->keys.push_back(tempKeys[median]); // Middle key moves up to new root
            newRoot->children.push_back(parent);       // Old root becomes left child of new root
            newRoot->children.push_back(newInternal);  // New internal node becomes right child of new root
            root = newRoot;                            // Update the root of the tree to be the new root node
        } else {
            // If the parent is not the root, we need to insert the median key into the parent's parent
            // This is a recursive process that may propagate up the tree
            insertInternal(tempKeys[median], findParent(root, parent), newInternal);
        }
    }
}

std::shared_ptr<Node> BPTree::findParent(const std::shared_ptr<Node>& cursor, const std::shared_ptr<Node>& child) {
    // Base case: if cursor is a leaf or direct parent of the child
    if (cursor->isLeaf || cursor->children[0]->isLeaf) {
        return nullptr; // Leaves do not have children, and direct parents of leaves are not further inspected
    }

    for (const auto& childNode : cursor->children) { // Use const auto& to match the parameter type
        if (childNode == child) {
            return cursor; // Direct parent found
        }
        auto potentialParent = findParent(childNode, child); // Recursively search in children
        if (potentialParent != nullptr) return potentialParent; // Parent found in a deeper level
    }

    return nullptr; // No parent found, this should not happen if the tree is correctly structured
}


std::vector<std::shared_ptr<Record>> BPTree::search(float key) const {
    std::shared_ptr<Node> cursor = root;
    while (!cursor->isLeaf) {
        // Navigate to the next level
        bool found = false;
        for (int i = 0; i < cursor->keys.size(); i++) {
            if (key < cursor->keys[i]) {
                cursor = cursor->children[i];
                found = true;
                break;
            }
        }
        if (!found) {
            // The key is greater than all the keys in the current node, go to the rightmost child
            cursor = cursor->children[cursor->children.size() - 1];
        }
    }

    // Now cursor is the leaf node, search for the key in the leaf
    for (int i = 0; i < cursor->keys.size(); i++) {
        if (cursor->keys[i] == key) {
            // Key found, return the associated records
            return cursor->records[i];
        }
    }

    // Key not found, return an empty vector
    return std::vector<std::shared_ptr<Record>>();
}

void BPTree::deleteKey(float key) {
    // Step 1: Find and remove the key from the leaf node
    std::shared_ptr<Node> leaf = findLeafNode(key);
    bool found = false;
    int idx; // Index of the key to be deleted
    for (idx = 0; idx < leaf->keys.size(); idx++) {
        if (leaf->keys[idx] == key) {
            found = true;
            break;
        }
    }

    if (!found) {
        std::cout << "Key " << key << " not found." << std::endl;
        return; // Key not found, nothing to delete
    }

    // Remove the key and the associated record
    leaf->keys.erase(leaf->keys.begin() + idx);
    leaf->records.erase(leaf->records.begin() + idx);

    if (leaf->keys.size() < minKeys) {
        auto parent = findParent(root, leaf);
        int childIndex = findChildIndex(parent, leaf);
        std::shared_ptr<Node> prevNode = (childIndex > 0) ? parent->children[childIndex - 1] : nullptr;
        std::shared_ptr<Node> nextNode = (childIndex < parent->children.size() - 1) ? parent->children[childIndex + 1] : nullptr;

        // Try to borrow from siblings
        if (prevNode && prevNode->keys.size() > minKeys) {
            borrowFromPrev(leaf, prevNode, parent, childIndex);
        } else if (nextNode && nextNode->keys.size() > minKeys) {
            borrowFromNext(leaf, nextNode, parent, childIndex);
             } else if (prevNode) { // Merge with previous sibling if borrowing is not possible
            mergeWithNext(prevNode, leaf, parent, childIndex - 1);
        } else if (nextNode) { // Alternatively, merge with next sibling
            mergeWithNext(leaf, nextNode, parent, childIndex);
        }

        // Check if parent needs rebalancing after a merge
        if (parent->keys.size() < minKeys && parent != root) {
            rebalanceAfterDeletion(parent);
        }
        if (root->keys.empty() && !root->isLeaf) {
        root = root->children.front(); // Make the single child the new root
}
    }
}

std::vector<std::shared_ptr<Record>> BPTree::rangeQuery(float lowerBound, float upperBound) const {
    std::vector<std::shared_ptr<Record>> results;

    // Start with finding the leaf node that contains the lower bound
    std::shared_ptr<Node> cursor = findLeafNode(lowerBound);

    // Iterate through the leaf nodes starting from the found leaf
    while (cursor != nullptr) {
        // Iterate through keys in the current leaf node
        for (int i = 0; i < cursor->keys.size(); ++i) {
            if (cursor->keys[i] >= lowerBound && cursor->keys[i] <= upperBound) {
                // Add all records associated with the current key to the results
                results.insert(results.end(), cursor->records[i].begin(), cursor->records[i].end());
            } else if (cursor->keys[i] > upperBound) {
                // Since keys are ordered, no need to check further if current key is beyond the upper bound
                return results;
            }
        }

        // Move to the next leaf node
        cursor = cursor->next;
    }

    return results;
}
void BPTree::borrowFromNext(std::shared_ptr<Node>& node, std::shared_ptr<Node>& nextNode, std::shared_ptr<Node>& parent, int parentIndex) {
    // Borrow the first key from the next sibling and insert it into the current node
    node->keys.push_back(nextNode->keys.front());
    nextNode->keys.erase(nextNode->keys.begin());

    // If the nodes are not leaves, also move the first child from the next sibling to the current node
    if (!node->isLeaf) {
        node->children.push_back(nextNode->children.front());
        nextNode->children.erase(nextNode->children.begin());
    }

    // Update the parent key that lies between the current node and the next sibling
    parent->keys[parentIndex] = nextNode->keys.front();
}

void BPTree::borrowFromPrev(std::shared_ptr<Node>& node, std::shared_ptr<Node>& prevNode, std::shared_ptr<Node>& parent, int parentIndex) {
    // Borrow the last key from the previous sibling and insert it at the beginning of the current node
    node->keys.insert(node->keys.begin(), prevNode->keys.back());
    prevNode->keys.pop_back();

    // If the nodes are not leaves, also move the last child from the previous sibling to the beginning of the current node
    if (!node->isLeaf) {
        node->children.insert(node->children.begin(), prevNode->children.back());
        prevNode->children.pop_back();
    }

    // Update the parent key that lies between the previous sibling and the current node
    parent->keys[parentIndex - 1] = node->keys.front();
}

void BPTree::mergeWithNext(std::shared_ptr<Node>& node, std::shared_ptr<Node>& nextNode, std::shared_ptr<Node>& parent, int parentIndex) {
    // Move all keys from the next sibling to the current node
    node->keys.insert(node->keys.end(), nextNode->keys.begin(), nextNode->keys.end());

    // If the nodes are not leaves, also move all children from the next sibling to the current node
    if (!node->isLeaf) {
        node->children.insert(node->children.end(), nextNode->children.begin(), nextNode->children.end());
    }

    // Remove the key and pointer for the next node from the parent
    parent->keys.erase(parent->keys.begin() + parentIndex);
    parent->children.erase(parent->children.begin() + parentIndex + 1);
}
int BPTree::findChildIndex(std::shared_ptr<Node>& parent, std::shared_ptr<Node>& child) {
    // Find the index of 'child' in 'parent's children list
    for (int i = 0; i < parent->children.size(); ++i) {
        if (parent->children[i] == child) return i;
    }
    return -1; // Should not happen if the tree is correctly structured
}
void BPTree::rebalanceAfterDeletion(std::shared_ptr<Node>& node) {
    auto parent = findParent(root, node);
    int childIndex = findChildIndex(parent, node);
    std::shared_ptr<Node> prevNode = (childIndex > 0) ? parent->children[childIndex - 1] : nullptr;
    std::shared_ptr<Node> nextNode = (childIndex < parent->children.size() - 1) ? parent->children[childIndex + 1] : nullptr;

    if (node->keys.size() < minKeys) {
        if (prevNode && prevNode->keys.size() > minKeys) {
            borrowFromPrev(node, prevNode, parent, childIndex);
        } else if (nextNode && nextNode->keys.size() > minKeys) {
            borrowFromNext(node, nextNode, parent, childIndex);
        } else if (prevNode) {
            mergeWithNext(prevNode, node, parent, childIndex - 1);
        } else if (nextNode) {
            mergeWithNext(node, nextNode, parent, childIndex);
        }

        // If the parent becomes underfull, continue rebalancing up the tree
        if (parent->keys.size() < minKeys && parent != root) {
            rebalanceAfterDeletion(parent);
        }
    }
}
