#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>

#define BUFFER_SIZE 1024 // 버퍼 크기 정의

void error(const char *msg) {
    perror(msg); // 에러 발생 시 메시지 출력
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFFER_SIZE];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]); // 사용 방법 안내
       exit(0);
    }

    portno = atoi(argv[2]); // 포트 번호
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // 소켓 생성
    if (sockfd < 0) 
        error("ERROR opening socket");

    server = gethostbyname(argv[1]); // 서버의 호스트 이름 해석
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); // 서버 주소 초기화
    serv_addr.sin_family = AF_INET; // 주소 체계 설정
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length); // 서버 IP 주소 복사
    serv_addr.sin_port = htons(portno); // 포트 설정

    // 서버에 연결
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    fd_set readfds; // 파일 디스크립터 세트
    while (1) {
        FD_ZERO(&readfds); // 세트 초기화
        FD_SET(sockfd, &readfds); // 소켓 추가
        FD_SET(STDIN_FILENO, &readfds); // 표준 입력 추가

        // select를 사용하여 읽기 가능한 데이터 확인
        if (select(sockfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            error("ERROR on select");
        }

        // 서버로부터 메시지 수신
        if (FD_ISSET(sockfd, &readfds)) {
            bzero(buffer, BUFFER_SIZE);
            n = read(sockfd, buffer, BUFFER_SIZE - 1);
            if (n < 0) {
                error("ERROR reading from socket");
            } else if (n == 0) {
                printf("Server disconnected\n"); // 서버 연결 종료
                break;
            }
            printf("Server: %s\n", buffer); // 서버 메시지 출력
        }

        // 사용자로부터 입력된 메시지 서버에 전송
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            bzero(buffer, BUFFER_SIZE);
            fgets(buffer, BUFFER_SIZE - 1, stdin);

            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0) {
                error("ERROR writing to socket");
            }
        }
    }

    close(sockfd); // 소켓 닫기
    return 0;
}
