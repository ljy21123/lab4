#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

// 계산기의 상태를 저장하는 구조체 정의
typedef struct {
    GtkWidget *entry;  // 텍스트 입력 필드
    char operator;     // 현재 선택된 연산자
    double operand;    // 첫 번째 피연산자 값
} Calculator;

// 계산 수행 함수
void calculate(Calculator *calc) {
    const char *text = gtk_entry_get_text(GTK_ENTRY(calc->entry));
    double value = atof(text);  // 텍스트 필드의 값을 실수로 변환

    // 선택된 연산자에 따라 계산 수행
    switch (calc->operator) {
        case '+': value = calc->operand + value; break;
        case '-': value = calc->operand - value; break;
        case '*': value = calc->operand * value; break;
        case '/': value = calc->operand / value; break;
    }

    // 계산 결과를 문자열로 변환하여 텍스트 필드에 표시
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%g", value);
    gtk_entry_set_text(GTK_ENTRY(calc->entry), buffer);
}

// 버튼 클릭 이벤트 핸들러
void on_button_clicked(GtkWidget *widget, gpointer data) {
    Calculator *calc = (Calculator *)data;  // Calculator 구조체 포인터를 얻음
    const char *label = gtk_button_get_label(GTK_BUTTON(widget));  // 클릭된 버튼의 라벨을 가져옴

    // '=' 버튼이 클릭된 경우
    if (strcmp(label, "=") == 0) {
        calculate(calc);  // 계산 수행
        calc->operator = '\0';  // 연산자를 초기화
    } 
    // 연산자 버튼(+, -, *, /)이 클릭된 경우
    else if (strchr("+-*/", label[0]) != NULL) {
        calc->operator = label[0];  // 선택된 연산자를 저장
        calc->operand = atof(gtk_entry_get_text(GTK_ENTRY(calc->entry)));  // 현재 입력 필드의 값을 피연산자로 저장
        gtk_entry_set_text(GTK_ENTRY(calc->entry), "");  // 입력 필드 초기화
    } 
    // 숫자 또는 소수점 버튼이 클릭된 경우
    else {
        // 현재 입력 필드의 텍스트를 가져옴
        const char *current_text = gtk_entry_get_text(GTK_ENTRY(calc->entry));
        // 새로운 텍스트를 생성하여 기존 텍스트와 클릭된 버튼의 라벨을 결합
        char *new_text = g_strdup_printf("%s%s", current_text, label);
        // 입력 필드에 결합된 텍스트를 설정
        gtk_entry_set_text(GTK_ENTRY(calc->entry), new_text);
        // 더 이상 필요하지 않은 new_text 메모리 해제
        g_free(new_text);
    }
}



int main(int argc, char *argv[]) {
    GtkWidget *window, *grid, *button;
    Calculator calc = { NULL, '\0', 0 };  // 계산기 상태 초기화

    gtk_init(&argc, &argv);

    // 윈도우 생성 및 설정
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 그리드 레이아웃 생성 및 윈도우에 추가
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // 텍스트 입력 필드 생성 및 그리드에 추가
    calc.entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), calc.entry, 0, 0, 4, 1);

    // 계산기 버튼 생성 및 그리드에 추가
    const char *buttons[] = {
        "7", "8", "9", "/",
        "4", "5", "6", "*",
        "1", "2", "3", "-",
        "0", "=", ".", "+"
    };

    // 버튼 위젯 생성 및 그리드에 배치
    for (int i = 0; i < 16; ++i) {
        button = gtk_button_new_with_label(buttons[i]);
        g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), &calc);
        gtk_grid_attach(GTK_GRID(grid), button, i % 4, 1 + i / 4, 1, 1);
    }

    // 모든 위젯 표시
    gtk_widget_show_all(window);

    // GTK+ 메인 이벤트 루프 시작
    gtk_main();

    return 0;
}
