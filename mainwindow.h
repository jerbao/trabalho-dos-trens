#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMutex>
#include <QSemaphore>
#include "trem.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


public slots:
    void updateInterface(int,int,int);

private:
    Ui::MainWindow *ui;

    //Cria os objetos TREM's
    Trem *trem[6];

    // 9 regiões críticas do circuito (1 mutex por RC, mapeadas pela Figura 2 do PDF):
    //   rc[0]  trilhoV_meio_C1    (x=260, y=250..430)   -> T2, T4
    //   rc[1]  chaveV_C1C2_cima   (x=260, y=20..267)    -> T1, T2
    //   rc[2]  chaveV_C2C3_cima   (x=520, y=20..267)    -> T2, T3
    //   rc[3]  chaveH_meio_C1     (y=250, x=0..260)     -> T1, T4
    //   rc[4]  chaveH_meio_C2     (y=430, x=260..520)   -> T2, T5
    //   rc[5]  chaveH_meio_C3     (y=250, x=520..780)   -> T3, T6
    //   rc[6]  chaveV_C1C2_baixo  (x=260, y=430..557)   -> T4, T5
    //   rc[7]  chaveV_C2C3_baixo  (x=520, y=430..557)   -> T5, T6
    //   rc[8]  trilhoV_meio_C3    (x=520, y=250..430)   -> T2, T6
    QMutex rc[9];

    // Semáforo da metade ESQUERDA da malha (x ≤ 260). Limite 2 quebra o
    // ciclo de deadlock de 3 trens (T1, T4, T2 ou T5) sem reduzir demais
    // a concorrência. Usado por T1 e T4 (exclusivos) e por T2/T5 quando
    // vão lockar RCs com x ≤ 260.
    QSemaphore semEsquerda{2};
    // Semáforo da metade DIREITA da malha (x ≥ 520). Limite 2, mesmo motivo.
    // Usado por T3 e T6 (exclusivos) e por T2/T5 quando vão lockar RCs com
    // x ≥ 520. A aquisição/liberação é puramente por coordenadas, não
    // depende do estado dos mutexes das RCs pontuais.
    QSemaphore semDireita{2};
};

#endif // MAINWINDOW_H
