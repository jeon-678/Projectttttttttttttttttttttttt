#pragma execution_character_set("utf-8")
#define _CRT_SECURE_NO_WARNINGS
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

void set_utf8_console(void) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

void clear_screen(void) { system("cls"); }

void gotoxy(int x, int y) {
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void draw_box(int x, int y, int width, int height) {
    int i;

    gotoxy(x, y);
    printf("┌");
    for (i = 0; i < width - 2; i++) printf("─");
    printf("┐");

    for (i = 1; i < height - 1; i++) {
        gotoxy(x, y + i);             printf("│");
        gotoxy(x + width - 1, y + i); printf("│");
    }

    gotoxy(x, y + height - 1);
    printf("└");
    for (i = 0; i < width - 2; i++) printf("─");
    printf("┘");
}

static void trim_newline(char* s) {
    if (!s) return;
    size_t len = strlen(s);
    while (len && (s[len - 1] == '\n' || s[len - 1] == '\r')) s[--len] = '\0';
}

int read_line(char* out, size_t n) {
    if (!out || n == 0) return 0;
    if (!fgets(out, (int)n, stdin)) return 0;
    trim_newline(out);
    return 1;
}

int read_int(int* out) {
    char buf[64];
    if (!read_line(buf, sizeof(buf))) return 0;
    char* end = NULL;
    long v = strtol(buf, &end, 10);
    if (end == buf || *end != '\0') return 0;
    *out = (int)v;
    return 1;
}

int read_int_range(int* out, int min, int max) {
    int v;
    if (!read_int(&v)) return 0;
    if (v < min || v > max) return 0;
    *out = v;
    return 1;
}

int read_positive_int(int* out) {
    return read_int_range(out, 1, 2147483647);
}

int confirm_yesno_at(int x, int y, const char* prompt) {
    gotoxy(x, y);
    printf("%s", prompt);
    int v;
    if (!read_int_range(&v, 1, 2)) return 0;
    return v == 1;
}

void pause_enter_at(int x, int y) {
    gotoxy(x, y);
    printf("계속하려면 Enter...");
    getchar();
}

void get_today_date(char* buf, size_t n) {
    if (!buf || n < 11) return;
    time_t t = time(NULL);
    struct tm lt;
    localtime_s(&lt, &t);
    snprintf(buf, n, "%04d-%02d-%02d", lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
}

int replace_file(const char* tmp_path, const char* final_path) {
    if (MoveFileExA(tmp_path, final_path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        return 1;
    }
    remove(final_path);
    return rename(tmp_path, final_path) == 0;
}
