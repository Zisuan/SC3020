#include "BPlusTree.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include "Storage.h" // Include the header file for storage-related functions
#include "Record.h"

void loadData(BPTree &bptree, const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line); // Skip the header line if present.

    int lineCounter = 0; // Add a counter for the lines read

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string tconst, averageRatingStr, numVotesStr;
        std::getline(iss, tconst, '\t');
        std::getline(iss, averageRatingStr, '\t');
        std::getline(iss, numVotesStr, '\t');

        try {
            float averageRating = std::stof(averageRatingStr);
            int numVotes = std::stoi(numVotesStr);

            auto record = std::make_shared<Record>(tconst, averageRating, numVotes);
            bptree.insert(numVotes, record); // Assuming the B+ tree is indexed on 'numVotes'.
        } catch (const std::invalid_argument& ia) {
            std::cerr << "Invalid argument: " << ia.what() << " for line: " << line << std::endl;
            continue; // Skip this record and continue with the next
        } catch (const std::out_of_range& oor) {
            std::cerr << "Out of range: " << oor.what() << " for line: " << line << std::endl;
            continue; // Skip this record and continue with the next
        }

        lineCounter++; // Increment the counter for each line read

        if (lineCounter % 10000 == 0) {
            std::cout << "Loaded " << lineCounter << " records." << std::endl;
        }
    }

    file.close();
    std::cout << "Data loaded successfully. Total records loaded: " << lineCounter << std::endl;
}

// Utility function to print experiment header
void printExperimentHeader(const std::string &title) {
    std::cout << "\n" << title << "\n";
    std::cout << std::string(title.length(), '=') << "\n";
}

// Experiment 1: Storage Statistics
void experiment1(const BPTree &bptree)
{
    printExperimentHeader("Experiment 1: Storage Statistics");

    SimulatedDisk disk(DISK_CAPACITY); // Initialize the simulated disk
    readTSVAndCreateBlocks("Data/data.tsv", disk);
    disk.writeToDisk("Data.dat");

    // Output formatting functions for display
    printHeader("Processing Summary");
    printKeyValue("Total lines processed", std::to_string(disk.totalRecords()));
    printKeyValue("Total number of records", std::to_string(disk.totalRecords()));

    printHeader("Record Information");
    printKeyValue("Size of a record", std::to_string(sizeof(Record)) + " bytes");
    printKeyValue("Number of records stored in a block", std::to_string(BLOCK_SIZE / sizeof(Record)));

    printHeader("Disk Usage");
    printKeyValue("Number of blocks for storing the data", std::to_string(disk.totalBlocks()));
    printKeyValue("Used disk capacity", std::to_string(disk.usedCapacity()) + " bytes");
    printKeyValue("Total disk capacity", std::to_string(DISK_CAPACITY) + " bytes");
    printKeyValue("Free disk capacity", std::to_string(DISK_CAPACITY - disk.usedCapacity()) + " bytes");

    // Assuming each leaf node is a block, and the size of a record is the size of the Record structure
    int recordSize = sizeof(Record);
    int recordsPerBlock = bptree.getMaxKeys(); // Assuming getMaxKeys() returns the maximum number of records per block

    // You need to implement getTotalRecords() in your BPTree to return the total number of records
    int totalRecords = bptree.getTotalRecords();

    // Assuming getTotalBlocks() is implemented in BPTree to return the total number of blocks (leaf nodes)
    int totalBlocks = bptree.getTotalBlocks();

    std::cout << "Total Records: " << totalRecords << std::endl;
    std::cout << "Record Size: " << recordSize << " bytes" << std::endl;
    std::cout << "Records per Block: " << recordsPerBlock << std::endl;
    std::cout << "Total Blocks: " << totalBlocks << std::endl;
}

// Experiment 2: B+ Tree Statistics
void experiment2(BPTree &bptree) {
    printExperimentHeader("Experiment 2: B+ Tree Statistics");

    if (bptree.getTotalNodes() == 0) {
        std::cout << "No data loaded in B+ Tree. Please load data first.\n";
        return;
    }

    bptree.experiment2Statistics();
}



// Experiment 3: Query for numVotes = 500
void experiment3(BPTree &bptree)
{
    printExperimentHeader("Experiment 3: Query for numVotes = 500");

    auto start = std::chrono::high_resolution_clock::now();

    auto results = bptree.search(500); // Assuming search() returns a vector of shared_ptr<Record> for the given key

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    float averageRating = 0;
    for (const auto &record : results)
    {
        averageRating += record->averageRating;
    }
    if (!results.empty())
    {
        averageRating /= results.size();
    }

    std::cout << "Number of Records Found: " << results.size() << std::endl;
    std::cout << "Average Rating: " << averageRating << std::endl;
    std::cout << "Elapsed Time: " << elapsed.count() << " ms" << std::endl;
}

// Experiment 4: Range Query for numVotes between 30,000 and 40,000
void experiment4(BPTree &bptree)
{
    printExperimentHeader("Experiment 4: Range Query for numVotes between 30,000 and 40,000");

    auto start = std::chrono::high_resolution_clock::now();

    auto results = bptree.rangeQuery(30000, 40000); // Implement rangeQuery() to return a vector of shared_ptr<Record> within the given range

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    float averageRating = 0;
    for (const auto &record : results)
    {
        averageRating += record->averageRating;
    }
    if (!results.empty())
    {
        averageRating /= results.size();
    }

    std::cout << "Number of Records Found: " << results.size() << std::endl;
    std::cout << "Average Rating: " << averageRating << std::endl;
    std::cout << "Elapsed Time: " << elapsed.count() << " ms" << std::endl;
}

// Experiment 5: Deletion of records with numVotes = 1,000
void experiment5(BPTree &bptree)
{
    printExperimentHeader("Experiment 5: Deletion of records with numVotes = 1,000");

    auto start = std::chrono::high_resolution_clock::now();

    bptree.deleteKey(1000); // Assuming deleteKey() is implemented to delete records with the specified key

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    int totalNodes = bptree.getTotalNodes();
    int treeLevels = bptree.getTreeLevels();
    auto rootContent = bptree.getRootContent();

    std::cout << "Total Nodes After Deletion: " << totalNodes << std::endl;
    std::cout << "Tree Levels After Deletion: " << treeLevels << std::endl;
    std::cout << "Root Content After Deletion: ";
    for (const auto &key : rootContent)
    {
        std::cout << key << " ";
    }
    std::cout << std::endl;
    std::cout << "Elapsed Time: " << elapsed.count() << " ms" << std::endl;
}

int main() {
    BPTree bptree(BLOCK_SIZE); // Initialize B+ tree with block size
    std::string filepath = "Data/data.tsv"; // Relative path to the data file

    int choice = 0;
    do {
        std::cout << "\nSelect an experiment to run (1-5) or 0 to exit:\n";
        std::cout << "1. Experiment 1: Storage Statistics\n";
        std::cout << "2. Experiment 2: B+ Tree Statistics\n";
        std::cout << "3. Experiment 3: Query for numVotes = 500\n";
        std::cout << "4. Experiment 4: Range Query for numVotes between 30,000 and 40,000\n";
        std::cout << "5. Experiment 5: Deletion of records with numVotes = 1,000\n";
        std::cout << "0. Exit\n";
        std::cout << "> ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                experiment1(bptree); // No need to load data into B+ tree here
                break;
            case 2:
                loadData(bptree, filepath); // Load data into B+ tree for building it
                experiment2(bptree);
                break;
            case 3:
                experiment3(bptree);
                break;
            case 4:
                experiment4(bptree);
                break;
            case 5:
                experiment5(bptree);
                break;
            default:
                break;
            }
        } while (choice != 0);

        return 0;
}
