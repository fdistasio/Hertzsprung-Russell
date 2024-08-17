## Hertzsprung-Russell Analyzer

The project allows you to analyze a database in ".csv" format of stars and classify them according to parameters in their evolutionary stage, in accordance with the Hertzsprung-Russell diagram.

- To increase performance, a multithreaded scheme was adopted where parsing threads parse the file (memory mapped) and consumer threads process the data. These communicate via a shared buffer according to the producer-consumer scheme.

- After splitting the file into chunks of the same size, each parser thread operates on one of them. Each valid entry is written to the shared buffer and when the chunk is finished the parser terminates.

- The consumer threads read data from the shared buffer and as soon as all parsers are finished, they update the global data with the locally processed data.

- The synchronization mechanisms are managed via spinlock, given that the critical section is very short, to avoid excessive overheads caused by mutexes or condition variables.
  
- File memory mapping is handled via the boost library.

- At the end, the statistics containing the total number of valid stars and the number of stars for each evolutionary phase with the attached percentage are printed.

- Database from [astronexus](https://github.com/astronexus/HYG-Database) : [HYG-Database
Public](https://github.com/astronexus/HYG-Database), [ATHYG-Database
Public](https://github.com/astronexus/ATHYG-Database).
