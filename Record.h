// Record.h
#ifndef RECORD_H
#define RECORD_H

#include <string>
#include <cstring>

struct Record {
    char tconst[10];     // 9 chars for the ID + 1 for null terminator
    float averageRating; // 4 bytes for the floating point
    int numVotes;        // 4 bytes for the integer

    Record() = default;

    Record(const std::string& id, float rating, int votes) {
        std::strncpy(tconst, id.c_str(), sizeof(tconst) - 1);
        tconst[sizeof(tconst) - 1] = '\0'; 
        averageRating = rating;
        numVotes = votes;
    }

    std::vector<unsigned char> serializeRecord(const Record& record) {
        std::vector<unsigned char> buffer;

        // Serialize tconst
        buffer.insert(buffer.end(), record.tconst, record.tconst + sizeof(record.tconst));

        // Serialize averageRating
        auto ratingPtr = reinterpret_cast<const unsigned char*>(&record.averageRating);
        buffer.insert(buffer.end(), ratingPtr, ratingPtr + sizeof(record.averageRating));

        // Serialize numVotes
        auto votesPtr = reinterpret_cast<const unsigned char*>(&record.numVotes);
        buffer.insert(buffer.end(), votesPtr, votesPtr + sizeof(record.numVotes));

        return buffer;
    }

    static Record deserializeRecord(const std::vector<unsigned char>& buffer) {
        Record record;
        size_t offset = 0;

        // Deserialize tconst
        std::memcpy(record.tconst, buffer.data() + offset, sizeof(record.tconst));
        offset += sizeof(record.tconst);

        // Deserialize averageRating
        std::memcpy(&record.averageRating, buffer.data() + offset, sizeof(record.averageRating));
        offset += sizeof(record.averageRating);

        // Deserialize numVotes
        std::memcpy(&record.numVotes, buffer.data() + offset, sizeof(record.numVotes));

        return record;
    }


};

#endif // RECORD_H
