#pragma once

typedef struct {
    int  account_number;
    char user_id[20];
    int  balance;
} Account;

int load_accounts(Account accounts[], int* account_count);
int save_accounts(Account accounts[], int account_count);

void tui_print_user_accounts(Account accounts[], int account_count, const char* user_id);
int  select_account_index_for_user_tui(Account accounts[], int account_count, const char* user_id);

int  create_account_tui(Account accounts[], int* account_count, const char* user_id);

int  deposit(Account accounts[], int account_count, int account_index, int amount);
int  withdraw(Account accounts[], int account_count, int account_index, int amount);

int  delete_account(Account accounts[], int* account_count, int account_index);
