#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

void MainWindow::Say(QString text){
    CurTime = QTime::currentTime();
    ui->debugFrame->append(CurTime.toString() + ": " + text);
}
MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_exitButton_clicked()
{
    emit ExitButton();
    QCoreApplication::exit();
}

void MainWindow::on_startstopButton_toggled(bool checked)
{
    ui->calibrateButton->setEnabled(!checked);
    if (checked)
        emit StartButton();
    else
        emit StopButton();
}

void MainWindow::on_calibrateButton_clicked()
{
    emit CalibrateButton();
}

void MainWindow::on_brukerCurrent_valueChanged(int arg1)
{
    // update energies
}
