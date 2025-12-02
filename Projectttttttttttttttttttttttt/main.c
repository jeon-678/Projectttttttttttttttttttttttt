#pragma execution_character_set("utf-8")
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>

#include "util.h"
#include "user.h"
#include "account.h"
#include "transaction.h"

User users[100];
int user_count = 0;

Account accounts[100];
int account_count = 0;

static void toast_at(int x, int y, int width, const char* msg) {
    gotoxy(x, y);
    printf("%-*s", width, "");
    gotoxy(x, y);
    printf("%.*s", width, msg);
}

static int load_accounts_or_toast(void) {
    if (!load_accounts(accounts, &account_count)) {
        toast_at(3, 21, 70, "오류: accounts.txt 로드 실패");
        pause_enter_at(3, 22);
        return 0;
    }
    return 1;
}

static void show_user_menu(int user_index) {
    while (1) {
        clear_screen();
        draw_box(1, 1, 80, 27);

        gotoxy(3, 3);  printf("사용자: %s (%s)", users[user_index].name, users[user_index].id);
        gotoxy(3, 5);  printf("1. 내 계좌 목록 보기");
        gotoxy(3, 6);  printf("2. 계좌 개설");
        gotoxy(3, 7);  printf("3. 입금");
        gotoxy(3, 8);  printf("4. 출금");
        gotoxy(3, 9);  printf("5. 거래 내역 보기");
        gotoxy(3, 10);  printf("6. 계좌 삭제 (잔액 0만)");
        gotoxy(3, 11);  printf("7. 로그아웃");
        gotoxy(3, 13); printf("메뉴 선택: ");

        int menu;
        if (!read_int_range(&menu, 1, 7)) {
            toast_at(3, 15, 70, "입력 오류: 1~7 숫자만 입력하세요.");
            pause_enter_at(3, 16);
            continue;
        }

        if (menu == 1) {
            if (!load_accounts_or_toast()) continue;
            tui_print_user_accounts(accounts, account_count, users[user_index].id);
            pause_enter_at(3, 23);
        }
        else if (menu == 2) {
            if (!load_accounts_or_toast()) continue;
            if (!create_account_tui(accounts, &account_count, users[user_index].id)) {
                toast_at(3, 21, 70, "계좌 개설 실패(최대 개수 초과 등).");
                pause_enter_at(3, 22);
                continue;
            }
            if (!save_accounts(accounts, account_count)) {
                toast_at(3, 21, 70, "오류: accounts.txt 저장 실패");
                pause_enter_at(3, 22);
                continue;
            }
            toast_at(3, 21, 70, "계좌 개설 완료!");
            pause_enter_at(3, 22);
        }
        else if (menu == 3) { // 입금
            if (!load_accounts_or_toast()) continue;

            int idx = select_account_index_for_user_tui(accounts, account_count, users[user_index].id);
            if (idx < 0) continue;

            int amount;
            gotoxy(3, 20); printf("입금 금액(양의 정수): ");
            if (!read_positive_int(&amount)) {
                toast_at(3, 21, 70, "입력 오류: 양의 정수만 가능합니다.");
                pause_enter_at(3, 22);
                continue;
            }

            if (!deposit(accounts, account_count, idx, amount)) {
                toast_at(3, 21, 70, "입금 실패(내부 오류).");
                pause_enter_at(3, 22);
                continue;
            }
            if (!save_accounts(accounts, account_count)) {
                toast_at(3, 21, 70, "오류: accounts.txt 저장 실패");
                pause_enter_at(3, 22);
                continue;
            }

            Transaction t;
            t.account_number = accounts[idx].account_number;
            strcpy(t.type, "deposit");
            t.amount = amount;
            get_today_date(t.date, sizeof(t.date));
            if (!append_transaction(&t)) {
                toast_at(3, 21, 70, "경고: 거래내역 기록 실패(transactions.txt).");
            }
            else {
                toast_at(3, 21, 70, "입금 완료!");
            }
            pause_enter_at(3, 22);
        }
        else if (menu == 4) {  // 출금
            if (!load_accounts_or_toast()) continue;

            int idx = select_account_index_for_user_tui(
                accounts, account_count, users[user_index].id);
            if (idx < 0) continue;

            int bal = accounts[idx].balance;
            if (bal <= 0) {
                gotoxy(3, 20); printf("잔액이 0이어서 출금할 수 없습니다.");
                pause_enter_at(3, 21);
                continue;
            }

            gotoxy(3, 18); printf("현재 잔액: %d", bal);
            gotoxy(3, 20); printf("1) 금액 입력 출금");
            gotoxy(3, 21); printf("2) 전액 출금(잔액 0 만들기)");
            gotoxy(3, 22); printf("0) 취소");
            gotoxy(3, 23); printf("선택: ");

            int sel;
            if (!read_int_range(&sel, 0, 2)) {
                gotoxy(3, 24); printf("입력 오류: 0~2만 가능합니다.");
                pause_enter_at(3, 24);
                continue;
            }
            if (sel == 0) continue;

            int amount = 0;

            if (sel == 2) {
                amount = bal;
            }
            else {
                gotoxy(35, 23); printf("출금 금액(1~%d): ", bal);
                if (!read_int_range(&amount, 1, bal)) {
                    gotoxy(3, 24); printf("범위 오류: 1~%d 사이로 입력하세요.", bal);
                    pause_enter_at(3, 24);
                    continue;
                }
            }

            if (!withdraw(accounts, account_count, idx, amount)) {
                gotoxy(3, 24); printf("출금 실패(잔액 부족/내부 오류).");
                pause_enter_at(3, 24);
                continue;
            }

            if (!save_accounts(accounts, account_count)) {
                gotoxy(3, 24); printf("오류: accounts.txt 저장 실패");
                pause_enter_at(3, 24);
                continue;
            }

            Transaction t;
            t.account_number = accounts[idx].account_number;
            strcpy(t.type, "withdraw");
            t.amount = amount;
            get_today_date(t.date, sizeof(t.date));
            append_transaction(&t);

            gotoxy(3, 24); printf("출금 완료! (출금액: %d, 남은잔액: %d)", amount, accounts[idx].balance);
            pause_enter_at(3, 25);
        }

        else if (menu == 5) {
            if (!load_accounts_or_toast()) continue;
            int idx = select_account_index_for_user_tui(accounts, account_count, users[user_index].id);
            if (idx < 0) continue;
            show_transactions_tui(accounts[idx].account_number);
        }
        else if (menu == 6) { // 삭제
            if (!load_accounts_or_toast()) continue;
            int idx = select_account_index_for_user_tui(accounts, account_count, users[user_index].id);
            if (idx < 0) continue;

            if (accounts[idx].balance != 0) {
                gotoxy(3, 20); printf("삭제 불가: 잔액이 0이 아닙니다. (잔액: %d)", accounts[idx].balance);
                pause_enter_at(3, 21);
                continue;
            }

            int accno = accounts[idx].account_number;
            if (!confirm_yesno_at(3, 20, "정말 계좌를 삭제할까요? (1=예, 2=아니오): ")) {
                toast_at(3, 21, 70, "삭제 취소.");
                pause_enter_at(3, 22);
                continue;
            }

            if (!delete_account(accounts, &account_count, idx)) {
                toast_at(3, 21, 70, "삭제 실패(내부 오류).");
                pause_enter_at(3, 22);
                continue;
            }
            if (!save_accounts(accounts, account_count)) {
                toast_at(3, 21, 70, "오류: accounts.txt 저장 실패");
                pause_enter_at(3, 22);
                continue;
            }

            // (선택) 거래내역도 지우고 싶으면 여기서 물어보기/호출 추가 가능
            toast_at(3, 21, 70, "계좌 삭제 완료!");
            pause_enter_at(3, 22);
        }
        else {
            toast_at(3, 15, 70, "로그아웃합니다.");
            pause_enter_at(3, 16);
            break;
        }
    }
}

static void show_main_menu(void) {
    while (1) {
        clear_screen();
        draw_box(1, 1, 60, 15);

        gotoxy(3, 3);  printf("계좌 관리 시스템 (TUI)");
        gotoxy(3, 5);  printf("1. 로그인");
        gotoxy(3, 6);  printf("2. 회원가입");
        gotoxy(3, 7);  printf("3. 종료");
        gotoxy(3, 9);  printf("메뉴 선택: ");

        int menu;
        if (!read_int_range(&menu, 1, 3)) {
            toast_at(3, 11, 55, "입력 오류: 1~3 숫자만 입력하세요.");
            pause_enter_at(3, 12);
            continue;
        }

        if (menu == 1) {
            int idx = login_tui(users, user_count);
            if (idx >= 0) show_user_menu(idx);
            else {
                toast_at(3, 11, 55, "로그인 실패.");
                pause_enter_at(3, 12);
            }
        }
        else if (menu == 2) {
            if (!signup_tui(users, &user_count)) toast_at(3, 11, 55, "회원가입 실패.");
            else toast_at(3, 11, 55, "회원가입 완료!");
            pause_enter_at(3, 12);
        }
        else {
            toast_at(3, 11, 55, "프로그램을 종료합니다.");
            pause_enter_at(3, 12);
            break;
        }
    }
}

int main(void) {
    set_utf8_console();
    load_users(users, &user_count); // 없으면 0명 시작
    show_main_menu();
    return 0;
}
