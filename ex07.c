#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define MAX_BUFFER 1024 // 버퍼 최대 크기
#define PORT 8080 // 사용할 포트 번호

void execute_cgi(int sockfd, const char *path, const char *method, const char *query_string);
void process_http_request(int sockfd);
int get_line(int sock, char *buf, int size);
void bad_request(int client);
void cannot_execute(int client);

// CGI 스크립트 실행 함수
void execute_cgi(int sockfd, const char *path, const char *method, const char *query_string) {
    char buffer[MAX_BUFFER];
    int cgi_output[2]; // CGI 스크립트 출력을 위한 파이프
    int cgi_input[2];  // CGI 스크립트 입력을 위한 파이프
    pid_t pid;         // 프로세스 ID
    int status;
    int i;
    char c;
    int numchars = 1;
    int content_length = -1; // POST 데이터의 길이

    buffer[0] = 'A'; buffer[1] = '\0';
    if (strcasecmp(method, "GET") == 0) {
        // GET 요청 처리: 헤더를 무시하고 마지막까지 읽음
        while ((numchars > 0) && strcmp("\n", buffer))
            numchars = get_line(sockfd, buffer, sizeof(buffer));
    } else {    // POST 요청 처리
        numchars = get_line(sockfd, buffer, sizeof(buffer));
        while ((numchars > 0) && strcmp("\n", buffer)) {
            buffer[15] = '\0';
            if (strcasecmp(buffer, "Content-Length:") == 0)
                content_length = atoi(&(buffer[16]));
            numchars = get_line(sockfd, buffer, sizeof(buffer));
        }
        if (content_length == -1) {
            bad_request(sockfd);
            return;
        }
    }

    sprintf(buffer, "HTTP/1.0 200 OK\r\n");
    send(sockfd, buffer, strlen(buffer), 0);

    // CGI 스크립트 실행을 위한 파이프 생성
    if (pipe(cgi_output) < 0) {
        cannot_execute(sockfd);
        return;
    }
    if (pipe(cgi_input) < 0) {
        cannot_execute(sockfd);
        return;
    }

    // CGI 스크립트 실행
    if ((pid = fork()) < 0 ) {
        cannot_execute(sockfd);
        return;
    }
    if (pid == 0)  /* 자식 프로세스: CGI 스크립트 실행 */
    {
        char meth_env[255];
        char query_env[255];
        char length_env[255];

        dup2(cgi_output[1], 1); // 표준 출력을 cgi_output 파이프로 리다이렉트
        dup2(cgi_input[0], 0);  // 표준 입력을 cgi_input 파이프로 리다이렉트
        close(cgi_output[0]);
        close(cgi_input[1]);

        // 환경 변수 설정
        sprintf(meth_env, "REQUEST_METHOD=%s", method);
        putenv(meth_env);
        if (strcasecmp(method, "GET") == 0) {
            sprintf(query_env, "QUERY_STRING=%s", query_string);
            putenv(query_env);
        } else {   // POST
            sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
            putenv(length_env);
        }

        // CGI 스크립트 실행
        execl(path, path, NULL);
        exit(0);
    } else {    /* 부모 프로세스: CGI 스크립트 결과 처리 */
        close(cgi_output[1]);
        close(cgi_input[0]);
        // POST 데이터를 CGI 스크립트에 전달
        if (strcasecmp(method, "POST") == 0)
            for (i = 0; i < content_length; i++) {
                recv(sockfd, &c, 1, 0);
                write(cgi_input[1], &c, 1);
            }
        // CGI 스크립트 출력을 클라이언트에게 전송
        while (read(cgi_output[0], &c, 1) > 0)
            send(sockfd, &c, 1, 0);

        close(cgi_output[0]);
        close(cgi_input[1]);
        waitpid(pid, &status, 0); // 자식 프로세스의 종료를 기다림
    }
}

// HTTP 요청을 처리하는 함수
void process_http_request(int sockfd) {
    char buffer[MAX_BUFFER];
    int n;

    bzero(buffer, MAX_BUFFER);
    n = read(sockfd, buffer, MAX_BUFFER - 1); // 클라이언트로부터 요청 읽기

    if (n < 0) {
        perror("ERROR reading from socket");
        return;
    }

    printf("Here is the HTTP request:\n%s\n", buffer);

    // HTTP GET 요청 처리
    if (strncmp(buffer, "GET", 3) == 0) {
        // CGI 스크립트 실행 여부 판단
        if (strncmp(buffer, "GET /cgi-bin/", 13) == 0) {
            // CGI 실행 코드
            execute_cgi(sockfd, "/path/to/cgi/script", "GET", ""); // CGI 스크립트 경로와 쿼리 스트링을 지정
        } else {
            // 정적 컨텐츠 응답
            char *response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><body><h1>Hello, World!</h1></body></html>\n";
            n = write(sockfd, response, strlen(response)); // 클라이언트에 응답 전송
        }
    }

    // HTTP POST 요청 처리
    if (strncmp(buffer, "POST", 4) == 0) {
        // CGI 스크립트 실행 여부 판단
        if (strncmp(buffer, "POST /cgi-bin/", 14) == 0) {
            // CGI 실행 코드
            execute_cgi(sockfd, "/path/to/cgi/script", "POST", ""); // CGI 스크립트 경로 지정
        } else {
            // 정적 컨텐츠 응답
            char *response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><body><h1>POST Data Received</h1></body></html>\n";
            n = write(sockfd, response, strlen(response)); // 클라이언트에 응답 전송
        }
    }

    if (n < 0) {
        perror("ERROR writing to socket");
    }
}

// 클라이언트로부터 한 줄을 읽는 함수
int get_line(int sock, char *buf, int size) {
    int i = 0;
    char c = '\0';
    int n;

    // 클라이언트로부터 문자를 하나씩 읽으며 '\n'이 나올 때까지 버퍼에 저장합니다.
    while ((i < size - 1) && (c != '\n')) {
        n = recv(sock, &c, 1, 0);
        // '\r' 문자를 '\n'으로 변환하여 줄의 끝을 처리합니다.
        if (n > 0) {
            if (c == '\r') {
                n = recv(sock, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n'))
                    recv(sock, &c, 1, 0);
                else
                    c = '\n';
            }
            buf[i] = c;
            i++;
        } else {
            c = '\n';
        }
    }
    buf[i] = '\0';

    return(i);
}

// 잘못된 요청을 처리하는 함수
void bad_request(int client) {
    char buf[1024];

    // 클라이언트에게 400 Bad Request 에러 메시지를 전송합니다.
    sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<p>Your browser sent a bad request.</p>");
    send(client, buf, strlen(buf), 0);
}

// CGI 실행 불가 에러를 처리하는 함수
void cannot_execute(int client) {
    char buf[1024];

    // 클라이언트에게 500 Internal Server Error 메시지를 전송합니다.
    sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<p>Error prohibited CGI execution.</p>");
    send(client, buf, strlen(buf), 0);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); // 서버 주소 구조체 초기화
    portno = PORT; // 포트 번호 설정

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // 모든 주소에서 수신
    serv_addr.sin_port = htons(portno); // 포트 설정

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding"); // 소켓에 주소 바인딩
        exit(1);
    }

    listen(sockfd, 5); // 연결 대기
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); // 클라이언트 연결 수락
        if (newsockfd < 0) {
            perror("ERROR on accept");
            continue;
        }

        process_http_request(newsockfd); // HTTP 요청 처리
        close(newsockfd); // 클라이언트 소켓 닫기
    }

    close(sockfd); // 서버 소켓 닫기
    return 0; 
}
