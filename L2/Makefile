CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
CC_FLAGS  := --std=c++11 -I./src -I../lib/PEGTL/include -I../lib -g3
LD_FLAGS  := 
CC        := g++

all: obj bin L2

obj:
	mkdir -p $@

bin:
	mkdir -p $@

L2: $(OBJ_FILES)
	$(CC) $(LD_FLAGS) -o ./bin/$@ $^

obj/%.o: src/%.cpp
	$(CC) $(CC_FLAGS) -c -o $@ $<

oracle: L2
	../scripts/generateOutput.sh $^ L2c

rm_tests_without_oracle:
	../scripts/rm_tests_without_oracle.sh L2

oracle_liveness: L2
	./scripts/generateOutputLiveness.sh

oracle_spill: L2
	./scripts/generateOutputSpill.sh

oracle_interference: L2
	./scripts/generateOutputInterference.sh

test: L2
	../scripts/test.sh L2 L2c

test_liveness: L2
	./scripts/testLiveness.sh

test_spill: L2
	./scripts/testSpill.sh

test_interference: L2
	./scripts/testInterference.sh

clean:
	rm -fr bin obj *.out *.L1 *.o *.S core.* tests/*/*.tmp tests/*.tmp
