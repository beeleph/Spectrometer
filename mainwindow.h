#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <lamelsconfiguration.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void ExitButton();
    void CalibrateButton();
    void StartButton();
    void StopButton();
    void BrukerCurrentChanged(double current);
    void GroupingChanged(int count);
    void WriteToFileButton(double current);

public slots:
    void Say(QString text);
    void UpdateHistogram(QVector<double> percentagies);

private slots:

    void UpdateEnergiesLabels(QVector<double> energies);

    void on_exitButton_clicked();

    void on_startstopButton_toggled(bool checked);

    void on_calibrateButton_clicked();


    void on_brukerCurrent_valueChanged(double arg1);

    void on_radioButtonOne_clicked();

    void on_radioButtonTwo_clicked();

    void on_radioButtonFour_clicked();

    void on_radioButtonEight_clicked();

    void on_writeToFileButton_clicked();

private:
    QTime CurTime;

    void ClearGui();

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
