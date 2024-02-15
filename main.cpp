// main.cpp
#include "BPlusTree.h"
#include <iostream>
#include <memory>

int main() {
    BPTree bptree(200); // Initialize B+ tree with a hypothetical block size

    // Example: Insert some data into the B+ tree
    bptree.insert(10, std::make_shared<Record>("ID1", 5.0, 10));
    bptree.insert(20, std::make_shared<Record>("ID2", 6.0, 20));
    bptree.insert(30, std::make_shared<Record>("ID3", 7.0, 30));
    // Add more inserts as needed...

    // Perform a range query
    float lowerBound = 15;
    float upperBound = 25;
    auto recordsInRange = bptree.rangeQuery(lowerBound, upperBound);

    std::cout << "Records in range [" << lowerBound << ", " << upperBound << "]:" << std::endl;
    for (const auto& record : recordsInRange) {
        std::cout << "ID: " << record->tconst << ", Rating: " << record->averageRating << ", Votes: " << record->numVotes << std::endl;
    }

    return 0;
}