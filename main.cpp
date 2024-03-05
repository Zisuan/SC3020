#include "BPlusTree.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include "Storage.h"
#include "Record.h"

int main() {
    int choice = 0;
    std::string filename = "Data/data.tsv"; // Specify the path to your TSV file
    SimulatedDisk disk(DISK_CAPACITY); // Initialize the simulated disk
    BPlusTree bptree; //initialise bptree

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
            case 1: {
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
                break;
            }
            case 2:{
                // Assuming `disk` is your SimulatedDisk instance with data already loaded
                disk.loadBPlusTree(bptree); // Make sure this method properly populates your BPlusTree
                bptree.experiment2(); // This will print the stats for Experiment 2
                break;
            }
            case 3:
                // Placeholder for Experiment 3
                break;
            case 4:
                // Placeholder for Experiment 4
                break;
            case 5:
                bptree.experiment5(1000);
                break;
            default:
                break;
        }
    } while (choice != 0);

    return 0;
}
