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
 *  Убираем все параметры которые не относятся напрямую к N6740.
 * В случае необходимости добавить поддержку других оцифровщиков - можно будет подтянуть соответсвующие параметры из оригинального WD.
 *
 *
 *
 *  Полностью заменить CheckKeyboardCommand и соответсвующий вызов в Oldmain.
 *  Нужные функции из CKC - quit, restart, sendSWtrigger, singlewrite/continuouswrite, set_relative_threshold -> swstartacquisition,
 * calibrate_DC_offset -> -> ,
 *  Зачем Freeevent при рестарте?, Что такое Interrupt? Только для VME?, CAEN_readData, CAEN_GetNumEvents, CAEN_ReadRegister(...,status,...)
 *
*/
