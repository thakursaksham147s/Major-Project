#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "finance.h"

#ifdef _WIN32
  #include <direct.h>
  #define MKDIR(dir) _mkdir(dir)
#else
  #include <sys/stat.h>
  #define MKDIR(dir) mkdir(dir, 0755)
#endif


static char *next_token(char **cur, char delim) {
    if (!cur || !*cur) return NULL;
    char *start = *cur;
    if (*start == '\0') return NULL;
    char *p = start;
    while (*p && *p != delim && *p != '\r' && *p != '\n') p++;
    if (*p == delim) {
        *p = '\0';
        *cur = p + 1;
    } else {
        *cur = p;
    }
    return start;
}

static void trim_inplace(char *s) {
    if (!s) return;
    size_t i = 0;
    while (isspace((unsigned char)s[i])) i++;
    if (i) memmove(s, s + i, strlen(s + i) + 1);
    size_t L = strlen(s);
    while (L > 0 && isspace((unsigned char)s[L-1])) s[--L] = '\0';
}

static void ensure_data_dir(void) {
    MKDIR("data");
}

static int date_to_key(const char *d) {
    if (!d || strlen(d) < 10) return -1;
    int day = (d[0]-'0')*10 + (d[1]-'0');
    int mon = (d[3]-'0')*10 + (d[4]-'0');
    int year = (d[6]-'0')*1000 + (d[7]-'0')*100 + (d[8]-'0')*10 + (d[9]-'0');
    if (day <=0 || mon <=0 || year <=0) return -1;
    return year*10000 + mon*100 + day;
}


static int ensure_capacity(ExpenseDB *db) {
    if (db->size < db->capacity) return 1;
    int newcap = db->capacity == 0 ? 8 : db->capacity * 2;
    Expense *tmp = realloc(db->arr, (size_t)newcap * sizeof(Expense));
    if (!tmp) return 0;
    db->arr = tmp;
    db->capacity = newcap;
    return 1;
}

void db_init(ExpenseDB *db) {
    db->arr = NULL;
    db->size = 0;
    db->capacity = 0;
    db->next_id = 1;
    db->cat_count = 0;
    add_category(db, "Food");
    add_category(db, "Transport");
    add_category(db, "Shopping");
    add_category(db, "Gym");
}

void db_free(ExpenseDB *db) {
    free(db->arr);
    db->arr = NULL;
    db->size = db->capacity = 0;
    db->next_id = 1;
    db->cat_count = 0;
}


int category_exists(const ExpenseDB *db, const char *cat) {
    if (!cat) return 0;
    for (int i = 0; i < db->cat_count; ++i)
        if (strcmp(db->categories[i], cat) == 0) return 1;
    return 0;
}

int add_category(ExpenseDB *db, const char *cat) {
    if (!cat || cat[0] == '\0') return 0;
    if (db->cat_count >= MAX_CATS) return 0;
    if (category_exists(db, cat)) return 1;
    strncpy(db->categories[db->cat_count], cat, CAT_LEN-1);
    db->categories[db->cat_count][CAT_LEN-1] = '\0';
    db->cat_count++;
    return 1;
}

int remove_category(ExpenseDB *db, const char *cat) {
    if (!category_exists(db, cat)) return 0;
    for (int i = 0; i < db->size; ++i) {
        if (strcmp(db->arr[i].category, cat) == 0) return 0; /* in use */
    }
    int idx = -1;
    for (int i = 0; i < db->cat_count; ++i) if (strcmp(db->categories[i], cat) == 0) { idx = i; break; }
    if (idx == -1) return 0;
    for (int i = idx; i + 1 < db->cat_count; ++i) strcpy(db->categories[i], db->categories[i+1]);
    db->cat_count--;
    return 1;
}

int rename_category(ExpenseDB *db, const char *oldname, const char *newname) {
    if (!category_exists(db, oldname)) return 0;
    if (!newname || newname[0]=='\0') return 0;
    if (category_exists(db, newname)) return 0; /* avoid duplicate */
    int idx = -1;
    for (int i = 0; i < db->cat_count; ++i) if (strcmp(db->categories[i], oldname) == 0) { idx = i; break; }
    if (idx == -1) return 0;
    strncpy(db->categories[idx], newname, CAT_LEN-1);
    db->categories[idx][CAT_LEN-1] = '\0';
    /* update expenses */
    for (int i = 0; i < db->size; ++i) {
        if (strcmp(db->arr[i].category, oldname) == 0) {
            strncpy(db->arr[i].category, newname, CAT_LEN-1);
            db->arr[i].category[CAT_LEN-1] = '\0';
        }
    }
    return 1;
}

void list_categories(const ExpenseDB *db) {
    if (db->cat_count == 0) { puts("No categories yet."); return; }
    puts("Categories:");
    for (int i = 0; i < db->cat_count; ++i) printf(" - %s\n", db->categories[i]);
}


int db_add(ExpenseDB *db, Expense e) {
    if (!ensure_capacity(db)) return 0;
    e.id = db->next_id++;
    e.date[DATE_LEN-1] = '\0';
    e.category[CAT_LEN-1] = '\0';
    e.description[DESCRIPTION_LEN-1] = '\0';
    db->arr[db->size++] = e;
    add_category(db, e.category);
    return 1;
}

int db_find_index_by_id(const ExpenseDB *db, int id) {
    for (int i = 0; i < db->size; ++i) if (db->arr[i].id == id) return i;
    return -1;
}

int db_delete_by_id(ExpenseDB *db, int id) {
    int idx = db_find_index_by_id(db, id);
    if (idx < 0) return 0;
    for (int i = idx; i + 1 < db->size; ++i) db->arr[i] = db->arr[i+1];
    db->size--;
    return 1;
}

void db_list(const ExpenseDB *db) {
    if (db->size == 0) { puts("No expenses recorded."); return; }
    printf("ID  Date       Amount    Category        Description\n");
    printf("-------------------------------------------------------------------\n");
    for (int i = 0; i < db->size; ++i) {
        const Expense *e = &db->arr[i];
        printf("%-3d %-10s  %8.2f  %-12s  %.40s\n", e->id, e->date, e->amount, e->category, e->description);
    }
}

void db_list_grouped(const ExpenseDB *db) {
    if (db->size == 0) { puts("No expenses recorded."); return; }
    if (db->cat_count > 0) {
        for (int ci = 0; ci < db->cat_count; ++ci) {
            const char *cat = db->categories[ci];
            double total = 0.0;
            for (int i = 0; i < db->size; ++i) if (strcmp(db->arr[i].category, cat) == 0) total += db->arr[i].amount;
            if (total <= 0.0) continue;
            printf("\n%s : %.2f\n", cat, total);
            for (int i = 0; i < db->size; ++i) {
                if (strcmp(db->arr[i].category, cat) != 0) continue;
                const Expense *e = &db->arr[i];
                printf("   id %-3d  %s  %8.2f  %s\n", e->id, e->date, e->amount, e->description);
            }
        }
    } else {
        char cats[MAX_CATS][CAT_LEN];
        int nc=0;
        for (int i=0;i<db->size;i++) {
            const char *c = db->arr[i].category;
            int idx=-1;
            for (int j=0;j<nc;j++) if (strcmp(cats[j],c)==0) { idx=j; break; }
            if (idx==-1) { strncpy(cats[nc], c, CAT_LEN-1); cats[nc][CAT_LEN-1]=0; nc++; }
        }
        for (int ci=0; ci<nc; ++ci) {
            const char *cat = cats[ci];
            double total=0;
            for (int i=0;i<db->size;i++) if (strcmp(db->arr[i].category, cat)==0) total+=db->arr[i].amount;
            printf("\n%s : %.2f\n", cat, total);
            for (int i=0;i<db->size;i++) {
                if (strcmp(db->arr[i].category, cat)!=0) continue;
                const Expense *e = &db->arr[i];
                printf("   id %-3d  %s  %8.2f  %s\n", e->id, e->date, e->amount, e->description);
            }
        }
    }
}


int db_save_binary(const ExpenseDB *db, const char *filename) {
    ensure_data_dir();
    FILE *f = fopen(filename, "wb");
    if (!f) return 0;
    if (fwrite(&db->size, sizeof(int), 1, f) != 1) { fclose(f); return 0; }
    if (fwrite(&db->next_id, sizeof(int), 1, f) != 1) { fclose(f); return 0; }
    if (fwrite(&db->cat_count, sizeof(int), 1, f) != 1) { fclose(f); return 0; }
    if (db->cat_count > 0) {
        if (fwrite(db->categories, CAT_LEN, db->cat_count, f) != (size_t)db->cat_count) { fclose(f); return 0; }
    }
    if (db->size > 0) {
        if (fwrite(db->arr, sizeof(Expense), db->size, f) != (size_t)db->size) { fclose(f); return 0; }
    }
    fclose(f);
    return 1;
}

int db_load_binary(ExpenseDB *db, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;
    int n;
    if (fread(&n, sizeof(int), 1, f) != 1) { fclose(f); return 0; }
    if (fread(&db->next_id, sizeof(int), 1, f) != 1) { fclose(f); return 0; }
    if (fread(&db->cat_count, sizeof(int), 1, f) != 1) { fclose(f); return 0; }
    if (db->cat_count > MAX_CATS) { fclose(f); return 0; }
    if (db->cat_count > 0) {
        if (fread(db->categories, CAT_LEN, db->cat_count, f) != (size_t)db->cat_count) { fclose(f); return 0; }
    }
    db_free(db);
    if (n > 0) {
        db->arr = malloc((size_t)n * sizeof(Expense));
        if (!db->arr) { fclose(f); return 0; }
        if (fread(db->arr, sizeof(Expense), n, f) != (size_t)n) { free(db->arr); db->arr = NULL; fclose(f); return 0; }
        db->size = n; db->capacity = n;
    }
    fclose(f);
    return 1;
}


int db_export_csv(const ExpenseDB *db, const char *filename) {
    ensure_data_dir();
    FILE *f = fopen(filename, "w");
    if (!f) return 0;
    fprintf(f, "id,date,amount,category,description\n");
    for (int i = 0; i < db->size; ++i) {
        const Expense *e = &db->arr[i];
        fprintf(f, "%d,%s,%.2f,%s,%s\n", e->id, e->date, e->amount, e->category, e->description);
    }
    fclose(f);
    return 1;
}

int db_import_csv(ExpenseDB *db, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;
    char line[1024];
    if (!fgets(line, sizeof line, f)) { fclose(f); return 0; } /* skip header */
    while (fgets(line, sizeof line, f)) {
        char buf[1024];
        strncpy(buf, line, sizeof buf-1);
        buf[sizeof buf-1] = '\0';
        char *cur = buf;
        Expense e; memset(&e,0,sizeof e);

        char *tok = next_token(&cur, ','); if (!tok) continue;

        tok = next_token(&cur, ','); if (!tok) continue;
        trim_inplace(tok);
        if (strlen(tok) >= 10 && tok[4] == '-') {
            char day[3] = { tok[8], tok[9], '\0' };
            char mon[3] = { tok[5], tok[6], '\0' };
            char year[5] = { tok[0], tok[1], tok[2], tok[3], '\0' };
            snprintf(e.date, DATE_LEN, "%s-%s-%s", day, mon, year);
        } else {
            strncpy(e.date, tok, DATE_LEN-1);
            e.date[DATE_LEN-1] = '\0';
        }

        tok = next_token(&cur, ','); if (!tok) continue;
        trim_inplace(tok);
        e.amount = atof(tok);

        tok = next_token(&cur, ','); if (!tok) continue;
        trim_inplace(tok);
        strncpy(e.category, tok, CAT_LEN-1);

        tok = cur;
        trim_inplace(tok);
        strncpy(e.description, tok, DESCRIPTION_LEN-1);

        if (e.category[0] == '\0') strncpy(e.category, "Misc", CAT_LEN-1);
        add_category(db, e.category);
        db_add(db, e);
    }
    fclose(f);
    return 1;
}


void db_monthly_summary(const ExpenseDB *db, const char *year_month) {
    double total = 0.0;
    int days_with_expense[32] = {0};
    for (int i = 0; i < db->size; ++i) {
        if (strncmp(db->arr[i].date + 3, year_month, 7) == 0) {
            total += db->arr[i].amount;
            int day = 0;
            if (sscanf(db->arr[i].date, "%2d", &day) == 1 && day >=1 && day <=31) days_with_expense[day] = 1;
        }
    }
    int count_days = 0;
    for (int d=1; d<=31; ++d) if (days_with_expense[d]) count_days++;
    printf("Summary for %s\n", year_month);
    printf("Total spent: %.2f\n", total);
    if (count_days > 0) printf("Average per active day: %.2f\n", total / count_days);
    else printf("No expenses recorded this month.\n");
}


void db_list_filtered(const ExpenseDB *db, const char *category,
                      const char *from_date, const char *to_date,
                      const char *substr_in_description) {
    if (!db || db->size == 0) { puts("No expenses recorded."); return; }

    int from_key = -1, to_key = -1;
    if (from_date && from_date[0]) from_key = date_to_key(from_date);
    if (to_date && to_date[0]) to_key = date_to_key(to_date);

    int found = 0;
    printf("ID  Date       Amount    Category        Description\n");
    printf("-------------------------------------------------------------------\n");
    for (int i = 0; i < db->size; ++i) {
        const Expense *e = &db->arr[i];
        if (category && category[0]) {
            if (strcmp(e->category, category) != 0) continue;
        }
        if (from_key != -1 || to_key != -1) {
            int k = date_to_key(e->date);
            if (k == -1) continue;
            if (from_key != -1 && k < from_key) continue;
            if (to_key != -1 && k > to_key) continue;
        }
        if (substr_in_description && substr_in_description[0]) {
            if (!strstr(e->description, substr_in_description)) continue;
        }
        printf("%-3d %-10s  %8.2f  %-12s  %.40s\n", e->id, e->date, e->amount, e->category, e->description);
        found = 1;
    }
    if (!found) puts("No matching expenses.");
}


int is_valid_date(const char *d) {
    if (!d) return 0;
    if (strlen(d) != 10) return 0;
    if (d[2] != '-' || d[5] != '-') return 0;
    for (int i = 0; i < 10; ++i) {
        if (i==2 || i==5) continue;
        if (!isdigit((unsigned char)d[i])) return 0;
    }
    int day = (d[0]-'0')*10 + (d[1]-'0');
    int mon = (d[3]-'0')*10 + (d[4]-'0');
    int year = (d[6]-'0')*1000 + (d[7]-'0')*100 + (d[8]-'0')*10 + (d[9]-'0');
    if (year < 1900 || mon < 1 || mon > 12 || day < 1) return 0;
    int mdays = 31;
    if (mon==4||mon==6||mon==9||mon==11) mdays=30;
    else if (mon==2) {
        int leap = (year%400==0) || (year%4==0 && year%100!=0);
        mdays = leap ? 29 : 28;
    }
    if (day > mdays) return 0;
    return 1;
}

int parse_amount(const char *s, double *out) {
    if (!s || !*s) return 0;
    char *end;
    double v = strtod(s, &end);
    if (end == s) return 0;
    if (v < 0.0) return 0;
    if (out) *out = v;
    return 1;
}
