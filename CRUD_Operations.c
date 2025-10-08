#include <stdio.h>
#include <string.h>
#include<stdlib.h>

#define FILE_NAME "records.txt"

struct Record {
    int id;
    char fullname[60];
    int years;
    };

//create a new user in a file
//if fille not exist then create file: records.txt
void addRecord() {
    struct Record r;
    FILE *fp = fopen(FILE_NAME, "a");
     if (fp == NULL) {
        printf("File not opened\n");
        return;
    }

    printf("Enter id No: ");
    scanf("%d", &r.id);
    getchar();
       printf("Enter Name: ");
    fgets(r.fullname, sizeof(r.fullname), stdin);
    r.fullname[strcspn(r.fullname, "\n")] = '\0';
       printf("Enter Age: ");
    scanf("%d", &r.years);

    fprintf(fp, "%d,%s,%d\n", r.id, r.fullname, r.years);
       fclose(fp);
     printf("Record inserted\n");
}

   //All records will show 
void showAll() {
    struct Record r;
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        printf("No data int the file found.\n");
        return;
    }

    printf("\n--- All Records ---\n");
      printf("ID\tName\tAge\n");
    while (fscanf(fp, "%d,%[^,],%d\n", &r.id, r.fullname, &r.years) != EOF) {
        printf("%d\t%s\t%d\n", r.id, r.fullname, r.years);
    }
    fclose(fp);
}


//update the existng recod according to the ID
void updaterecordbyId() {
    int id, ok = 0;
    struct Record r;
      FILE *fp = fopen(FILE_NAME, "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!fp || !temp) {
        printf("Error in opening the file\n");
        return;
    }

    printf("Enter id to modify the record: ");
    scanf("%d", &id);
    getchar();

    while (fscanf(fp, "%d,%[^,],%d\n", &r.id, r.fullname, &r.years) != EOF) {
        if (r.id == id) {
            ok = 1;
            printf("New Name: ");
              fgets(r.fullname, sizeof(r.fullname), stdin);
            r.fullname[strcspn(r.fullname, "\n")] = '\0';
              printf("New Age: ");
            scanf("%d", &r.years);
            getchar();
        }
        fprintf(temp, "%d,%s,%d\n", r.id, r.fullname, r.years);
    }

    fclose(fp);
    fclose(temp);

    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    if (ok) printf("Updated successfully\n");
    else printf("Not Found\n");
}




//remove the record by id:
void removeRecord() {
    int id, ok = 0;
    struct Record r;
    FILE *fp = fopen(FILE_NAME, "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!fp || !temp) {
        printf("File error!\n");
        return;
    }

    printf("Enter id to delete: ");
    scanf("%d", &id);

    while (fscanf(fp, "%d,%[^,],%d\n", &r.id, r.fullname, &r.years) != EOF) {
        if (r.id == id) {
            ok = 1;
            continue;
        }
        fprintf(temp, "%d,%s,%d\n", r.id, r.fullname, r.years);
    }

    fclose(fp);
    fclose(temp);

    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    if (ok) printf("Deleted successfully\n");
    else printf("Not Found\n");
}





//main() function starts form here 
int main() {
    int choice;
    while (1) {
        printf("\n--- Type the options of menu ---\n");
        printf("1. Add User\n");
         printf("2. Display Users\n");
        printf("3. Update User\n");
         printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: addRecord(); break;
            case 2: showAll(); break;
            case 3: updaterecordbyId(); break;
            case 4: removeRecord(); break;
            case 5: exit(0);
            default: printf("Invalid choice, Try again.\n");
        }
    }
    return 0;
}
