#include "co_routine.h"

#if __APPLE__ && __MACH__
	#include <sys/ucontext.h>
#else 
	#include <ucontext.h>
#endif

// 如果是x64，替代为自己的实现
#if defined(__x86_64__)
#define USE_COCTX
#include "co_ctx.h"
#endif

#define MIN_STACK_SIZE (128*1024)
#define MAX_STACK_SIZE (1024*1024)
#define DEFAULT_COROUTINE 128
#define MAIN_CO_ID 0

struct cort;

// 每个线程的调度器
typedef struct cosched {
	int stsize;				// 栈大小
	int nco;				// 当前有几个协程
	int cap;				// 协程数组容量
	int running;			// 当前正在运行的协程ID
	struct cort **co;	// 协程数组
} cosched_t;

// 协程数据
typedef struct cort {
	cort_func_t func;			// 协程回调函数
	void *ud;				// 用户数据
	int pco;				// 前一个协程，即resume这个协程的那个协程
#ifdef USE_COCTX
	coctx_t ctx;			// 协程的执行环境
#else
	ucontext_t ctx;			// 协程的执行环境
#endif
	cosched_t * sch;		// 调度器
	int status;				// 当前状态：CO_STATUS_RUNNING...
	char *stack;			// 栈内存
} cort_t;

cosched_t* cort_open(int stsize) {
	cosched_t *S = CO_MALLOC(sizeof(*S));
	S->nco = 0;
	S->stsize = CO_MIN(CO_MAX(stsize, MIN_STACK_SIZE), MAX_STACK_SIZE);
	S->cap = DEFAULT_COROUTINE;
	S->co = CO_MALLOC(sizeof(cort_t *) * S->cap);
	memset(S->co, 0, sizeof(cort_t *) * S->cap);

	// 创建主协程
	int id = cort_new(S, NULL, NULL);
	assert(id == MAIN_CO_ID);
	// 主协程为运行状态
	cort_t *co = S->co[MAIN_CO_ID];
	co->status = CO_STATUS_RUNNING;
	S->running = id;
	return S;
}

void cort_close(cosched_t *S) {
	assert(S->running == MAIN_CO_ID);
	int i;
	for (i=0;i<S->cap;i++) {
		cort_t * co = S->co[i];
		if (co) {
			CO_FREE(co->stack);
			CO_FREE(co);
		}
	}
	CO_FREE(S->co);
	S->co = NULL;
	CO_FREE(S);
}

static void cofunc(uint32_t low32, uint32_t hi32) {
	uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
	cosched_t *S = (cosched_t *)ptr;
	int id = S->running;
	cort_t *co = S->co[id];
	co->func(S, co->ud);
	// 标记协程为死亡
	co->status = CO_STATUS_DEAD;
	--S->nco;
	// 恢复前一个协程
	cort_t *pco = S->co[co->pco];
	pco->status = CO_STATUS_RUNNING;
	S->running = co->pco;
#ifdef USE_COCTX
	coctx_t dummy;
	coctx_swap(&dummy, &pco->ctx);
#else
	ucontext_t dummy;
	swapcontext(&dummy, &pco->ctx);
#endif
}

int cort_new(cosched_t *S, cort_func_t func, void *ud) {
	int cid = -1;
	if (S->nco >= S->cap) {
		cid = S->cap;
		S->co = CO_REALLOC(S->co, S->cap * 2 * sizeof(cort_t *));
		memset(S->co + S->cap , 0 , sizeof(cort_t *) * S->cap);
		S->cap *= 2;
	} else {
		int i;
		for (i=0;i<S->cap;i++) {
			int id = (i+S->nco) % S->cap;
			if (S->co[id] == NULL) {
				cid = id;
				break;
			} 
			else if (S->co[id]->status == CO_STATUS_DEAD) {
				// printf("reuse dead cort: %d\n", id);
				cid = id;
				break;
			}
		}
	}
	
	if (cid >= 0) {
		cort_t *co;
		if (S->co[cid])
			co = S->co[cid];
		else {
			co = CO_MALLOC(sizeof(*co));
			co->pco = 0;
			co->stack = cid != MAIN_CO_ID ? CO_MALLOC(S->stsize) : 0;
			S->co[cid] = co;
		}
		++S->nco;
		
		co->func = func;
		co->ud = ud;
		co->sch = S;
		co->status = CO_STATUS_SUSPEND;

		if (func) {
			cort_t *curco = S->co[S->running];
			assert(curco);
#ifdef USE_COCTX
			co->ctx.ss_sp = co->stack;
			co->ctx.ss_size = S->stsize;
			uintptr_t ptr = (uintptr_t)S;
			coctx_make(&co->ctx, cofunc, (uint32_t)ptr, (uint32_t)(ptr>>32));
#else
			getcontext(&co->ctx);
			co->ctx.uc_stack.ss_sp = co->stack;
			co->ctx.uc_stack.ss_size = S->stsize;
			co->ctx.uc_link = &curco->ctx;
			uintptr_t ptr = (uintptr_t)S;
			makecontext(&co->ctx, (void (*)(void))cofunc, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));
#endif
		}
	}

	return cid;
}

int cort_resume(cosched_t * S, int id) {
	assert(id >=0 && id < S->cap);
	cort_t *co = S->co[id];
	cort_t *curco = S->co[S->running];
	if (co == NULL || curco == NULL)
		return -1;
	int status = co->status;
	switch(status) {
	case CO_STATUS_SUSPEND:
		curco->status = CO_STATUS_NORMAL;
		co->pco = S->running;
		co->status = CO_STATUS_RUNNING;
		S->running = id;
#ifdef USE_COCTX
		coctx_swap(&curco->ctx, &co->ctx);
#else
		swapcontext(&curco->ctx, &co->ctx);
#endif
		return 0;
	default:
		return -1;
	}
}

int cort_yield(cosched_t * S) {
	int id = S->running;
	// 主协程不能yield
	if (id == MAIN_CO_ID)
		return -1;
	// 恢复当前协程环境
	assert(id >= 0);
	cort_t * co = S->co[id];
	cort_t *pco = S->co[co->pco];
	co->status = CO_STATUS_SUSPEND;
	pco->status = CO_STATUS_RUNNING;
	S->running = co->pco;
#ifdef USE_COCTX
	coctx_swap(&co->ctx ,&pco->ctx);
#else
	swapcontext(&co->ctx ,&pco->ctx);
#endif
	return 0;
}

int cort_status(cosched_t * S, int id) {
	assert(id>=0 && id < S->cap);
	if (S->co[id] == NULL) {
		return CO_STATUS_DEAD;
	}
	return S->co[id]->status;
}

int cort_running(cosched_t * S) {
	return S->running;
}
