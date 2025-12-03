#pragma once

typedef struct {
    int  account_number;
    char type[16];
    int  amount;
    char date[16];
} Transaction;

int append_transaction(const Transaction* t);

void show_transactions_tui(int account_number);

int purge_transactions_for_account(int account_number);
