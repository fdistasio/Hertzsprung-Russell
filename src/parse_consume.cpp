#include "../include/parse_consume.hpp"

int absmagIndex;                                // Index of csv file absmag
int ciIndex;                                    // Index of csv file ci

boost::atomic_flag sharedBufferSpinlock;        // Spinlock for shared buffer
boost::atomic_flag consumeFlagSpinlock;         // Spinlock for consumeFlag
boost::atomic_flag globalDataSpinlock;          // Spinlock for global data

Star sharedBuffer[BUF_SIZE];                    // Shared buffer


int bufferHead = 0;                             // Shared buffer head pointer
int bufferTail = 0;                             // Shared buffer tail pointer

bool stopConsume = false;                       // Flag to notify consumers

int mainSequenceNGlobal = 0;                    // Total number of stars in main sequence
int giantsNGlobal = 0;                          // Total number of giants
int superGiantsNGlobal = 0;                     // Total number of supergiants
int whiteDwarfsNGlobal = 0;                     // Total number of white dwarfs

// Parser thread: parse a chunk of the csv file and send data on shared buffer
void parse(const char* chunkStart, const char* chunckEnd) {

    // Number of stars of the chunk
    int chunkStarNumber = 0;

    // Current entry data
    double currentAbsoluteMagnitude = -1;
    double currentColorIndex = -1;
    int writeInBuffer = 0;

    // Counter of ','
    int i = 0;

    // Fix chunk pointer
    fixChunkPointer(&chunkStart, &chunckEnd);

    // Read each chunk entry
    while (chunkStart < chunckEnd) {
       
        std::string tempField = "";

        // Read only absmag, ci
        if (i == absmagIndex)
           readField(&chunkStart, &tempField, &currentAbsoluteMagnitude, &writeInBuffer);

        else if (i == ciIndex)
            readField(&chunkStart, &tempField, &currentColorIndex, &writeInBuffer);
        
        // Write current star in shared buffer
        if(writeInBuffer == READ_FIELD_NUMBER) {
            
            writeBuffer(currentAbsoluteMagnitude, currentColorIndex);
            
            ++chunkStarNumber;
            writeInBuffer = 0;
        
        }

        // Increase counter if the current elem is ','
        if(*chunkStart == ',') 
            ++i;
        
        // Increase number of stars and reset counter if the current elem is '\n'
        if(*chunkStart == '\n')
            i = 0;

        // Current elem ++
        ++chunkStart;

    }

}

// Write star object on the shared buffer
void writeBuffer(double currentAbsoluteMagnitude, double currentColorIndex) {

    while(true) {

        // Spinlock test & set
        while(sharedBufferSpinlock.test_and_set(boost::memory_order_acquire));

        // Wait if buffer full
        if((bufferHead + 1) % BUF_SIZE == bufferTail) {
            
            // Spinlock release
            sharedBufferSpinlock.clear(boost::memory_order_release);
            continue;

        }

        // New Star
        Star currentStar(currentAbsoluteMagnitude, currentColorIndex);
        sharedBuffer[bufferHead] = currentStar;

        // Update head
        bufferHead = (bufferHead + 1) % BUF_SIZE;

        break;

    }

    // Spinlock release
    sharedBufferSpinlock.clear(boost::memory_order_release);

}

// Fix chunk pointer to the next full line
void fixChunkPointer(const char** chunkStart, const char** chunckEnd) {

    while (*chunkStart < *chunckEnd && *(*chunkStart) != '\n')
        ++(*chunkStart);

    if (chunkStart < chunckEnd)
        ++(*chunkStart);

}

// Read current field into "tempField"
void readField(const char** chunkStart, std::string* tempField, double* currentValueToUpdate, int* writeInBuffer) {

    // Read field
    while(*(*chunkStart) != ',') {

        *tempField += *(*chunkStart);                
        ++(*chunkStart);

    }

    // If value is valid
    if((*tempField).length()) {

        *currentValueToUpdate = std::stod(*tempField);
        *tempField = "";
        ++(*writeInBuffer);
       
    }

}

// Thread: get data from the shared buffer and classify them
void consume() {

    // Number of stars for each phase
    int mainSequenceN = 0;
    int giantsN = 0;
    int superGiantsN = 0;
    int whiteDwarfsN = 0;

    // Current params
    double currentAbsoluteMagnitude;
    double currentColorIndex;

    while(true) {

        currentAbsoluteMagnitude = -1;
        currentColorIndex = -1;

        // Spinlock test & set
        while(sharedBufferSpinlock.test_and_set(boost::memory_order_acquire));

        // Check if parsers ended
        if(checkConsumeFlag()) {

            // Spinlock release
            sharedBufferSpinlock.clear(boost::memory_order_release);
            break;
        }

        // Wait if buffer empty
        if((bufferHead == bufferTail)) {
            
            // Spinlock release
            sharedBufferSpinlock.clear(boost::memory_order_release);
            continue;

        }

        // Get current params
        currentAbsoluteMagnitude = sharedBuffer[bufferTail].getAbs();
        currentColorIndex = sharedBuffer[bufferTail].getCi();

        // Update head
        bufferTail = (bufferTail + 1) % BUF_SIZE;

        // Spinlock release
        sharedBufferSpinlock.clear(boost::memory_order_release);

        // Update data
        updateConsumerData(currentColorIndex, currentAbsoluteMagnitude, &mainSequenceN, &giantsN, &superGiantsN, &whiteDwarfsN);

    }

    updateGlobalData(mainSequenceN, giantsN, superGiantsN, whiteDwarfsN);

}

// Check global flag "stopConsume"
bool checkConsumeFlag() {

    bool returnValue = false; 

    // Spinlock test & set
    while(consumeFlagSpinlock.test_and_set(boost::memory_order_acquire));

    if(stopConsume) returnValue = true;

    // Spinlock release
    consumeFlagSpinlock.clear(boost::memory_order_release);

    return returnValue;

}

// Classify a star
void updateConsumerData(double currentColorIndex, double currentAbsoluteMagnitude, int* mainSequenceN, int* giantsN, int* superGiantsN, int* whiteDwarfsN) {

    if (currentAbsoluteMagnitude > MAGNITUDE_LOW && currentAbsoluteMagnitude < MAGNITUDE_HIGH && currentColorIndex >= COLOR_INDEX_LOW && currentColorIndex <= COLOR_INDEX_VERY_HIGH) {
        
        if (currentAbsoluteMagnitude > 0 && currentAbsoluteMagnitude <= MAGNITUDE_HIGH) 
            (*mainSequenceN)++;

        else if (currentAbsoluteMagnitude <= 0  && currentColorIndex < COLOR_INDEX_HIGH) 
            (*mainSequenceN)++;

        else if (currentAbsoluteMagnitude >= MAGNITUDE_LOW && currentColorIndex > COLOR_INDEX_HIGH)
            (*giantsN)++;

    } 
    
    else if (currentAbsoluteMagnitude < MAGNITUDE_LOW)
        (*superGiantsN)++;

    else if (currentAbsoluteMagnitude > MAGNITUDE_HIGH && currentAbsoluteMagnitude <= MAGNITUDE_VERY_HIGH && currentColorIndex >= COLOR_INDEX_LOW && currentColorIndex < COLOR_INDEX_HIGH)
        (*whiteDwarfsN)++;
    
}

// Update global data with each chunk data
void updateGlobalData(int mainSequenceN, int giantsN, int superGiantsN, int whiteDwarfsN) {

    // Spinlock test & set
    while(globalDataSpinlock.test_and_set(boost::memory_order_acquire));

    // Update global data
    mainSequenceNGlobal += mainSequenceN;
    giantsNGlobal += giantsN;
    superGiantsNGlobal += superGiantsN;
    whiteDwarfsNGlobal += whiteDwarfsN;

    // Spinlock release
    globalDataSpinlock.clear(boost::memory_order_release);

}

// Print final statistics
void printStats() {

    // Total valid stars
    int starNumber = mainSequenceNGlobal + giantsNGlobal + superGiantsNGlobal + whiteDwarfsNGlobal;

    // If there are no stars
    if(!starNumber) {

        std::cout << "\nEmpty database\n\n";
        return;

    }

    // Percentages
    double msPercent = ((double)mainSequenceNGlobal / (double)starNumber) * 100;
    double gPercent = ((double)giantsNGlobal / (double)starNumber) * 100;
    double sgPercent = ((double)superGiantsNGlobal / (double)starNumber) * 100;
    double wdPercent = ((double)whiteDwarfsNGlobal / (double)starNumber) * 100;

    // Print
    std::cout << "\nResults:\n\n";
    std::cout << "Total valid Stars: " << starNumber << std::endl;
    std::cout << "Main Sequence Stars: " << mainSequenceNGlobal << ", " << msPercent << "%" << std::endl;
    std::cout << "Giants Stars: " << giantsNGlobal << ", " << gPercent << "%" << std::endl;
    std::cout << "Supergiants Stars: " << superGiantsNGlobal << ", " << sgPercent << "%" << std::endl;
    std::cout << "White Dwarfs Stars: " << whiteDwarfsNGlobal << ", " << wdPercent << "%" << std::endl;

}

// Check command line params
void argsCheckGet(int argc, char* argv[], int minArgs, std::string* fileName, int* absmagIndex, int* ciIndex) {

    // Check
    if(argc < minArgs) {

        std::cerr << "Invalid args (usage: ./hr path/filename absmag_index ci_index)\n";
        exit(EXIT_FAILURE);

    }

    // Get
    *fileName = argv[1];
    *absmagIndex = std::stoi(argv[2]);
    *ciIndex = std::stoi(argv[3]);

}

// Stop consumers from reading
void stopConsumers() {

    // Spinlock test & set
    while(consumeFlagSpinlock.test_and_set(boost::memory_order_acquire));

    stopConsume = true;

    // Spinlock release
    consumeFlagSpinlock.clear(boost::memory_order_release);

}