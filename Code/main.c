#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "finance.h"

static void print_menu(void)
{
    puts("\n--- Personal Finance Tracker ---");
    puts("1. Add expense");
    puts("2. List all expenses (grouped + detailed)");
    puts("3. Delete expense by ID");
    puts("4. Save DB (data/expenses.bin)");
    puts("5. Load DB (data/expenses.bin)");
    puts("6. Export CSV (data/export.csv)");
    puts("7. Import CSV (ask filename)");
    puts("8. Monthly summary (MM-YYYY)");
    puts("9. Manage categories");
    puts("10. Search / Filter expenses");
    puts("0. Exit");
    printf("Choose: ");
}

static void read_line(const char *prompt, char *buf, int n)
{
    printf("%s", prompt);
    if (!fgets(buf, n, stdin))
    {
        buf[0] = 0;
        return;
    }
    buf[strcspn(buf, "\n")] = 0;
}

static void manage_categories(ExpenseDB *db)
{
    while (1)
    {
        puts("\n-- Manage categories --");
        puts("1. List categories");
        puts("2. Add category");
        puts("3. Rename category");
        puts("4. Remove category (only if unused)");
        puts("0. Back");
        printf("Choose: ");
        char tmp[128];
        if (!fgets(tmp, sizeof tmp, stdin))
            return;
        int ch = atoi(tmp);
        if (ch == 1)
        {
            list_categories(db);
        }
        else if (ch == 2)
        {
            char name[CAT_LEN];
            read_line("New category name: ", name, sizeof name);
            if (name[0] == '\0')
                puts("Empty name.");
            else if (add_category(db, name))
                printf("Added \"%s\"\n", name);
            else
                puts("Failed to add (maybe exists or limit).");
        }
        else if (ch == 3)
        {
            char oldn[CAT_LEN], newn[CAT_LEN];
            read_line("Existing category name: ", oldn, sizeof oldn);
            read_line("New name: ", newn, sizeof newn);
            if (rename_category(db, oldn, newn))
                printf("Renamed %s -> %s\n", oldn, newn);
            else
                puts("Rename failed (check names).");
        }
        else if (ch == 4)
        {
            char name[CAT_LEN];
            read_line("Category to remove: ", name, sizeof name);
            if (remove_category(db, name))
                printf("Removed \"%s\"\n", name);
            else
                puts("Remove failed (maybe in use or not exist).");
        }
        else if (ch == 0)
            return;
        else
            puts("Invalid.");
    }
}

static void search_filter_ui(ExpenseDB *db)
{
    char cat[CAT_LEN] = {0};
    char from[DATE_LEN] = {0};
    char to[DATE_LEN] = {0};
    char substr[DESCRIPTION_LEN] = {0};
    read_line("Filter by category (leave empty to ignore): ", cat, sizeof cat);
    read_line("From date (DD-MM-YYYY, leave empty to ignore): ", from, sizeof from);
    read_line("To date (DD-MM-YYYY, leave empty to ignore): ", to, sizeof to);
    read_line("Search text in description (leave empty to ignore): ", substr, sizeof substr);
    if (from[0] && !is_valid_date(from))
    {
        puts("From date invalid.");
        return;
    }
    if (to[0] && !is_valid_date(to))
    {
        puts("To date invalid.");
        return;
    }
    db_list_filtered(db, cat[0] ? cat : NULL, from[0] ? from : NULL, to[0] ? to : NULL, substr[0] ? substr : NULL);
}

int main(void)
{
    ExpenseDB db;
    db_init(&db);

    db_load_binary(&db, "data/expenses.bin");

    int choice;
    char tmp[512];
    do
    {
        print_menu();
        if (!fgets(tmp, sizeof tmp, stdin))
            break;
        choice = atoi(tmp);
        if (choice == 1)
        {
            Expense e;
            memset(&e, 0, sizeof e);

            while (1)
            {
                read_line("Date (DD-MM-YYYY): ", e.date, sizeof e.date);
                if (is_valid_date(e.date))
                    break;
                printf("Invalid date. Use DD-MM-YYYY (e.g. 25-11-2025). Try again.\n");
            }

            while (1)
            {
                read_line("Category: ", e.category, sizeof e.category);
                if (e.category[0] == '\0')
                {
                    printf("Category cannot be empty. Try again.\n");
                    continue;
                }
                if (category_exists(&db, e.category))
                {
                    break;
                }
                else
                {
                    char resp[8];
                    printf("Category \"%s\" not found. Create it? (y/n): ", e.category);
                    if (!fgets(resp, sizeof resp, stdin))
                    {
                        resp[0] = 'n';
                        resp[1] = 0;
                    }
                    if (resp[0] == 'y' || resp[0] == 'Y')
                    {
                        if (add_category(&db, e.category))
                        {
                            printf("Category \"%s\" created.\n", e.category);
                            break;
                        }
                        else
                        {
                            puts("Failed to create category (limit reached). Choose another.");
                            continue;
                        }
                    }
                    else
                    {
                        puts("Okay, enter another category.");
                        continue;
                    }
                }
            }

            while (1)
            {
                char amtbuf[64];
                read_line("Amount: ", amtbuf, sizeof amtbuf);
                double v;
                if (!parse_amount(amtbuf, &v))
                {
                    puts("Invalid amount. Enter a non-negative number (e.g. 150.50).");
                    continue;
                }
                e.amount = v;
                break;
            }

            read_line("Description: ", e.description, sizeof e.description);

            if (db_add(&db, e))
                printf("Added expense id %d\n", db.next_id - 1);
            else
                puts("Failed to add expense (memory).");
        }
        else if (choice == 2)
        {
            db_list_grouped(&db);
            puts("\nFull list:");
            db_list(&db);
        }
        else if (choice == 3)
        {
            char idbuf[32];
            read_line("Enter ID to delete: ", idbuf, sizeof idbuf);
            int id = atoi(idbuf);
            if (db_delete_by_id(&db, id))
                puts("Deleted.");
            else
                puts("ID not found.");
        }
        else if (choice == 4)
        {
            if (db_save_binary(&db, "data/expenses.bin"))
                puts("Saved to data/expenses.bin");
            else
                puts("Save failed.");
        }
        else if (choice == 5)
        {
            if (db_load_binary(&db, "data/expenses.bin"))
                puts("Loaded data/expenses.bin");
            else
                puts("Load failed or file missing.");
        }
        else if (choice == 6)
        {
            if (db_export_csv(&db, "data/export.csv"))
                puts("Exported to data/export.csv");
            else
                puts("Export failed.");
        }
        else if (choice == 7)
        {
            char filename[256];
            read_line("Enter CSV filename to import (e.g. data/sample_import.csv): ", filename, sizeof filename);
            if (filename[0] == '\0')
                strcpy(filename, "data/sample_import.csv");
            if (db_import_csv(&db, filename))
                printf("Imported %s\n", filename);
            else
                printf("Import failed: %s\n", filename);
        }
        else if (choice == 8)
        {
            char ym[16];
            read_line("Enter month-year (MM-YYYY): ", ym, sizeof ym);
            db_monthly_summary(&db, ym);
        }
        else if (choice == 9)
        {
            manage_categories(&db);
        }
        else if (choice == 10)
        {
            search_filter_ui(&db);
        }
        else if (choice == 0)
        {
            puts("Exiting. Auto-saving to data/expenses.bin");
            db_save_binary(&db, "data/expenses.bin");
        }
        else
        {
            puts("Invalid option.");
        }
    } while (choice != 0);

    db_free(&db);
    return 0;
}
