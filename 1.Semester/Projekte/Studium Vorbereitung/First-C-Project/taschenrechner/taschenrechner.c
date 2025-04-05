#include <stdio.h>
#include <conio.h>

void verrechnen() {
    int continuewert = 1;
    char operator;
    double num1, num2, ergebnis;

    while (continuewert) {
        printf("Gebe deinen Operator ein: +, -, *, / oder druecke ESC zum Beenden. \n");

        // Leere den Eingabepuffer, um unerwünschte Zeichen zu vermeiden
        fflush(stdin);

        operator = _getch();
        if (operator == 27) {
            continuewert = 0;
            break;
        } else {
            printf("%c\n", operator); // Operator anzeigen

            printf("Welche 2 Zahlen willst du verrechnen? Gebe 2 Zahlen an. \n");
            scanf("%lf %lf", &num1, &num2);

            switch (operator) {
                case '+':
                    ergebnis = num1 + num2;
                    printf("Das Ergebnis deiner Addition lautet: %.2lf \n\n", ergebnis);
                    break;
                case '-':
                    ergebnis = num1 - num2;
                    printf("Das Ergebnis deiner Subtraktion lautet: %.2lf \n\n", ergebnis);
                    break;
                case '*':
                    ergebnis = num1 * num2;
                    printf("Das Ergebnis deiner Multiplikation lautet: %.2lf \n\n", ergebnis);
                    break;
                case '/':
                    if (num2 != 0) {
                        ergebnis = num1 / num2;
                        printf("Das Ergebnis deiner Division lautet: %.2lf \n\n", ergebnis);
                    } else {
                        printf("Fehler: Division durch Null ist nicht erlaubt.\n");
                    }
                    break;
                default:
                    printf("Ungültige Eingabe!\n");
            }
        }
    }
}

int main() {
    //verrechnen();
    //starstoptobottom();
    //starsbottomtotop();
    //doublepyramide();
    //bubblesort();
    //uebung1();
    //Stringumgang();
    dateienhandling();
    return 0;
}
