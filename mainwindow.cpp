#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ClearGui();
    ui->absoluteLabel->setVisible(false);
    ui->relativeViewButton->setChecked(true);
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
    ui->calibrateButton->setEnabled(checked);
    if (checked){
        emit StartButton();
        ui->startstopButton->setText("Остановка");
    }
    else{
        emit StopButton();
        ui->startstopButton->setText("Запуск");
    }
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
}

void MainWindow::on_brukerCurrent_valueChanged(double arg1)
{
    emit BrukerCurrentChanged(arg1);
}

void MainWindow::on_radioButtonOne_clicked()
{
    emit GroupingChanged(1);
}

void MainWindow::on_radioButtonTwo_clicked()
{
    emit GroupingChanged(2);
}

void MainWindow::on_radioButtonFour_clicked()
{
    emit GroupingChanged(4);
}

void MainWindow::on_radioButtonEight_clicked()
{
    emit GroupingChanged(8);
}

void MainWindow::UpdateHistogram(QVector<double> percentagies){
    ui->ch0EnergyBar->setValue(percentagies[0]);
    ui->ch1EnergyBar->setValue(percentagies[1]);
    ui->ch2EnergyBar->setValue(percentagies[2]);
    ui->ch3EnergyBar->setValue(percentagies[3]);
    ui->ch4EnergyBar->setValue(percentagies[4]);
    ui->ch5EnergyBar->setValue(percentagies[5]);
    ui->ch6EnergyBar->setValue(percentagies[6]);
    ui->ch7EnergyBar->setValue(percentagies[7]);
    ui->ch8EnergyBar->setValue(percentagies[8]);
    ui->ch9EnergyBar->setValue(percentagies[9]);
    ui->ch10EnergyBar->setValue(percentagies[10]);
    ui->ch11EnergyBar->setValue(percentagies[11]);
    ui->ch12EnergyBar->setValue(percentagies[12]);
    ui->ch13EnergyBar->setValue(percentagies[13]);
    ui->ch14EnergyBar->setValue(percentagies[14]);
    ui->ch15EnergyBar->setValue(percentagies[15]);
    ui->ch16EnergyBar->setValue(percentagies[16]);
    ui->ch17EnergyBar->setValue(percentagies[17]);
    ui->ch18EnergyBar->setValue(percentagies[18]);
    ui->ch19EnergyBar->setValue(percentagies[19]);
    ui->ch20EnergyBar->setValue(percentagies[20]);
    ui->ch21EnergyBar->setValue(percentagies[21]);
    ui->ch22EnergyBar->setValue(percentagies[22]);
    ui->ch23EnergyBar->setValue(percentagies[23]);
    ui->ch24EnergyBar->setValue(percentagies[24]);
    ui->ch25EnergyBar->setValue(percentagies[25]);
    ui->ch26EnergyBar->setValue(percentagies[26]);
    ui->ch27EnergyBar->setValue(percentagies[27]);
    ui->ch28EnergyBar->setValue(percentagies[28]);
    ui->ch29EnergyBar->setValue(percentagies[29]);
    ui->ch30EnergyBar->setValue(percentagies[30]);
    ui->ch31EnergyBar->setValue(percentagies[31]);
}

void MainWindow::on_writeToFileButton_clicked()
{
    emit WriteToFileButton(ui->brukerCurrent->value());
}

void MainWindow::ClearGui(){
    ui->brukerCurrent->setValue(100);
    ui->radioButtonOne->setChecked(true);
    ui->radioButtonTwo->setChecked(false);
    ui->radioButtonFour->setChecked(false);
    ui->radioButtonEight->setChecked(false);
    ui->ch0EnergyBar->setValue(0);
    ui->ch1EnergyBar->setValue(0);
    ui->ch2EnergyBar->setValue(0);
    ui->ch3EnergyBar->setValue(0);
    ui->ch4EnergyBar->setValue(0);
    ui->ch5EnergyBar->setValue(0);
    ui->ch6EnergyBar->setValue(0);
    ui->ch7EnergyBar->setValue(0);
    ui->ch8EnergyBar->setValue(0);
    ui->ch9EnergyBar->setValue(0);
    ui->ch10EnergyBar->setValue(0);
    ui->ch11EnergyBar->setValue(0);
    ui->ch12EnergyBar->setValue(0);
    ui->ch13EnergyBar->setValue(0);
    ui->ch14EnergyBar->setValue(0);
    ui->ch15EnergyBar->setValue(0);
    ui->ch16EnergyBar->setValue(0);
    ui->ch17EnergyBar->setValue(0);
    ui->ch18EnergyBar->setValue(0);
    ui->ch19EnergyBar->setValue(0);
    ui->ch20EnergyBar->setValue(0);
    ui->ch21EnergyBar->setValue(0);
    ui->ch22EnergyBar->setValue(0);
    ui->ch23EnergyBar->setValue(0);
    ui->ch24EnergyBar->setValue(0);
    ui->ch25EnergyBar->setValue(0);
    ui->ch26EnergyBar->setValue(0);
    ui->ch27EnergyBar->setValue(0);
    ui->ch28EnergyBar->setValue(0);
    ui->ch29EnergyBar->setValue(0);
    ui->ch30EnergyBar->setValue(0);
    ui->ch31EnergyBar->setValue(0);
    ui->ch0EnergyLabel->setText("0");
    ui->ch1EnergyLabel->setText("0");
    ui->ch2EnergyLabel->setText("0");
    ui->ch3EnergyLabel->setText("0");
    ui->ch4EnergyLabel->setText("0");
    ui->ch5EnergyLabel->setText("0");
    ui->ch6EnergyLabel->setText("0");
    ui->ch7EnergyLabel->setText("0");
    ui->ch8EnergyLabel->setText("0");
    ui->ch9EnergyLabel->setText("0");
    ui->ch10EnergyLabel->setText("0");
    ui->ch11EnergyLabel->setText("0");
    ui->ch12EnergyLabel->setText("0");
    ui->ch13EnergyLabel->setText("0");
    ui->ch14EnergyLabel->setText("0");
    ui->ch15EnergyLabel->setText("0");
    ui->ch16EnergyLabel->setText("0");
    ui->ch17EnergyLabel->setText("0");
    ui->ch18EnergyLabel->setText("0");
    ui->ch19EnergyLabel->setText("0");
    ui->ch20EnergyLabel->setText("0");
    ui->ch21EnergyLabel->setText("0");
    ui->ch22EnergyLabel->setText("0");
    ui->ch23EnergyLabel->setText("0");
    ui->ch24EnergyLabel->setText("0");
    ui->ch25EnergyLabel->setText("0");
    ui->ch26EnergyLabel->setText("0");
    ui->ch27EnergyLabel->setText("0");
    ui->ch28EnergyLabel->setText("0");
    ui->ch29EnergyLabel->setText("0");
    ui->ch30EnergyLabel->setText("0");
    ui->ch31EnergyLabel->setText("0");
}

void MainWindow::on_relativeViewButton_clicked()
{
    emit ViewButton(1);
    ui->relativeLabel->setVisible(1);
    ui->absoluteLabel->setVisible(0);
    ui->absoluteViewButton->setChecked(0);
}

void MainWindow::on_absoluteViewButton_clicked()
{
    emit ViewButton(0);
    ui->relativeLabel->setVisible(0);
    ui->absoluteLabel->setVisible(1);
    ui->relativeViewButton->setChecked(0);
}

void MainWindow::on_pushButton_toggled(bool checked)
{
    emit AverageButton(checked);
}
