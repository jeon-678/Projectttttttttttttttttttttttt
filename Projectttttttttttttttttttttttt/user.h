#pragma once

typedef struct {
    char id[20];
    char pw[20];
    char name[30];
} User;

int load_users(User users[], int* user_count);
int save_users(User users[], int user_count);

int login_tui(User users[], int user_count);
int signup_tui(User users[], int* user_count);
