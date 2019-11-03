all: test

test: test_falloc test_dict test_vec test_queue test_list test_buffer \
	test_utf8 test_timingwheel test_timerservice test_coroutine \
	test_echo_server test_echo_client test_udp_dns test_echo_tps \
	test_echo_server2 test_echo_client2 test_udp_dns2 test_pqueue \
	test_set

test_falloc: test/test_falloc.c src/co_utils.h src/co_falloc.h src/co_falloc.c
	gcc -g -Wall -o test_falloc test/test_falloc.c src/co_falloc.c

test_dict: test/test_dict.c src/co_utils.h src/co_dict.h src/co_dict.c
	gcc -g -Wall -o test_dict test/test_dict.c src/co_dict.c

test_vec: test/test_vec.c src/co_utils.h src/co_vec.h src/co_vec.c
	gcc -g -Wall -o test_vec test/test_vec.c src/co_vec.c

test_queue: test/test_queue.c src/co_queue.c src/co_utils.h src/co_queue.h src/co_vec.c src/co_vec.h
	gcc -g -Wall -o test_queue test/test_queue.c src/co_queue.c src/co_vec.c

test_pqueue: test/test_pqueue.c src/co_pqueue.c src/co_utils.h src/co_pqueue.h src/co_vec.c src/co_vec.h
	gcc -g -Wall -o test_pqueue test/test_pqueue.c src/co_pqueue.c src/co_vec.c

test_list: test/test_list.c src/co_list.c src/co_list.h src/co_utils.h
	gcc -g -Wall -o test_list test/test_list.c src/co_list.c

test_set: test/test_set.c src/co_set.c src/co_set.h src/co_utils.h
	gcc -g -Wall -o test_set test/test_set.c src/co_set.c

test_buffer: test/test_buffer.c src/co_buffer.c src/co_buffer.h src/co_utils.h src/co_endian.h
	gcc -g -Wall -o test_buffer test/test_buffer.c src/co_buffer.c

test_utf8: test/test_utf8.c src/co_utf8.c src/co_utf8.h src/co_utils.h
	gcc -g -Wall -o test_utf8 test/test_utf8.c src/co_utf8.c

test_timingwheel: test/test_timingwheel.c src/co_timingwheel.c src/co_timingwheel.h src/co_utils.h src/co_clink.h
	gcc -g -Wall -o test_timingwheel test/test_timingwheel.c src/co_timingwheel.c

test_timerservice: test/test_timerservice.c src/co_timerservice.c src/co_timerservice.h src/co_timingwheel.c \
	src/co_timingwheel.h src/co_utils.h src/co_clink.h src/co_falloc.c src/co_falloc.h
	gcc -g -Wall -o test_timerservice test/test_timerservice.c src/co_timerservice.c src/co_timingwheel.c src/co_falloc.c

test_coroutine: test/test_coroutine.c src/co_routine.c src/co_routine.h src/co_ctx.c src/co_ctx.h src/coctx_swap.S src/co_utils.h
	gcc -g -Wall -o test_coroutine test/test_coroutine.c src/co_routine.c src/co_ctx.c src/coctx_swap.S


csrc = $(wildcard src/*.[cS])
cinc = $(wildcard src/*.h)

test_echo_server: test/test_echo_server.c $(csrc) $(cinc)
	gcc -g -Wall -o test_echo_server test/test_echo_server.c $(csrc)
test_echo_client: test/test_echo_client.c $(csrc) $(cinc)
	gcc -g -Wall -o test_echo_client test/test_echo_client.c $(csrc)
test_udp_dns: test/test_udp_dns.c $(csrc) $(cinc)
	gcc -g -Wall -o test_udp_dns test/test_udp_dns.c $(csrc)
test_echo_tps: test/test_echo_tps.c $(csrc) $(cinc)
	gcc -g -Wall -o test_echo_tps test/test_echo_tps.c $(csrc)
test_echo_server2: test/test_echo_server2.c $(csrc) $(cinc)
	gcc -g -Wall -o test_echo_server2 test/test_echo_server2.c $(csrc)
test_echo_client2: test/test_echo_client2.c $(csrc) $(cinc)
	gcc -g -Wall -o test_echo_client2 test/test_echo_client2.c $(csrc)
test_udp_dns2: test/test_udp_dns2.c $(csrc) $(cinc)
	gcc -g -Wall -o test_udp_dns2 test/test_udp_dns2.c $(csrc)


.PHONY: clean
clean:
	rm -f test_*