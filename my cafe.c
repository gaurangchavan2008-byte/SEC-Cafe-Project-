
#include <stdio.h>
#include <string.h>

#define MENU_SIZE 12
#define MAX_ORDER 30
#define GST_RATE 0.05f

typedef struct {
    int id;
    char name[30];
    char category[12];
    float price;
} Item;

typedef struct {
    int menuIndex;
    int qty;
} OrderLine;

Item menu[MENU_SIZE] = {
    { 1, "Espresso",          "Coffee",   120.0f},
    { 2, "Cappuccino",        "Coffee",   160.0f},
    { 3, "Cafe Latte",        "Coffee",   170.0f},
    { 4, "Cold Coffee",       "Coffee",   180.0f},
    { 5, "Masala Chai",       "Tea",       60.0f},
    { 6, "Green Tea",         "Tea",       80.0f},
    { 7, "Veg Sandwich",      "Snacks",   110.0f},
    { 8, "French Fries",      "Snacks",    99.0f},
    { 9, "Veg Burger",        "Snacks",   130.0f},
    {10, "Chocolate Brownie", "Dessert",  140.0f},
    {11, "Cheesecake Slice",  "Dessert",  190.0f},
    {12, "Ice Cream Scoop",   "Dessert",   70.0f}
};

OrderLine order[MAX_ORDER];
int orderCount = 0;
int billNumber = 1;
float daySales = 0.0f;

void clearInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int readInt(const char *prompt, int min, int max) {
    int value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1 && value >= min && value <= max) return value;
        printf("Please enter a number between %d and %d.\n", min, max);
        clearInput();
    }
}

void showMenu(void) {
    const char *lastCategory = "";
    printf("\n================ CAFE MENU ================\n");
    for (int i = 0; i < MENU_SIZE; i++) {
        if (strcmp(lastCategory, menu[i].category) != 0) {
            printf("\n-- %s --\n", menu[i].category);
            lastCategory = menu[i].category;
        }
        printf("  %2d. %-20s Rs %6.2f\n", menu[i].id, menu[i].name, menu[i].price);
    }
}

void addItem(void) {
    showMenu();
    int id = readInt("\nEnter item number (0 to cancel): ", 0, MENU_SIZE);
    if (id == 0) return;
    int qty = readInt("Enter quantity: ", 1, 50);

    for (int i = 0; i < orderCount; i++) {
        if (order[i].menuIndex == id - 1) {
            order[i].qty += qty;
            printf("Updated: %s x %d\n", menu[id - 1].name, order[i].qty);
            return;
        }
    }
    if (orderCount >= MAX_ORDER) {
        printf("Order is full!\n");
        return;
    }
    order[orderCount].menuIndex = id - 1;
    order[orderCount].qty = qty;
    orderCount++;
    printf("Added: %s x %d\n", menu[id - 1].name, qty);
}

float orderSubtotal(void) {
    float total = 0;
    for (int i = 0; i < orderCount; i++)
        total += menu[order[i].menuIndex].price * order[i].qty;
    return total;
}

void printOrder(FILE *out) {
    fprintf(out, "\n%-3s %-20s %-4s %10s\n", "No", "Item", "Qty", "Amount");
    fprintf(out, "------------------------------------------\n");
    for (int i = 0; i < orderCount; i++) {
        Item *it = &menu[order[i].menuIndex];
        fprintf(out, "%-3d %-20s %-4d %10.2f\n",
                i + 1, it->name, order[i].qty, it->price * order[i].qty);
    }
    fprintf(out, "------------------------------------------\n");
    fprintf(out, "Subtotal: Rs %.2f\n", orderSubtotal());
}

void viewOrder(void) {
    if (orderCount == 0) {
        printf("\nYour order is empty.\n");
        return;
    }
    printOrder(stdout);
}

void removeItem(void) {
    if (orderCount == 0) {
        printf("\nYour order is empty.\n");
        return;
    }
    printOrder(stdout);
    int n = readInt("\nEnter line number to remove (0 to cancel): ", 0, orderCount);
    if (n == 0) return;
    for (int i = n - 1; i < orderCount - 1; i++) order[i] = order[i + 1];
    orderCount--;
    printf("Item removed.\n");
}

void generateBill(void) {
    char name[40];
    char code[20];

    if (orderCount == 0) {
        printf("\nNothing to bill. Add some items first.\n");
        return;
    }

    clearInput();
    printf("Customer name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    printf("Discount code (press Enter if none): ");
    fgets(code, sizeof(code), stdin);
    code[strcspn(code, "\n")] = '\0';

    float subtotal = orderSubtotal();
    float discount = 0.0f;
    if (strcmp(code, "CAFE10") == 0) {
        discount = subtotal * 0.10f;
        printf("Code applied: 10%% off!\n");
    } else if (code[0] != '\0') {
        printf("Invalid code, no discount applied.\n");
    }

    float taxable = subtotal - discount;
    float gst = taxable * GST_RATE;
    float grand = taxable + gst;

    char filename[40];
    sprintf(filename, "bill_%d.txt", billNumber);
    FILE *files[2] = {stdout, fopen(filename, "w")};

    for (int f = 0; f < 2; f++) {
        FILE *out = files[f];
        if (!out) continue;
        fprintf(out, "\n========== BREW & BITE CAFE ==========\n");
        fprintf(out, "Bill No: %d\nCustomer: %s\n", billNumber, name[0] ? name : "Guest");
        printOrder(out);
        if (discount > 0) fprintf(out, "Discount: -Rs %.2f\n", discount);
        fprintf(out, "GST (5%%): Rs %.2f\n", gst);
        fprintf(out, "GRAND TOTAL: Rs %.2f\n", grand);
        fprintf(out, "\nThank you for visiting! :)\n");
    }
    if (files[1]) {
        fclose(files[1]);
        printf("\nBill saved to %s\n", filename);
    }

    FILE *log = fopen("sales_log.txt", "a");
    if (log) {
        fprintf(log, "Bill %d | %s | Rs %.2f\n", billNumber, name[0] ? name : "Guest", grand);
        fclose(log);
    }

    daySales += grand;
    billNumber++;
    orderCount = 0;
}

int main(void) {
    int choice;
    printf("*** Welcome to Brew & Bite Cafe ***\n");
    do {
        printf("\n1. View Menu\n2. Add Item\n3. View Order\n4. Remove Item\n"
               "5. Generate Bill\n6. Today's Sales\n7. Exit\n");
        choice = readInt("Choice: ", 1, 7);
        switch (choice) {
            case 1: showMenu();     break;
            case 2: addItem();      break;
            case 3: viewOrder();    break;
            case 4: removeItem();   break;
            case 5: generateBill(); break;
            case 6: printf("\nBills today: %d | Total sales: Rs %.2f\n",
                           billNumber - 1, daySales); break;
            case 7: printf("Goodbye!\n"); break;
        }
    } while (choice != 7);
    return 0;
}