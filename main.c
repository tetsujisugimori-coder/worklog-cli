#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_LOGS 1000
#define MAX_CATEGORIES 100
#define CSV_FILE "log.csv"

typedef struct {
    char date[20];
    char category[50];
    char content[200];
    int minutes;
} WorkLog;

typedef struct {
    char category[50];
    int totalMinutes;
} CategorySummary;

void setupConsoleEncoding(void);
void removeNewline(char str[]);
void readLine(char prompt[], char buffer[], int size);
int isEmpty(char str[]);
void printMenu(void);
void addWorkLog(void);
int loadLogs(WorkLog logs[], int maxLogs);
void showLogsByDate(void);
void showCategorySummary(void);
void showAllLogs(void);
void printMinutes(int minutes);
void printLogItem(int number, WorkLog log, int showDate);
int isFileEmptyOrMissing(void);
int hasUtf8Bom(void);
void ensureCsvBom(void);
void ensureCsvHeader(FILE *file, int needsHeader);
void saveLogsToCsv(WorkLog logs[], int count);

int main(void) {
    char input[20];
    int choice;

    setupConsoleEncoding();
    ensureCsvBom();

    while (1) {
        printMenu();
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("入力を読み取れませんでした。終了します。\n");
            break;
        }
        removeNewline(input);
        choice = atoi(input);

        if (choice == 1) {
            addWorkLog();
        } else if (choice == 2) {
            showLogsByDate();
        } else if (choice == 3) {
            showCategorySummary();
        } else if (choice == 4) {
            showAllLogs();
        } else if (choice == 5) {
            printf("終了します。\n");
            break;
        } else {
            printf("不正な番号です。1から5の番号を入力してください。\n");
        }
    }

    return 0;
}

void setupConsoleEncoding(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

void removeNewline(char str[]) {
    size_t length = strlen(str);

    if (length > 0 && str[length - 1] == '\n') {
        str[length - 1] = '\0';
    }
}

void readLine(char prompt[], char buffer[], int size) {
    printf("%s\n", prompt);
    printf("> ");

    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }

    removeNewline(buffer);
}

int isEmpty(char str[]) {
    return str[0] == '\0';
}

void printMenu(void) {
    printf("\n==== 作業ログ集計アプリ ====\n\n");
    printf("1. 作業ログを追加する\n");
    printf("2. 日付別に作業ログを見る\n");
    printf("3. カテゴリ別に集計する\n");
    printf("4. 全ログを見る\n");
    printf("5. 終了する\n\n");
    printf("番号を選んでください:\n");
    printf("> ");
}

void addWorkLog(void) {
    WorkLog log;
    char minutesInput[20];
    FILE *file;
    int needsHeader;

    ensureCsvBom();

    readLine("日付を入力してください 例: 2026-06-13", log.date, sizeof(log.date));
    readLine("カテゴリを入力してください", log.category, sizeof(log.category));
    readLine("作業内容を入力してください", log.content, sizeof(log.content));
    readLine("作業時間を分で入力してください", minutesInput, sizeof(minutesInput));

    log.minutes = atoi(minutesInput);

    if (isEmpty(log.date) || isEmpty(log.category) || isEmpty(log.content) || log.minutes <= 0) {
        printf("入力内容が正しくないため、保存しませんでした。\n");
        return;
    }

    needsHeader = isFileEmptyOrMissing();
    file = fopen(CSV_FILE, "a");
    if (file == NULL) {
        printf("ファイルを開けませんでした。\n");
        return;
    }

    ensureCsvHeader(file, needsHeader);
    fprintf(file, "%s,%s,%s,%d\n", log.date, log.category, log.content, log.minutes);
    fclose(file);

    printf("保存しました。\n");
}

int loadLogs(WorkLog logs[], int maxLogs) {
    FILE *file;
    char line[512];
    int count = 0;
    char *token;

    file = fopen(CSV_FILE, "r");
    if (file == NULL) {
        return 0;
    }

    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file) != NULL && count < maxLogs) {
        removeNewline(line);

        token = strtok(line, ",");
        if (token == NULL) {
            continue;
        }
        strcpy(logs[count].date, token);

        token = strtok(NULL, ",");
        if (token == NULL) {
            continue;
        }
        strcpy(logs[count].category, token);

        token = strtok(NULL, ",");
        if (token == NULL) {
            continue;
        }
        strcpy(logs[count].content, token);

        token = strtok(NULL, ",");
        if (token == NULL) {
            continue;
        }
        logs[count].minutes = atoi(token);

        count++;
    }

    fclose(file);
    return count;
}

void showLogsByDate(void) {
    WorkLog logs[MAX_LOGS];
    char targetDate[20];
    int count;
    int foundCount = 0;
    int totalMinutes = 0;
    int i;

    readLine("表示したい日付を入力してください 例: 2026-06-13", targetDate, sizeof(targetDate));

    count = loadLogs(logs, MAX_LOGS);

    printf("\n==== %s の作業ログ ====\n\n", targetDate);

    for (i = 0; i < count; i++) {
        if (strcmp(logs[i].date, targetDate) == 0) {
            foundCount++;
            printLogItem(foundCount, logs[i], 0);
            totalMinutes += logs[i].minutes;
        }
    }

    if (foundCount == 0) {
        printf("%s のログはありません。\n", targetDate);
        return;
    }

    printf("合計: ");
    printMinutes(totalMinutes);
    printf("\n");
}

void showCategorySummary(void) {
    WorkLog logs[MAX_LOGS];
    CategorySummary summaries[MAX_CATEGORIES];
    int logCount;
    int categoryCount = 0;
    int totalMinutes = 0;
    int i;
    int j;
    int found;

    logCount = loadLogs(logs, MAX_LOGS);

    printf("\n==== カテゴリ別集計 ====\n\n");

    if (logCount == 0) {
        printf("まだ作業ログがありません。\n");
        return;
    }

    for (i = 0; i < logCount; i++) {
        found = 0;

        for (j = 0; j < categoryCount; j++) {
            if (strcmp(summaries[j].category, logs[i].category) == 0) {
                summaries[j].totalMinutes += logs[i].minutes;
                found = 1;
                break;
            }
        }

        if (!found && categoryCount < MAX_CATEGORIES) {
            strcpy(summaries[categoryCount].category, logs[i].category);
            summaries[categoryCount].totalMinutes = logs[i].minutes;
            categoryCount++;
        }

        totalMinutes += logs[i].minutes;
    }

    for (i = 0; i < categoryCount; i++) {
        printf("%s: ", summaries[i].category);
        printMinutes(summaries[i].totalMinutes);
        printf("\n");
    }

    printf("\n合計: ");
    printMinutes(totalMinutes);
    printf("\n");
}

void showAllLogs(void) {
    WorkLog logs[MAX_LOGS];
    int count;
    int totalMinutes = 0;
    int i;

    count = loadLogs(logs, MAX_LOGS);

    printf("\n==== 全ログ ====\n\n");

    if (count == 0) {
        printf("まだ作業ログがありません。\n");
        return;
    }

    for (i = 0; i < count; i++) {
        printLogItem(i + 1, logs[i], 1);
        totalMinutes += logs[i].minutes;
    }

    printf("合計: ");
    printMinutes(totalMinutes);
    printf("\n");
}

void printMinutes(int minutes) {
    int hours = minutes / 60;
    int restMinutes = minutes % 60;

    if (hours > 0) {
        printf("%d時間%d分", hours, restMinutes);
    } else {
        printf("%d分", restMinutes);
    }
}

void printLogItem(int number, WorkLog log, int showDate) {
    if (showDate) {
        printf("%d. %s / %s\n", number, log.date, log.category);
    } else {
        printf("%d. %s\n", number, log.category);
    }

    printf("   内容: %s\n", log.content);
    printf("   時間: ");
    printMinutes(log.minutes);
    printf("\n\n");
}

int isFileEmptyOrMissing(void) {
    FILE *file;
    long size;

    file = fopen(CSV_FILE, "r");
    if (file == NULL) {
        return 1;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fclose(file);

    return size == 0;
}

int hasUtf8Bom(void) {
    FILE *file;
    unsigned char bom[3];
    size_t readSize;

    file = fopen(CSV_FILE, "rb");
    if (file == NULL) {
        return 0;
    }

    readSize = fread(bom, 1, 3, file);
    fclose(file);

    return readSize == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF;
}

void ensureCsvBom(void) {
    WorkLog logs[MAX_LOGS];
    int count;

    if (isFileEmptyOrMissing() || hasUtf8Bom()) {
        return;
    }

    count = loadLogs(logs, MAX_LOGS);
    saveLogsToCsv(logs, count);
}

void ensureCsvHeader(FILE *file, int needsHeader) {
    if (needsHeader) {
        fprintf(file, "\xEF\xBB\xBF");
        fprintf(file, "date,category,content,minutes\n");
    }
}

void saveLogsToCsv(WorkLog logs[], int count) {
    FILE *file;
    int i;

    file = fopen(CSV_FILE, "wb");
    if (file == NULL) {
        printf("ファイルを保存できませんでした。\n");
        return;
    }

    fprintf(file, "\xEF\xBB\xBF");
    fprintf(file, "date,category,content,minutes\n");

    for (i = 0; i < count; i++) {
        fprintf(file, "%s,%s,%s,%d\n",
                logs[i].date,
                logs[i].category,
                logs[i].content,
                logs[i].minutes);
    }

    fclose(file);
}
