#include "lamelsconfiguration.h"

LamelsConfiguration::LamelsConfiguration(QObject *parent) : QObject(parent)
{
    for (int i = 0; i < MAXCH; i++){
        radius[i] = 1.0;    // in case readconfig fails. evade divide by zero error
    }

}

int LamelsConfiguration::ReadConfig(){
    QSettings *settings;
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, ".");
    settings = new QSettings("LamelsRadius.ini", QSettings::IniFormat);
    for (int i = 0; i < MAXCH; ++i){
        this->radius[i] = settings->value("r" + QString::number(i), 0).toDouble();
        if (!this->radius[i])
            return -1;
    }
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
    return;
}

void LamelsConfiguration::CalculateEnergies(){
    for (int i = 0; i < MAXCH / groupBy; i++){
        energies[i] = brukerCurrent / (std::accumulate(radius[i], radius[i] + groupBy, 0.0) / groupBy);
    }
    for (int i = MAXCH / groupBy; i < MAXCH; i++){
        energies[i] = 0.0;
    }
    // emit updateEnergiesLabels
}
