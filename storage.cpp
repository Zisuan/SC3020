#include "Storage.h"

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
        std::cerr << "Invalid argument: '" << str << "' in line: " << fullLine << std::endl;
        return 0;
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << "Out of range: '" << str << "' in line: " << fullLine << std::endl;
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
        std::cerr << "Invalid argument: '" << str << "' in line: " << fullLine << std::endl;
        return 0.0f;
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << "Out of range: '" << str << "' in line: " << fullLine << std::endl;
        return 0.0f;
    }
}

void printHeader(const std::string &title)
{
    std::cout << "\n"
              << title << "\n"
              << std::string(title.length(), '=') << "\n";
}

void printKeyValue(const std::string &key, const std::string &value)
{
    std::cout << std::left << std::setw(30) << key << ": " << value << "\n";
}

void Block::writeToDisk(std::ofstream &out) const
{
    for (const Record &record : records)
    {
        out.write(reinterpret_cast<const char *>(&record), sizeof(Record));
    }
}

bool Block::canAddRecord() const
{
    return records.size() < (BLOCK_SIZE / sizeof(Record));
}

void Block::addRecord(const Record &record)
{
    if (canAddRecord())
    {
        records.push_back(record);
    }
}

size_t Block::size() const
{
    return records.size();
}

SimulatedDisk::SimulatedDisk(size_t diskCapacity) : capacity(diskCapacity) {}

bool SimulatedDisk::canAddBlock() const
{
    return (blocks.size() + 1) * BLOCK_SIZE <= capacity;
}

void SimulatedDisk::addBlock(const Block &block)
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

void SimulatedDisk::writeToDisk(const std::string &filename)
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

size_t SimulatedDisk::totalBlocks() const
{
    return blocks.size();
}

size_t SimulatedDisk::totalRecords() const
{
    size_t total = 0;
    for (const Block &block : blocks)
    {
        total += block.size();
    }
    return total;
}

size_t SimulatedDisk::usedCapacity() const
{
    return blocks.size() * BLOCK_SIZE;
}

void readTSVAndCreateBlocks(const std::string &filename, SimulatedDisk &disk)
{
    std::ifstream tsvFile(filename);
    std::string line;
    Block currentBlock;
    size_t lineNumber = 0;

    if (!std::getline(tsvFile, line))
    {
        std::cerr << "File is empty or cannot be read." << std::endl;
        return;
    }

    while (std::getline(tsvFile, line))
    {
        lineNumber++;
        if (lineNumber % 10000 == 0)
        {
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
            disk.addBlock(currentBlock);
            currentBlock = Block();
        }
        currentBlock.addRecord(record);
    }

    if (currentBlock.size() > 0)
    {
        disk.addBlock(currentBlock);
    }

    std::cout << "Finished processing. Total lines read: " << lineNumber << std::endl;

    tsvFile.close();
}

void SimulatedDisk::loadBPlusTree(BPlusTree &tree)
{
    for (auto &block : blocks)
    {
        for (auto &record : block.records)
        {
            unsigned char *recordPtr = reinterpret_cast<unsigned char *>(&record);
            tree.insertKey(record.numVotes, recordPtr);
        }
    }
}
