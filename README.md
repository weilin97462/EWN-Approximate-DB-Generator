# EWN-Approximate-DB-Generator
This code generates an approximate database for "Einstein Würfelt Nicht!".

You can get an executable named `DB_builder` after `make`. You can then execute `DB_builder` to generate the database.

Make sure you modified all the settings in `src/config.hpp` before you compile, these settings include:
* Number of threads
* Memory space this program can use
* Size per instance
* Default read/write permissions for the database

If your os is not Linux, please uncomment `// #define USE_PORTABLE_FALLOCATE` in `src/config.hpp`.

This program is designed and optimized for Linux. Compatibility with other OS is not guaranteed.
