all: test

test: test_falloc test_dict test_vec test_queue test_list test_buffer \
	test_utf8

test_falloc: test/test_falloc.c src/co_utils.h src/co_falloc.h src/co_falloc.c
	gcc -g -Wall -o test_falloc test/test_falloc.c src/co_falloc.c

test_dict: test/test_dict.c src/co_utils.h src/co_dict.h src/co_dict.c
	gcc -g -Wall -o test_dict test/test_dict.c src/co_dict.c

test_vec: test/test_vec.c src/co_utils.h src/co_vec.h src/co_vec.c
	gcc -g -Wall -o test_vec test/test_vec.c src/co_vec.c

test_queue: test/test_queue.c src/co_queue.c src/co_utils.h src/co_queue.h src/co_vec.c src/co_vec.h
	gcc -g -Wall -o test_queue test/test_queue.c src/co_queue.c src/co_vec.c

test_list: test/test_list.c src/co_list.c src/co_list.h src/co_utils.h
	gcc -g -Wall -o test_list test/test_list.c src/co_list.c

test_buffer: test/test_buffer.c src/co_buffer.c src/co_buffer.h src/co_utils.h src/co_endian.h
	gcc -g -Wall -o test_buffer test/test_buffer.c src/co_buffer.c

test_utf8: test/test_utf8.c src/co_utf8.c src/co_utf8.h src/co_utils.h
	gcc -g -Wall -o test_utf8 test/test_utf8.c src/co_utf8.c

.PHONY: clean
clean:
	rm -f test_*