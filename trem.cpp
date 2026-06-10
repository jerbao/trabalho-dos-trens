#include "trem.h"
#include <QtCore>

// =====================================================================
// Helpers de detecção de RC (ponto atual → índice da RC).
// Geometria alinhada com mainwindow.h:31-41.
// =====================================================================
static int detectaRC(int x, int y){
    if (x == 260 && y >= 250 && y <= 430) return 0;  // V3 meio C1
    if (x == 260 && y >=  20 && y <= 267) return 1;  // V3 sup C1C2
    if (x == 520 && y >=  20 && y <= 267) return 2;  // V3 sup C2C3
    if (y == 250 && x >=   0 && x <= 260) return 3;  // H meio C1
    if (y == 430 && x >= 260 && x <= 520) return 4;  // H meio C2
    if (y == 250 && x >= 520 && x <= 780) return 5;  // H meio C3
    if (x == 260 && y >= 430 && y <= 557) return 6;  // V3 inf C1C2
    if (x == 520 && y >= 430 && y <= 557) return 7;  // V3 inf C2C3
    if (x == 520 && y >= 250 && y <= 430) return 8;  // V3 meio C3
    return -1;
}
static bool rcVertical(int i){
    return i==0 || i==1 || i==2 || i==6 || i==7 || i==8;
}
static int rcComprimento(int i){
    switch(i){
    case 0: return 180;  // V3 meio C1
    case 1: return 247;  // V3 sup C1C2
    case 2: return 247;  // V3 sup C2C3
    case 3: return 260;  // H meio C1
    case 4: return 260;  // H meio C2
    case 5: return 260;  // H meio C3
    case 6: return 127;  // V3 inf C1C2
    case 7: return 127;  // V3 inf C2C3
    case 8: return 180;  // V3 meio C3
    }
    return 0;
}
// rcContem: TRUE se (x,y) está dentro do retângulo da RC i.
// Usado pelo leave detection (1b) para liberar RCs quando o trem sai delas.
static bool rcContem(int i, int x, int y){
    switch(i){
    case 0: return x==260 && y>=250 && y<=430;  // V3 meio C1
    case 1: return x==260 && y>=20  && y<=267;  // V3 sup C1C2
    case 2: return x==520 && y>=20  && y<=267;  // V3 sup C2C3
    case 3: return y==250 && x>=0   && x<=260;  // H meio C1
    case 4: return y==430 && x>=260 && x<=520;  // H meio C2
    case 5: return y==250 && x>=520 && x<=780;  // H meio C3
    case 6: return x==260 && y>=430 && y<=557;  // V3 inf C1C2
    case 7: return x==520 && y>=430 && y<=557;  // V3 inf C2C3
    case 8: return x==520 && y>=250 && y<=430;  // V3 meio C3
    }
    return false;
}

//Construtor
Trem::Trem(int ID, int x, int y, int xmin, int xmax, int ymin, int ymax,
           QMutex *rc, QSemaphore *semEsq, QSemaphore *semDir){
    this->ID = ID;
    this->x = x;
    this->y = y;
    this->xmin = xmin;
    this->xmax = xmax;
    this->ymin = ymin;
    this->ymax = ymax;
    velocidade = 100;
    this->rc = rc;
    this->semEsq = semEsq;      // semáforo da metade esquerda
    this->semDir = semDir;      // semáforo da metade direita
    for (int i = 0; i < 9; ++i) {
        rc_locked[i] = false;
    }
}

//Função a ser executada após executar trem->START
void Trem::run(){
    while(true){
        int sleep = 0;
        if (velocidade > 0) sleep = 200 - velocidade;

        if (velocidade == 0) {
            emit updateGUI(ID, x, y);
            msleep(50);
            continue;
        }

        // 1. MÁQUINA DE ESTADOS E CONCORRÊNCIA
        //    Cada guarda dispara no ponto de FREIO (W/H antes da quina real)
        //    e locka a RC da frente + acquire do semáforo da macro-região.
        //    A posição é setada para a QUINA DE SAÍDA (xmax, ymax, etc.) para
        //    que o wall-following reconheça a próxima direção na iteração
        //    seguinte. A saída de cada RC é detectada automaticamente pelo
        //    bloco abaixo (distância percorrida >= comprimento).
        switch(ID){
            case 1:
                // Anel C1 sup: → ↓ ← ↑. Guardas disparam em (xmax, ymin) e
                // (xmin, ymax), que são os clamps reais do wf puro. Sem
                // set x artificial: o wf faz a transição da quina sozinho.
                if (x == 260 && y == 20) {
                    semEsq->acquire(1);            // vira ↓
                }
                else if (x == 0 && y == 250) {
                    semEsq->release(1);            // vira ↑
                }
                break;

            case 4:
                // Anel C1 inf: → ↓ ← ↑. ymin=250, ymax=540.
                if (x == 260 && y == 250) {
                    semEsq->acquire(1);
                }
                else if (x == 0 && y == 540) {
                    semEsq->release(1);
                }
                break;

            case 3:
                // Anel C3 sup: → ↓ ← ↑.
                if (x == 780 && y == 20) {
                    semDir->acquire(1);
                }
                else if (x == 520 && y == 250) {
                    semDir->release(1);
                }
                break;

            case 6:
                // Anel C3 inf: → ↓ ← ↑. ymin=250, ymax=540.
                if (x == 780 && y == 250) {
                    semDir->acquire(1);
                }
                else if (x == 520 && y == 540) {
                    semDir->release(1);
                }
                break;

            case 2:
                // Anel C2 grande: → (H1 sup) ↓ (V4 sup/meio) ← (H2) ↑ (V3 sup/meio)
                // Com wf puro, as guardas disparam em (xmax, y) e (xmin, y),
                // que são os clamps reais. Sem set artificial: wf faz a
                // transição da quina sozinho, percorrendo os W últimos pixels
                // do trilho horizontal antes de virar ↓ ou ↑.
                if (x == 520 && y == 20) {
                    semDir->acquire(1);            // entra em V4
                }
                else if (x == 520 && y == 430) {
                    semDir->release(1);            // sai de V4
                    semEsq->acquire(1);            // entra em V3
                }
                else if (x == 260 && y == 20) {
                    semEsq->release(1);            // sai de V3
                }
                break;

            case 5:
                // Anel C2 inf: → (H1 inf) ↓ (V4 inf) ← (H2 inf) ↑ (V3 inf)
                if (x == 520 && y == 430) {
                    semDir->acquire(1);
                }
                else if (x == 520 && y == 540) {
                    semDir->release(1);
                    semEsq->acquire(1);
                }
                else if (x == 260 && y == 430) {
                    semEsq->release(1);
                }
                break;
        }

        // 1b. DETECÇÃO DE RC POR LEAVE DETECTION
        //     Entrada: ponto atual pertence a uma RC que ainda não travamos?
        //              Lock.
        //     Saída:   o ponto atual está FORA do retângulo da RC i que
        //              temos travada? Unlock. (Mais robusto que distância,
        //              que falha para RCs horizontais onde o trem não
        //              percorre o comprimento total.)
        int rc_frente = detectaRC(x, y);
        if (rc_frente != -1 && !rc_locked[rc_frente]) {
            rc[rc_frente].lock();
            rc_locked[rc_frente] = true;
        }
        for (int i = 0; i < 9; ++i) {
            if (!rc_locked[i]) continue;
            if (!rcContem(i, x, y)) {
                rc[i].unlock();
                rc_locked[i] = false;
            }
        }

        // 2. MOVIMENTO ESPACIAL (wf puro: sem desconto de W nas condições
        //    verticais). O label do trem tem 21x17 px. O freio acontece em
        //    y=ymin, y=ymax, x=xmax ou x=xmin. As transições → ↓ e ← ↑ são
        //    feitas pelo próprio wf em (xmax, ymin) e (xmin, ymax) — o label
        //    "atravessa" a quina H+V nos últimos/pixels de W, sem salto
        //    artificial (sem `x = xmax` na guarda). Para T2/T5 (xmin=260), o
        //    ↑ fica automaticamente em x=260 (=xmin), com label sobre V3
        //    (x=260..281). Não há teletransporte nas quinas.
        const int W = 21, H = 17;
        int step = velocidade;
        if (y == ymin && x < xmax) {
            x += step;
            if (x > xmax) x = xmax;
        }
        else if (x >= xmax && y < ymax) {
            y += step;
            if (y > ymax) y = ymax;
        }
        else if (y == ymax && x > xmin) {
            x -= step;
            if (x < xmin) x = xmin;
        }
        else if (x <= xmin && y > ymin) {
            y -= step;
            if (y < ymin) y = ymin;
        }

        // 3. ATUALIZAÇÃO DA UI
        emit updateGUI(ID, x, y);
        msleep(sleep);
    }
}

//Slot que permite ao QSlider ajustar a velocidade (msleep) desta thread
void Trem::setVelocidade(int v){
    velocidade = v;
}
