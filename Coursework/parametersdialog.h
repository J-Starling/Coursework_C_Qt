#ifndef PARAMETERSDIALOG_H
#define PARAMETERSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRegularExpressionValidator>

struct CalculationData {
    QString userName;
    double annualMileage;
    double averageSpeed;
    double enginePower;
    double dieselPricePerTon;
    double lngPricePerTon;
    double modernizationCost;
    QString fuelType;

    double annualEnergy;
    double fuelConsumption;
    double annualFuelCost;
    double savings;
    double paybackPeriod;
    double emissionReductionNOx;
    double emissionReductionCO2;
    double emissionReductionPM;
};

class ParametersDialog : public QDialog {
    Q_OBJECT
public:
    explicit ParametersDialog(QWidget *parent = nullptr);
    CalculationData getData() const;

private:
    QLineEdit *annualMileageEdit;
    QLineEdit *averageSpeedEdit;
    QLineEdit *enginePowerEdit;
    QLineEdit *dieselPriceEdit;
    QLineEdit *lngPriceEdit;
    QLineEdit *modernizationCostEdit;
};

#endif
