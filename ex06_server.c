#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 30 // 최대 클라이언트 수
#define BUFFER_SIZE 1024 // 메시지 버퍼 크기

void error(const char *msg) {
    perror(msg); // 에러 메시지 출력
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, clilen;
    char buffer[BUFFER_SIZE]; // 메시지를 저장할 버퍼
    struct sockaddr_in serv_addr, cli_addr; // 서버와 클라이언트 주소 구조체
    int n, i, max_clients = MAX_CLIENTS;
    int client_socket[MAX_CLIENTS]; // 클라이언트 소켓 배열
    fd_set readfds; // 파일 디스크립터 세트

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성
    if (sockfd < 0) 
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]); // 포트 번호

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // 모든 인터페이스 수신
    serv_addr.sin_port = htons(portno); // 포트 설정

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding"); // 소켓에 주소 바인딩

    listen(sockfd, 5); // 연결 대기 큐 설정
    clilen = sizeof(cli_addr);

    // 모든 클라이언트 소켓 초기화
    for (i = 0; i < max_clients; i++) {
        client_socket[i] = 0;
    }

    while (1) {
        FD_ZERO(&readfds); // 파일 디스크립터 세트 초기화
        FD_SET(sockfd, &readfds); // 서버 소켓 추가
        int max_sd = sockfd;

        // 모든 클라이언트 소켓을 세트에 추가
        for (i = 0; i < max_clients; i++) {
            int sd = client_socket[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // 소켓 상태를 모니터링
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            error("ERROR on select");
        }

        // 새로운 연결 요청 처리
        if (FD_ISSET(sockfd, &readfds)) {
            newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
            if (newsockfd < 0) {
                error("ERROR on accept");
            }

            // 새 클라이언트를 배열에 추가
            for (i = 0; i < max_clients; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = newsockfd;
                    break;
                }
            }
        }

        // 클라이언트로부터 메시지 수신 및 브로드캐스트
        for (i = 0; i < max_clients; i++) {
            int sd = client_socket[i];
            if (FD_ISSET(sd, &readfds)) {
                bzero(buffer, BUFFER_SIZE);
                n = read(sd, buffer, BUFFER_SIZE);
                if (n <= 0) {
                    close(sd); // 클라이언트 연결 종료
                    client_socket[i] = 0;
                } else {
                    buffer[n] = '\0';
                    // 모든 클라이언트에게 메시지 전송
                    for (int j = 0; j < max_clients; j++) {
                        if (client_socket[j] != 0) {
                            write(client_socket[j], buffer, strlen(buffer));
                        }
                    }
                }
            }
        }
    }

    close(sockfd);
    return 0;
}
