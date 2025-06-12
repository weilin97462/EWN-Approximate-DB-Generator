# EWN-Approximate-DB-Generator
This code generates an approximate database for "Einstein Würfelt Nicht!".

You can get a executable named `DB_builder` after `make`.

Make sure you modified all the settings in the `src/config.hpp` before you compile, these settings include:
* number of threads
* memory space available for the program
* size per instance
* default file permission for the database

If your os is not Linux, please uncomment `// #define USE_PORTABLE_FALLOCATE` in `src/config.hpp`.
