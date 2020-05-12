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


void MainWindow::UpdateEnergiesLabels(QVector<double> energies){
    ui->ch0EnergyLabel->setText(QString::number(energies[0], 'f', 0));
    ui->ch1EnergyLabel->setText(QString::number(energies[1], 'f', 0));
    ui->ch2EnergyLabel->setText(QString::number(energies[2], 'f', 0));
    ui->ch3EnergyLabel->setText(QString::number(energies[3], 'f', 0));
    ui->ch4EnergyLabel->setText(QString::number(energies[4], 'f', 0));
    ui->ch5EnergyLabel->setText(QString::number(energies[5], 'f', 0));
    ui->ch6EnergyLabel->setText(QString::number(energies[6], 'f', 0));
    ui->ch7EnergyLabel->setText(QString::number(energies[7], 'f', 0));
    ui->ch8EnergyLabel->setText(QString::number(energies[8], 'f', 0));
    ui->ch9EnergyLabel->setText(QString::number(energies[9], 'f', 0));
    ui->ch10EnergyLabel->setText(QString::number(energies[10], 'f', 0));
    ui->ch11EnergyLabel->setText(QString::number(energies[11], 'f', 0));
    ui->ch12EnergyLabel->setText(QString::number(energies[12], 'f', 0));
    ui->ch13EnergyLabel->setText(QString::number(energies[13], 'f', 0));
    ui->ch14EnergyLabel->setText(QString::number(energies[14], 'f', 0));
    ui->ch15EnergyLabel->setText(QString::number(energies[15], 'f', 0));
    ui->ch16EnergyLabel->setText(QString::number(energies[16], 'f', 0));
    ui->ch17EnergyLabel->setText(QString::number(energies[17], 'f', 0));
    ui->ch18EnergyLabel->setText(QString::number(energies[18], 'f', 0));
    ui->ch19EnergyLabel->setText(QString::number(energies[19], 'f', 0));
    ui->ch20EnergyLabel->setText(QString::number(energies[20], 'f', 0));
    ui->ch21EnergyLabel->setText(QString::number(energies[21], 'f', 0));
    ui->ch22EnergyLabel->setText(QString::number(energies[22], 'f', 0));
    ui->ch23EnergyLabel->setText(QString::number(energies[23], 'f', 0));
    ui->ch24EnergyLabel->setText(QString::number(energies[24], 'f', 0));
    ui->ch25EnergyLabel->setText(QString::number(energies[25], 'f', 0));
    ui->ch26EnergyLabel->setText(QString::number(energies[26], 'f', 0));
    ui->ch27EnergyLabel->setText(QString::number(energies[27], 'f', 0));
    ui->ch28EnergyLabel->setText(QString::number(energies[28], 'f', 0));
    ui->ch29EnergyLabel->setText(QString::number(energies[29], 'f', 0));
    ui->ch30EnergyLabel->setText(QString::number(energies[30], 'f', 0));
    ui->ch31EnergyLabel->setText(QString::number(energies[31], 'f', 0));
    //turn off else
}

void MainWindow::on_brukerCurrent_valueChanged(double arg1)
{
    emit BrukerCurrentChanged(arg1);
}
