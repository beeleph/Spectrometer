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
    QObject::connect(&w, SIGNAL(WriteToFileButton(double)), &digitizer, SLOT(WriteOutputFiles(double))); // HERE
    digitizer.Init();
    //digitizer.test();
    LamelsConfiguration lamConfig;
    QObject::connect(&lamConfig, SIGNAL(EnergiesChanged(QVector<double>)), &w, SLOT(UpdateEnergiesLabels(QVector<double>)));
    QObject::connect(&lamConfig, SIGNAL(EnergiesChanged(QVector<double>)), &digitizer, SLOT(UpdateEnergies(QVector<double>)));
    QObject::connect(&w, SIGNAL(BrukerCurrentChanged(double)), &lamConfig, SLOT(ChangeBrukerCurrent(double)));
    QObject::connect(&w, SIGNAL(GroupingChanged(int)), &lamConfig, SLOT(ChangeLamelGrouping(int)));
    if (lamConfig.ReadConfig())
        w.Say("Failed to read lamelsConfig");
    //QVector<double> z(32);
    //z.fill(75.0);
    //w.UpdateHistogram(z);

    return a.exec();
}


/*  Todo:
 *
 *  Readout error
 *
 *  Протестировать работу с внутренним/внешним триггером
 *  !чтобы гулять до 10В нужно использовать offset
 *  Запретить нажимать какие-либо копки кроме выхода пока не подключиться к оцифровщику, сделаю это на последнем этапе, сейчас будет мешать отладке
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
 *  To commit:
 *  to PrepareHistogramUpdate() calibration added
 *  PerformCalibrate rewrite
 *  CalibrateButton active state changed
 *
*/
