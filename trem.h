#ifndef TREM_H
#define TREM_H

#include <QThread>
#include <QMutex>
#include <QSemaphore>

/*
 * Classe Trem herda QThread
 * Classe Trem passa a ser uma thread.
 * A função START inicializa a thread. Após inicializada, a thread irá executar a função RUN.
 * Para parar a execução da função RUN da thread, basta executar a função TERMINATE.
 *
*/
class Trem: public QThread{
 Q_OBJECT
public:
    Trem(int, int, int, int, int, int, int, QMutex*, QSemaphore*, QSemaphore*);
                                       //(ID, x, y, xmin, xmax, ymin, ymax, rc*, semEsq*, semDir*)
    void run();                        //função a ser executada pela thread

//Cria um sinal
signals:
    void updateGUI(int,int,int);

public slots:
    void setVelocidade(int);

private:
   int x, y;            //posição atual do trem na tela
   int ID;              //ID do trem
   int xmin, xmax;      //limites do anel do trem (paredes verticais)
   int ymin, ymax;      //limites do anel do trem (paredes horizontais)
   int velocidade;      //Velocidade. É o tempo de dormir em milisegundos entre a mudança de posição do trem
   QMutex *rc;          //ponteiro para o vetor de 9 mutexes (pertence à MainWindow)
   QSemaphore *semEsq;  //ponteiro para o semáforo da metade esquerda (pertence à MainWindow)
   QSemaphore *semDir;  //ponteiro para o semáforo da metade direita (pertence à MainWindow)
   // Flags de estado das 9 RCs: rc_locked[i] = true se este trem está
   // atualmente dentro da RC i (lock feito, unlock ainda não).
   bool rc_locked[9];
};

#endif // TREM_H

