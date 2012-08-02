CFLAGS := -Wall -Wno-format -O2 $(shell pkg-config --cflags libusb-1.0)
LDFLAGS := -pthread -lreadline $(shell pkg-config --libs libusb-1.0)

BIN := umulud

HEADERS := $(wildcard *.h)

all: $(BIN)
	strip -s $(BIN)

$(BIN): $(HEADERS)
	$(CC) $(CFLAGS) $(BIN).c -o $(BIN) $(LDFLAGS)

clean:
	rm -f $(BIN)

test: all
	./$(BIN)
