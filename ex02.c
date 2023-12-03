#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// 쓰레드가 실행할 함수 정의
void *print_message_function( void *ptr );

int main() {
    pthread_t thread1, thread2;
    const char *message1 = "Thread 1";
    const char *message2 = "Thread 2";
    int iret1, iret2;

    // 첫 번째 쓰레드 생성
    iret1 = pthread_create(&thread1, NULL, print_message_function, (void*) message1);
    if(iret1) {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
        exit(EXIT_FAILURE);
    }

    // 두 번째 쓰레드 생성
    iret2 = pthread_create(&thread2, NULL, print_message_function, (void*) message2);
    if(iret2) {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret2);
        exit(EXIT_FAILURE);
    }

    printf("pthread_create() for thread 1 returns: %d\n",iret1);
    printf("pthread_create() for thread 2 returns: %d\n",iret2);

    // 쓰레드가 종료될 때까지 대기
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    exit(EXIT_SUCCESS);
}

// 쓰레드 함수 구현
void *print_message_function( void *ptr ) {
    char *message;
    message = (char *) ptr;
    printf("%s \n", message);
    return NULL;
}
