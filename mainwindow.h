#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>

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

private slots:
    void Say(QString text);

    void on_exitButton_clicked();

    void on_startstopButton_toggled(bool checked);

    void on_calibrateButton_clicked();

private:
    QTime CurTime;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
