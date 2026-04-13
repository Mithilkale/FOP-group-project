#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ACCOUNTS 100
#define MAX_NAME 100
#define MAX_TRANSACTIONS 1000

typedef struct {
    int account_number;
    char name[MAX_NAME];
    double balance;
    char type[20];
    char created_at[30];
} Account;

typedef struct {
    int account_number;
    char type[20]; // DEPOSIT or WITHDRAW
    double amount;
    char timestamp[30];
} Transaction;

Account accounts[MAX_ACCOUNTS];
Transaction transactions[MAX_TRANSACTIONS];
int account_count = 0;
int transaction_count = 0;
int next_account_number = 1001;

void get_timestamp(char *buffer) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", tm_info);
}

void save_accounts() {
    FILE *fp = fopen("accounts.dat", "wb");
    if (fp) {
        fwrite(&account_count, sizeof(int), 1, fp);
        fwrite(&next_account_number, sizeof(int), 1, fp);
        fwrite(accounts, sizeof(Account), account_count, fp);
        fwrite(&transaction_count, sizeof(int), 1, fp);
        fwrite(transactions, sizeof(Transaction), transaction_count, fp);
        fclose(fp);
    }
}

void load_accounts() {
    FILE *fp = fopen("accounts.dat", "rb");
    if (fp) {
        fread(&account_count, sizeof(int), 1, fp);
        fread(&next_account_number, sizeof(int), 1, fp);
        fread(accounts, sizeof(Account), account_count, fp);
        fread(&transaction_count, sizeof(int), 1, fp);
        fread(transactions, sizeof(Transaction), transaction_count, fp);
        fclose(fp);
    }
}

void add_transaction(int acc_num, const char *type, double amount) {
    if (transaction_count < MAX_TRANSACTIONS) {
        transactions[transaction_count].account_number = acc_num;
        strcpy(transactions[transaction_count].type, type);
        transactions[transaction_count].amount = amount;
        get_timestamp(transactions[transaction_count].timestamp);
        transaction_count++;
        save_accounts();
    }
}

void create_account(const char *name, double initial_balance, const char *type) {
    if (account_count >= MAX_ACCOUNTS) {
        printf("ERROR: Maximum accounts reached\n");
        return;
    }
    
    accounts[account_count].account_number = next_account_number;
    strncpy(accounts[account_count].name, name, MAX_NAME - 1);
    accounts[account_count].balance = initial_balance;
    strncpy(accounts[account_count].type, type, 19);
    get_timestamp(accounts[account_count].created_at);
    
    account_count++;
    next_account_number++;
    
    if (initial_balance > 0) {
        add_transaction(accounts[account_count - 1].account_number, "DEPOSIT", initial_balance);
    }
    
    save_accounts();
    printf("SUCCESS: Account created with number %d\n", accounts[account_count - 1].account_number);
}

void deposit(int account_number, double amount) {
    for (int i = 0; i < account_count; i++) {
        if (accounts[i].account_number == account_number) {
            accounts[i].balance += amount;
            add_transaction(account_number, "DEPOSIT", amount);
            printf("SUCCESS: Deposited %.2f. New balance: %.2f\n", amount, accounts[i].balance);
            return;
        }
    }
    printf("ERROR: Account not found\n");
}

void withdraw(int account_number, double amount) {
    for (int i = 0; i < account_count; i++) {
        if (accounts[i].account_number == account_number) {
            if (accounts[i].balance >= amount) {
                accounts[i].balance -= amount;
                add_transaction(account_number, "WITHDRAW", amount);
                printf("SUCCESS: Withdrew %.2f. New balance: %.2f\n", amount, accounts[i].balance);
            } else {
                printf("ERROR: Insufficient funds. Current balance: %.2f\n", accounts[i].balance);
            }
            return;
        }
    }
    printf("ERROR: Account not found\n");
}

void delete_account(int account_number) {
    for (int i = 0; i < account_count; i++) {
        if (accounts[i].account_number == account_number) {
            // Withdraw all balance first
            if (accounts[i].balance > 0) {
                add_transaction(account_number, "WITHDRAW", accounts[i].balance);
            }
            // Shift all accounts after this one
            for (int j = i; j < account_count - 1; j++) {
                accounts[j] = accounts[j + 1];
            }
            account_count--;
            save_accounts();
            printf("SUCCESS: Account %d deleted\n", account_number);
            return;
        }
    }
    printf("ERROR: Account not found\n");
}

void get_balance(int account_number) {
    for (int i = 0; i < account_count; i++) {
        if (accounts[i].account_number == account_number) {
            printf("BALANCE: %.2f\n", accounts[i].balance);
            return;
        }
    }
    printf("ERROR: Account not found\n");
}

void get_account(int account_number) {
    for (int i = 0; i < account_count; i++) {
        if (accounts[i].account_number == account_number) {
            printf("ACCOUNT,%d,%s,%.2f,%s,%s\n", 
                   accounts[i].account_number,
                   accounts[i].name,
                   accounts[i].balance,
                   accounts[i].type,
                   accounts[i].created_at);
            return;
        }
    }
    printf("ERROR: Account not found\n");
}

void list_accounts() {
    if (account_count == 0) {
        printf("NO_ACCOUNTS\n");
        return;
    }
    printf("ACCOUNTS,%d\n", account_count);
    for (int i = 0; i < account_count; i++) {
        printf("%d,%s,%.2f,%s\n", 
               accounts[i].account_number,
               accounts[i].name,
               accounts[i].balance,
               accounts[i].type);
    }
}

void get_history(int account_number) {
    int count = 0;
    for (int i = 0; i < transaction_count; i++) {
        if (transactions[i].account_number == account_number) {
            count++;
        }
    }
    printf("HISTORY,%d\n", count);
    for (int i = 0; i < transaction_count; i++) {
        if (transactions[i].account_number == account_number) {
            printf("%s,%.2f,%s\n", 
                   transactions[i].type,
                   transactions[i].amount,
                   transactions[i].timestamp);
        }
    }
}

void process_command(char *command) {
    char cmd[20], name[MAX_NAME], type[20];
    int account_number;
    double amount;
    
    if (strncmp(command, "CREATE,", 7) == 0) {
        char *p = command + 7;
        char *name_start = p;
        char *comma1 = strchr(p, ',');
        if (!comma1) {
            printf("ERROR: Invalid command format\n");
            return;
        }
        *comma1 = '\0';
        char *balance_start = comma1 + 1;
        char *comma2 = strchr(balance_start, ',');
        if (!comma2) {
            printf("ERROR: Invalid command format\n");
            return;
        }
        *comma2 = '\0';
        strcpy(name, name_start);
        amount = atof(balance_start);
        strcpy(type, comma2 + 1);
        
        // Trim whitespace from name
        char *end = name + strlen(name) - 1;
        while (end > name && *end == ' ') end--;
        *(end + 1) = '\0';
        
        create_account(name, amount, type);
    } 
    else if (sscanf(command, "DELETE,%d", &account_number) == 1) {
        delete_account(account_number);
    }
    else if (sscanf(command, "DEPOSIT,%d,%lf", &account_number, &amount) == 2) {
        deposit(account_number, amount);
    }
    else if (sscanf(command, "WITHDRAW,%d,%lf", &account_number, &amount) == 2) {
        withdraw(account_number, amount);
    }
    else if (sscanf(command, "BALANCE,%d", &account_number) == 1) {
        get_balance(account_number);
    }
    else if (sscanf(command, "GET,%d", &account_number) == 1) {
        get_account(account_number);
    }
    else if (strcmp(command, "LIST") == 0) {
        list_accounts();
    }
    else if (sscanf(command, "HISTORY,%d", &account_number) == 1) {
        get_history(account_number);
    }
    else {
        printf("ERROR: Unknown command\n");
    }
}

int main() {
    load_accounts();
    
    char command[500];
    while (fgets(command, sizeof(command), stdin) != NULL) {
        // Remove newline
        command[strcspn(command, "\n")] = 0;
        process_command(command);
        fflush(stdout);
    }
    
    return 0;
}

