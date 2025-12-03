🌟 Personal Finance Tracker (C Project)

A modular C-based console application to track daily expenses with category management, CSV import/export, binary storage, and monthly summaries.
Designed following the UPES Major Project Repository Structure.

📌 Table of Contents

Overview

Features

Tech Stack

Project Structure

Flowchart

How to Run

Screenshots

Future Enhancements

Author

📖 Overview

The Personal Finance Tracker helps users efficiently record and analyse their expenses.
This project demonstrates:

File handling (CSV + binary)

Modular programming

Dynamic memory usage

Error handling & input validation

Menu-driven UI

Clean repository with documentation & report

🛠 Features

✔ Add expenses (date, category, amount, description)
✔ Auto-create category if it doesn’t exist
✔ Delete expenses by ID
✔ Grouped & detailed listing
✔ Monthly summary (total + average per active day)
✔ CSV import & export
✔ Binary database (expenses.bin)
✔ Category management (add/rename/delete)
✔ Search / Filter (date range, category, text)

⚙ Tech Stack

Language: C

Tools: GCC, VSCode

Storage: Binary files + CSV

Documentation: Markdown + Flowcharts

📁 Project Structure
📦 Personal-Finance-Tracker
│
├── src/
│   ├── main.c
│   ├── finance.c
│   └── finance.h
│
├── data/
│   ├── expenses.bin
│   └── export.csv
│
└── README.md

▶ How to Run
Compile
gcc main.c finance.c -o main.exe

Run
./main.exe


🚀 Future Enhancements

CLI graph visualization

Export PDF reports

Authentication system

Monthly budgets & overspend alerts

Android app version

👤 Author

Saksham Thakur
UPES – B.Tech CSE