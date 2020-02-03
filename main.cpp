#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}


/*  Todo:
 *  Придумать как передавать дебаг инфу вместо принтфа. Пусть кидает сигнал а там сами разберемся? Принимать в QDebug а потом только нужное в окно ошибок?
 *  Почистить все лишнее.
 *  Собрать!
 *
 *
 *
*/
