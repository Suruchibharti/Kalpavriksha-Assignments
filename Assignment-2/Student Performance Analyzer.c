#include <stdio.h>
#include <string.h>

// Structure for student information:
struct Student {
    int rollNumber;
    char studentName[50];
    float marks[3];
    float totalMarks;
    float averageMarks;
    char grade;
};

// Function to calculate total marks
float calculateTotal(float marks[]) {
    float total = 0;
    for (int i = 0; i < 3; i++) {
        total += marks[i];
    }
    return total;
}

//  calculate average marks
float calculateAverage(float totalMarks) {
    return totalMarks / 3.0;
}

// assign grades based on average
char getGrade(float averageMarks) {
    if (averageMarks >= 85) return 'A';
    else if (averageMarks >= 70) return 'B';
    else if (averageMarks >= 50) return 'C';
    else if (averageMarks >= 35) return 'D';
    else return 'F';
}

// display performance stars based on grade
void showPerformance(char grade) {
    int starCount = 0;

    switch (grade) {
        case 'A': starCount = 5; break;
        case 'B': starCount = 4; break;
        case 'C': starCount = 3; break;
        case 'D': starCount = 2; break;
        default: return;
    }

    for (int i = 0; i < starCount; i++) {
        printf("*");
    }
    printf("\n");
}

// Recursive function to print all roll numbers
void printRollNumbers(struct Student students[], int index, int totalStudents) {
    if (index == totalStudents) return;
    printf("%d ", students[index].rollNumber);
    printRollNumbers(students, index + 1, totalStudents);
}

int main() {
    int totalStudents;

    printf("Enter the number of students: ");
    scanf("%d", &totalStudents);

    struct Student students[totalStudents];

    for (int i = 0; i < totalStudents; i++) {
        printf("\nEnter details for student %d (RollNo Name Marks1 Marks2 Marks3): ", i + 1);
        scanf("%d %[a-zA-Z ] %f %f %f",
              &students[i].rollNumber,
              students[i].studentName,
              &students[i].marks[0],
              &students[i].marks[1],
              &students[i].marks[2]);

        students[i].totalMarks = calculateTotal(students[i].marks);
        students[i].averageMarks = calculateAverage(students[i].totalMarks);
        students[i].grade = getGrade(students[i].averageMarks);
    }

     // Sort students by roll number
    for (int i = 0; i < totalStudents - 1; i++) {
        for (int j = i + 1; j < totalStudents; j++) {
            if (students[i].rollNumber > students[j].rollNumber) {
                struct Student temporaryStudent = students[i];
                students[i] = students[j];
                students[j] = temporaryStudent;
            }
        }
    }

    printf("\n\n");

    // Display student results
    for (int i = 0; i < totalStudents; i++) {
        printf("Roll: %d\n", students[i].rollNumber);
        printf("Name: %s\n", students[i].studentName);
        printf("Total: %.2f\n", students[i].totalMarks);
        printf("Average: %.2f\n", students[i].averageMarks);
        printf("Grade: %c\n", students[i].grade);

        if (students[i].averageMarks >= 35) {
            printf("Performance: ");
            showPerformance(students[i].grade);
        }
        printf("\n\n");
    }

    printf("List of Roll Numbers : ");
    printRollNumbers(students, 0, totalStudents);
    printf("\n");

    return 0;
}
