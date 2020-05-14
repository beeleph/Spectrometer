#include "mainwindow.h"

#include <QApplication>
#include <N6740.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    N6740 digitizer;
    QObject::connect(&digitizer, SIGNAL(N6740Say(QString)), &w, SLOT(Say(QString)));
    QObject::connect(&w, SIGNAL(ExitButton()), &digitizer, SLOT(Exit()));
    QObject::connect(&w, SIGNAL(StartButton()), &digitizer, SLOT(Run()));
    QObject::connect(&w, SIGNAL(StopButton()), &digitizer, SLOT(Stop()));
    QObject::connect(&w, SIGNAL(CalibrateButton()), &digitizer, SLOT(PerformCalibrate()));
    QObject::connect(&digitizer, SIGNAL(DataChanged(QVector<double>)), &w, SLOT(UpdateHistogram(QVector<double>)));
    digitizer.Init();
    //digitizer.test();
    LamelsConfiguration lamConfig;
    QObject::connect(&lamConfig, SIGNAL(EnergiesChanged(QVector<double>)), &w, SLOT(UpdateEnergiesLabels(QVector<double>)));
    QObject::connect(&w, SIGNAL(BrukerCurrentChanged(double)), &lamConfig, SLOT(ChangeBrukerCurrent(double)));
    QObject::connect(&w, SIGNAL(GroupingChanged(int)), &lamConfig, SLOT(ChangeLamelGrouping(int)));
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
 *  Протестировать работу алгоритма с тестовыми данными
 *  Проверить чтобы все структуры данных сбрасывали свои значения там где это нужно
 *  Придумать условия записи в файл. Всегда писать не надо.
 *  Подумать над инициализацией всех переменных на интерфейсе
 *  Тестирование в кьют? альт8?
 *
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
 *  Вывод графика распределения энергии пучка
*/
