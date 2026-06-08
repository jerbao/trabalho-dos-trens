#include "trem.h"
#include <QtCore>

//Construtor
Trem::Trem(int ID, int x, int y){
    this->ID = ID;
    this->x = x;
    this->y = y;
    velocidade = 100;
}

//Função a ser executada após executar trem->START
void Trem::run(){
    while(true){
        //Lógica de velocidade: 0 = parado, 200 (max do slider) = mais rápido.
        //O slider entrega valores em [0,200]; convertemos para sleep em ms.
        int sleep = 0;
        if (velocidade > 0) sleep = 200 - velocidade;

        if (velocidade == 0) {
            // Trem parado: não mexe em x,y. Emite sinal para a GUI redesenhar
            // na mesma posição. Em seguida, dorme um pouco para não rodar solto.
            emit updateGUI(ID, x, y);
            msleep(50);
            continue;
        }

        switch(ID){
        case 1: //T1: anel horário X∈[0,260], Y∈[20,250]
            if (y == 20  && x < 260)  x += 10;
            else if (x == 260 && y < 250) y += 10;
            else if (y == 250 && x > 0)   x -= 10;
            else if (x == 0   && y > 20)  y -= 10;
            break;
        case 2: //T2: anel horário X∈[260,520], Y∈[20,430]
            if (y == 20  && x < 520)  x += 10;
            else if (x == 520 && y < 430) y += 10;
            else if (y == 430 && x > 260) x -= 10;
            else if (x == 260 && y > 20)  y -= 10;
            break;
        case 3: //T3: anel horário X∈[520,780], Y∈[20,250]
            if (y == 20  && x < 780)  x += 10;
            else if (x == 780 && y < 250) y += 10;
            else if (y == 250 && x > 520) x -= 10;
            else if (x == 520 && y > 20)  y -= 10;
            break;
        case 4: //T4: anel horário X∈[0,260], Y∈[250,540]
            if (y == 250 && x < 260)  x += 10;
            else if (x == 260 && y < 540) y += 10;
            else if (y == 540 && x > 0)   x -= 10;
            else if (x == 0   && y > 250) y -= 10;
            break;
        case 5: //T5: anel horário X∈[260,520], Y∈[430,540]
            if (y == 430 && x < 520)  x += 10;
            else if (x == 520 && y < 540) y += 10;
            else if (y == 540 && x > 260) x -= 10;
            else if (x == 260 && y > 430) y -= 10;
            break;
        case 6: //T6: anel horário X∈[520,780], Y∈[250,540]
            if (y == 250 && x < 780)  x += 10;
            else if (x == 780 && y < 540) y += 10;
            else if (y == 540 && x > 520) x -= 10;
            else if (x == 520 && y > 250) y -= 10;
            break;
        default:
            break;
        }
        emit updateGUI(ID, x, y);    //Emite um sinal
        msleep(sleep);
    }
}

//Slot que permite ao QSlider ajustar a velocidade (msleep) desta thread
void Trem::setVelocidade(int v){
    velocidade = v;
}
