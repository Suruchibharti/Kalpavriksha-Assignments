#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define MAX_NAME_LENGTH 50

typedef struct
{
    int productId;
    char productName[MAX_NAME_LENGTH];
    float productPrice;
    int productQuantity;
} Product;

void addNewProduct(Product **inventory, int *productCount);
void viewAllProducts(Product *inventory, int productCount);
void updateProductQuantity(Product *inventory, int productCount);
void searchProductById(Product *inventory, int productCount);
void searchProductByName(Product *inventory, int productCount);
void searchProductByPriceRange(Product *inventory, int productCount);
void deleteProductById(Product **inventory, int *productCount);
void freeInventoryMemory(Product *inventory);
bool checkIdExist(Product *inventory, int productCount, int product_id);
void clearInputBuffer();

int main()
{
    int initial_totalProduct;
    char input_buffer[30];
    char extraChar;

    while (1)
    {
        printf("Enter the initial number of products (1-100): ");
        fgets(input_buffer, sizeof(input_buffer), stdin);
        int scannedItems = sscanf(input_buffer, "%d %c", &initial_totalProduct, &extraChar);

        if ((scannedItems == 1 || (scannedItems == 2 && extraChar == '\n')) &&
            (initial_totalProduct >= 1 && initial_totalProduct <= 100))
        {
            break;
        }
        printf("Please enter a number between 1 and 100.\n");
    }

    Product *inventory = (Product *)calloc(initial_totalProduct, sizeof(Product));
    if (inventory == NULL)
    {
        printf("Memory allocation failed!\n");
        return 1;
    }

    for (int productNo = 0; productNo < initial_totalProduct; productNo++)
    {
        printf("\nEnter details for product %d:\n", productNo + 1);
        char userInput[150];
        char extraChar;

        // Product ID
        while (1)
        {
            printf("Product ID (1-10000): ");
            int temp_id;
            fgets(userInput, sizeof(userInput), stdin);
            int scannedItems = sscanf(userInput, "%d %c", &temp_id, &extraChar);

            if ((scannedItems == 1 || (scannedItems == 2 && extraChar == '\n')) &&
                (temp_id >= 1 && temp_id <= 10000))
            {
                if (checkIdExist(inventory, productNo , temp_id))
                {
                    printf("Product with this ID already exists, Please use a unique ID.\n");
                    continue;
                }
                inventory[productNo].productId = temp_id;
                break;
            }
            else
                printf("Please enter a number between 1 and 10000.\n");
        }

        // Product Name
        while (1)
        {
            printf("Product Name: ");
            fgets(inventory[productNo].productName, sizeof(inventory[productNo].productName), stdin);
            inventory[productNo].productName[strcspn(inventory[productNo].productName, "\n")] = '\0';
            if (strlen(inventory[productNo].productName) >= 1 && strlen(inventory[productNo].productName) <= 50)
                break;
            else
                printf("Please enter a product name (1 to 50 characters): ");
        }

        // Product Price
        while (1)
        {
            printf("Product Price: ");
            fgets(userInput, sizeof(userInput), stdin);
            int scannedItems = sscanf(userInput, "%f %c", &inventory[productNo].productPrice, &extraChar);

            if ((scannedItems == 1 || (scannedItems == 2 && extraChar == '\n')) &&
                (inventory[productNo].productPrice >= 0 && inventory[productNo].productPrice <= 100000))
                break;
            else
                printf("Please enter a product price between 0 and 100000.\n");
        }

        // Quantity
        while (1)
        {
            printf("Enter Quantity (0 - 1000000): ");
            int newQuantity;
            char buffer[50], extraChar;
            fgets(buffer, sizeof(buffer), stdin);
            int scannedItems = sscanf(buffer, "%d %c", &newQuantity, &extraChar);

            if (scannedItems == 1 && newQuantity >= 0 && newQuantity <= 1000000)
            {
                inventory[productNo].productQuantity = newQuantity;
                break;
            }
            printf("Please enter a valid quantity between 0 and 1000000.\n");
        }

        printf("Product added successfully!\n");
    }

    int userChoice;
    do
    {
        printf("\n========= INVENTORY MENU =========\n");
        printf("1. Add New Product\n");
        printf("2. View All Products\n");
        printf("3. Update Quantity\n");
        printf("4. Search Product by ID\n");
        printf("5. Search Product by Name\n");
        printf("6. Search Product by Price Range\n");
        printf("7. Delete Product\n");
        printf("8. Exit\n");

        while (1)
        {
            printf("Enter your choice: ");
            char buffer[30], extraChar;
            fgets(buffer, sizeof(buffer), stdin);
            int scannedItems = sscanf(buffer, "%d %c", &userChoice, &extraChar);
            if ((scannedItems == 1 || (scannedItems == 2 && extraChar == '\n')) &&
                (userChoice >= 1 && userChoice <= 8))
                break;
            printf("Invalid input. Please enter a number between 1 and 8.\n");
        }

        switch (userChoice)
        {
        case 1:
            addNewProduct(&inventory, &initial_totalProduct);
            break;
        case 2:
            viewAllProducts(inventory, initial_totalProduct);
            break;
        case 3:
            updateProductQuantity(inventory, initial_totalProduct);
            break;
        case 4:
            searchProductById(inventory, initial_totalProduct);
            break;
        case 5:
            searchProductByName(inventory, initial_totalProduct);
            break;
        case 6:
            searchProductByPriceRange(inventory, initial_totalProduct);
            break;
        case 7:
            deleteProductById(&inventory, &initial_totalProduct);
            break;
        case 8:
            freeInventoryMemory(inventory);
            printf("Memory released successfully. Exiting program...\n");
            break;
        }
    } while (userChoice != 8);

    return 0;
}

void addNewProduct(Product **inventory, int *productCount)
{
    *inventory = (Product *)realloc(*inventory, (*productCount + 1) * sizeof(Product));
    if (*inventory == NULL)
    {
        printf("Memory reallocation failed!\n");
        return;
    }

    char userInput[150];
    char extraChar;

    while (1)
    {
        printf("Product ID (1-10000): ");
        int temp_id;
        fgets(userInput, sizeof(userInput), stdin);
        int scannedItems = sscanf(userInput, "%d %c", &temp_id, &extraChar);

        if ((scannedItems == 1 || (scannedItems == 2 && extraChar == '\n')) &&
            (temp_id >= 1 && temp_id <= 10000))
        {
            if (checkIdExist(*inventory, *productCount, temp_id))
            {
                printf("Product with this ID already exists, Please use a unique ID.\n");
                continue;
            }
            (*inventory)[*productCount].productId = temp_id;
            break;
        }
        else
            printf("Please enter a number between 1 and 10000.\n");
    }

    while (1)
    {
        printf("Product Name: ");
        fgets((*inventory)[*productCount].productName, sizeof((*inventory)[*productCount].productName), stdin);
        (*inventory)[*productCount].productName[strcspn((*inventory)[*productCount].productName, "\n")] = '\0';
        if (strlen((*inventory)[*productCount].productName) >= 1 && strlen((*inventory)[*productCount].productName) <= 50)
            break;
        else
            printf("Please enter a product name (1 to 50 characters): ");
    }

    while (1)
    {
        printf("Product Price: ");
        fgets(userInput, sizeof(userInput), stdin);
        int scannedItems = sscanf(userInput, "%f %c", &(*inventory)[*productCount].productPrice, &extraChar);

        if ((scannedItems == 1 || (scannedItems == 2 && extraChar == '\n')) &&
            ((*inventory)[*productCount].productPrice >= 0 && (*inventory)[*productCount].productPrice <= 100000))
            break;
        else
            printf("Please enter a product price between 0 and 100000.\n");
    }

    while (1)
    {
        printf("Product Quantity (0 - 1000000): ");
        int newQuantity;
        char buffer[50], extraChar;
        fgets(buffer, sizeof(buffer), stdin);
        int scannedItems = sscanf(buffer, "%d %c", &newQuantity, &extraChar);

        if (scannedItems == 1 && newQuantity >= 0 && newQuantity <= 1000000)
        {
            (*inventory)[*productCount].productQuantity = newQuantity;
            break;
        }
        printf("Please enter a valid quantity between 0 and 1000000.\n");
    }

    (*productCount)++;
    printf("Product added successfully!\n");
}

void viewAllProducts(Product *inventory, int productCount)
{
    if (productCount == 0)
    {
        printf("No products available.\n");
        return;
    }

    printf("\n========= PRODUCT LIST =========\n");
    for (int i = 0; i < productCount; i++)
    {
        printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
               inventory[i].productId, inventory[i].productName,
               inventory[i].productPrice, inventory[i].productQuantity);
    }
}

bool checkIdExist(Product *inventory, int productCount, int product_id)
{
    for (int i = 0; i < productCount; i++)
    {
        if (product_id == inventory[i].productId)
            return true;
    }
    return false;
}

void clearInputBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void updateProductQuantity(Product *inventory, int productCount)
{
    int searchId, newQuantity;
    char buffer[50], extraChar;

    while (1)
    {
        printf("Enter Product ID to update quantity: ");
        fgets(buffer, sizeof(buffer), stdin);
        int scannedItems = sscanf(buffer, "%d %c", &searchId, &extraChar);
        if ((scannedItems == 1 || (scannedItems == 2 && extraChar == '\n')))
            break;
        printf("Invalid input. Please enter a valid Product ID.\n");
    }

    for (int i = 0; i < productCount; i++)
    {
        if (inventory[i].productId == searchId)
        {
            while (1)
            {
                printf("Enter new Quantity (0-1000000): ");
                fgets(buffer, sizeof(buffer), stdin);
                int scannedItems = sscanf(buffer, "%d %c", &newQuantity, &extraChar);
                if (scannedItems == 1 && newQuantity >= 0 && newQuantity <= 1000000)
                {
                    inventory[i].productQuantity = newQuantity;
                    printf("Quantity updated successfully!\n");
                    return;
                }
                printf("Invalid quantity! Try again.\n");
            }
        }
    }
    printf("Product with ID %d not found.\n", searchId);
}

void searchProductById(Product *inventory, int productCount)
{
    int searchId;
    char buffer[50], extraChar;

    while (1)
    {
        printf("Enter Product ID to search: ");
        fgets(buffer, sizeof(buffer), stdin);
        int scannedItems = sscanf(buffer, "%d %c", &searchId, &extraChar);
        if ((scannedItems == 1 || (scannedItems == 2 && extraChar == '\n')))
            break;
        printf("Invalid input! Please enter a valid Product ID.\n");
    }

    for (int i = 0; i < productCount; i++)
    {
        if (inventory[i].productId == searchId)
        {
            printf("Product Found: Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   inventory[i].productId, inventory[i].productName,
                   inventory[i].productPrice, inventory[i].productQuantity);
            return;
        }
    }
    printf("Product not found.\n");
}

void searchProductByName(Product *inventory, int productCount)
{
    char nameSearch[MAX_NAME_LENGTH];
    int found = 0;

    while (1)
    {
        printf("Enter product name to search (partial match allowed): ");
        fgets(nameSearch, sizeof(nameSearch), stdin);
        nameSearch[strcspn(nameSearch, "\n")] = '\0';

        if (strlen(nameSearch) > 0)
            break;
        printf("Product name cannot be empty.\n");
    }

    printf("\nProducts Found:\n");
    for (int i = 0; i < productCount; i++)
    {
        if (strstr(inventory[i].productName, nameSearch) != NULL)
        {
            printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   inventory[i].productId, inventory[i].productName,
                   inventory[i].productPrice, inventory[i].productQuantity);
            found = 1;
        }
    }

    if (!found)
        printf("No products found matching '%s'.\n", nameSearch);
}

void searchProductByPriceRange(Product *inventory, int productCount)
{
    float minPrice, maxPrice;
    char buffer[50], extraChar;
    int found = 0;

    while (1)
    {
        printf("Enter minimum price: ");
        fgets(buffer, sizeof(buffer), stdin);
        int scannedItems = sscanf(buffer, "%f %c", &minPrice, &extraChar);
        if (scannedItems == 1)
            break;
        printf("Invalid input. Please enter a valid minimum price.\n");
    }

    while (1)
    {
        printf("Enter maximum price: ");
        fgets(buffer, sizeof(buffer), stdin);
        int scannedItems = sscanf(buffer, "%f %c", &maxPrice, &extraChar);
        if (scannedItems == 1 && maxPrice >= minPrice)
            break;
        printf("Invalid input. Maximum price should be >= minimum price.\n");
    }

    printf("Products in price range:\n");
    for (int i = 0; i < productCount; i++)
    {
        if (inventory[i].productPrice >= minPrice && inventory[i].productPrice <= maxPrice)
        {
            printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   inventory[i].productId, inventory[i].productName,
                   inventory[i].productPrice, inventory[i].productQuantity);
            found = 1;
        }
    }
    if (!found)
        printf("No products found in this price range.\n");
}

void deleteProductById(Product **inventory, int *productCount)
{
    int deleteId;
    char buffer[50], extraChar;

    while (1)
    {
        printf("Enter Product ID to delete: ");
        fgets(buffer, sizeof(buffer), stdin);
        int scannedItems = sscanf(buffer, "%d %c", &deleteId, &extraChar);
        if ((scannedItems == 1 || (scannedItems == 2 && extraChar == '\n')))
            break;
        printf("Invalid input! Please enter a valid Product ID.\n");
    }

    for (int i = 0; i < *productCount; i++)
    {
        if ((*inventory)[i].productId == deleteId)
        {
            for (int j = i; j < *productCount - 1; j++)
                (*inventory)[j] = (*inventory)[j + 1];

            (*productCount)--;

            *inventory = (Product *)realloc(*inventory, (*productCount) * sizeof(Product));
            if (*inventory == NULL && *productCount > 0)
            {
                fprintf(stderr, "Memory reallocation failed during delete!\n");
                return;
            }

            printf("Product deleted successfully!\n");
            return;
        }
    }
    printf("Product with ID %d not found.\n", deleteId);
}

void freeInventoryMemory(Product *inventory)
{
    free(inventory);
}
