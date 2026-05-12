// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// ANSI color codes
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define RESET "\033[0m"

// New fields for clientData struct
struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
    char accountType;     // 'S' for Savings, 'C' for Checking
    double interestRate;  // Interest rate for Savings accounts
    unsigned int pin;     // 4-digit PIN for security
};                        // end structure clientData

// prototypes
void displayBanner(void);
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void listAllAccounts(FILE *fPtr);
void searchAccount(FILE *fPtr);
void exportToCSV(FILE *fPtr);
void sortAccounts(FILE *fPtr);
void accountSummary(FILE *fPtr);
void applyInterest(FILE *fPtr);
void logTransaction(const char *type, unsigned int acctNum, double amount, double balance);
void viewTransactionHistory(void);
void sortAccountsByBalance(FILE *fPtr, int ascending);
void sortAccountsByName(FILE *fPtr);
void exportToJSON(FILE *fPtr);
void generateHTMLDashboard(void);

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    (void)argc;  // suppress unused parameter warning
    displayBanner(); // Display styled banner on startup

    // fopen opens the file; exits if file cannot be opened
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf(RED "%s: File could not be opened.\n" RESET, argv[0]);
        exit(-1);
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 11)
    {
        switch (choice)
        {
        // create text file from record file
        case 1:
            textFile(cfPtr);
            break;
        // update record
        case 2:
            updateRecord(cfPtr);
            break;
        // create record
        case 3:
            newRecord(cfPtr);
            break;
        // delete existing record
        case 4:
            deleteRecord(cfPtr);
            break;
        // list all accounts
        case 5:
            listAllAccounts(cfPtr);
            break;
        // search for an account
        case 6:
            searchAccount(cfPtr);
            break;
        // export to CSV
        case 7:
            exportToCSV(cfPtr);
            break;
        // sort accounts
        case 8:
            sortAccountsByBalance(cfPtr, 1); // Ascending
            break;
        // account summary
        case 9:
            applyInterest(cfPtr);
            break;
        // view transaction history
        case 10:
            viewTransactionHistory();
            break;
        // export to JSON
        case 11:
            exportToJSON(cfPtr);
            generateHTMLDashboard();
            break;
        // display if user does not select valid choice
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    }     // end while

    fclose(cfPtr); // fclose closes the file
    return 0;
} // end main

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    int result;     // used to test whether fread read any bytes
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0, 'C', 0.0, 0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } // end if
    else
    {
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        // copy all records from random-access file into text file
        while (!feof(readPtr))
        {
            result = fread(&client, sizeof(struct clientData), 1, readPtr);

            // write single record to text file
            if (result != 0 && client.acctNum != 0)
            {
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                        client.balance);
            } // end if
        }     // end while

        fclose(writePtr); // fclose closes the file
    }                     // end else
} // end function textFile

// update balance in record
void updateRecord(FILE *fPtr)
{
    unsigned int account; // account number
    double transaction;   // transaction amount
    // create clientData with no information
    struct clientData client = {0, "", "", 0.0, 'C', 0.0, 0};

    // obtain number of account to update
    printf("%s", "Enter account to update ( 1 - 100 ): ");
    scanf("%d", &account);

    // move file pointer to correct record in file
    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
    }
    else
    { // update record
        printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // request transaction amount from user
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        scanf("%lf", &transaction);
        client.balance += transaction; // update record balance

        printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // move file pointer to correct record in file
        // move back by 1 record length
        fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);
        // write updated record over old record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function updateRecord

// delete an existing record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;                              // stores record read from file
    struct clientData blankClient = {0, "", "", 0, 'C', 0.0, 0}; // blank client
    unsigned int accountNum;                               // account number

    // obtain number of account to delete
    printf("%s", "Enter account number to delete ( 1 - 100 ): ");
    scanf("%d", &accountNum);

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if record does not exist
    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", accountNum);
    } // end if
    else
    { // delete record
        // move file pointer to correct record in file
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        // replace existing record with blank record
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function deleteRecord

// create and insert record
void newRecord(FILE *fPtr)
{
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0, 'C', 0.0, 0};
    unsigned int accountNum; // account number

    // obtain number of account to create
    printf("%s", "Enter new account number ( 1 - 100 ): ");
    scanf("%d", &accountNum);

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account already exists
    if (client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
    } // end if
    else
    { // create record
        // user enters last name, first name and balance
        printf("%s", "Enter lastname, firstname, balance\n? ");
        scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance);

        client.acctNum = accountNum;
        // move file pointer to correct record in file
        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        // insert record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function newRecord

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    // display available options
    printf("%s", "\nEnter your choice\n"
                 "1 - store a formatted text file of accounts called\n"
                 "    \"accounts.txt\" for printing\n"
                 "2 - update an account\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - list all accounts\n"
                 "6 - search for an account\n"
                 "7 - export accounts to CSV\n"
                 "8 - sort accounts\n"
                 "9 - account summary\n"
                 "10 - view transaction history\n"
                 "11 - export to JSON and generate HTML dashboard\n"
                 "12 - end program\n? ");

    scanf("%u", &menuChoice); // receive choice from user
    return menuChoice;
} // end function enterChoice

void displayBanner(void) {
    printf(BLUE "========================================\n" RESET);
    printf(BLUE "        Welcome to Bank Manager 2.0     \n" RESET);
    printf(BLUE "========================================\n" RESET);
}

void applyInterest(FILE *fPtr) {
    struct clientData client = {0, "", "", 0.0, 'C', 0.0, 0};
    rewind(fPtr);
    while (fread(&client, sizeof(struct clientData), 1, fPtr)) {
        if (client.accountType == 'S') {
            client.balance += client.balance * client.interestRate;
            fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);
            fwrite(&client, sizeof(struct clientData), 1, fPtr);
        }
    }
    printf(GREEN "Interest applied to all Savings accounts.\n" RESET);
}

void logTransaction(const char *type, unsigned int acctNum, double amount, double balance) {
    FILE *logFile = fopen("history.log", "a");
    if (logFile == NULL) {
        printf(RED "Could not open transaction log.\n" RESET);
        return;
    }
    time_t now = time(NULL);
    fprintf(logFile, "[%s] ACCT#%u | %s | %+0.2f | BAL: %0.2f\n", ctime(&now), acctNum, type, amount, balance);
    fclose(logFile);
}

void viewTransactionHistory(void) {
    FILE *logFile = fopen("history.log", "r");
    if (logFile == NULL) {
        printf(RED "No transaction history found.\n" RESET);
        return;
    }
    char line[256];
    int count = 0;
    printf(BLUE "Last 20 Transactions:\n" RESET);
    while (fgets(line, sizeof(line), logFile) && count < 20) {
        printf("%s", line);
        count++;
    }
    fclose(logFile);
}

void exportToJSON(FILE *fPtr) {
    FILE *jsonFile = fopen("accounts.json", "w");
    if (jsonFile == NULL) {
        printf(RED "Could not create JSON file.\n" RESET);
        return;
    }
    struct clientData client = {0, "", "", 0.0, 'C', 0.0, 0};
    rewind(fPtr);
    fprintf(jsonFile, "[\n");
    while (fread(&client, sizeof(struct clientData), 1, fPtr)) {
        if (client.acctNum != 0) {
            fprintf(jsonFile, "  {\"acctNum\": %u, \"lastName\": \"%s\", \"firstName\": \"%s\", \"balance\": %.2f, \"accountType\": \"%c\", \"interestRate\": %.2f}\n",
                    client.acctNum, client.lastName, client.firstName, client.balance, client.accountType, client.interestRate);
        }
    }
    fprintf(jsonFile, "]\n");
    fclose(jsonFile);
    printf(GREEN "Accounts exported to accounts.json\n" RESET);
}

void generateHTMLDashboard(void) {
    FILE *htmlFile = fopen("dashboard.html", "w");
    if (htmlFile == NULL) {
        printf(RED "Could not create HTML dashboard.\n" RESET);
        return;
    }
    fprintf(htmlFile, "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<title>Bank Dashboard</title>\n<style>body { background-color: #121212; color: #fff; font-family: Arial; }</style>\n</head>\n<body>\n<h1>Bank Dashboard</h1>\n<p>Load accounts.json for data visualization.</p>\n</body>\n</html>");
    fclose(htmlFile);
    printf(GREEN "HTML dashboard generated as dashboard.html\n" RESET);
}

void listAllAccounts(FILE *fPtr) {
    struct clientData client = {0, "", "", 0.0, 'C', 0.0, 0};
    rewind(fPtr);
    printf(BLUE "\n%-6s%-16s%-11s%12s%-8s\n" RESET, "Acct", "Last Name", "First Name", "Balance", "Type");
    printf(BLUE "============================================================\n" RESET);
    while (fread(&client, sizeof(struct clientData), 1, fPtr)) {
        if (client.acctNum != 0) {
            printf("%-6d%-16s%-11s%12.2f%-8c\n", client.acctNum, client.lastName, client.firstName, client.balance, client.accountType);
        }
    }
    printf(BLUE "============================================================\n" RESET);
}

void searchAccount(FILE *fPtr) {
    struct clientData client = {0, "", "", 0.0, 'C', 0.0, 0};
    unsigned int searchAcct;
    printf("Enter account number to search ( 1 - 100 ): ");
    scanf("%u", &searchAcct);
    
    fseek(fPtr, (searchAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);
    
    if (client.acctNum == 0) {
        printf(RED "Account #%u not found.\n" RESET, searchAcct);
    } else {
        printf(GREEN "\nAccount Found:\n" RESET);
        printf("%-6d%-16s%-11s%12.2f%-8c\n", client.acctNum, client.lastName, client.firstName, client.balance, client.accountType);
    }
}

void exportToCSV(FILE *fPtr) {
    FILE *csvFile = fopen("accounts.csv", "w");
    if (csvFile == NULL) {
        printf(RED "Could not create CSV file.\n" RESET);
        return;
    }
    struct clientData client = {0, "", "", 0.0, 'C', 0.0, 0};
    rewind(fPtr);
    fprintf(csvFile, "AccountNum,LastName,FirstName,Balance,Type,InterestRate\n");
    while (fread(&client, sizeof(struct clientData), 1, fPtr)) {
        if (client.acctNum != 0) {
            fprintf(csvFile, "%u,%s,%s,%.2f,%c,%.2f\n", client.acctNum, client.lastName, client.firstName, client.balance, client.accountType, client.interestRate);
        }
    }
    fclose(csvFile);
    printf(GREEN "Accounts exported to accounts.csv\n" RESET);
}

void sortAccountsByBalance(FILE *fPtr, int ascending) {
    struct clientData client = {0, "", "", 0.0, 'C', 0.0, 0};
    struct clientData temp;
    struct clientData accounts[100];
    int count = 0;
    
    rewind(fPtr);
    while (fread(&client, sizeof(struct clientData), 1, fPtr)) {
        if (client.acctNum != 0) {
            accounts[count++] = client;
        }
    }
    
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if ((ascending && accounts[j].balance > accounts[j+1].balance) ||
                (!ascending && accounts[j].balance < accounts[j+1].balance)) {
                temp = accounts[j];
                accounts[j] = accounts[j+1];
                accounts[j+1] = temp;
            }
        }
    }
    
    printf(BLUE "\n%-6s%-16s%-11s%12s\n" RESET, "Acct", "Last Name", "First Name", "Balance");
    printf(BLUE "============================================================\n" RESET);
    for (int i = 0; i < count; i++) {
        printf("%-6d%-16s%-11s%12.2f\n", accounts[i].acctNum, accounts[i].lastName, accounts[i].firstName, accounts[i].balance);
    }
    printf(BLUE "============================================================\n" RESET);
}

void sortAccounts(FILE *fPtr) {
    sortAccountsByBalance(fPtr, 1);
}

void accountSummary(FILE *fPtr) {
    struct clientData client = {0, "", "", 0.0, 'C', 0.0, 0};
    double totalBalance = 0;
    int totalAccounts = 0;
    double savingsBalance = 0;
    double checkingBalance = 0;
    
    rewind(fPtr);
    while (fread(&client, sizeof(struct clientData), 1, fPtr)) {
        if (client.acctNum != 0) {
            totalBalance += client.balance;
            totalAccounts++;
            if (client.accountType == 'S') savingsBalance += client.balance;
            else checkingBalance += client.balance;
        }
    }
    
    printf(BLUE "\n====== Account Summary ======\n" RESET);
    printf("Total Accounts: %d\n", totalAccounts);
    printf("Total Balance: $%.2f\n", totalBalance);
    printf("Savings Total: $%.2f\n", savingsBalance);
    printf("Checking Total: $%.2f\n" RESET, checkingBalance);
}

void sortAccountsByName(FILE *fPtr) {
    struct clientData client = {0, "", "", 0.0, 'C', 0.0, 0};
    struct clientData temp;
    struct clientData accounts[100];
    int count = 0;
    
    rewind(fPtr);
    while (fread(&client, sizeof(struct clientData), 1, fPtr)) {
        if (client.acctNum != 0) {
            accounts[count++] = client;
        }
    }
    
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (strcmp(accounts[j].lastName, accounts[j+1].lastName) > 0) {
                temp = accounts[j];
                accounts[j] = accounts[j+1];
                accounts[j+1] = temp;
            }
        }
    }
    
    printf(BLUE "\n%-6s%-16s%-11s%12s\n" RESET, "Acct", "Last Name", "First Name", "Balance");
    printf(BLUE "============================================================\n" RESET);
    for (int i = 0; i < count; i++) {
        printf("%-6d%-16s%-11s%12.2f\n", accounts[i].acctNum, accounts[i].lastName, accounts[i].firstName, accounts[i].balance);
    }
    printf(BLUE "============================================================\n" RESET);
}