// Record.h
#ifndef RECORD_H
#define RECORD_H

#include <string>
#include <cstring>

struct Record {
    char tconst[10];     // IMDb identifier, ensure it's null-terminated
    float averageRating; // Movie rating
    int numVotes;        // Number of votes

    Record(const std::string& id, float rating, int votes) {
        std::strncpy(tconst, id.c_str(), sizeof(tconst) - 1);
        tconst[sizeof(tconst) - 1] = '\0'; 
        averageRating = rating;
        numVotes = votes;
    }
};

#endif // RECORD_H
