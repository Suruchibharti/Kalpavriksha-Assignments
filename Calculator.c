#include <stdio.h>
#include <string.h>

#define SIZE 900   

//use to check operator
int operatorCheck(char c) {
    return (c=='+' || c=='-' || c=='*' || c=='/');
}

//use for priority check of operator
int precedence(char c) {
    if (c == '/') return 3;   // Changed precedence for strict DMAS
    if (c == '*') return 2;
    if (c == '+') return 1;
    if (c == '-') return 0;
    return -1;
}

//use for evaluate the operation b/w two operand
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

//use to check space
int is_space(char c) {
    return (c == ' ');
}

//use to check digit
int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

// merge numbers with spaces like "2 2" → 22
int parse_number(char *exp, int *i, int len) {
    int val = 0;
    while (*i < len) {
        if (is_space(exp[*i])) {
            (*i)++; // skip space between digits
            continue;
        }
        if (!is_digit(exp[*i])) break;
        val = val * 10 + (exp[*i] - '0');
        (*i)++;
    }
    return val;
}

//it evaluates the expression
int calc(char *exp, int *err) {
    int numStack[SIZE], nTop=-1;     
    char opStack[SIZE]; int oTop=-1;
    int len = strlen(exp);

    for (int i = 0; i < len;) {
        if (is_space(exp[i])) { i++; continue; }

        if (is_digit(exp[i])) {
           
            int val = parse_number(exp, &i, len);
            numStack[++nTop] = val;
            continue;
        }

        if (operatorCheck(exp[i])) {
            while (oTop >= 0 && precedence(opStack[oTop]) > precedence(exp[i])) {
                int b = numStack[nTop--];
                int a = numStack[nTop--];
                char op = opStack[oTop--];
                numStack[++nTop] = calculateTheOperations(a, b, op, err);
                if (*err) return 0;
            }
            opStack[++oTop] = exp[i];
        } else {
            *err = 2; return 0;
        }
        i++;
    }

    // Remaining operations
    while (oTop >= 0) {
        int b = numStack[nTop--];
        int a = numStack[nTop--];
        char op = opStack[oTop--];
        numStack[++nTop] = calculateTheOperations(a, b, op, err);
        if (*err) return 0;
    }

    return numStack[nTop];
}

//this is the main function
int main() {
    char expression[SIZE];
    printf("Enter the expression: ");
    scanf(" %[^\n]", expression);  // fixes space reading

    int err=0;
    int output=calc(expression, &err);

    if (err==1) printf("Error: Division by zero.\n");
    else if (err==2) printf("Error: Invalid expression.\n");
    else printf("Output = %d\n", output);

return 0;
}
