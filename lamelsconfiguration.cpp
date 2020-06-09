#include "lamelsconfiguration.h"

LamelsConfiguration::LamelsConfiguration(QObject *parent) : QObject(parent)
{
    energies.resize(MAXCH);
}

int LamelsConfiguration::ReadConfig(){
    qDebug() << " MAXCH = " << MAXCH;
    QSettings *settings;
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, ".");
    settings = new QSettings("LamelsRadius.ini", QSettings::IniFormat);
    for (int i = 0; i < MAXCH; ++i){
        this->radius[i] = settings->value("r" + QString::number(i), 0).toDouble();
        if (!this->radius[i])
            return -1;
    }
    CalculateEnergies();
    return 0;
}

int LamelsConfiguration::ChangeLamelGrouping(int groupCount){
    switch(groupCount) {
        case 1:
            this->groupBy = 1;
            break;
        case 2:
            this->groupBy = 2;
            break;
        case 4:
            this->groupBy = 4;
            break;
        case 8:
            this->groupBy = 8;
            break;
        default:
            return  -1;
    }
    CalculateEnergies();
    return 0;
}

void LamelsConfiguration::ChangeBrukerCurrent(double current){
    brukerCurrent = current;
    CalculateEnergies();
}

void LamelsConfiguration::CalculateEnergies(){
    double sum;
    for (int i = 0; i < MAXCH / groupBy; i++){
        sum = 0;
        for (int j = i * groupBy; j < i * groupBy + groupBy; j++){      // calculating average radius of group
            sum += radius[j];
        }
        energies[i] = 0.3 * (22.445 * brukerCurrent + 18.766) * (sum / groupBy) / 10;
    }
    for (int i = MAXCH / groupBy; i < MAXCH; i++){
        energies[i] = 0.0;
    }
    emit EnergiesChanged(energies);
}
