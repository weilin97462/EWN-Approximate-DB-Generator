BASIC_TABLES = src/tables/basic_tables/basic_tables.cpp

MANHATTAN_DISTANCE = src/manhattan_distance/manhattan_distance.cpp

ENCODE_COMMON = src/encode/encode.cpp src/encode/combination_encoding/combination_encoding.cpp src/encode/permutation_encoding/perm_encode.cpp
ENCODE_WITHOUT_CUT = src/encode/without_cut/encode.cpp src/tables/encode_tables/without_cut/diag_symm_tables.cpp
ENCODE_WITH_CUT = src/encode/with_cut/encode.cpp src/tables/encode_tables/with_cut/diag_symm_tables.cpp 
ENCODE_WITH_SORTED_MD = src/encode/with_sorted_md/encode.cpp src/tables/encode_tables/with_sorted_md/md_encode_table.cpp
ENCODING_LIBS = $(ENCODE_COMMON) $(ENCODE_WITHOUT_CUT) $(ENCODE_WITH_CUT) $(ENCODE_WITH_SORTED_MD)

DECODE_COMMON = src/decode/decode.cpp src/decode/combination_decoding/combination_decoding.cpp src/decode/permutation_decoding/perm_decode.cpp
DECODE_WITHOUT_CUT = src/decode/without_cut/decode.cpp src/tables/decode_tables/without_cut/diag_symm_tables.cpp
DECODE_WITH_CUT = src/decode/with_cut/decode.cpp src/tables/decode_tables/with_cut/diag_symm_tables.cpp
DECODE_WITH_SORTED_MD = src/decode/with_sorted_md/decode.cpp src/tables/decode_tables/with_sorted_md/md_decode_table.cpp
DECODING_LIBS = $(DECODE_COMMON) $(DECODE_WITHOUT_CUT) $(DECODE_WITH_CUT) $(DECODE_WITH_SORTED_MD)

EWN_BOARD = src/EWN_board/EWN_board.cpp

FRAC_NUM = src/frac_num/frac_num.cpp

COUNT_SIZE = src/buildDB/dependency_manager/count_size.cpp
ENTRY = src/buildDB/dependency_manager/dependency_data.cpp
DEPENDENCY_MANAGER = src/buildDB/dependency_manager/dependency_manager.cpp src/buildDB/dependency_manager/dependency_manager_MD.cpp
DEPENDENCY_TABLES = src/buildDB/dependency_manager/dependency_tables/dependency_tables.cpp
NO_MD = src/buildDB/no_MD/buildDB.cpp
WITH_MD = src/buildDB/with_MD/buildDB.cpp src/buildDB/with_MD/MD_bound/MD_bound.cpp
DB_TABLES = src/tables/DB_tables/md_size_table.cpp
CHECK_POINT = src/buildDB/checkpoint/checkpoint.cpp
TRIAL_CALCULATION = src/buildDB/trial_calculation/trial_calculation.cpp
BUILD_DB = src/buildDB/buildDB.cpp $(COUNT_SIZE) $(ENTRY) $(DEPENDENCY_MANAGER) $(DEPENDENCY_TABLES) $(NO_MD) $(WITH_MD) $(DB_TABLES) $(CHECK_POINT) $(TRIAL_CALCULATION)

PRECOMPUTED_TABLES = src/tables/DB_tables/md_size_table.cpp src/tables/encode_tables/with_cut/diag_symm_tables.cpp src/tables/encode_tables/without_cut/diag_symm_tables.cpp src/tables/encode_tables/with_sorted_md/md_encode_table.cpp src/tables/decode_tables/with_cut/diag_symm_tables.cpp src/tables/decode_tables/without_cut/diag_symm_tables.cpp src/tables/decode_tables/with_sorted_md/md_decode_table.cpp
