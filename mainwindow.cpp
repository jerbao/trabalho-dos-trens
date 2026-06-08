#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Posições iniciais (x,y) dos 6 trens — canto superior-esquerdo de cada
    //anel, sobrepostas com o widget de label_tremN correspondente. Cada trem
    //entra no anel pelo trilho superior-esquerdo (borda externa), o que casa
    //com a primeira condição `y == Y_MIN && x < X_MAX` em trem.cpp.
    const int X[6] = {   0, 260, 520,   0, 260, 520 };
    const int Y[6] = {  20,  20,  20, 250, 430, 250 };

    //Atalhos para os 6 QSliders do .ui
    QSlider *sl[6] = {
        ui->slider_t1, ui->slider_t2, ui->slider_t3,
        ui->slider_t4, ui->slider_t5, ui->slider_t6
    };

    //Cria os 6 trens (ID, posição X, posição Y)
    for (int id = 1; id <= 6; ++id)
        trem[id-1] = new Trem(id, X[id-1], Y[id-1]);

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
