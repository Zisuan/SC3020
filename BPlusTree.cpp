#include <iostream>
#include <climits>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "BPlusTree.h"
#include "Record.h"
#include <queue>
#include <set>
#include <chrono>

using namespace std;

// size of node = size of block = 200
// size of node = 2 +4N + 4 + 8(N+1)
// 200 = 12N +14
// N = (200-14)/12
const int N = (200 - 14) / 12;

Node::Node()
{
  key = new int[N];
  ptr = new Node *[N + 1];
  records = new unsigned char *[N];
};

BPlusTree::BPlusTree(){};

void BPlusTree::insertKey(int x, unsigned char *record) {
  if (root == nullptr) {
    root = createNewLeafNode(x, record);
    cout << "Root created:  " << x << endl;
    ++nodes;
    ++levels;
    ++numKeys;
    return;
  }

  Node* parent = traverseToLeafNode(x)[0];
  Node* curNode = traverseToLeafNode(x)[1];
  int insertIndex = 0;

  while (insertIndex < curNode->size && x > curNode->key[insertIndex]) {
    ++insertIndex;
  }

  if (insertIndex < curNode->size && x == curNode->key[insertIndex]) {
    Node* curBuffer = curNode->ptr[insertIndex];
    while (curBuffer->ptr[0] != nullptr) {
      curBuffer = curBuffer->ptr[0];
    }
    if (curBuffer->size < N) {
      curBuffer->records[curBuffer->size++] = record;
    } else {
      Node* newBuffer = createNewBufferNode(x, record);
      curBuffer->ptr[0] = newBuffer;
    }
  }
  else if (curNode->size < N) {
    ++numKeys;
    for (int i = curNode->size; i > insertIndex; --i) {
      curNode->key[i] = curNode->key[i - 1];
      curNode->ptr[i] = curNode->ptr[i - 1];
    }
    curNode->key[insertIndex] = x;
    curNode->ptr[insertIndex] = createNewBufferNode(x, record);
    ++curNode->size;
    cout << "Inserted " << x << endl;
  }
  else {
    ++nodes;
    ++numKeys;
    splitLeafNode(curNode, x, record, parent);
    cout << "Inserted " << x << endl;
  }
}

void BPlusTree::splitLeafNode(Node* curNode, int x, unsigned char* record, Node* parent) {
  Node* newLeaf = new Node;
  int tempKeys[N + 1];
  Node* tempPtrs[N + 1];

  int insertIndex = 0;
  while (insertIndex < curNode->size && x > curNode->key[insertIndex]) {
    ++insertIndex;
  }
  for (int i = 0; i <= N; ++i) {
    if (i < insertIndex) {
      tempKeys[i] = curNode->key[i];
      tempPtrs[i] = curNode->ptr[i];
    } else if (i == insertIndex) {
      tempKeys[i] = x;
      tempPtrs[i] = createNewBufferNode(x, record);
    } else {
      tempKeys[i] = curNode->key[i - 1];
      tempPtrs[i] = curNode->ptr[i - 1];
    }
  }
  curNode->size = (N + 1) / 2;
  newLeaf->size = (N + 1) - curNode->size;
  for (int i = 0; i < curNode->size; ++i) {
    curNode->key[i] = tempKeys[i];
    curNode->ptr[i] = tempPtrs[i];
  }
  for (int i = 0; i < newLeaf->size; ++i) {
    newLeaf->key[i] = tempKeys[i + curNode->size];
    newLeaf->ptr[i] = tempPtrs[i + curNode->size];
  }

  newLeaf->ptr[N] = curNode->ptr[N];
  curNode->ptr[N] = newLeaf;
  newLeaf->IS_LEAF = true;

  if (curNode == root) {
    createNewRoot(curNode, newLeaf);
  } else {
    insertInternal(newLeaf->key[0], parent, newLeaf);
  }
}

void BPlusTree::createNewRoot(Node* leftChild, Node* rightChild) {
  Node* newRoot = new Node();

  newRoot->key[0] = rightChild->key[0];
  newRoot->ptr[0] = leftChild;
  newRoot->ptr[1] = rightChild;
  newRoot->IS_LEAF = false;
  newRoot->size = 1;
  root = newRoot;
  ++levels;
}

void BPlusTree::insertInternal(int x, Node *parent, Node *child) {
    if (parent->size < N) {
        int pos = 0;
        while (x > parent->key[pos] && pos < parent->size) {
            pos++;
        }
        for (int j = parent->size; j > pos; j--) {
            parent->key[j] = parent->key[j - 1];
            parent->ptr[j + 1] = parent->ptr[j];
        }
        parent->key[pos] = x;
        parent->ptr[pos + 1] = child;
        parent->size++;
    } else {
        this->nodes++;
        Node *splitNode = new Node();
        int tempKeys[N + 1];
        Node *tempPointers[N + 2];
        memcpy(tempKeys, parent->key, N * sizeof(int));
        memcpy(tempPointers, parent->ptr, (N + 1) * sizeof(Node *));
        tempPointers[N] = parent->ptr[N];
        int idx = 0;
        while (x > tempKeys[idx] && idx < N) {
            idx++;
        }
        for (int k = N; k > idx; k--) {
            tempKeys[k] = tempKeys[k - 1];
        }
        tempKeys[idx] = x;
        for (int l = N + 1; l > idx; l--) {
            tempPointers[l] = tempPointers[l - 1];
        }
        tempPointers[idx + 1] = child;
        splitNode->IS_LEAF = false;
        parent->size = (N + 1) / 2;
        splitNode->size = N - parent->size;
        memcpy(splitNode->key, tempKeys + parent->size + 1, splitNode->size * sizeof(int));
        memcpy(splitNode->ptr, tempPointers + parent->size + 1, (splitNode->size + 1) * sizeof(Node *));
        if (parent == root) {
            this->nodes++;
            this->levels++;
            Node *newRoot = new Node();
            newRoot->key[0] = parent->key[parent->size];
            newRoot->ptr[0] = parent;
            newRoot->ptr[1] = splitNode;
            newRoot->IS_LEAF = false;
            newRoot->size = 1;
            root = newRoot;
        } else {
            insertInternal(parent->key[parent->size], findParent(root, parent), splitNode);
        }
    }
}

void BPlusTree::experiment3(int numVotesToRetrieve)
{
  int indexNodesAccessed = 0;
  int dataBlocksAccessed = 0;
  double totalRatings = 0.0;
  int recordsAccessed = 0; // The number of records examined
  int matchingRecordsCount = 0;

  auto targetedSearchStart = std::chrono::high_resolution_clock::now();

  // The traversal part is hypothetical and needs to be replaced with your actual traversal logic
  Node *current = root;
  while (current != nullptr && !current->IS_LEAF)
  {
    current = current->ptr[0]; // Traverse down to the leftmost leaf
    indexNodesAccessed++;
  }

  // Start iterating through leaf nodes
  while (current != nullptr)
  {
    dataBlocksAccessed++;
    for (int i = 0; i < current->size; i++)
    {
      if (current->key[i] == numVotesToRetrieve)
      {
        // Access the buffer node linked to this key
        Node *bufferNode = current->ptr[i];
        while (bufferNode != nullptr)
        {
          for (int j = 0; j < bufferNode->size; j++)
          {
            recordsAccessed++; // Incremented for each record examined
            Record *record = reinterpret_cast<Record *>(bufferNode->records[j]);
            if (record->numVotes == numVotesToRetrieve)
            {
              totalRatings += record->averageRating;
              matchingRecordsCount++;
            }
          }
          bufferNode = bufferNode->ptr[0]; // Assuming this is how you traverse buffer nodes
        }
      }
    }
    current = current->ptr[N]; // Move to the next leaf node. Adjust this access according to your implementation
  }

  auto targetedSearchEnd = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> targetedDuration = targetedSearchEnd - targetedSearchStart;

  double averageRating = matchingRecordsCount > 0 ? totalRatings / matchingRecordsCount : 0.0;
  // Brute-force linear scan
  int bruteForceDataBlocksAccessed = 0;
  int bruteForceRecordsAccessed = 0;
  double bruteForceTotalRatings = 0.0;
  int bruteForceMatchingRecordsCount = 0;
  auto bruteForceStart = std::chrono::high_resolution_clock::now();

  // Re-initialize traversal from the root to the leftmost leaf for a full scan
  Node *bruteCurrent = root;
  while (bruteCurrent != nullptr && !bruteCurrent->IS_LEAF)
  {
    bruteCurrent = bruteCurrent->ptr[0]; // Navigate to the leftmost leaf
  }

  // Start brute-force scan iterating through all leaf nodes
  while (bruteCurrent != nullptr)
  {
    bruteForceDataBlocksAccessed++;
    for (int i = 0; i < bruteCurrent->size; i++)
    {
      // Instead of checking the key, directly access all records
      Node *bufferNode = bruteCurrent->ptr[i];
      while (bufferNode != nullptr)
      {
        for (int j = 0; j < bufferNode->size; j++)
        {
          Record *record = reinterpret_cast<Record *>(bufferNode->records[j]);
          // Count this as accessing a data block, since we're inspecting the record
          bruteForceRecordsAccessed++; // Correctly count each inspected record
          if (record->numVotes == numVotesToRetrieve)
          {
            bruteForceTotalRatings += record->averageRating;
            bruteForceMatchingRecordsCount++;
          }
        }
        bufferNode = bufferNode->ptr[0]; // Move to the next buffer node
      }
    }
    bruteCurrent = bruteCurrent->ptr[N]; // Moving to the next leaf node
  }

  auto bruteForceEnd = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> bruteForceDuration = bruteForceEnd - bruteForceStart;

  double bruteForceAverageRating = bruteForceMatchingRecordsCount > 0 ? bruteForceTotalRatings / bruteForceMatchingRecordsCount : 0.0;

  std::cout << "Experiment 3 Statistics:" << std::endl;
  std::cout << "Number of index nodes accessed: " << indexNodesAccessed << std::endl;
  std::cout << "Number of data blocks accessed: " << dataBlocksAccessed << std::endl;
  std::cout << "Number of records accessed: " << recordsAccessed << std::endl; //
  std::cout << "Average rating of matching records: " << averageRating << std::endl;
  std::cout << "Running time of the retrieval process: " << targetedDuration.count() << " milliseconds." << std::endl;

  // Display statistics for brute-force scan
  std::cout << "\nBrute-Force Scan Statistics:" << std::endl;
  std::cout << "Number of data blocks accessed: " << bruteForceDataBlocksAccessed << std::endl;
  std::cout << "Number of records accessed: " << bruteForceRecordsAccessed << std::endl; //
  std::cout << "Average rating of matching records: " << bruteForceAverageRating << std::endl;
  std::cout << "Running time of the brute-force scan process: " << bruteForceDuration.count() << " milliseconds." << std::endl;
}

void BPlusTree::experiment4(int minVotes, int maxVotes)
{
  int indexNodesAccessed = 0;
  int dataBlocksAccessed = 0;
  int recordsAccessed = 0;
  double totalRatings = 0.0;
  int matchingRecordsCount = 0;
  auto start = std::chrono::high_resolution_clock::now();

  // Start with the root and traverse down to the first relevant leaf node
  Node *current = root;
  while (current != nullptr && !current->IS_LEAF)
  {
    indexNodesAccessed++;
    bool found = false;
    for (int i = 0; i < current->size; i++)
    {
      if (minVotes <= current->key[i])
      {
        current = current->ptr[i];
        found = true;
        break;
      }
    }
    if (!found)
    {
      current = current->ptr[current->size];
    }
  }

  // Now current points to the first relevant leaf or the leftmost leaf
  while (current != nullptr)
  {
    dataBlocksAccessed++;
    for (int i = 0; i < current->size; i++)
    {
      if (current->key[i] >= minVotes && current->key[i] <= maxVotes)
      {
        // Assuming the record structure is available here
        // For each relevant record, accumulate ratings and count
        Node *bufferNode = current->ptr[i];
        while (bufferNode != nullptr)
        {
          for (int j = 0; j < bufferNode->size; j++)

          {
            recordsAccessed++;
            Record *record = reinterpret_cast<Record *>(bufferNode->records[j]);
            totalRatings += record->averageRating;
            matchingRecordsCount++;
          }
          bufferNode = bufferNode->ptr[0]; // Move to the next buffer node
        }
      }
    }
    if (current->key[current->size - 1] > maxVotes)
      break;                   // Stop if the last key is beyond the range
    current = current->ptr[N]; // Move to the next leaf node
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = end - start;

  double averageRating = (matchingRecordsCount > 0) ? totalRatings / matchingRecordsCount : 0.0;

  int bruteForceDataBlocksAccessed = 0;
  int bruteForceRecordsAccessed = 0;
  double bruteForceTotalRatings = 0.0;
  int bruteForceMatchingRecordsCount = 0;
  auto bruteStart = std::chrono::high_resolution_clock::now();

  Node *bruteCurrent = root;
  while (bruteCurrent != nullptr && !bruteCurrent->IS_LEAF)
  {
    // Navigate to the leftmost leaf without checking keys
    bruteCurrent = bruteCurrent->ptr[0];
  }

  // Start brute-force scan iterating through all leaf nodes
  while (bruteCurrent != nullptr)
  {
    bruteForceDataBlocksAccessed++;
    for (int i = 0; i < bruteCurrent->size; i++)
    {
      // Directly access all records without key-based filtering
      Node *bufferNode = bruteCurrent->ptr[i];
      while (bufferNode != nullptr)
      {
        for (int j = 0; j < bufferNode->size; j++)
        {
          bruteForceRecordsAccessed++; // Count each inspected record
          Record *record = reinterpret_cast<Record *>(bufferNode->records[j]);
          if (record->numVotes >= minVotes && record->numVotes <= maxVotes)
          {
            bruteForceTotalRatings += record->averageRating;
            bruteForceMatchingRecordsCount++;
          }
        }
        bufferNode = bufferNode->ptr[0]; // Move to the next buffer node
      }
    }
    bruteCurrent = bruteCurrent->ptr[N]; // Move to the next leaf node
  }

  auto bruteEnd = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> bruteDuration = bruteEnd - bruteStart;
  double bruteForceAverageRating = (bruteForceMatchingRecordsCount > 0) ? bruteForceTotalRatings / bruteForceMatchingRecordsCount : 0.0;

  // Display the statistics
  std::cout << "Experiment 4 Statistics:\n";
  std::cout << "Number of index nodes accessed: " << indexNodesAccessed << "\n";
  std::cout << "Number of data blocks accessed: " << dataBlocksAccessed << "\n";
  std::cout << "Number of records accessed: " << recordsAccessed << "\n";
  std::cout << "Average rating of matching records: " << averageRating << "\n";
  std::cout << "Running time of the retrieval process: " << duration.count() << " milliseconds.\n";

  // Display the statistics for brute-force scan
  std::cout << "\nBrute-Force Scan Statistics:\n";
  std::cout << "Number of data blocks accessed: " << bruteForceDataBlocksAccessed << "\n";
  std::cout << "Number of records accessed: " << bruteForceRecordsAccessed << "\n";
  std::cout << "Average rating of matching records: " << bruteForceAverageRating << "\n";
  std::cout << "Running time of the brute-force scan process: " << bruteDuration.count() << " milliseconds.\n";
}

// experiment 3 and 4 and re-formatted search functions
void BPlusTree::search(int x) {
  // Check for non-empty tree
  if (root == nullptr) {
    cout << "Not found\n";
    return;
  }

  // Traverse to the potential leaf node containing x
  Node* curNode = traverseToLeafNode(x)[1];
  
  // Iterate over keys in the current node
  for (int i = 0; i < curNode->size; ++i) {
    if (curNode->key[i] != x) continue;  // Skip non-matching keys
    
    cout << "Found\n";

    // Initialize count and set buffer to the specific child node
    int count = 0;
    Node* buffer = curNode->ptr[i];

    // Loop to aggregate counts, breaking when a non-full node is encountered
    do {
      count += buffer->size;
      buffer = (buffer->size == N) ? buffer->ptr[0] : nullptr;
    } while (buffer != nullptr);

    cout << count << endl;
    return;  // Exit after processing the found key
  }

  // Key not found in the tree
  cout << "Not found\n";
}

void BPlusTree::experiment2()
{
  cout << "Experiment 2" << endl;
  // cout << "Number of Unique Key Values: " << this->numKeys << endl;
  cout << "Parameter N: " << N << endl;
  cout << "Number of Nodes: " << this->nodes << endl;
  cout << "Number of Levels: " << this->levels << endl;
  cout << "Keys of Root Node:" << endl;
  if (root != NULL)
  {
    for (int i = 0; i < root->size; i++)
    {
      cout << "Key " << i << ": " << root->key[i] << endl;
    }
  }
}
void BPlusTree::experiment5(int numVotesToDelete)
{
  int bruteForceBlocksAccessed = 0;
  int totalCount = 0;
  auto bfStart = std::chrono::high_resolution_clock::now();
  Node *current = root;
  while (current != NULL && !current->IS_LEAF)
  {
    current = current->ptr[0];
  }
  while (current != NULL)
  {
    bruteForceBlocksAccessed++;
    for (int i = 0; i < current->size; i++)
    {
      if (current->key[i] == numVotesToDelete)
      {
        Node *bufferNode = current->ptr[i];
        while (bufferNode != NULL)
        {
          totalCount += bufferNode->size;
          bufferNode = bufferNode->ptr[0];
        }
      }
    }
    current = current->ptr[N];
  }
  auto bfEnd = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> bfDuration = bfEnd - bfStart;
  bfDuration = bfDuration * 1000;
  std::cout << "Brute-force scan accessed " << bruteForceBlocksAccessed << " blocks." << std::endl;
  std::cout << "Found " << totalCount << " records with numVotes equal to " << numVotesToDelete << std::endl;
  std::cout << "Brute-force scan running time: " << bfDuration.count() << " milliseconds." << std::endl;

  auto start = std::chrono::high_resolution_clock::now();
  deleteKey(numVotesToDelete);

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;
  duration = duration * 1000;
  int numberOfNodes = this->nodes;
  int numberOfLevels = this->levels;
  std::vector<int> rootKeys;
  if (root != NULL)
  {
    for (int i = 0; i < root->size; i++)
    {
      rootKeys.push_back(root->key[i]);
    }
  }

  std::cout << "Experiment 5 Statistics:" << std::endl;
  std::cout << "Number of nodes in the B+ tree: " << numberOfNodes << std::endl;
  std::cout << "Number of levels in the B+ tree: " << numberOfLevels << std::endl;
  std::cout << "Keys of the root node: ";
  for (int key : rootKeys)
  {
    std::cout << key << " ";
  }
  std::cout << std::endl;
  std::cout << "Running time of the deletion process: " << duration.count() << " millieconds." << std::endl;
}

Node* BPlusTree::createNewLeafNode(int key, unsigned char* data) {
    Node* leafNode = new Node(); // Ensure Node's constructor initializes arrays dynamically based on N.
    if (!leafNode) {
        // Handle allocation failure if applicable in your environment. Typically, new throws, so this is more illustrative.
        std::cerr << "Memory allocation for new leaf node failed." << std::endl;
        return nullptr;
    }

    leafNode->key[0] = key;
    leafNode->IS_LEAF = true;
    leafNode->size = 1;
    leafNode->ptr[0] = createNewBufferNode(key, data); // Link to buffer node containing the record
    leafNode->ptr[N] = nullptr; // Rightmost pointer in a leaf node is always null

    return leafNode;
}

Node* BPlusTree::createNewBufferNode(int key, unsigned char* data) {
    Node* bufferNode = new Node(); // Assume Node's constructor handles array initialization.
    if (!bufferNode) {
        // Similar illustrative error handling for memory allocation failure.
        std::cerr << "Memory allocation for new buffer node failed." << std::endl;
        return nullptr;
    }

    bufferNode->records[0] = data;
    bufferNode->size = 1;
    bufferNode->key[0] = key;
    bufferNode->ptr[0] = nullptr; // Buffer nodes do not point to other nodes.

    return bufferNode;
}

// returns [Node* parentOfLeafNode, Node *leafNode]
Node **BPlusTree::traverseToLeafNode(int targetKey) {
    Node **path = new Node *[2]; // Array to hold the parent and child nodes on the path to the targetKey
    path[1] = root; // Start with the root as the current node (child in the context of the first iteration)

    // Traverse down to the leaf node
    while (!path[1]->IS_LEAF) {
        path[0] = path[1]; // Update the parent node
        bool foundLesserKey = false; // Flag to check if a lesser key is found

        // Search for the first key greater than targetKey
        for (int i = 0; i < path[1]->size; i++) {
            if (targetKey < path[1]->key[i]) {
                path[1] = path[1]->ptr[i]; // Move to the appropriate child node
                foundLesserKey = true;
                break; // Exit the loop as the correct child node is found
            }
        }

        // If targetKey is greater than all keys, move to the rightmost child
        if (!foundLesserKey) {
            path[1] = path[1]->ptr[path[1]->size];
        }
    }
    // At this point, path[1] is the leaf node where targetKey should be located, and path[0] is its parent
    return path;
}

Node* BPlusTree::findParent(Node* currentNode, Node* targetChild) {
    // Base condition to stop if currentNode is a leaf or its children are leaves
    if (currentNode->IS_LEAF || (currentNode->ptr[0]->IS_LEAF)) {
        return NULL;
    }

    int childIndex = 0; // Initialize index to iterate through child pointers
    while (childIndex < currentNode->size + 1) {
        if (currentNode->ptr[childIndex] == targetChild) {
            // If the direct child is the target, currentNode is the parent
            return currentNode;
        }

        // If not directly found, search recursively in the subtree
        Node* foundParent = findParent(currentNode->ptr[childIndex], targetChild);
        if (foundParent != NULL) {
            // Parent found in the subtree
            return foundParent;
        }
        
        childIndex++; // Move to the next child pointer
    }

    // If the function reaches here, no parent was found in the current path
    return NULL;
}

void BPlusTree::deleteKey(int x) {
    if (root == NULL)
        return;

    Node *curNode = root, *parent = nullptr;
    int leftPtrIndex, rightPtrIndex;
    int index = -1;
    while (!curNode->IS_LEAF) {
        index = -1;
        for (int i = 0; i < curNode->size; i++) {
            parent = curNode;
            if (x < curNode->key[i]) {
                curNode = curNode->ptr[i];
                index = i;
                break;
            }
        }
        if (index == -1) {
            index = curNode->size;
            curNode = curNode->ptr[curNode->size];
        }
        leftPtrIndex = index - 1;
        rightPtrIndex = index;
    }

    bool found = false;
    for (int i = 0; i < curNode->size; i++) {
        if (curNode->key[i] == x) {
            for (int j = i; j < curNode->size - 1; j++) {
                curNode->key[j] = curNode->key[j + 1];
                curNode->ptr[j] = curNode->ptr[j + 1];
            }
            curNode->size--;
            this->deleteCounter++;
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "Key not found" << endl;
        return;
    }

    // Handle underflow in the leaf node
    if (curNode->size < (N + 1) / 2 && curNode != root) {
        Node *leftSibling = (leftPtrIndex >= 0) ? parent->ptr[leftPtrIndex] : nullptr;
        Node *rightSibling = (rightPtrIndex < parent->size) ? parent->ptr[rightPtrIndex] : nullptr;

        if (leftSibling && leftSibling->size > (N + 1) / 2) {
            // Borrow from left sibling
            for (int i = curNode->size; i > 0; i--) {
                curNode->key[i] = curNode->key[i - 1];
                curNode->ptr[i] = curNode->ptr[i - 1];
            }
            curNode->key[0] = leftSibling->key[leftSibling->size - 1];
            curNode->ptr[0] = leftSibling->ptr[leftSibling->size - 1];
            curNode->size++;
            leftSibling->size--;
            parent->key[leftPtrIndex] = curNode->key[0];
        } else if (rightSibling && rightSibling->size > (N + 1) / 2) {
            // Borrow from right sibling
            curNode->key[curNode->size] = rightSibling->key[0];
            curNode->ptr[curNode->size] = rightSibling->ptr[0];
            curNode->size++;
            for (int i = 0; i < rightSibling->size - 1; i++) {
                rightSibling->key[i] = rightSibling->key[i + 1];
                rightSibling->ptr[i] = rightSibling->ptr[i + 1];
            }
            rightSibling->size--;
            parent->key[rightPtrIndex - 1] = rightSibling->key[0];
        } else {
            // Merge with a sibling
            if (leftSibling) {
                // Merge curNode with leftSibling
                for (int i = leftSibling->size, j = 0; j < curNode->size; i++, j++) {
                    leftSibling->key[i] = curNode->key[j];
                    leftSibling->ptr[i] = curNode->ptr[j];
                }
                leftSibling->size += curNode->size;
                deleteInternal(parent->key[leftPtrIndex], parent, curNode);
                deallocate(curNode); // Assuming deallocate properly frees the node
            } else if (rightSibling) {
                // Merge curNode into rightSibling
                for (int i = curNode->size, j = 0; j < rightSibling->size; i++, j++) {
                    curNode->key[i] = rightSibling->key[j];
                    curNode->ptr[i] = rightSibling->ptr[j];
                }
                curNode->size += rightSibling->size;
                deleteInternal(parent->key[rightPtrIndex], parent, rightSibling);
                deallocate(rightSibling); // Assuming deallocate properly frees the node
            }
        }
    }

    cout << "Deleted key Successfully" << endl;
}

void BPlusTree::deleteInternal(int x, Node* curNode, Node* child) {
    if (curNode == root && curNode->size == 1) {
        root = (curNode->ptr[0] == child) ? curNode->ptr[1] : curNode->ptr[0];
        deallocate(curNode);
        cout << "Changed root node\n";
        return;
    }

    int index = -1;
    for (int i = 0; i < curNode->size; i++) {
        if (curNode->key[i] == x) {
            index = i;
            break;
        }
    }

    // Shift keys and pointers left to delete the key and its right child pointer
    for (int i = index; i < curNode->size - 1; i++) {
        curNode->key[i] = curNode->key[i + 1];
        curNode->ptr[i + 1] = curNode->ptr[i + 2];
    }
    curNode->size--;

    if (curNode->size < N / 2 && curNode != root) {
        Node *parent = findParent(root, curNode);
        int leftPtrIndex = -1, rightPtrIndex = -1;

        for (int i = 0; i <= parent->size; i++) {
            if (parent->ptr[i] == curNode) {
                leftPtrIndex = i - 1;
                rightPtrIndex = i + 1;
                break;
            }
        }

        Node* leftSibling = leftPtrIndex >= 0 ? parent->ptr[leftPtrIndex] : nullptr;
        Node* rightSibling = rightPtrIndex <= parent->size ? parent->ptr[rightPtrIndex] : nullptr;

        if (leftSibling && leftSibling->size > N / 2) {
            for (int i = curNode->size; i > 0; i--) {
                curNode->key[i] = curNode->key[i - 1];
                curNode->ptr[i] = curNode->ptr[i - 1];
            }
            curNode->key[0] = parent->key[leftPtrIndex];
            curNode->ptr[0] = leftSibling->ptr[leftSibling->size];
            curNode->size++;
            leftSibling->size--;
            parent->key[leftPtrIndex] = leftSibling->key[leftSibling->size];
        } else if (rightSibling && rightSibling->size > N / 2) {
            curNode->key[curNode->size] = parent->key[rightPtrIndex - 1];
            curNode->ptr[curNode->size + 1] = rightSibling->ptr[0];
            curNode->size++;
            for (int i = 0; i < rightSibling->size - 1; i++) {
                rightSibling->key[i] = rightSibling->key[i + 1];
                rightSibling->ptr[i] = rightSibling->ptr[i + 1];
            }
            rightSibling->size--;
            parent->key[rightPtrIndex - 1] = rightSibling->key[0];
        } else {
            if (leftSibling) {
                // Merge curNode with leftSibling
                leftSibling->key[leftSibling->size] = parent->key[leftPtrIndex];
                for (int i = leftSibling->size + 1, j = 0; j < curNode->size; i++, j++) {
                    leftSibling->key[i] = curNode->key[j];
                    leftSibling->ptr[i] = curNode->ptr[j];
                }
                leftSibling->size += curNode->size + 1;
                // Adjust parent
                for (int i = leftPtrIndex; i < parent->size - 1; i++) {
                    parent->key[i] = parent->key[i + 1];
                    parent->ptr[i + 1] = parent->ptr[i + 2];
                }
                parent->size--;
                deallocate(curNode);
            } else if (rightSibling) {
                // Merge curNode into rightSibling
                curNode->key[curNode->size] = parent->key[rightPtrIndex - 1];
                for (int i = curNode->size + 1, j = 0; j < rightSibling->size; i++, j++) {
                    curNode->key[i] = rightSibling->key[j];
                    curNode->ptr[i] = rightSibling->ptr[j];
                }
                curNode->size += rightSibling->size + 1;
                // Adjust parent
                for (int i = rightPtrIndex - 1; i < parent->size - 1; i++) {
                    parent->key[i] = parent->key[i + 1];
                    parent->ptr[i + 1] = parent->ptr[i + 2];
                }
                parent->size--;
                deallocate(rightSibling);
            }
            // If parent is now underflowed and not the root, recurse
            if (parent->size < N / 2 && parent != root) {
                Node* grandparent = findParent(root, parent);
                deleteInternal(parent->key[0], grandparent, parent); // Use the first key as an example; adjust based on your deletion policy
            }
        }
    }
}

// helper functions
void BPlusTree::deallocate(Node *node)
{
  delete[] node->key;
  delete[] node->ptr;
  delete node;
}