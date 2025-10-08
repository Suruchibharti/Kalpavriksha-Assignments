#include <stdio.h>
#include <string.h>

#define MAX_SIZE 900   

//use to check operator
int isOperator(char c) {
    return (c=='+' || c=='-' || c=='*' || c=='/');
}

//use for priority check of operator
int getPrecedence(char c) {
    if (c == '/') return 3;   // Changed precedence for strict DMAS
    if (c == '*') return 2;
    if (c == '+') return 1;
    if (c == '-') return 0;
    return -1;
}

//use for evaluate the operation b/w two operand
int performOperation(int x, int y, char c, int *err) {
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
int isSpace(char c) {
    return (c == ' ');
}

//use to check digit
int isDigit(char c) {
    return (c >= '0' && c <= '9');
}

// merge numbers with spaces like "2 2" → 22
int parseNumber(char *exp, int *i, int len) {
    int val = 0;
    while (*i < len) {
        if (isSpace(exp[*i])) {
            (*i)++; // skip space between digits
            continue;
        }
        if (!isDigit(exp[*i])) break;
        val = val * 10 + (exp[*i] - '0');
        (*i)++;
    }
    return val;
}

//it evaluates the expression
int evaluateExpression(char *expression, int *errorFlag) {
    int numberStack[MAX_SIZE], numberTop = -1;     
    char operatorStack[MAX_SIZE]; 
    int operatorTop = -1;
    int length = strlen(expression);

    for (int i = 0; i < length;) {
        if (isSpace(expression[i])) { 
            i++; 
            continue; 
        }

        if (isDigit(expression[i])) {
            int value = parseNumber(expression, &i, length);
            numberStack[++numberTop] = value;
            continue;
        }

        if (isOperator(expression[i])) {
            while (operatorTop >= 0 && getPrecedence(operatorStack[operatorTop]) > getPrecedence(expression[i])) {
                int rightOperand = numberStack[numberTop--];
                int leftOperand = numberStack[numberTop--];
                char currentOperator = operatorStack[operatorTop--];
                numberStack[++numberTop] = performOperation(leftOperand, rightOperand, currentOperator, errorFlag);
                if (*errorFlag) return 0;
            }
            operatorStack[++operatorTop] = expression[i];
        } else {
            *errorFlag = 2;
            return 0;
        }
        i++;
    }

    // Process remaining operators
    while (operatorTop >= 0) {
        int rightOperand = numberStack[numberTop--];
        int leftOperand = numberStack[numberTop--];
        char currentOperator = operatorStack[operatorTop--];
        numberStack[++numberTop] = performOperation(leftOperand, rightOperand, currentOperator, errorFlag);
        if (*errorFlag) return 0;
    }

    return numberStack[numberTop];
}

//this is the main function
int main() {
     char expression[MAX_SIZE];
    printf("Enter the expression: ");
    scanf(" %[^\n]", expression);  // Read expression including spaces

    int errorFlag = 0;
    int result = evaluateExpression(expression, &errorFlag);

    if (errorFlag == 1) 
        printf("Error: Division by zero.\n");
    else if (errorFlag == 2) 
        printf("Error: Invalid expression.\n");
    else 
        printf("Output = %d\n", result);

   return 0;
}
