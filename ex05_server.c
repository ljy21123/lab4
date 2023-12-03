#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
    perror(msg); // 에러 메시지 출력 후 종료
    exit(1);
}

int main(int argc, char *argv[]) {
     int sockfd, newsockfd, portno; // 소켓 파일 디스크립터와 포트 번호
     socklen_t clilen;
     char buffer[256]; // 데이터 버퍼
     struct sockaddr_in serv_addr, cli_addr; // 서버와 클라이언트 주소 구조체
     int n;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성
     if (sockfd < 0) 
        error("ERROR opening socket");

     bzero((char *) &serv_addr, sizeof(serv_addr)); // 주소 구조체 초기화
     portno = atoi(argv[1]); // 포트 번호 설정

     serv_addr.sin_family = AF_INET; // 주소 체계 설정 (IPv4)
     serv_addr.sin_addr.s_addr = INADDR_ANY; // 모든 인터페이스 수신 대기
     serv_addr.sin_port = htons(portno); // 네트워크 바이트 순서로 포트 설정

     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
              error("ERROR on binding"); // 소켓에 주소 바인딩

     listen(sockfd,5); // 연결 대기 큐 설정
     clilen = sizeof(cli_addr);

     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); // 클라이언트 연결 수락
     if (newsockfd < 0) 
          error("ERROR on accept");

     bzero(buffer,256); // 버퍼 초기화
     n = read(newsockfd,buffer,255); // 데이터 수신
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer); // 수신된 메시지 출력

     n = write(newsockfd,"I got your message",18); // 응답 전송
     if (n < 0) error("ERROR writing to socket");
     close(newsockfd); // 새 소켓 닫기
     close(sockfd); // 원래 소켓 닫기
     return 0; 
}
