TARGET = main
LDFLAGS = -pthread

all: $(TARGET) reader writer

$(TARGET): main.o
	gcc -o $(TARGET) main.o $(LDFLAGS)

reader: reader.o
	gcc -o reader reader.o $(LDFLAGS)

writer: writer.o
	gcc -o writer writer.o $(LDFLAGS)

main.o: main.c
	gcc -c main.c -o main.o

reader.o: reader.c
	gcc -c reader.c -o reader.o

writer.o: writer.c
	gcc -c writer.c -o writer.o

clean:
	rm -f $(TARGET) reader writer main.o reader.o writer.o logs.txt
	rm -f logs.txt
