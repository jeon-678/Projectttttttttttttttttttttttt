#define _CRT_SECURE_NO_WARNINGS
#pragma execution_character_set("utf-8")

#include "transaction.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

#define TX_FILE "transactions.txt"
#define TX_TMP  "transactions.tmp"
#define MAX_TX  2000

int append_transaction(const Transaction* t) {
    if (!t) return 0;

    FILE* fp = fopen(TX_FILE, "a");
    if (!fp) return 0;

    fprintf(fp, "%d\t%s\t%d\t%s\n",
        t->account_number, t->type, t->amount, t->date);

    fclose(fp);
    return 1;
}

static int load_transactions_for_account(int accno, Transaction out[], int maxn) {
    FILE* fp = fopen(TX_FILE, "r");
    if (!fp) return 0;

    char line[256];
    int cnt = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (cnt >= maxn) break;

        char buf[256];
        strncpy(buf, line, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        char* accs = strtok(buf, "\t");
        char* type = strtok(NULL, "\t");
        char* amts = strtok(NULL, "\t");
        char* date = strtok(NULL, "\n");
        if (!accs || !type || !amts || !date) continue;

        int no = atoi(accs);
        if (no != accno) continue;

        out[cnt].account_number = no;
        strncpy(out[cnt].type, type, sizeof(out[cnt].type) - 1);
        out[cnt].type[sizeof(out[cnt].type) - 1] = '\0';
        out[cnt].amount = atoi(amts);
        strncpy(out[cnt].date, date, sizeof(out[cnt].date) - 1);
        out[cnt].date[sizeof(out[cnt].date) - 1] = '\0';

        cnt++;
    }

    fclose(fp);
    return cnt;
}

static char first_char_or_enter(void) {
    char buf[32];
    if (!read_line(buf, sizeof(buf))) return '\0';
    if (buf[0] == '\0') return '\0';
    // 대문자화
    if (buf[0] >= 'a' && buf[0] <= 'z') buf[0] = (char)(buf[0] - 'a' + 'A');
    return buf[0];
}

void show_transactions_tui(int account_number) {
    Transaction tx[MAX_TX];
    int cnt = load_transactions_for_account(account_number, tx, MAX_TX);

    int page = 0;
    const int per_page = 14; // 헤더/푸터 감안

    while (1) {
        clear_screen();
        draw_box(1, 1, 80, 25);
        gotoxy(3, 3); printf("거래 내역 (계좌: %d)", account_number);

        if (cnt <= 0) {
            gotoxy(3, 7); printf("거래 내역이 없습니다.");
            pause_enter_at(3, 23);
            return;
        }

        int total_pages = (cnt + per_page - 1) / per_page;
        if (page < 0) page = 0;
        if (page >= total_pages) page = total_pages - 1;

        int start = page * per_page;
        int end = start + per_page;
        if (end > cnt) end = cnt;

        int y = 5;
        gotoxy(3, y++); printf("번호  날짜         구분  금액");
        gotoxy(3, y++); printf("------------------------------------------");

        for (int i = start; i < end; i++) {
            const char* ktype = (strcmp(tx[i].type, "deposit") == 0) ? "입금" :
                (strcmp(tx[i].type, "withdraw") == 0) ? "출금" : tx[i].type;
            gotoxy(3, y++);
            printf("%-5d %-12s %-4s %d", i + 1, tx[i].date, ktype, tx[i].amount);
        }

        gotoxy(3, 22);
        printf("페이지 %d/%d  (N=다음, P=이전, Enter=나가기): ", page + 1, total_pages);

        char c = first_char_or_enter();
        if (c == '\0') return;
        if (c == 'N') {
            if (page + 1 < total_pages) page++;
        }
        else if (c == 'P') {
            if (page > 0) page--;
        }
    }
}

int purge_transactions_for_account(int account_number) {
    FILE* in = fopen(TX_FILE, "r");
    if (!in) {
        return 1;
    }
    FILE* out = fopen(TX_TMP, "w");
    if (!out) {
        fclose(in);
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), in)) {
        char buf[256];
        strncpy(buf, line, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        char* accs = strtok(buf, "\t");
        if (!accs) continue;

        int no = atoi(accs);
        if (no == account_number) continue;
        fputs(line, out);
    }

    fclose(in);
    fclose(out);

    return replace_file(TX_TMP, TX_FILE);
}
