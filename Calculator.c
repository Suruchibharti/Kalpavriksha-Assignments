#include <stdio.h>
#include <string.h>

#define MAX_SIZE 900   

// Checks if a character is an operator
int isOperator(char character) {
    return (character == '+' || character == '-' || character == '*' || character == '/');
}

// Returns the precedence of an operator
int getPrecedence(char operatorChar) {
    if (operatorChar == '/') return 3;   // Highest priority for strict DMAS
    if (operatorChar == '*') return 2;
    if (operatorChar == '+') return 1;
    if (operatorChar == '-') return 0;
    return -1;
}

// Performs the operation between two operands
int performOperation(int leftOperand, int rightOperand, char operatorChar, int *errorFlag) {
    if (operatorChar == '+') return leftOperand + rightOperand;
    if (operatorChar == '-') return leftOperand - rightOperand;
    if (operatorChar == '*') return leftOperand * rightOperand;
    if (operatorChar == '/') {
        if (rightOperand == 0) {
            *errorFlag = 1; 
            return 0;
        }
        return leftOperand / rightOperand;
    }
    return 0;
}

// Checks if a character is a space
int isSpace(char character) {
    return (character == ' ');
}

// Checks if a character is a digit
int isDigit(char character) {
    return (character >= '0' && character <= '9');
}

// Parses and merges digits into a number, handling spaces between them (e.g., "2 2" → 22)
int parseNumber(char *expression, int *index, int length) {
    int value = 0;
    while (*index < length) {
        if (isSpace(expression[*index])) {
            (*index)++; // skip spaces
            continue;
        }
        if (!isDigit(expression[*index])) break;
        value = value * 10 + (expression[*index] - '0');
        (*index)++;
    }
    return value;
}

// Evaluates a mathematical expression
int evaluateExpression(char *expression, int *errorFlag) {
    int numberStack[MAX_SIZE], numberTop = -1;     
    char operatorStack[MAX_SIZE]; 
    int operatorTop = -1;
    int expressionLength = strlen(expression);

    for (int index = 0; index < expressionLength;) {
        if (isSpace(expression[index])) { 
            index++; 
            continue; 
        }

        if (isDigit(expression[index])) {
            int numberValue = parseNumber(expression, &index, expressionLength);
            numberStack[++numberTop] = numberValue;
            continue;
        }

        if (isOperator(expression[index])) {
            while (operatorTop >= 0 && getPrecedence(operatorStack[operatorTop]) > getPrecedence(expression[index])) {
                int rightOperand = numberStack[numberTop--];
                int leftOperand = numberStack[numberTop--];
                char currentOperator = operatorStack[operatorTop--];
                numberStack[++numberTop] = performOperation(leftOperand, rightOperand, currentOperator, errorFlag);
                if (*errorFlag) return 0;
            }
            operatorStack[++operatorTop] = expression[index];
        } else {
            *errorFlag = 2;
            return 0;
        }
        index++;
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

// Main function
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
