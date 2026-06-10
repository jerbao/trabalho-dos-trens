#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Posições iniciais (x,y) dos 6 trens — qualquer ponto dentro do anel serve;
    //o wall-following em trem.cpp não depende da posição inicial (a trava de
    //limite garante que o trem será puxado para a quina correta no próximo frame).
    // Y inicial = ymin/ymax (label sobre H1/H2). Com o wf fixo (clamp em
    // ymin/ymax, não ymin+H/ymax-H), o trem nasce já no trilho horizontal.
    const int X[6]     = {   0, 281, 520,   0, 281, 520 };
    const int Y[6]     = {  20,  20,  20, 250, 430, 250 };
    //Limites do anel (paredes) de cada trem, usados pelo wall-following.
    const int X_MIN[6] = {   0, 260, 520,   0, 260, 520 };
    const int X_MAX[6] = { 260, 520, 780, 260, 520, 780 };
    const int Y_MIN[6] = {  20,  20,  20, 250, 430, 250 };
    const int Y_MAX[6] = { 250, 430, 250, 540, 540, 540 };

    //Atalhos para os 6 QSliders do .ui
    QSlider *sl[6] = {
        ui->slider_t1, ui->slider_t2, ui->slider_t3,
        ui->slider_t4, ui->slider_t5, ui->slider_t6
    };

    // Mapeamento das 9 regiões críticas (geometria 3 colunas × 2 anéis):
    //   rc[0]  trilhoV_meio_C1       (x=260, y=250..430)   -> T2, T4
    //   rc[1]  chaveV_C1C2_cima      (x=260, y=20..267)    -> T1, T2
    //   rc[2]  chaveV_C2C3_cima      (x=520, y=20..267)    -> T2, T3
    //   rc[3]  chaveH_meio_C1        (y=250, x=0..260)     -> T1, T4
    //   rc[4]  chaveH_meio_C2        (y=430, x=260..520)   -> T2, T5
    //   rc[5]  chaveH_meio_C3        (y=250, x=520..780)   -> T3, T6
    //   rc[6]  chaveV_C1C2_baixo     (x=260, y=430..557)   -> T4, T5
    //   rc[7]  chaveV_C2C3_baixo     (x=520, y=430..557)   -> T5, T6
    //   rc[8]  trilhoV_meio_C3       (x=520, y=250..430)   -> T2, T6

    //Cria os 6 trens (ID, x, y, xmin, xmax, ymin, ymax, ponteiro para os 9 mutexes,
    //ponteiro para semEsquerda, ponteiro para semDireita).
    for (int id = 1; id <= 6; ++id)
        trem[id-1] = new Trem(id, X[id-1], Y[id-1],
                              X_MIN[id-1], X_MAX[id-1],
                              Y_MIN[id-1], Y_MAX[id-1],
                              rc, &semEsquerda, &semDireita);

    /*
     * Conecta o sinal UPDATEGUI de cada trem à função UPDATEINTERFACE.
     * Sempre que updateGUI for emitido, updateInterface será executada
     * com os 3 inteiros (id, x, y) emitidos pelo trem.
     */
    for (int id = 1; id <= 6; ++id)
        connect(trem[id-1], SIGNAL(updateGUI(int,int,int)),
                this,       SLOT(updateInterface(int,int,int)));

    //Sincroniza cada slider com a velocidade inicial (100 ms) e conecta-o ao seu trem.
    //setValue(100) dispara valueChanged, que já chama setVelocidade(100) no trem correspondente.
    for (int i = 0; i < 6; ++i) {
        sl[i]->setValue(100);
        connect(sl[i], SIGNAL(valueChanged(int)),
                trem[i], SLOT(setVelocidade(int)));
    }

    //Inicia as threads automaticamente ao abrir a janela
    for (int i = 0; i < 6; ++i)
        trem[i]->start();

}

//Função que será executada quando o sinal UPDATEGUI for emitido
void MainWindow::updateInterface(int id, int x, int y){
    switch(id){
    case 1: //Atualiza a posição do objeto da tela (quadrado) que representa o trem1
        ui->label_trem1->setGeometry(x,y,21,17);
        break;
    case 2: //Atualiza a posição do objeto da tela (quadrado) que representa o trem2
        ui->label_trem2->setGeometry(x,y,21,17);
        break;
    case 3: //Trem 3
        ui->label_trem3->setGeometry(x,y,21,17);
        break;
    case 4: //Trem 4
        ui->label_trem4->setGeometry(x,y,21,17);
        break;
    case 5: //Trem 5
        ui->label_trem5->setGeometry(x,y,21,17);
        break;
    case 6: //Trem 6
        ui->label_trem6->setGeometry(x,y,21,17);
        break;
    default:
        break;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
