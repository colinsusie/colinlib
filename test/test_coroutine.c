#include "../src/co_routine.h"
#include <sys/time.h>
#include <sys/types.h>

struct args {
	int n;
};

double timediff(struct timeval *begin, struct timeval *end) {
    return (end->tv_sec + end->tv_usec * 1.0 / 1000000) -
           (begin->tv_sec + begin->tv_usec * 1.0 / 1000000);
}

///////////////////////////////////////////////////////////////////////////////////
// 简单测试
static void foo(cosched_t * S, void *ud) {
	struct args * arg = ud;
	int start = arg->n;
	int i;
	for (i=0;i<5;i++) {
		printf("coroutine %d : %d\n", cort_running(S) , start + i);
		cort_yield(S);
	}
}

static void test(cosched_t *S) {
	struct args arg1 = { 0 };
	struct args arg2 = { 100 };

	int co1 = cort_new(S, foo, &arg1);
	int co2 = cort_new(S, foo, &arg2);
	printf("test start===============\n");
	while (cort_status(S,co1) && cort_status(S,co2)) {
		cort_resume(S,co1);
		cort_resume(S,co2);
	} 
	printf("test end==============\n");
}

///////////////////////////////////////////////////////////////////////////////////
// 递归创建1000个协程
static void foo2(cosched_t * S, void *ud) {
	struct args * arg = ud;
	if (arg->n < 1000) {
		arg->n++;
		int co = cort_new(S, foo2, arg);
		printf("coroutine: %d, n=%d\n", co, arg->n);
		cort_resume(S, co);
	}
}

static void test2(cosched_t *S) {
	printf("test2 start===============\n");
	struct args arg = { 0 };
	int co = cort_new(S, foo2, &arg);
	printf("coroutine: %d, n=%d\n", co, arg.n);
	cort_resume(S, co);
	printf("test2 end==============\n");
}

///////////////////////////////////////////////////////////////////////////////////
// 主协程测试
static void test3(cosched_t *S) {
	printf("test3 start===============\n");
	int mainco = cort_running(S);
	printf("main co: %d\n", mainco);
	printf("yield mainco: %d\n", cort_yield(S));
	printf("test3 end==============\n");
}

///////////////////////////////////////////////////////////////////////////////////
// 协程相互resume
static int co1, co2, co3;

static void foo4_3(cosched_t *S, void *ud) {
	printf("[%d] run\n", cort_running(S));
	cort_resume(S, co2);
	printf("[%d] run done\n", cort_running(S));
}

static void foo4_2(cosched_t *S, void *ud) {
	printf("[%d] run\n", cort_running(S));
	cort_yield(S);
	printf("[%d] run done\n", cort_running(S));
}

static void foo4_1(cosched_t *S, void *ud) {
	printf("[%d] run\n", cort_running(S));
	cort_resume(S, co2);
	co3 = cort_new(S, foo4_3, NULL);
	cort_resume(S, co3);
	printf("[%d] run done\n", cort_running(S));
}

static void test4(cosched_t *S) {
	printf("test4 start===============\n");
	co1 = cort_new(S, foo4_1, NULL);
	co2 = cort_new(S, foo4_2, NULL);
	cort_resume(S, co1);
	printf("test4 end===============\n");
}

///////////////////////////////////////////////////////////////////////////////////
// 协程的创建和切换消耗
int stop = 0;
static void foo_5(cosched_t *S, void *ud) {
	while (!stop) {
		cort_yield(S);
	}
}

static void test5(cosched_t *S) {
	printf("test5 start===============\n");
	struct timeval begin;
	struct timeval end;
	int i;
	int count = 1000;

	gettimeofday(&begin, NULL);
	for (i = 0; i < count; ++i) {
		cort_new(S, foo_5, NULL);
	}
	gettimeofday(&end, NULL);
	printf("create time=%f\n", timediff(&begin, &end));
	
	gettimeofday(&begin, NULL);
	for (i =0; i < 5000000; ++i) {
		int co = (i % count) + 1;
		cort_resume(S, co);
	}
	gettimeofday(&end, NULL);
	printf("swap time=%f\n", timediff(&begin, &end));

	// 先释放掉原来的
	stop = 1;
	for (i = 0; i < count; ++i) {
		int co = (i % count) + 1;
		cort_resume(S, co);
	}
	gettimeofday(&begin, NULL);
	for (i = 0; i < count; ++i) {
		cort_new(S, foo_5, NULL);
	}
	gettimeofday(&end, NULL);
	printf("create time2=%f\n", timediff(&begin, &end));
	printf("test5 end===============\n");
}

int main() {
	cosched_t * S = cort_open(0);
	test(S);
	test2(S);
	test3(S);
	test4(S);
	test5(S);
	cort_close(S);	
	return 0;
}

