#pragma execution_character_set("utf-8")
#define _CRT_SECURE_NO_WARNINGS
#include "account.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

#define ACCOUNTS_FILE "accounts.txt"
#define ACCOUNTS_TMP  "accounts.tmp"
#define MAX_ACCOUNTS  100

static int file_existsA(const char* path) {
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

static int account_number_exists(Account accounts[], int account_count, int accno) {
    for (int i = 0; i < account_count; i++) {
        if (accounts[i].account_number == accno) return 1;
    }
    return 0;
}

static int generate_unique_account_number(Account accounts[], int account_count) {
    int max = 10000000;
    for (int i = 0; i < account_count; i++) {
        if (accounts[i].account_number > max) max = accounts[i].account_number;
    }

    int cand = max + 1;
    while (account_number_exists(accounts, account_count, cand)) cand++;
    return cand;
}

int load_accounts(Account accounts[], int* account_count) {
    if (!accounts || !account_count) return 0;

    FILE* fp = fopen(ACCOUNTS_FILE, "r");
    if (!fp) {
        if (!file_existsA(ACCOUNTS_FILE)) {
            *account_count = 0;
            return 1;
        }
        return 0;
    }

    char line[256];
    int cnt = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (cnt >= MAX_ACCOUNTS) break;

        char* accno = strtok(line, "\t");
        char* uid = strtok(NULL, "\t");
        char* bal = strtok(NULL, "\n");
        if (!accno || !uid || !bal) continue;

        accounts[cnt].account_number = atoi(accno);
        strncpy(accounts[cnt].user_id, uid, sizeof(accounts[cnt].user_id) - 1);
        accounts[cnt].user_id[sizeof(accounts[cnt].user_id) - 1] = '\0';
        accounts[cnt].balance = atoi(bal);

        cnt++;
    }

    fclose(fp);
    *account_count = cnt;
    return 1;
}

int save_accounts(Account accounts[], int account_count) {
    FILE* fp = fopen(ACCOUNTS_TMP, "w");
    if (!fp) return 0;

    for (int i = 0; i < account_count; i++) {
        fprintf(fp, "%d\t%s\t%d\n",
            accounts[i].account_number,
            accounts[i].user_id,
            accounts[i].balance);
    }
    fclose(fp);

    return replace_file(ACCOUNTS_TMP, ACCOUNTS_FILE);
}

void tui_print_user_accounts(Account accounts[], int account_count, const char* user_id) {
    clear_screen();
    draw_box(1, 1, 80, 25);
    gotoxy(3, 3); printf("내 계좌 목록");

    int y = 5;
    gotoxy(3, y++); printf("번호   계좌번호          잔액");
    gotoxy(3, y++); printf("----------------------------------------");

    int idx = 1;
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].user_id, user_id) != 0) continue;
        gotoxy(3, y++);
        printf("%-6d %-16d %d", idx, accounts[i].account_number, accounts[i].balance);
        idx++;
        if (y >= 22) break;
    }

    if (idx == 1) {
        gotoxy(3, 9); printf("보유한 계좌가 없습니다.");
    }
}

int select_account_index_for_user_tui(Account accounts[], int account_count, const char* user_id) {
    clear_screen();
    draw_box(1, 1, 80, 25);
    gotoxy(3, 3); printf("계좌 선택 (0 입력 시 취소)");

    int map[100];
    int map_count = 0;

    int y = 5;
    gotoxy(3, y++); printf("번호   계좌번호          잔액");
    gotoxy(3, y++); printf("----------------------------------------");

    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].user_id, user_id) != 0) continue;

        map[map_count] = i;
        gotoxy(3, y++);
        printf("%-6d %-16d %d", map_count + 1, accounts[i].account_number, accounts[i].balance);

        map_count++;
        if (y >= 21) break;
    }

    if (map_count == 0) {
        gotoxy(3, 10); printf("선택할 계좌가 없습니다.");
        pause_enter_at(3, 12);
        return -1;
    }

    gotoxy(3, 22); printf("선택 번호: ");
    int sel;
    if (!read_int(&sel)) {
        gotoxy(3, 23); printf("입력 오류.");
        pause_enter_at(3, 24);
        return -1;
    }
    if (sel == 0) return -1;
    if (sel < 1 || sel > map_count) {
        gotoxy(3, 23); printf("범위 오류 (1~%d).", map_count);
        pause_enter_at(3, 24);
        return -1;
    }
    return map[sel - 1];
}

int create_account_tui(Account accounts[], int* account_count, const char* user_id) {
    if (!accounts || !account_count || !user_id) return 0;
    if (*account_count >= MAX_ACCOUNTS) return 0;

    clear_screen();
    draw_box(1, 1, 80, 18);
    gotoxy(3, 3); printf("계좌 개설");

    int new_no = generate_unique_account_number(accounts, *account_count);

    accounts[*account_count].account_number = new_no;
    strncpy(accounts[*account_count].user_id, user_id, sizeof(accounts[*account_count].user_id) - 1);
    accounts[*account_count].user_id[sizeof(accounts[*account_count].user_id) - 1] = '\0';
    accounts[*account_count].balance = 0;
    (*account_count)++;

    gotoxy(3, 6); printf("새 계좌번호: %d", new_no);
    gotoxy(3, 7); printf("초기 잔액: 0");
    return 1;
}

int deposit(Account accounts[], int account_count, int account_index, int amount) {
    if (!accounts) return 0;
    if (account_index < 0 || account_index >= account_count) return 0;
    if (amount <= 0) return 0;
    accounts[account_index].balance += amount;
    return 1;
}

int withdraw(Account accounts[], int account_count, int account_index, int amount) {
    if (!accounts) return 0;
    if (account_index < 0 || account_index >= account_count) return 0;
    if (amount <= 0) return 0;

    // ✅ 전액 출금 허용: "<" 로 비교해야 함
    if (accounts[account_index].balance < amount) return 0;

    accounts[account_index].balance -= amount;
    return 1;
}

int delete_account(Account accounts[], int* account_count, int account_index) {
    if (!accounts || !account_count) return 0;
    if (account_index < 0 || account_index >= *account_count) return 0;

    for (int i = account_index; i < (*account_count) - 1; i++) {
        accounts[i] = accounts[i + 1];
    }
    (*account_count)--;
    return 1;
}
