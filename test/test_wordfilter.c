#include "../misc/co_wordfilter.h"


int main(int argc, char const *argv[])
{
    void *wf = cowf_new();

    cowf_loadwords(wf, "words");

    char text[] = "今天非常bad啊, 非常bad";
    cowf_fitler(wf, text, '*');
    printf("%s\n", text);
    bool find = cowf_check(wf, "today");
    printf("find=%d\n", find);

    cowf_addword(wf, "queue");
    find = cowf_check(wf, "queue event");
    printf("find=%d\n", find);

    cowf_delword(wf, "queue");
    find = cowf_check(wf, "queue event");
    printf("find=%d\n", find);

    char text2[] = "hello 请问你是特朗普吗，你好啊特朗普，我有点事件找你呢";
    cowf_fitler(wf, text2, '*');
    printf("%s\n", text2);

    char text3[] = "我fuck，这是什么玩意儿";
    cowf_fitler(wf, text3, '*');
    printf("%s\n", text3);

    cowf_free(wf);
    return 0;
}
