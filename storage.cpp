#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <string>
#include <sstream>
#include <algorithm>

const int BLOCK_SIZE = 200;
const size_t DISK_CAPACITY = 100 * 1024 * 1024;

int convertToInt(const std::string &str, const std::string &fullLine)
{
    try
    {
        return std::stoi(str);
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Invalid argument: '" << str << "' in line: " << fullLine << " could not be converted to int." << std::endl;
        return 0;
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << "Out of range: '" << str << "' in line: " << fullLine << " is out of range for an int." << std::endl;
        return 0;
    }
}

float convertToFloat(const std::string &str, const std::string &fullLine)
{
    try
    {
        return std::stof(str);
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Invalid argument: '" << str << "' in line: " << fullLine << " could not be converted to float." << std::endl;
        return 0.0f;
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << "Out of range: '" << str << "' in line: " << fullLine << " is out of range for a float." << std::endl;
        return 0.0f;
    }
}

struct Record
{
    char tconst[10];     // 9 chars for the ID + 1 for null terminator
    float averageRating; // 4 bytes for the floating point
    int32_t numVotes;    // 4 bytes for the integer

    Record(const std::string &id, float rating, int32_t votes)
    {
        strncpy(tconst, id.c_str(), sizeof(tconst) - 1);
        tconst[sizeof(tconst) - 1] = '\0'; // Ensure null-termination
        averageRating = rating;            // Assume rating is already converted to float
        numVotes = votes;                  // Assume votes is already an int
    }
};

class Block
{
public:
    std::vector<Record> records;

    void writeToDisk(std::ofstream &out) const
    {
        for (const Record &record : records)
        {
            out.write(reinterpret_cast<const char *>(&record), sizeof(Record));
        }
    }

    bool canAddRecord() const
    {
        return records.size() < (BLOCK_SIZE / sizeof(Record));
    }

    void addRecord(const Record &record)
    {
        if (canAddRecord())
        {
            records.push_back(record);
        }
    }

    size_t size() const
    {
        return records.size();
    }
};

class SimulatedDisk
{
private:
    std::vector<Block> blocks;
    size_t capacity;

public:
    SimulatedDisk(size_t diskCapacity = DISK_CAPACITY) : capacity(diskCapacity) {}

    bool canAddBlock() const
    {
        // Each block uses BLOCK_SIZE bytes, calculate if a new block can fit
        return (blocks.size() + 1) * BLOCK_SIZE <= capacity;
    }

    void addBlock(const Block &block)
    {
        if (canAddBlock())
        {
            blocks.push_back(block);
        }
        else
        {
            std::cerr << "Disk capacity exceeded, cannot add more blocks." << std::endl;
        }
    }

    void writeToDisk(const std::string &filename)
    {
        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile.is_open())
        {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return;
        }
        for (const Block &block : blocks)
        {
            block.writeToDisk(outFile);
        }
        outFile.close();
    }

    size_t totalBlocks() const
    {
        return blocks.size();
    }

    size_t totalRecords() const
    {
        size_t total = 0;
        for (const Block &block : blocks)
        {
            total += block.size();
        }
        return total;
    }

    size_t usedCapacity() const
    {
        return blocks.size() * BLOCK_SIZE;
    }
};

void readTSVAndCreateBlocks(const std::string &filename, SimulatedDisk &disk)
{

    std::ifstream tsvFile(filename);
    std::string line;
    Block currentBlock;
    size_t lineNumber = 0;      //Line counter for debugging purpose

    if (!std::getline(tsvFile, line))
    {
        std::cerr << "File is empty or cannot be read." << std::endl;
        return;
    }

    while (std::getline(tsvFile, line))
    {
        lineNumber++;
        if (lineNumber % 10000 == 0) { //Print a message every 10000 lines
            std::cout << "Processing line: " << lineNumber << std::endl;
        }

        std::istringstream iss(line);
        std::string tconst, rating, votes;

        std::getline(iss, tconst, '\t');
        std::getline(iss, rating, '\t');
        std::getline(iss, votes, '\t');

        float avgRating = convertToFloat(rating, line);
        int numVotes = convertToInt(votes, line);

        Record record(tconst, avgRating, numVotes);

        if (!currentBlock.canAddRecord())
        {
            std::cout << "Adding block to disk after " << currentBlock.size() << "records. Total processed lines: " << lineNumber << std::endl;
            disk.addBlock(currentBlock); // Add the current block to the disk
            currentBlock = Block();      // Start a new block
        }
        currentBlock.addRecord(record);
    }

    if (currentBlock.size() > 0)
    {
        std::cout << "Adding final block to disk. Total lines processed: " << lineNumber << std::endl;
        disk.addBlock(currentBlock); // Don't forget to add the last block
    }

    std::cout << "Finished processing. Total lines read: " << lineNumber << std::endl;

    tsvFile.close();
}

int main()
{
    SimulatedDisk disk(DISK_CAPACITY); // Initialize the simulated disk
    readTSVAndCreateBlocks("Data/data.tsv", disk);
    disk.writeToDisk("Data.dat");

    std::cout << "Total number of records: " << disk.totalRecords() << std::endl;
    std::cout << "Size of a record: " << sizeof(Record) << " bytes" << std::endl;
    std::cout << "Number of records stored in a block: " << BLOCK_SIZE / sizeof(Record) << std::endl;
    std::cout << "Number of blocks for storing the data: " << disk.totalBlocks() << std::endl;
    std::cout << "Used disk capacity: " << disk.usedCapacity() << " bytes" << std::endl;
    std::cout << "Total disk capacity: " << DISK_CAPACITY << " bytes" << std::endl;
    std::cout << "Free disk capacity: " << (DISK_CAPACITY - disk.usedCapacity()) << " bytes" << std::endl;

    return 0;
}
