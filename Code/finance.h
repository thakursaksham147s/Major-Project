#ifndef FINANCE_H
#define FINANCE_H

#define DESCRIPTION_LEN 128
#define DATE_LEN 11    
#define CAT_LEN 32
#define MAX_CATS 64

typedef struct {
    int id;
    char date[DATE_LEN];    
    double amount;
    char category[CAT_LEN];
    char description[DESCRIPTION_LEN];
} Expense;

typedef struct {
    Expense *arr;
    int size;
    int capacity;
    int next_id;

   
    char categories[MAX_CATS][CAT_LEN];
    int cat_count;
} ExpenseDB;

void db_init(ExpenseDB *db);
void db_free(ExpenseDB *db);

int category_exists(const ExpenseDB *db, const char *cat);
int add_category(ExpenseDB *db, const char *cat);
int remove_category(ExpenseDB *db, const char *cat); 
int rename_category(ExpenseDB *db, const char *oldname, const char *newname);
void list_categories(const ExpenseDB *db);

int db_add(ExpenseDB *db, Expense e);
int db_delete_by_id(ExpenseDB *db, int id);
int db_find_index_by_id(const ExpenseDB *db, int id);
void db_list(const ExpenseDB *db); 
void db_list_grouped(const ExpenseDB *db); 

void db_list_filtered(const ExpenseDB *db, const char *category,
                      const char *from_date, const char *to_date,
                      const char *substr_in_description);


int db_save_binary(const ExpenseDB *db, const char *filename);
int db_load_binary(ExpenseDB *db, const char *filename);
int db_export_csv(const ExpenseDB *db, const char *filename);
int db_import_csv(ExpenseDB *db, const char *filename);


void db_monthly_summary(const ExpenseDB *db, const char *year_month);

int is_valid_date(const char *d); 
int parse_amount(const char *s, double *out);

#endif 