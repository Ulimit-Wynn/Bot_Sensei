/**
 * Author: Moad EL KAZDADI
 * Contat: moad.el-kazdadi@institutoptique.fr
 * Le code a été conçu pour commander une platine 2 axes à deux moteurs 
 * pas à pas à travers une liaison RS232.
 */

#include "mbed.h"

#define pas_hor 1720 //nombres de pas pour parcourir l'entiereté de la grille horizontalement
#define pas_ver 1610 //nombres de pas pour parcourir l'entiereté de la grille verticalement

DigitalOut signal_pos_horizontal_A(A0); //signal positif de la bobine A du moteur horizontal
DigitalOut signal_neg_horizontal_A(D5); //signal negatif de la bobine A du moteur horizontal
DigitalOut signal_pos_horizontal_B(D4); //signal positif de la bobine B du moteur horizontal
DigitalOut signal_neg_horizontal_B(D3); //signal negatif de la bobine B du moteur horizontal
DigitalOut signal_pos_vertical_A(D6); //signal positif de la bobine A du moteur vertical
DigitalOut signal_neg_vertical_A(D7); //signal negatif de la bobine A du moteur vertical
DigitalOut signal_pos_vertical_B(D8); //signal positif de la bobine B du moteur vertical
DigitalOut signal_neg_vertical_B(D9); //signal negatif de la bobine B du moteur vertical

DigitalOut enable(A1);


DigitalIn switchH(D13); // Variable qui vaut 1 quand le moteur horizontal atteint la position 0
DigitalIn switchV(D12); // Variable qui vaut 1 quand le moteur vertical atteint la position 0

Serial rs232(USBTX, USBRX); // liason RS232 connectée à l'interface MATLAB

int init_x = 0;
int init_y = 0;

char instructions [100];
int nbr_instructions = 0;

char data_received = 0; // Variable pour stocker les commandes venant de l'interface

void ISR_get_data(void);

void initialiser(void);

void reset(void);

void reglage_sigs(int dim, int sig1, int sig2, int sig3, int sig4);

void prochainStep(int dim, int numero);

void marche_up(float attente, int nombre_de_pas);

void marche_down(float attente, int nombre_de_pas);

void marche_right(float attente, int nombre_de_pas);

void marche_left(float attente, int nombre_de_pas);

int main() {
    rs232.baud(115200);
    rs232.attach(&ISR_get_data);
    while(1) {
        }
        
}


void reglage_sigs(int dim, int sig1, int sig2, int sig3, int sig4)
{
//USAGE: Permet de couper ou laisser passer les courants traversant les 4 bobines
//dim: Variable valant 0 pour piloter le moteur horizontal, et 1 pour le vertical
//sigi: Signal binaire pour piloter le courant traversant la bobine i
    
    if (dim==0){
        signal_pos_horizontal_A= sig1;
        signal_neg_horizontal_A= sig2;
        signal_pos_horizontal_B= sig3;
        signal_neg_horizontal_B= sig4;
        signal_pos_vertical_A= 0;
        signal_neg_vertical_A= 0;
        signal_pos_vertical_B= 0;
        signal_neg_vertical_B= 0;
    }
    if (dim==1){
        signal_pos_horizontal_A= 0;
        signal_neg_horizontal_A= 0;
        signal_pos_horizontal_B= 0;
        signal_neg_horizontal_B= 0;
        signal_pos_vertical_A= sig1;
        signal_neg_vertical_A= sig2;
        signal_pos_vertical_B= sig3;
        signal_neg_vertical_B= sig4;
    }
}



void prochainStep(int dim, int numero)
{
// Usage: Cette fonction permet de mettre les bobines en une configuration voulue
// (entre 0 et 3), et donc permet d'appliquer un champ magnetique pour ramener 
// le roteur en une position
//dim: Variable de dimension pour choisir en les moteurs vertical et horizontal
//numero: Numero entre 0 et 3 indiquant la position du moteur
    if (numero == 0) {
        reglage_sigs(dim, 1, 0, 1, 0);
    }
    if (numero == 1) {
        reglage_sigs(dim, 0, 1, 1, 0);
    }
    if (numero == 2) {
        reglage_sigs(dim, 0, 1, 0, 1);
    }
    if (numero == 3) {
        reglage_sigs(dim, 1, 0, 0, 1);
    }
}

void marche_down(float attente, int nombre_de_pas)
{
// Usage: Cette fonction permet d'effectuer un nombre de pas avec le moteur vers le haut, en le réglant
// successivement avec la fonction prochainStep. Un pas correspond aux fait de tourner
// le moteur 4 fois afin qu'il fasse un cycle entier
// Attente: Nombre réel indiquant la durée à attendre entre chaque cycle.
// nombre_de_pas: Nombre entier indiquant le nombre de cycles à effectuer.
    enable = 1;
    for (int i=0; i <= nombre_de_pas; i++) {
        if (switchV.read()==1) {
            // si le detecteur de bors (du bas) est activé, on arrete la marche
            return;
        }
        prochainStep(1, 3 - (i % 4));
        wait(attente);
    }
    enable = 0;
}

void marche_up(float attente, int nombre_de_pas)
{
// Usage: Cette fonction permet d'effectuer un nombre de pas avec le moteur vers le bas, en le réglant
// successivement avec la fonction prochainStep. Un pas correspond aux fait de tourner
// le moteur 4 fois afin qu'il fasse un cycle entier
// Attente: Nombre réel indiquant la durée à attendre entre chaque cycle.
// nombre_de_pas: Nombre entier indiquant le nombre de cycles à effectuer.
    enable = 1;
    for (int i=0; i <= nombre_de_pas; i++) {
        prochainStep(1, i % 4);
        wait(attente);
    }
    enable = 0;
}

void marche_right(float attente, int nombre_de_pas)
{
// Usage: Cette fonction permet d'effectuer un nombre de pas avec le moteur vers la droite, en le réglant
// successivement avec la fonction prochainStep. Un pas correspond aux fait de tourner
// le moteur 4 fois afin qu'il fasse un cycle entier
// Attente: Nombre réel indiquant la durée à attendre entre chaque cycle.
// nombre_de_pas: Nombre entier indiquant le nombre de cycles à effectuer.
    enable = 1;
    for (int i=0; i <= nombre_de_pas; i++) {
        if (switchH.read()==1) {
            // si le detecteur de bors (du bas) est activé, on arrete la marche
            return;
        } 
        prochainStep(0, i % 4);
        wait(attente);
    }
    enable = 0;
}

void marche_left(float attente, int nombre_de_pas)
{
// Usage: Cette fonction permet d'effectuer un nombre de pas avec le moteur vers la gauche, en le réglant
// successivement avec la fonction prochainStep. Un pas correspond aux fait de tourner
// le moteur 4 fois afin qu'il fasse un cycle entier
// Attente: Nombre réel indiquant la durée à attendre entre chaque cycle.
// nombre_de_pas: Nombre entier indiquant le nombre de cycles à effectuer.
    enable = 1;
    for (int i=0; i <= nombre_de_pas; i++) {
        prochainStep(0, 3 - (i % 4));
        wait(attente);
    }
    enable = 0;
}
void initialiser(){
    //Usage: Met le moteur en position de bas-gauche en utilisant les capteur.
    while(switchH.read()==0){marche_right(0.005, 10);}
    while(switchV.read()==0){marche_down(0.005, 10);}
}

void reset(){
    for (int i=nbr_instructions-1; i>-1; i--){
            switch(instructions[i]){
        case 'd':
        // commande pour descendre
            marche_up(0.005, pas_ver/9);
            instructions[i] = 'x';
            break;
        case 'u':
        // commande pour remonter
            marche_down(0.005, pas_ver/9);
            instructions[i] = 'x';
            break;
        case 'r':
            marche_left(0.005, pas_hor/9);
            instructions[i] = 'x';
        // commande pour aller à droite
            break;
        case 'l':
        // commande aller à gauche
            marche_right(0.005, pas_hor/9);
            instructions[i] = 'x';
            break;
        default:
            marche_up(0, 3);
            instructions[i] = 'x';
    }
    }
    nbr_instructions = 0;
}
void ISR_get_data(){
    //Usage: Cette fonction permet de récupérer depuis la liaison RS232 les 
    //charactères représentantles commandes à effectuer.
    data_received = rs232.getc();
    switch(data_received){
        case 'd':
        // commande pour descendre
            marche_down(0.005, pas_ver/9);
            rs232.printf("n");
            instructions[nbr_instructions] = 'd';
            nbr_instructions++;
            break;
        case 'u':
        // commande pour remonter
            marche_up(0.005, pas_ver/9);
            rs232.printf("n");
            instructions[nbr_instructions] = 'u';
            nbr_instructions++;
            break;
        case 'r':
        // commande pour aller à droite
            marche_right(0.005, pas_hor/9);
            rs232.printf("n");
            instructions[nbr_instructions] = 'r';
            nbr_instructions++;
            break;
        case 'l':
        // commande aller à gauche
            marche_left(0.005, pas_hor/9);
            rs232.printf("n");
            instructions[nbr_instructions] = 'l';
            nbr_instructions++;
            break;
        case 'z':
        // commande pour réinitialiser
            reset();
            rs232.printf("n");
            break;
        case 'i':
            initialiser();

            marche_left(pas_hor, 0.005);
            /*wait(0.01);
            init_x = (int)rs232.getc() - 49;
            marche_right(pas_hor*init_x, 0.005);
            rs232.printf("n");
            wait(0.01);
            init_y = (int)rs232.getc() - 49;
            marche_up(pas_ver*init_y, 0.005);
           rs232.printf("n");
           */ break;
        default:
            marche_up(0, 3);
    }
}