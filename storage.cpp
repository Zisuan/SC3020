#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <string>
#include <sstream>

const int BLOCK_SIZE = 200;

struct Record
{
    char tconst[10];     // 9 chars for the ID + 1 for null terminator
    float averageRating; // 4 bytes for the floating point
    int32_t numVotes;    // 4 bytes for the integer

    Record(const std::string &id = "", float rating = 0.0f, int32_t votes = 0)
    {
        strncpy(tconst, id.c_str(), sizeof(tconst) - 1);
        tconst[sizeof(tconst) - 1] = '\0'; // Ensure null-termination
        averageRating = rating;
        numVotes = votes;
    }
};

class Block
{
public:
    std::vector<Record> records;

    void writeToDisk(std::ofstream &out)
    {
        for (auto &record : records)
        {
            out.write(reinterpret_cast<char *>(&record), sizeof(Record));
        }
    }

    bool canAddRecord() const
    {
        // Calculate the remaining space in the block
        int remainingSpace = BLOCK_SIZE - (records.size() * sizeof(Record));
        return sizeof(Record) <= remainingSpace;
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

// Function to read TSV data and create blocks
void readTSVAndCreateBlocks(const std::string &filename, std::vector<Block> &blocks)
{
    std::ifstream tsvFile(filename);
    std::string line;
    Block currentBlock;

    while (std::getline(tsvFile, line))
    {
        std::istringstream iss(line);
        std::string tconst, rating, votes;

        std::getline(iss, tconst, '\t');
        std::getline(iss, rating, '\t');
        std::getline(iss, votes, '\t');

        Record record(tconst, std::stof(rating), std::stoi(votes));

        if (!currentBlock.canAddRecord())
        {
            blocks.push_back(currentBlock);
            currentBlock = Block();
        }
        currentBlock.addRecord(record);
    }

    if (currentBlock.size() > 0)
    {
        blocks.push_back(currentBlock);
    }

    tsvFile.close();
}

int main()
{
    std::vector<Block> blocks;

    readTSVAndCreateBlocks("Data/data.tsv", blocks);

    std::ofstream outFile("Data.dat", std::ios::binary);
    for (auto &block : blocks)
    {
        block.writeToDisk(outFile);
    }
    outFile.close();

    size_t totalRecords = 0;
    for (const auto &block : blocks)
    {
        totalRecords += block.size();
    }
    std::cout << "Total number of records: " << totalRecords << std::endl;
    std::cout << "Size of a record: " << sizeof(Record) << " bytes" << std::endl;
    std::cout << "Number of records stored in a block: " << BLOCK_SIZE / sizeof(Record) << std::endl;
    std::cout << "Number of blocks for storing the data: " << blocks.size() << std::endl;

    return 0;
}
