#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CLIENTS 5 // 최대 클라이언트 수
#define BUFFER_SIZE 1024 // 버퍼 크기

// 공유 데이터 구조체
typedef struct {
    char buffer[BUFFER_SIZE]; // 메시지를 저장할 버퍼
    int new_message; // 새 메시지 여부를 나타내는 플래그
} shared_data;

shared_data data; // 공유 데이터 인스턴스
pthread_mutex_t mutex; // 뮤텍스
pthread_cond_t cond; // 조건 변수

// 서버(부모) 쓰레드 함수
void *server_thread(void *param) {
    while (1) {
        pthread_mutex_lock(&mutex); // 뮤텍스 잠금

        // 새 메시지가 있을 때까지 대기
        while (!data.new_message) { 
            pthread_cond_wait(&cond, &mutex);
        }

        // 메시지 방송
        printf("Server broadcasting: %s\n", data.buffer);
        data.new_message = 0; // 메시지 상태 초기화

        pthread_cond_broadcast(&cond); // 모든 클라이언트에게 신호 전송
        pthread_mutex_unlock(&mutex); // 뮤텍스 해제
    }

    return NULL;
}

// 클라이언트(자식) 쓰레드 함수
void *client_thread(void *param) {
    int num = *(int *)param; // 클라이언트 ID
    char message[BUFFER_SIZE]; // 메시지 버퍼

    while (1) {
        snprintf(message, BUFFER_SIZE, "Message from client %d", num); // 메시지 생성

        pthread_mutex_lock(&mutex); // 뮤텍스 잠금

        // 이전 메시지 처리를 기다림
        while (data.new_message) { 
            pthread_cond_wait(&cond, &mutex);
        }

        // 새 메시지를 공유 데이터에 저장
        strcpy(data.buffer, message);
        data.new_message = 1;

        pthread_cond_signal(&cond); // 서버에 메시지 전송 신호
        pthread_mutex_unlock(&mutex); // 뮤텍스 해제

        sleep(1); // 시뮬레이션을 위한 대기
    }

    return NULL;
}

int main() {
    pthread_t server; // 서버 쓰레드
    pthread_t clients[MAX_CLIENTS]; // 클라이언트 쓰레드들
    int client_ids[MAX_CLIENTS]; // 클라이언트 ID 배열

    // 뮤텍스와 조건 변수 초기화
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    data.new_message = 0; // 공유 데이터 초기화

    pthread_create(&server, NULL, server_thread, NULL); // 서버 쓰레드 생성

    // 클라이언트 쓰레드 생성
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_ids[i] = i;
        pthread_create(&clients[i], NULL, client_thread, &client_ids[i]);
    }

    // 쓰레드가 종료될 때까지 대기
    pthread_join(server, NULL);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_join(clients[i], NULL);
    }

    // 뮤텍스와 조건 변수 해제
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
