CFLAGS := -Wall -Wno-format -O2
LDFLAGS := -pthread -lusb -lreadline

BIN := umulud

HEADERS := debug.h driver.h usblcd.h

all: $(BIN)
	strip -s $(BIN)

$(BIN): $(HEADERS)

clean:
	rm -f $(BIN)

test: all
	./$(BIN)
