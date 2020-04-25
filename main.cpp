#include "mainwindow.h"

#include <QApplication>
#include <WaveDump.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    qDebug() << "qDebug (with lower case q) testing. everything is ok, i suppose";
    N6740 test;
    test.oldMain(0,0);

    return a.exec();
}


/*  Todo:
 *
 *
 *
 *  Сначала довести проект до этапа где можно подключиться к оцифровщику и выполнить все функции которые были доступны в родном WD,
 * и только потом чистить проект от лишнего мусора, попутно проверяя работоспособность.
 *
 *
 *  Полностью заменить CheckKeyboardCommand и соответсвующий вызов в Oldmain.
 *  Нужные функции из CKC - quit, restart, sendSWtrigger, singlewrite/continuouswrite, set_relative_threshold -> swstartacquisition,
 * calibrate_DC_offset -> -> ,
 *  Зачем Freeevent при рестарте?, Что такое Interrupt? Только для VME?, CAEN_readData, CAEN_GetNumEvents, CAEN_ReadRegister(...,status,...)
 *
*/
