#include <stdio.h>

void starstoptobottom() {
    int userinput = 0;

    printf("Wie viele Reihen möchtest du haben? ");
    scanf("%d", &userinput);

    printf("Du möchtest %d Reihen haben\n", userinput);

    for(int i=1; i<=userinput; i++){
        char stars[100] = "";
        for(int j=0; j<i; j++){
            strcat(stars, "* ");
        }
    printf("%s\n", stars);
    }
}

void starsbottomtotop(){
    int userinput = 0;

    printf("Wie viele Reihen möchtest du haben? ");
    scanf("%d", &userinput);

    printf("Du möchtest %d Reihen haben\n", userinput);

    while(userinput != 0){
        char stars[100] = "";
        for(int i=userinput; i>0; i--){
            strcat(stars, "*");
        }
        printf("%s\n", stars);
        userinput -= 1;
    }

}

void doublepyramide(){
    int userinput = 0;
    int userinputcopy = 0;

    printf("Wie viele Reihen möchtest du haben? ");
    scanf("%d", &userinput);
    userinputcopy = userinput;

    printf("Du möchtest %d Reihen haben\n", userinput);

    while(userinput != 0){
        char topstars[100] = "";
        for(int i=userinput; i>0; i--){
            strcat(topstars, "* ");
        }
        printf("%s\n", topstars);
        userinput -=1;
    }

    int j=1;
    while (j<=userinputcopy){
        char bottomstars[100]="";
        for(int x=0; x<j; x++){
            strcat(bottomstars, "* ");
        }
        printf("%s\n", bottomstars);
        j++;
    }
}
