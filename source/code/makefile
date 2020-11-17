CFLAGS = -std=c++11 -stdlib=libc++
CC = clang++

SRC = q_2_modified_main.cpp
OBJ = $(SRC:.cpp = .o)

simulation: $(OBJ)
	$(CC) $(CFLAGS) -o simulation $(OBJ)

run:
	./simulation 2 0.01 256 16 4 0.5 128

clean:
	rm -f core *.o