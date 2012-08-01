CFLAGS := -Wall -Wno-format -O2
LDFLAGS := -pthread -lusb -lreadline

BIN := umulud

all: $(BIN)
	strip -s $(BIN)

clean:
	rm -f $(BIN)

test: all
	./$(BIN)