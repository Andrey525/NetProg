all: bin part1 part2

part1: bin bin/part1/server bin/part1/client

part2: bin bin/part2/server bin/part2/client

bin/part1/server: src/part1/server.c bin/part1
	gcc -Wall -Werror $< -o $@

bin/part1/client: src/part1/client.c bin/part1
	gcc -Wall -Werror $< -o $@

bin/part2/server: src/part2/server.c bin/part2
	gcc -Wall -Werror $< -o $@

bin/part2/client: src/part2/client.c bin/part2
	gcc -Wall -Werror $< -o $@


bin:
	mkdir bin

bin/part1:
	mkdir bin/part1

bin/part2:
	mkdir bin/part2

clean:
	rm -rf bin

.PHONY: all clean part1 part2