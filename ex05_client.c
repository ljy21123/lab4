#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) {
    perror(msg); // 에러 메시지 출력 후 종료
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n; // 소켓 파일 디스크립터와 포트 번호
    struct sockaddr_in serv_addr; // 서버 주소 구조체
    struct hostent *server; // 호스트 엔티티

    char buffer[256]; // 데이터 버퍼
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]); // 포트 번호 설정
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성
    if (sockfd < 0) 
        error("ERROR opening socket");

    server = gethostbyname(argv[1]); // 서버 호스트 이름 조회
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); // 주소 구조체 초기화
    serv_addr.sin_family = AF_INET; // 주소 체계 설정 (IPv4)
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length); // 서버 주소 복사
    serv_addr.sin_port = htons(portno); // 네트워크 바이트 순서로 포트 설정

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting"); // 서버에 연결

    printf("Please enter the message: ");
    bzero(buffer,256); // 버퍼 초기화
    fgets(buffer,255,stdin); // 사용자로부터 메시지 입력 받음
    n = write(sockfd,buffer,strlen(buffer)); // 서버에 메시지 전송
    if (n < 0) 
         error("ERROR writing to socket");

    bzero(buffer,256); // 버퍼 초기화
    n = read(sockfd,buffer,255); // 서버로부터 응답 수신
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer); // 수신된 응답 출력
    close(sockfd); // 소켓 닫기
    return 0;
}
