CC = gcc
TARGET = adzip
OBJ_FILES = adzip.o

$(TARGET): $(OBJ_FILES)
	$(CC) -o $(TARGET) $(OBJ_FILES)

adzip.o: adzip.c main.h metadata.h
	$(CC) -c adzip.c

clean:
	rm -f $(TARGET) $(OBJ_FILES)
