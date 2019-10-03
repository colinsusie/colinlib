all: test

test: test_falloc test_dict test_vec test_queue

test_falloc: test/test_falloc.c src/co_utils.h src/co_falloc.h src/co_falloc.c
	gcc -g -Wall -o test_falloc test/test_falloc.c src/co_falloc.h src/co_falloc.c

test_dict: test/test_dict.c src/co_utils.h src/co_dict.h src/co_dict.c
	gcc -g -Wall -o test_dict test/test_dict.c src/co_dict.c

test_vec: test/test_vec.c src/co_utils.h src/co_vec.h src/co_vec.c
	gcc -g -Wall -o test_vec test/test_vec.c src/co_vec.c

test_queue: test/test_queue.c src/co_queue.c src/co_utils.h src/co_queue.h src/co_vec.c src/co_vec.h
	gcc -g -Wall -o test_queue test/test_queue.c src/co_queue.c src/co_vec.c

.PHONY: clean
clean:
	rm -f test_*