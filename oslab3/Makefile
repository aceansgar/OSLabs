CC = g++

SDIR = src # source directory
ODIR = ./objects

CFLAGS  = -I$(SDIR)

_DEPS = myenv.h utils.h manager.h
DEPS = $(patsubst %, src/%,$(_DEPS))

_OBJ = utils.o manager.o mmu.o
OBJ = $(patsubst %, objects/%,$(_OBJ))

./objects/%.o : ./src/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -o $@ -c $< 

mmu: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) 

# mmu: $(OBJ)
# 	$(CC) -o $@ $^ $(CFLAGS) 

.PHONY: clean

clean:
	rm -f linker *~ *.o