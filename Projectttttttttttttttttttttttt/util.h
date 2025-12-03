#pragma once
#include <stddef.h>

void set_utf8_console(void);
void clear_screen(void);
void gotoxy(int x, int y);
void draw_box(int x, int y, int width, int height);

void pause_enter_at(int x, int y);

int read_line(char* out, size_t n);
int read_int(int* out);
int read_int_range(int* out, int min, int max);
int read_positive_int(int* out);
int confirm_yesno_at(int x, int y, const char* prompt);

void get_today_date(char* buf, size_t n);

int replace_file(const char* tmp_path, const char* final_path);
