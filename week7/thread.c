#include <stdio.h>
#include <pthread.h>

void* thread_function (void* arg) {
    int t_num = *(int*)arg;
    printf("Thread %d is runnning.\n", t_num);
    return NULL;
}

int main() {
    pthread_t tid;
    int args[10];

    for(int i=0;i<10;i++){
        args[i] = i+1;
        if(0 != pthread_create(&tid, NULL, thread_function, &args[i])) { //성공 시 0 반환
            fprintf(stderr, "pthread_create() failed\n");
            return 1;
        }
    }

    //`thread 기다리기
    pthread_join(tid, NULL);
    printf("Thread terminated\n");

    return 0;
}