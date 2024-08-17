#ifndef HR_HPP
#define HR_HPP

#include <iostream>
#include <math.h>
#include <thread>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/atomic.hpp>

#include "star.hpp"

// Shared buffer size
#define BUF_SIZE 4096

// Min command line args
#define MIN_ARGS 4

// Fields to read
#define READ_FIELD_NUMBER 2

// Classification params
#define MAGNITUDE_LOW -5.0
#define MAGNITUDE_HIGH 5.0
#define MAGNITUDE_VERY_HIGH 20.0

#define COLOR_INDEX_LOW -0.3
#define COLOR_INDEX_MEDIUM 0.5
#define COLOR_INDEX_HIGH 1.0
#define COLOR_INDEX_VERY_HIGH 2.0

extern int absmagIndex;
extern int ciIndex;

// Spinlocks
extern boost::atomic_flag sharedBufferSpinlock;
extern boost::atomic_flag consumeFlagSpinlock;
extern boost::atomic_flag globalDataSpinlock;

// Shared buffer
extern Star sharedBuffer[BUF_SIZE];

// Shared buffer pointers
extern int bufferHead;
extern int bufferTail;

// Flag to notify consumers
extern bool stopConsume;

// Global parameters
extern int mainSequenceNGlobal;
extern int giantsNGlobal;
extern int superGiantsNGlobal;
extern int whiteDwarfsNGlobal;

// Parsers threads
void parse(const char* chunkStart, const char* chunckEnd);
void writeBuffer(double currentAbsoluteMagnitude, double currentColorIndex);
void fixChunkPointer(const char** chunkStart, const char** chunckEnd);
void readField(const char** chunkStart, std::string* tempField, double* currentValueToUpdate, int* writeInBuffer);

// Consumers threads
void consume();
bool checkConsumeFlag();
void updateConsumerData(double currentColorIndex, double currentAbsoluteMagnitude, int* mainSequenceN, int* giantsN, int* superGiantsN, int* whiteDwarfsN);
void updateGlobalData(int mainSequenceN, int giantsN, int superGiantsN, int whiteDwarfsN);

// Print final statistics
void printStats();

// Check and Get command line params
void argsCheckGet(int argc, char* argv[], int minArgs, std::string* fileName, int* absmagIndex, int* ciIndex);

// Stop consumers from reading
void stopConsumers();

#endif