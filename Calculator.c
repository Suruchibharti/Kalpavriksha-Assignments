#include <stdio.h>
#include <string.h>

#define SIZE 900   

//use to chek operator
int operatorCheck(char c) {
    return (c=='+' || c=='-' || c=='*' || c=='/');
}

//use for priority check of operator
int precedence(char c) {
    if (c=='*' || c=='/') return 2;
    if (c=='+' || c=='-') return 1;
    return 0;
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

//use to chek digit
int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

//it evaluates the expression
//we are using two stacks: numStacks for number and opStack for operator
//nTop : index of top number in the numStacks 
//opTop : index of top operator in the opStack
int calc(char *exp, int *err) {
    int numStack[SIZE], nTop=-1;     
    char opStack[SIZE]; int oTop=-1;
    int len=strlen(exp);

    for (int i=0;i<len;) {
        if (is_space(exp[i])) { i++; continue; }

        if (is_digit(exp[i])) {
            int val=0;
            while (i<len && is_digit(exp[i])) {
                val=val*10+(exp[i]-'0');
                i++;
            }
            numStack[++nTop]=val;
            continue;
        }

        if (operatorCheck(exp[i])) {
            while (oTop>=0 && precedence(opStack[oTop]) >= precedence(exp[i])) {
                int b=numStack[nTop];
                nTop--;
                int a=numStack[nTop];
                nTop--;
                char op=opStack[oTop];
                oTop--;
                nTop++;
                numStack[nTop]=calculateTheOperations(a,b,op,err);
                if (*err) return 0;
            }
            ++oTop;
            opStack[oTop]=exp[i];
        } else {
            *err=2; return 0;
        }
        i++;
    }

    while (oTop>=0) {
        int b=numStack[nTop--];
        int a=numStack[nTop--];
        char op=opStack[oTop--];
        numStack[++nTop]=calculateTheOperations(a,b,op,err);
        if (*err) return 0;
    }
    return numStack[nTop];
}

//this is the main function
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
