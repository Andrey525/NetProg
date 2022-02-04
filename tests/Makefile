all: bin bin/server bin/client

bin/server: src/server.c
	gcc -Wall -Werror $< -o $@

bin/client: src/client.c
	gcc -Wall -Werror $< -o $@

bin:
	mkdir bin

clean:
	rm -rf bin

.PHONY: all clean