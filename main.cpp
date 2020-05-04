#include "mainwindow.h"

#include <QApplication>
#include <N6740.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    qDebug() << "qDebug (with lower case q) testing. everything is ok, i suppose";
    N6740 test;
    //test.oldMain(0,0);

    return a.exec();
}


/*  Todo:
 *
 *  Убираем все параметры которые не относятся напрямую к N6740.
 * В случае необходимости добавить поддержку других оцифровщиков - можно будет подтянуть соответсвующие параметры из оригинального WD.
 *
 *  Убираем текующую обработку ошибок. Переделаем на try catch.
 *
 * EventInfo и прочее можно добавить в класс N6740. Переменная так и так создавалась заранее и крутилась до бесконечности в цикле работы.
 *
 *  Полностью заменить CheckKeyboardCommand и соответсвующий вызов в Oldmain.
 *  Нужные функции из CKC - quit, restart, sendSWtrigger, singlewrite/continuouswrite, set_relative_threshold -> swstartacquisition,
 * calibrate_DC_offset -> -> ,
 *  Зачем Freeevent при рестарте?, Что такое Interrupt? Только для VME?, CAEN_readData, CAEN_GetNumEvents, CAEN_ReadRegister(...,status,...)
 *
 *  Улучшайзинги:
 *  По-возможности попробовать удалить директивы компилятора.
 *  Убрать чтение предварительного конфиг файла.
 *  Переделать парсинг конфиг файла на нормальный. (благо это не трудно)
 *  Реализовать поддержку двух языков (англ/рус) благо в Qt это делается изи.
 *
 *  Кнопочки (Базовые):
 *  Старт/стоп  (S in CKC)
 *  Калибровка (D in CKC)
 *  Выход (q in CKC)
*/
