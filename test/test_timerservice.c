#include <unistd.h>
#include <sys/time.h>
#include "../src/co_timerservice.h"

cots_t *sv;
void *thandle;
int count;

void on_timer(cots_t *ts, void *ud1, void *ud2, void *ud3) {
    printf("on_timer: %ju\n", co_gettime());
    ++count;
    if (count == 10)
        cots_del_timer(sv, &thandle);
}

void run() {
    while (1) {
        usleep(1000);
        cots_update(sv, co_gettime());
    }
}


int main(int argc, char const *argv[])
{
    sv = cots_new(10, co_gettime());

    thandle = cots_add_timer(sv, 100, 100, on_timer, NULL, NULL, NULL);

    run();

    cots_del_timer(sv, &thandle);
    cots_free(sv);
    return 0;
}
