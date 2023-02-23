TARGET = mvote
CFLAGS = -lm

# separate compilation of mvote
$(TARGET): mvote.o
	gcc -o $(TARGET) mvote.o $(CFLAGS)

# compile mvote.c
mvote.o: mvote.c
	gcc -c mvote.c $(CFLAGS)

# clean up
clean:
	rm -f $(TARGET).o $(TARGET)