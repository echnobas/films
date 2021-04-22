.PHONY     : debug
debug      : build_debug run
all        : build run
build      :
	@clang -o ./bin/main -D LOG_ENABLED=0 -l sqlite3 src/*.c

build_debug:
	@clang -o ./bin/main -D LOG_ENABLED=1 -l sqlite3 src/*.c

run        :
	@bin/main

clean      : bin/main
	rm -f bin/main