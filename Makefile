#------------------------------------------------------------------------------

SOURCE=broadcastService.cpp
MYPROGRAM=broadcastService
OBJ=broadcast
CC=g++
LIB= pthread
#------------------------------------------------------------------------------
all: $(MYPROGRAM)

$(MYPROGRAM): $(SOURCE)

	$(CC) $(SOURCE) -o $(OBJ) -l $(LIB)

clean:

	rm -rf $(OBJ)


