#ifndef STORAGE_H
#define STORAGE_H
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include "Record.h"

// Constants
extern const int BLOCK_SIZE;
extern const size_t DISK_CAPACITY;

// Convert string to int with error handling
int convertToInt(const std::string &str, const std::string &fullLine);

// Convert string to float with error handling
float convertToFloat(const std::string &str, const std::string &fullLine);

// Print a header
void printHeader(const std::string &title);

// Print a key-value pair
void printKeyValue(const std::string &key, const std::string &value);

// Block class
class Block
{
public:
    std::vector<Record> records;
    void writeToDisk(std::ofstream &out) const;
    bool canAddRecord() const;
    void addRecord(const Record &record);
    size_t size() const;
};

// SimulatedDisk class
class SimulatedDisk
{
private:
    std::vector<Block> blocks;
    size_t capacity;

public:
    SimulatedDisk(size_t diskCapacity = DISK_CAPACITY);
    bool canAddBlock() const;
    void addBlock(const Block &block);
    void writeToDisk(const std::string &filename);
    size_t totalBlocks() const;
    size_t totalRecords() const;
    size_t usedCapacity() const;
};

// Function to read TSV and create blocks
void readTSVAndCreateBlocks(const std::string &filename, SimulatedDisk &disk);

#endif // STORAGE_H
