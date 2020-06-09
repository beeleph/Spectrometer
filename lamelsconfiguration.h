#ifndef LAMELSCONFIGURATION_H
#define LAMELSCONFIGURATION_H

#include <QObject>
#include <QSettings>
#include <QDebug>

#define MAXCH 32    // max number of channels

class LamelsConfiguration : public QObject
{
    Q_OBJECT
public:
    explicit LamelsConfiguration(QObject *parent = nullptr);

    int ReadConfig();

    double radius[MAXCH];


signals:
    void EnergiesChanged(QVector<double> energies);

public slots:
    int ChangeLamelGrouping(int groupCount);
    void ChangeBrukerCurrent(double current);
    void CalculateEnergies();

private:
    int groupBy = 1;                    // possible inputs: 1, 2, 4, 8;
    double brukerCurrent = 100.0;
    QVector<double> energies;
};

#endif // LAMELSCONFIGURATION_H
