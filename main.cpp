#include "mainwindow.h"

#include <QApplication>
#include <N6740.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    qDebug() << "qDebug (with lower case q) testing. everything is ok, i suppose";
    N6740 digitizer;
    QObject::connect(&digitizer, SIGNAL(N6740Say(QString)), &w, SLOT(Say(QString)));
    QObject::connect(&w, SIGNAL(ExitButton()), &digitizer, SLOT(Exit()));
    QObject::connect(&w, SIGNAL(StartButton()), &digitizer, SLOT(Run()));
    QObject::connect(&w, SIGNAL(StopButton()), &digitizer, SLOT(Stop()));
    QObject::connect(&w, SIGNAL(CalibrateButton()), &digitizer, SLOT(PerformCalibrate()));
    digitizer.Init();
    LamelsConfiguration lamConfig;
    QObject::connect(&lamConfig, SIGNAL(EnergiesChanged(QVector<double>)), &w, SLOT(UpdateEnergiesLabels(QVector<double>)));
    QObject::connect(&w, SIGNAL(BrukerCurrentChanged(double)), &lamConfig, SLOT(ChangeBrukerCurrent(double)));
    if (lamConfig.ReadConfig())
        w.Say("Failed to read lamelsConfig");
    for (int i = 0; i < 32; i ++){
        w.Say("r" + QString::number(i) + " = " + QString::number(lamConfig.radius[i]));
    }
    //digitizer.test();
    //test.oldMain(0,0);

    return a.exec();
}


/*  Todo:
 *
 *  Поэлементно связываем интерфейс и N6740
 *  Протестировать работу алгоритма с тестовыми данными
 *  Проверить чтобы все структуры данных сбрасывали свои значения там где это нужно
 *  не забудь вставить правильную формулу расчета энергии
 *  проверить заполнение Lamelsradius единицами.
 * проверить правильное высчитывание среднего радиуса.
 *
 *  Реализовать установку радиусов ламелей.
 *  Запретить нажимать какие-либо копки кроме выхода пока не подключиться к оцифровщику
 *
 *  Убираем текующую обработку ошибок. Переделаем на try catch.
 *  В первую очередь в Loop()
 *
 *  Улучшайзинги:
 *  По-возможности попробовать удалить директивы компилятора.
 *  Убрать чтение предварительного конфиг файла.
 *  Переделать парсинг конфиг файла на нормальный. (благо это не трудно)
 *  Реализовать поддержку двух языков (англ/рус) благо в Qt это делается изи.
 *  Отрефакторить родные определения/инициализации переменных. Они вызывают вопросы. Особенно в Loop()
 *  Выделить в отдельный класс конфиг, таймер, еще чо-нить
 *
 *  Кнопочки (Базовые):
 *  Старт/стоп  (S in CKC)
 *  Калибровка (D in CKC)
 *  Выход (q in CKC)
 *  Ввод тока с брукера
 *  Вывод полученных интегралов (ЗЦП) с каждого канала (с подписью энергии?)
 *  Вывод средней энергии, максимальной энергии, минимальной энергии
 *  Вывод графика распределения энергии пучка
 *  Кнопочка вызова переопределения радиусов ламелей (групп ламелей)
*/
