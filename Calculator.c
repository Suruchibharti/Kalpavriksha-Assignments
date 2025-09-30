#include <stdio.h>
#include <string.h>

#define SIZE 900   

int operatorCheck(char c) {
    return (c=='+' || c=='-' || c=='*' || c=='/');
}

int precedence(char c) {
    if (c=='*' || c=='/') return 2;
    if (c=='+' || c=='-') return 1;
    return 0;
}

int calculateTheOperations(int x, int y, char c, int *err) {
    if (c=='+') return x+y;
    if (c=='-') return x-y;
    if (c=='*') return x*y;
    if (c=='/') {
        if (y==0) {
            *err=1; return 0;
        }
        return x/y;
    }
    return 0;
}

int is_space(char c) {
    return (c == ' ');
}

int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

int main() {
    char expression[SIZE];
       printf("Enter the expression: ");
     scanf("%[^\n]", expression); 
     int err=0;
   int output=calc(expression,&err);

    if (err==1) printf("Error: Division by zero.\n");
    else if (err==2) printf("Error: Invalid expression.\n");
    else printf("Output = %d\n", output);

    return 0;
}
