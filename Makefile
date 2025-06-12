include sources.mk
.PHONY: all interface
FLAGS = -std=c++17 -O2 -march=native -lpthread  -Wall
IGNORED_WARNINGS = -Wno-char-subscripts -Wno-unused-result

all: $(PRECOMPUTED_TABLES)
	g++ src/main.cpp $(MANHATTAN_DISTANCE) $(BASIC_TABLES) $(ENCODING_LIBS) $(DECODING_LIBS) $(FRAC_NUM) $(EWN_BOARD) $(BUILD_DB) $(FLAGS) -o DB_builder $(IGNORED_WARNINGS)
$(PRECOMPUTED_TABLES):
	g++ gen_precompute_table/main.cpp gen_precompute_table/*/*.cpp -O2 -o gen_table
	./gen_table
	rm gen_table
clean:
	rm $(PRECOMPUTED_TABLES) DB_builder -f
