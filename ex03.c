#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 10 // 버퍼의 크기

int buffer[BUFFER_SIZE]; // 공유 버퍼
int count = 0; // 버퍼 내 항목 수

// 동기화를 위한 뮤텍스와 조건 변수
pthread_mutex_t mutex;
pthread_cond_t can_produce;
pthread_cond_t can_consume;

// 생산자 함수
void *producer(void *param) {
    int item;
    while (1) {
        item = rand() % 100; // 임의의 항목 생성
        pthread_mutex_lock(&mutex);

        while (count == BUFFER_SIZE) { // 버퍼가 꽉 찼는지 확인
            pthread_cond_wait(&can_produce, &mutex);
        }

        buffer[count++] = item; // 버퍼에 항목 추가
        printf("Producer produced %d\n", item);

        pthread_cond_signal(&can_consume);
        pthread_mutex_unlock(&mutex);
        sleep(1); // 시뮬레이션을 위한 대기
    }
    return NULL;
}

// 소비자 함수
void *consumer(void *param) {
    int item;
    while (1) {
        pthread_mutex_lock(&mutex);

        while (count == 0) { // 버퍼가 비었는지 확인
            pthread_cond_wait(&can_consume, &mutex);
        }

        item = buffer[--count]; // 버퍼에서 항목 제거
        printf("Consumer consumed %d\n", item);

        pthread_cond_signal(&can_produce);
        pthread_mutex_unlock(&mutex);
        sleep(1); // 시뮬레이션을 위한 대기
    }
    return NULL;
}

int main() {
    pthread_t prod1, prod2, cons1, cons2;

    // 뮤텍스와 조건 변수 초기화
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&can_produce, NULL);
    pthread_cond_init(&can_consume, NULL);

    // 생산자와 소비자 쓰레드 생성
    pthread_create(&prod1, NULL, producer, NULL);
    pthread_create(&prod2, NULL, producer, NULL);
    pthread_create(&cons1, NULL, consumer, NULL);
    pthread_create(&cons2, NULL, consumer, NULL);

    // 쓰레드가 종료될 때까지 대기
    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);

    // 뮤텍스와 조건 변수 해제
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&can_produce);
    pthread_cond_destroy(&can_consume);

    return 0;
}
