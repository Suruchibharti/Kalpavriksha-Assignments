#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define MAX_NAME_LENGTH 50

// Structure for product details
typedef struct
{
    int productId;
    char productName[MAX_NAME_LENGTH];
    float productPrice;
    int productQuantity;
} Product;

// Function declarations
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

    printf("Enter the initial number of products (1-100): ");

    fgets(input_buffer, sizeof(input_buffer), stdin);
    int scannedItems = sscanf(input_buffer, "%d %c", &initial_totalProduct, &extraChar);

    if (!((scannedItems == 1 || (scannedItems == 2 && extraChar == '\n')) && (initial_totalProduct >= 1 && initial_totalProduct <= 100)))
    {
        printf("Please enter a number between 1 and 100.\n");
        return 1;
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
        printf("Product ID (1-10000):");
        char userInput[150];
        while (true)
        {
            int temp_id;
            fgets(userInput, sizeof(userInput), stdin);
            int scannedItems = sscanf(userInput, "%d %c", &temp_id, &extraChar);

            if ((scannedItems == 1 || (scannedItems == 2 && extraChar == '\n')) &&
                (temp_id >= 1 && temp_id <= 10000))
            {
                if (checkIdExist(inventory, initial_totalProduct, temp_id))
                {
                    printf("Product with this ID already exists, Please use a unique ID.\n");
                    continue;
                }
                inventory[productNo].productId = temp_id;
                break;
            }
            else
            {
                printf("Please enter a number between 1 and 10000.\n");
            }
        }

        printf("Product Name: ");
        while (true)
        {
            fgets(inventory[productNo].productName, sizeof(inventory[productNo].productName), stdin);
            inventory[productNo].productName[strcspn(inventory[productNo].productName, "\n")] = '\0';
            if (strlen(inventory[productNo].productName) >= 1 && strlen(inventory[productNo].productName) <= 50)
            {
                break;
            }
            else
            {
                printf("Please enter a product name (1 to 50 characters): ");
            }
        }

        printf("Product Price: ");
        while (true)
        {
            fgets(userInput, sizeof(userInput), stdin);
            int scannedItems = sscanf(userInput, "%f %c", &inventory[productNo].productPrice, &extraChar);

            if (!((scannedItems == 1 || (scannedItems == 2 && extraChar == '\n')) &&
                  (inventory[productNo].productPrice >= 0 && inventory[productNo].productPrice <= 100000)))
            {
                printf("Please enter a product price between 0 and 100000.\n");
                continue;
            }
            else
                break;
        }

        printf("Enter Quantity (0 - 1000000): ");
        while (true)
        {
            int newQuantity;
            char buffer[50], extraChar;
            fgets(buffer, sizeof(buffer), stdin);
            int scannedItems = sscanf(buffer, "%d %c", &newQuantity, &extraChar);

            if (scannedItems != 1 || (scannedItems == 2 && extraChar != '\n') ||
                newQuantity < 0 || newQuantity > 1000000)
            {
                printf("Please enter a valid quantity between 0 and 1000000: ");
                continue;
            }

            inventory[productNo].productQuantity = newQuantity;
            break;
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
        printf("Enter your choice: ");

        while (scanf("%d", &userChoice) != 1)
        {
            printf("Invalid input. Enter a valid choice: ");
            clearInputBuffer();
        }

        clearInputBuffer();

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
        default:
            printf("Invalid choice! Please try again.\n");
        }

        // Automatically reprint menu until exit
    } while (userChoice != 8);

    return 0;
}

// Function to add a new product
void addNewProduct(Product **inventory, int *productCount)
{
    *inventory = (Product *)realloc(*inventory, (*productCount + 1) * sizeof(Product));
    if (*inventory == NULL)
    {
        printf("Memory reallocation failed!\n");
        return;
    }

    printf("\nEnter new product details:\n");
    char userInput[150];
    char extraChar;

    printf("Product ID: ");
    while (true)
    {
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
        {
            printf("Please enter a number between 1 and 10000.\n");
        }
    }

    printf("Product Name: ");
    while (true)
    {
        fgets((*inventory)[*productCount].productName, sizeof((*inventory)[*productCount].productName), stdin);
        (*inventory)[*productCount].productName[strcspn((*inventory)[*productCount].productName, "\n")] = '\0';
        if (strlen((*inventory)[*productCount].productName) >= 1 && strlen((*inventory)[*productCount].productName) <= 50)
        {
            break;
        }
        else
        {
            printf("Please enter a product name (1 to 50 characters): ");
        }
    }

    printf("Product Price: ");
    while (true)
    {
        fgets(userInput, sizeof(userInput), stdin);
        int scannedItems = sscanf(userInput, "%f %c", &(*inventory)[*productCount].productPrice, &extraChar);

        if (!((scannedItems == 1 || (scannedItems == 2 && extraChar == '\n')) &&
              ((*inventory)[*productCount].productPrice >= 0 && (*inventory)[*productCount].productPrice <= 100000)))
        {
            printf("Please enter a product price between 0 and 100000.\n");
            continue;
        }
        else
            break;
    }

    printf("Product Quantity: ");
    while (true)
    {
        int newQuantity;
        char buffer[50], extraChar;
        fgets(buffer, sizeof(buffer), stdin);
        int scannedItems = sscanf(buffer, "%d %c", &newQuantity, &extraChar);

        if (scannedItems != 1 || (scannedItems == 2 && extraChar != '\n') ||
            newQuantity < 0 || newQuantity > 1000000)
        {
            printf("Please enter a valid quantity between 0 and 1000000: ");
            continue;
        }

        (*inventory)[*productCount].productQuantity = newQuantity;
        break;
    }

    (*productCount)++;
    printf("Product added successfully!\n");
}

// Function to view all products
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

// Function to check if ID already exists
bool checkIdExist(Product *inventory, int productCount, int product_id)
{
    for (int i = 0; i < productCount; i++)
    {
        if (product_id == inventory[i].productId)
        {
            return true;
        }
    }
    return false;
}

// Function to clear input buffer
void clearInputBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
        
}

// Function to update product quantity
void updateProductQuantity(Product *inventory, int productCount)
{
    int searchId, newQuantity;
    printf("Enter Product ID to update quantity: ");
    scanf("%d", &searchId);

    for (int i = 0; i < productCount; i++)
    {
        if (inventory[i].productId == searchId)
        {
            printf("Enter new Quantity: ");
            scanf("%d", &newQuantity);
            inventory[i].productQuantity = newQuantity;
            printf("Quantity updated successfully!\n");
            clearInputBuffer();
            return;
        }
    }
    printf("Product with ID %d not found.\n", searchId);
    clearInputBuffer();
}

// Function to search product by ID
void searchProductById(Product *inventory, int productCount)
{
    int searchId;
    printf("Enter Product ID to search: ");
    scanf("%d", &searchId);
    clearInputBuffer();

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

// Function to search product by name
void searchProductByName(Product *inventory, int productCount)
{
    char nameSearch[MAX_NAME_LENGTH];
    int found = 0;

    printf("Enter product name to search (partial match allowed): ");
    fgets(nameSearch, sizeof(nameSearch), stdin);
    nameSearch[strcspn(nameSearch, "\n")] = '\0';

    if (strlen(nameSearch) == 0)
    {
        printf("Product name cannot be empty.\n");
        return;
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

// Function to search products by price range
void searchProductByPriceRange(Product *inventory, int productCount)
{
    float minPrice, maxPrice;
    int found = 0;

    printf("Enter minimum price: ");
    scanf("%f", &minPrice);
    printf("Enter maximum price: ");
    scanf("%f", &maxPrice);
    clearInputBuffer();

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

// Function to delete a product by ID
void deleteProductById(Product **inventory, int *productCount)
{
    int deleteId;
    printf("Enter Product ID to delete: ");
    scanf("%d", &deleteId);
    clearInputBuffer();

    for (int i = 0; i < *productCount; i++)
    {
        if ((*inventory)[i].productId == deleteId)
        {
            for (int j = i; j < *productCount - 1; j++)
            {
                (*inventory)[j] = (*inventory)[j + 1];
            }
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

// Function to release allocated memory
void freeInventoryMemory(Product *inventory)
{
    free(inventory);
}
