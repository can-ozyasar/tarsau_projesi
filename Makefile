CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -pedantic -O2
TARGET := tarsau
SRC := tarsau.c

.PHONY: all clean run-example

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

run-example: $(TARGET)
	./$(TARGET) -b t1.txt -o ornek.sau
	rm -rf ornek_cikti
	./$(TARGET) -a ornek.sau ornek_cikti

clean:
	rm -f $(TARGET) *.o *.sau *.exe
	rm -rf ornek_cikti
