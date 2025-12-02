#pragma once

typedef struct {
    int  account_number;
    char type[16];   // deposit / withdraw
    int  amount;
    char date[16];   // YYYY-MM-DD
} Transaction;

int append_transaction(const Transaction* t);

// 페이지네이션 포함 TUI 출력
void show_transactions_tui(int account_number);

// 계좌 삭제 시 거래내역 정리(옵션)
int purge_transactions_for_account(int account_number);
