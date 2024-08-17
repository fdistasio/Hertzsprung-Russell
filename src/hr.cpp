#include "../include/parse_consume.hpp"
#include "../include/star.hpp"


int main(int argc, char* argv[]) {

    std::string fileName;               // Csv file name
    
    std::size_t fileSize;               // Csv file size
    std::size_t chunkSize;              // Chunk size for each parser thread

    const char* fileData;               // Pointer to csv memory mapped data
    const char* chunkStart;             // Pointer to chunk start
    const char* chunkEnd;               // Pointer to chunk end

    int numParsers;                     // Number of parsers threads
    int numConsumers;                   // Number of consumers threads
    unsigned int numConcurrentThreads;  // Number of concurrent threads in the machine

    // Threads vectors
    std::vector<std::thread> consumersThreads;  
    std::vector<std::thread> parserThreads;

    std::cout << "Starting...\n";

    // Arguments check and get
    argsCheckGet(argc, argv, MIN_ARGS, &fileName, &absmagIndex, &ciIndex);

    // Get number of concurrent threads in the machine
    numConcurrentThreads = std::thread::hardware_concurrency();

    // Define threads number
    numConsumers = numConcurrentThreads / 2;
    numParsers = numConcurrentThreads / 2;

    // Memory mapping
    boost::iostreams::mapped_file_source mmFileCsv(fileName);

    // Get memory mapped params
    fileData = mmFileCsv.data();
    fileSize = mmFileCsv.size();

    // Define chunk size for each thread
    chunkSize = fileSize / numParsers;

    // Consumers Threads Creation
    for (int i = 0 ; i < numConsumers ; ++i)
        consumersThreads.emplace_back(consume);

    // Parsers Threads Creation
    for (int i = 0 ; i < numParsers; ++i) {

        // Define chunk pointers for each thread
        chunkStart = fileData + i * chunkSize;
        chunkEnd = (i == numParsers - 1) ? (fileData + fileSize) : (chunkStart + chunkSize);

        parserThreads.emplace_back(parse, chunkStart, chunkEnd);
    }

    std::cout << "Parsing file...\n";

    // Wait Parsers Threads
    for (auto& pars : parserThreads)
        pars.join();

    // Stop consumers
    stopConsumers();

    // Wait Consumers Threads
    for (auto& cons : consumersThreads)
        cons.join();

    // Print final statistics
    printStats();

}