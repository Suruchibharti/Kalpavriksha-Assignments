#include <stdio.h>
#include <string.h>

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
