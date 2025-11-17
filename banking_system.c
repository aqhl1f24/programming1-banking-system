#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

struct Account
{
    char acc_number[10];
    char name[100];
    char id[20];
    char acc_type[10];
    char pin[5];
    double balance;
    char created_at[30];
};

int count_acc()
{
    FILE *index_file = fopen("database/index.txt", "r");
    if (index_file == NULL)
    {
        return 0;
    }
    int count = 0;
    char line[100];

    while (fgets(line, sizeof(line), index_file))
    {
        line[strcspn(line, "\n")] = 0; // Remove newline character
        if (strlen(line) > 0)
        count++;
    }
    fclose(index_file);
    return count;
}

bool checkinput(char input[])
{
    const char *valid_inputs[] = {"1", "2", "3", "4", "5", "6", "create", "delete", "deposit", "withdraw", "remit", "exit"};

    for (int i = 0; input[i]; i++)
    {
        input[i] = tolower(input[i]);
    }

    for (int i = 0; i < sizeof(valid_inputs) / sizeof(valid_inputs[0]); i++)
    {
        if (strcmp(input, valid_inputs[i]) == 0)
        {
            return true;
        }
    }
    return false;
}

bool checkname(char *name)
{
    if (strlen(name) == 0)
    {
        return false;
    }

    for (int i = 0; name[i] != '\0'; i++)
    {
        if (!isalpha(name[i]) && name[i] != ' ')
        {
            return false;
        }
    }
    return true;
}

bool checkpin(char *pin)
{
    for (int i = 0; i < 4; i++)
    {
        if (!isdigit(pin[i]))
        {
            return false;
        }
    }
    return true;
}

bool checkid(char *id)
{
    for (int i = 0; id[i]; i++)
    {
        if (!isdigit(id[i]))
        {
            return false;
        }
    }
    return true;
}

char *acc_num()
{
    static char acc_num[10];

    while (true)
    {
        int digits = 7 + rand() % 3;
        long min = 1;
        long max = 1;

        for (int i = 0; i < digits - 1; i++)
        {
            min *= 10;
            max *= 10;
        }
        max = max * 10 - 1;
        long random_num = min + rand() % (max - min + 1);
        sprintf(acc_num, "%ld", random_num);

        char filename[100];
        sprintf(filename, "database/%s.txt", acc_num);

        FILE *file = fopen(filename, "r");
        if (file == NULL)
        {
            break;
        }
        fclose(file);
    }
    return acc_num;
}

bool save_acc(struct Account *acc)
{
    char filename[100];
    sprintf(filename, "database/%s.txt", acc->acc_number);

    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        return false;
    }

    fprintf(file, "Account Number: %s\n", acc->acc_number);
    fprintf(file, "Name: %s\n", acc->name);
    fprintf(file, "ID: %s\n", acc->id);
    fprintf(file, "Account Type: %s\n", acc->acc_type);
    fprintf(file, "PIN: %s\n", acc->pin);
    fprintf(file, "Balance: %.2f\n", acc->balance);
    fprintf(file, "Created At: %s\n", acc->created_at);
    fclose(file);

    return true;
}

void log_action(char *action, char *details)
{
    time_t now = time(NULL);
    char timestamp[50];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    FILE *logfile = fopen("database/transaction.log", "a");
    if (logfile != NULL)
    {
        fprintf(logfile, "%s  %s  %s\n", timestamp, action, details);
        fclose(logfile);
    }
}

bool load_account(char *acc_number, struct Account *acc)
{
    char filename[100];
    sprintf(filename, "database/%s.txt", acc_number);

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return false;
    }

    char line[200];
    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "Account Number:", 15) == 0)
        {
            sscanf(line + 16, " %9s", acc->acc_number);
        }
        else if (strncmp(line, "Name:", 5) == 0)
        {
            sscanf(line + 6, " %99[^\n]", acc->name);
        }
        else if (strncmp(line, "ID:", 3) == 0)
        {
            sscanf(line + 4, " %19s", acc->id);
        }
        else if (strncmp(line, "Account Type:", 13) == 0)
        {
            sscanf(line + 14, " %9s", acc->acc_type);
        }
        else if (strncmp(line, "PIN:", 4) == 0)
        {
            sscanf(line + 5, " %4s", acc->pin);
        }
        else if (strncmp(line, "Balance:", 8) == 0)
        {
            sscanf(line + 9, " %lf", &acc->balance);
        }
        else if (strncmp(line, "Created At:", 11) == 0)
        {
            sscanf(line + 12, " %29[^\n]", acc->created_at);
        }
    }
    fclose(file);
    return true;
}

void update_index_file(char *acc_num, bool addorremoveindex)
{

    if (addorremoveindex == true)
    {
        FILE *index_file = fopen("database/index.txt", "a");
        if (index_file == NULL)
        {
            return;
        }
        else
        {
            fprintf(index_file, "%s\n", acc_num);
            fclose(index_file);
        }
    }
    else
    {
        FILE *index_file = fopen("database/index.txt", "r");
        if (index_file == NULL)
        {
            return;
        }
        // Remove account number from index file
        FILE *temp_file = fopen("database/temp_index.txt", "w");
        if (temp_file == NULL)
        {
            fclose(index_file);
            return;
        }

        char line[100];
        while (fgets(line, sizeof(line), index_file))
        {
            line[strcspn(line, "\n")] = 0; // Remove newline character
            if (strcmp(line, acc_num) != 0)
            {
                fprintf(temp_file, "%s\n", line);
            }
        }

        fclose(index_file);
        fclose(temp_file);

        remove("database/index.txt");
        rename("database/temp_index.txt", "database/index.txt");
    }
}

bool check_if_acc_deletable(char *acc_num, char *id_last4, char *pin)
{
    struct Account acc;
    if (!load_account(acc_num, &acc))
    {
        return false;
    }

    if (strcmp(acc.pin, pin) != 0)
    {
        return false;
    }

    int id_len = strlen(acc.id);
    if (id_len < 4)
    {
        return false;
    }

    char id_last4_actual[5];
    strncpy(id_last4_actual, acc.id + id_len - 4, 4);
    id_last4_actual[4] = '\0';

    if (strcmp(id_last4_actual, id_last4) != 0)
    {
        return false;
    }

    return true;
}

bool update_balance(char *acc_num, double new_balance)
{
    char filename[100];
    sprintf(filename, "database/%s.txt", acc_num);

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return false;
    }

    char temp_filename[100];
    sprintf(temp_filename, "database/temp_%s.txt", acc_num);
    FILE *temp_file = fopen(temp_filename, "w");
    if (temp_file == NULL)
    {
        fclose(file);
        return false;
    }

    char line[200];
    bool balance_updated = false;
    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "Balance:", 8) == 0)
        {
            fprintf(temp_file, "Balance: %.2f\n", new_balance);
            balance_updated = true;
        }
        else
        {
            fputs(line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (balance_updated == false)
    {
        remove(temp_filename);
        return false;
    }

    remove(filename);
    rename(temp_filename, filename);
    return true;
}

void create_account()
{
    struct Account new_acc;

    while (true)
    {
        printf("Enter account name: ");
        scanf("%99[^\n]", new_acc.name);
        while ((getchar()) != '\n');

        if (checkname(new_acc.name))
        {
            break;
        }
        else
        {
            printf("Name should only contain letters and spaces and cannot be empty. Please try again.\n");
        }
    }

    while (true)
    {
        printf("Enter account ID: ");
        scanf("%19s", new_acc.id);
        while ((getchar()) != '\n');

        if (checkid(new_acc.id))
        {
            break;
        }
        else
        {
            printf("ID cannot be empty.\n");
        }
    }

    while (true)
    {
        printf("Select your account type:\n");
        printf("1. Savings\n");
        printf("2. Current\n");
        printf("Please select an option: ");
        scanf("%9s", new_acc.acc_type);
        while ((getchar()) != '\n');

        for (int i = 0; new_acc.acc_type[i]; i++)
        {
            new_acc.acc_type[i] = tolower(new_acc.acc_type[i]);
        }

        if (strcmp(new_acc.acc_type, "1") == 0 || strcmp(new_acc.acc_type, "savings") == 0)
        {
            strcpy(new_acc.acc_type, "savings");
            break;
        }
        else if (strcmp(new_acc.acc_type, "2") == 0 || strcmp(new_acc.acc_type, "current") == 0)
        {
            strcpy(new_acc.acc_type, "current");
            break;
        }
        else
        {
            printf("Invalid account type. Please choose again.\n");
        }
    }

    while (true)
    {
        char pin_input[10];
        printf("Set a 4 digit pin for your account: ");

        if (scanf("%9s", pin_input) != 1)
        {
            while ((getchar()) != '\n');
            printf("Input error. Please try again.\n");
            continue;
        }
        while ((getchar()) != '\n');

        if (strlen(pin_input) != 4)
        {
            printf("Invalid PIN format. Please try again.\n");
            continue;
        }

        if (checkpin(pin_input))
        {
            strcpy(new_acc.pin, pin_input);
            break;
        }
        else
        {
            printf("Invalid PIN format. Please try again.\n");
        }
    }

    char *account_number = acc_num();
    strcpy(new_acc.acc_number, account_number);

    new_acc.balance = 0.0;

    time_t now = time(NULL);
    strftime(new_acc.created_at, sizeof(new_acc.created_at), "%Y-%m-%d %H:%M:%S", localtime(&now));

    if (save_acc(&new_acc))
    {
        update_index_file(new_acc.acc_number, true);
        printf("Account created successfully!\n");
        printf("Your account number is: %s\n", new_acc.acc_number);
        printf("Your account type is: %s\n", new_acc.acc_type);
        printf("Your initial balance is: %.2f\n", new_acc.balance);

        char log[500];
        sprintf(log, "Account %s created for %s with ID %s", new_acc.acc_number, new_acc.name, new_acc.id);
        log_action("CREATE ACCOUNT", log);
    }
    else
    {
        printf("There is an error when creating your account. Please try again.\n");
    }
}

void delete_account()
{
    char acc_num[15];
    char id_last4[5];
    char pin[10];

    printf("Enter your account number: ");
    if (scanf("%14s", acc_num) != 1 || strlen(acc_num) < 7 || strlen(acc_num) > 9)
    {
        while ((getchar()) != '\n');
        printf("Invalid account number. Please try again.\n");
        return;
    }
    while ((getchar()) != '\n');

    printf("Enter the last 4 digits of your ID: ");
    if (scanf("%9s", id_last4) != 1 || strlen(id_last4) != 4 || !checkid(id_last4))
    {
        while ((getchar()) != '\n');
        printf("Invalid last 4 digits of IDPlease try again.\n");
        return;
    }

    printf("Enter your account PIN: ");
    if (scanf("%9s", pin) != 1 || strlen(pin) != 4 || !checkpin(pin))
    {
        while ((getchar()) != '\n');
        printf("Invalid PIN. Please try again.\n");
        return;
    }
    while ((getchar()) != '\n');

    if (check_if_acc_deletable(acc_num, id_last4, pin))
    {
        char filename[100];
        sprintf(filename, "database/%s.txt", acc_num);

        if (remove(filename) == 0)
        {
            printf("Account %s deleted successfully.\n", acc_num);
            update_index_file(acc_num, false);
            char log[500];
            sprintf(log, "Account %s deleted", acc_num);
            log_action("DELETE ACCOUNT", log);
        }
        else
        {
            printf("Error deleting account %s. Please try again.\n", acc_num);
        }
    }
    else
    {
        printf("Account deletion failed. Please check your details and try again.\n");
    }
}

void deposit_funds()
{
    char acc_num[15];
    char pin[10];

    printf("Enter your account number: ");
    if (scanf("%14s", acc_num) != 1 || strlen(acc_num) < 7 || strlen(acc_num) > 9)
    {
        while ((getchar()) != '\n');
        printf("Invalid account number. Please try again.\n");
        return;
    }
    while ((getchar()) != '\n');

    printf("Enter your account PIN: ");
    if (scanf("%9s", pin) != 1 || strlen(pin) != 4 || !checkpin(pin))
    {
        while ((getchar()) != '\n');
        printf("Invalid PIN. Please try again.\n");
        return;
    }
    while ((getchar()) != '\n');

    struct Account acc;
    if (load_account(acc_num, &acc))
    {
        if (strcmp(acc.pin, pin) == 0)
        {
            printf("Current balance: %.2f\n", acc.balance);

            double amount;
            int check_if_numeric;
            while (true)
            {
                printf("Enter the amount you want to deposit (minimum 0.01 and maximum 50000.00): ");
                check_if_numeric = scanf("%lf", &amount);
                while ((getchar()) != '\n');
    

                if (check_if_numeric != 1)
                {
                    printf("Please enter a numeric value only.\n");
                    continue;
                }
                else if (amount < 0.01 || amount > 50000.00)
                {
                    printf("Please enter an amount between 0.01 and 50000.00 only.\n");
                    continue;
                }
                else
                {
                    break;
                }
            }
            acc.balance += amount;
            if (update_balance(acc_num, acc.balance))
            {
                printf("Deposit successful! New balance: RM%.2f\n", acc.balance);
                char log[500];
                sprintf(log, "Deposited %.2f to account %s", amount, acc_num);
                log_action("DEPOSIT FUNDS", log);
            }
            else
            {
                printf("Error updating balance. Please try again.\n");
            }
        }
        else
        {
            printf("Invalid PIN. Please try again.\n");
        }
    }
    else
    {
        printf("Account not found. Please check your account number and try again.\n");
    }
}

void withdrawal()
{
    char acc_num[15];
    char pin[10];
    printf("Enter your account number: ");
    if (scanf("%14s", acc_num) != 1 || strlen(acc_num) < 7 || strlen(acc_num) > 9)
    {
        while ((getchar()) != '\n');
        printf("Invalid account number. Please try again.\n");
        return;
    }
    while ((getchar()) != '\n');

    printf("Enter your account PIN: ");
    if (scanf("%9s", pin) != 1 || strlen(pin) != 4 || !checkpin(pin))
    {
        while ((getchar()) != '\n');
        printf("Invalid PIN. Please try again.\n");
        return;
    }
    while ((getchar()) != '\n');

    struct Account acc;

    if (load_account(acc_num, &acc))
    {
        if (strcmp(acc.pin, pin) == 0)
        {
            printf("Current balance: RM%.2f\n", acc.balance);

            double amount;
            int check_if_numeric;
            while (true)
            {
                printf("Enter the amount you want to withdraw: ");
                check_if_numeric = scanf("%lf", &amount);
                while ((getchar()) != '\n');

                if (check_if_numeric != 1)
                {
                    printf("Please enter a numeric value only.\n");
                    continue;
                }
                else if (amount <= 0.00 || amount > acc.balance)
                {
                    printf("Insufficient funds or invalid amount. Please try again.\n");
                    continue;
                }
                else
                {
                    break;
                }
            }

            acc.balance -= amount;

            if (update_balance(acc_num, acc.balance))
            {
                printf("Withdrawal successful! New balance: RM%.2f\n", acc.balance);
                char log[200];
                sprintf(log, "Withdraw %.2f from account %s", amount, acc_num);
                log_action("WITHDRAW FUNDS", log);
            }
            else
            {
                printf("Error updating balance. Please try again.\n");
            }
        }
        else
        {
            printf("Invalid PIN. Please try again.\n");
        }
    }
    else
    {
        printf("Account not found. Please check your account number and try again.\n");
    }
}

void remittance()
{
    char sender_acc_num[15];
    char receiver_acc_num[15];
    char pin[10];

    printf("Enter your account number: ");
    if (scanf("%14s", sender_acc_num) != 1 || strlen(sender_acc_num) < 7 || strlen(sender_acc_num) > 9)
    {
        while ((getchar()) != '\n');
        printf("Invalid sender account number. Please try again.\n");
        return;
    }
    while ((getchar()) != '\n');

    printf("Enter receiver account number: ");
    if (scanf("%14s", receiver_acc_num) != 1 || strlen(receiver_acc_num) < 7 || strlen(receiver_acc_num) > 9)
    {
        while ((getchar()) != '\n');
        printf("Invalid receiver account number. Please try again.\n");
        return;
    }
    while ((getchar()) != '\n');

    printf("Enter your account PIN: ");
    if (scanf("%9s", pin) != 1 || strlen(pin) != 4 || !checkpin(pin))
    {
        while ((getchar()) != '\n');
        printf("Invalid PIN. Please try again.\n");
        return;
    }
    while ((getchar()) != '\n');

    struct Account sender_acc;
    struct Account receiver_acc;

    if (load_account(sender_acc_num, &sender_acc))
    {
        if (strcmp(sender_acc.pin, pin) == 0)
        {
            if (strcmp(sender_acc_num, receiver_acc_num) == 0)
            {
                printf("You cannot remit funds to the same account.\n");
                return;
            }
            if (load_account(receiver_acc_num, &receiver_acc))
            {
                double amount;
                int check_if_numeric;
                while (true)
                {
                    printf("Enter the amount you want to remit: ");
                    check_if_numeric = scanf("%lf", &amount);
                    while ((getchar()) != '\n');

                    if (check_if_numeric != 1)
                    {
                        printf("Please enter a numeric value only.\n");
                        continue;
                    }
                    else if (amount <= 0.00)
                    {
                        printf("The amount you want to remit must be greater than 0.00.\n");
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                double fee = 0.00;
                double final_amount = amount;

                if (strcmp(sender_acc.acc_type, "savings") == 0 && strcmp(receiver_acc.acc_type, "current") == 0)
                {
                    fee = amount * 0.02;
                    final_amount = amount + fee;
                }
                else if (strcmp(sender_acc.acc_type, "current") == 0 && strcmp(receiver_acc.acc_type, "savings") == 0)
                {
                    fee = amount * 0.03;
                    final_amount = amount + fee;
                }

                if (final_amount > sender_acc.balance)
                {
                    printf("Insufficient funds to complete the remittance including fees (RM%.2f). Your current balance is RM%.2f\n", final_amount, sender_acc.balance);
                    return;
                }
                sender_acc.balance -= final_amount;
                receiver_acc.balance += amount;

                if (update_balance(sender_acc_num, sender_acc.balance) && update_balance(receiver_acc_num, receiver_acc.balance))
                {
                    printf("Funds successfully remitted from %s to %s. Amount: RM%.2f, Fee: RM%.2f\n", sender_acc_num, receiver_acc_num, amount, fee);
                    char log[500];
                    sprintf(log, "Remitted RM%.2f from account %s to account %s with fee RM%.2f", amount, sender_acc_num, receiver_acc_num, fee);
                    log_action("REMIT FUNDS", log);
                }
                else
                {
                    printf("Error updating balances. Please try again.\n");
                    return;
                }
            }
            else
            {
                printf("Receiver account not found. Please check the account number and try again.\n");
            }
        }
        else
        {
            printf("Invalid PIN. Please try again.\n");
        }
    }
    else
    {
        printf("Sender account not found. Please check your account number and try again.\n");
    }
}

int main()
{
    #ifdef _WIN32
        _mkdir("database");
    #else
        mkdir("database", 0755);
    #endif

    FILE *fp;
    fp = fopen("database/index.txt", "a");
    if (fp!= NULL)
    {
        fclose(fp);
    }

    fp = fopen("database/transaction.log", "a");
    if (fp!= NULL)
    {
        fclose(fp);
    }

    srand(time(NULL));

    time_t now = time(NULL);
    printf("Welcome to UOSM Banking System\n");
    printf("Current date and time: %s", ctime(&now));

    int acc_count = count_acc();
    printf("Total number of bank accounts: %d\n", acc_count);

    char input[30];

    while (true)
    {
        printf("1. Create New Bank Account\n");
        printf("2. Delete Bank Account\n");
        printf("3. Deposit Funds\n");
        printf("4. Withdraw Funds\n");
        printf("5. Remit Funds\n");
        printf("6. Exit\n");
        printf("Please select an option:\n");

        scanf("%19s", input);
        while ((getchar()) != '\n');

        if (!checkinput(input))
        {
            printf("Invalid option. Please choose an option from the menu only.\n");
            continue;
        }
        else if (strcmp(input, "1") == 0 || strcmp(input, "create") == 0)
        {
            create_account();
        }
        else if (strcmp(input, "2") == 0 || strcmp(input, "delete") == 0)
        {
            delete_account();
        }
        else if (strcmp(input, "3") == 0 || strcmp(input, "deposit") == 0)
        {
            deposit_funds();
        }
        else if (strcmp(input, "4") == 0 || strcmp(input, "withdraw") == 0)
        {
            withdrawal();
        }
        else if (strcmp(input, "5") == 0 || strcmp(input, "remit") == 0)
        {
            remittance();
        }
        else if (strcmp(input, "6") == 0 || strcmp(input, "exit") == 0)
        {
            printf("Exiting the system. Goodbye!\n");
            break;
        }
    }

    return 0;
}