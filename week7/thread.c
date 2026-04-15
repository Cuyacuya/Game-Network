#include <stdio.h>
#include <pthread.h>

//여러개의 데이터 사용 시 구조체 이용
struct arg_sum {
    int a;
    int b;
    int result;
};

void* thread_function (void* arg) {
    struct arg_sum* data = (struct arg_sum*)arg;
    data->result = data->a + data->b;
    printf("%d + %d = %d\n", data->a, data->b, data->result);
    return NULL;
}
//thread는 메인의 메모리를 참조할 뿐...

int main() {
    pthread_t tid[10];
    struct arg_sum args[10];

    for(int i=0;i<10;i++){
        args[i].a = i+1;
        args[i].b = i+2;

        if(0 != pthread_create(&tid[i], NULL, thread_function, &args[i])) { //성공 시 0 반환
            fprintf(stderr, "pthread_create() failed\n");
            return 1;
        }
    }

    //`thread 기다리기
    for(int i=0;i<10;i++) {
        pthread_join(tid[i], NULL); //자동으로 close.
        printf("Thread return");
    }
    printf("Thread terminated\n");

    return 0;
}