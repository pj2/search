SRC_F=src/
INC_F=include/
OBJ_F=obj/

CC=gcc
PROJECT=search
LFLAGS=-lm `pkg-config --libs glib-2.0 freeglut gl glu`
CFLAGS=-Wall -g -I$(INC_F) `pkg-config --cflags glib-2.0 freeglut gl glu`
OBJS=$(patsubst $(SRC_F)%.c,$(OBJ_F)%.o,$(wildcard $(SRC_F)*.c))

all: $(OBJS)
	$(CC) -o $(PROJECT) $^ $(LFLAGS) $(CFLAGS)

$(OBJ_F)%.o: $(SRC_F)%.c $(INC_F)%.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ_F)*.o
	rm -f $(PROJECT)
