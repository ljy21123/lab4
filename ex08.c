#include <gtk/gtk.h>

// 버튼 클릭 이벤트 핸들러
void on_button_clicked(GtkWidget *button, gpointer data) {
    // 라벨의 텍스트 변경
    gtk_label_set_text(GTK_LABEL(data), "버튼이 클릭되었습니다!");
}

int main(int argc, char *argv[]) {
    GtkWidget *window, *button, *label, *vbox;

    // GTK+ 초기화
    gtk_init(&argc, &argv);

    // 윈도우 생성
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK+ Example");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 수직 박스 생성
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // 라벨 생성 및 박스에 추가
    label = gtk_label_new("버튼을 클릭하세요.");
    gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);

    // 버튼 생성 및 크기 설정
    button = gtk_button_new_with_label("클릭");
    gtk_widget_set_size_request(button, 100, 50);
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), label);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0); // 팽창과 채우기를 FALSE로 설정

    // 모든 위젯 표시
    gtk_widget_show_all(window);

    // GTK+ 메인 이벤트 루프 시작
    gtk_main();

    return 0;
}
