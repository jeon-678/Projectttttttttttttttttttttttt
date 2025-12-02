#define _CRT_SECURE_NO_WARNINGS
#pragma execution_character_set("utf-8")
#include "user.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

#define USERS_FILE "users.txt"
#define USERS_TMP  "users.tmp"
#define MAX_USERS  100

static int user_exists(User users[], int user_count, const char* id) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].id, id) == 0) return 1;
    }
    return 0;
}

int load_users(User users[], int* user_count) {
    if (!users || !user_count) return 0;

    FILE* fp = fopen(USERS_FILE, "r");
    if (!fp) {
        *user_count = 0;
        return 0; // 없으면 0명 시작 가능
    }

    char line[256];
    int cnt = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (cnt >= MAX_USERS) break;

        char* id = strtok(line, "\t");
        char* pw = strtok(NULL, "\t");
        char* name = strtok(NULL, "\n");
        if (!id || !pw || !name) continue;

        strncpy(users[cnt].id, id, sizeof(users[cnt].id) - 1);
        users[cnt].id[sizeof(users[cnt].id) - 1] = '\0';

        strncpy(users[cnt].pw, pw, sizeof(users[cnt].pw) - 1);
        users[cnt].pw[sizeof(users[cnt].pw) - 1] = '\0';

        strncpy(users[cnt].name, name, sizeof(users[cnt].name) - 1);
        users[cnt].name[sizeof(users[cnt].name) - 1] = '\0';

        cnt++;
    }

    fclose(fp);
    *user_count = cnt;
    return 1;
}

int save_users(User users[], int user_count) {
    FILE* fp = fopen(USERS_TMP, "w");
    if (!fp) return 0;

    for (int i = 0; i < user_count; i++) {
        fprintf(fp, "%s\t%s\t%s\n", users[i].id, users[i].pw, users[i].name);
    }
    fclose(fp);

    return replace_file(USERS_TMP, USERS_FILE);
}

int login_tui(User users[], int user_count) {
    clear_screen();
    draw_box(1, 1, 60, 14);
    gotoxy(3, 3); printf("로그인");

    char id[64], pw[64];

    gotoxy(3, 5); printf("아이디: ");
    if (!read_line(id, sizeof(id)) || strlen(id) == 0) return -1;

    gotoxy(3, 6); printf("비밀번호: ");
    if (!read_line(pw, sizeof(pw)) || strlen(pw) == 0) return -1;

    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].id, id) == 0 && strcmp(users[i].pw, pw) == 0) {
            return i;
        }
    }
    return -1;
}

int signup_tui(User users[], int* user_count) {
    if (!users || !user_count) return 0;
    if (*user_count >= MAX_USERS) return 0;

    clear_screen();
    draw_box(1, 1, 70, 18);
    gotoxy(3, 3);
    printf("회원가입");

    char id[64], pw[64], name[64];

    gotoxy(3, 5);
    printf("아이디(공백X): ");
    if (!read_line(id, sizeof(id)) || strlen(id) == 0) return 0;

    if (user_exists(users, *user_count, id)) {
        gotoxy(3, 7);
        printf("이미 존재하는 아이디입니다.");
        return 0;
    }

    gotoxy(3, 6); printf("비밀번호(공백X): ");
    if (!read_line(pw, sizeof(pw)) || strlen(pw) == 0) return 0;

    gotoxy(3, 7); printf("이름: ");
    if (!read_line(name, sizeof(name)) || strlen(name) == 0) return 0;

    strncpy(users[*user_count].id, id, sizeof(users[*user_count].id) - 1);
    users[*user_count].id[sizeof(users[*user_count].id) - 1] = '\0';

    strncpy(users[*user_count].pw, pw, sizeof(users[*user_count].pw) - 1);
    users[*user_count].pw[sizeof(users[*user_count].pw) - 1] = '\0';

    strncpy(users[*user_count].name, name, sizeof(users[*user_count].name) - 1);
    users[*user_count].name[sizeof(users[*user_count].name) - 1] = '\0';

    (*user_count)++;

    if (!save_users(users, *user_count)) {
        (*user_count)--;
        return 0;
    }
    return 1;
}
