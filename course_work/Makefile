all: bin bin/server bin/client

bin/server: src/server.c
	gcc -Wall -Werror $< -o $@ -lpthread -g

bin/client: src/client.c
	gcc -Wall -Werror $< -o $@ -lpthread -g

bin:
	mkdir bin

clean:
	rm -rf bin

.PHONY: all clean 