#include "ParametersDialog.h"
#include "qpushbutton.h"

ParametersDialog::ParametersDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Параметры расчета");
    setModal(true);
    setFixedSize(400, 350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *operationalGroup = new QGroupBox("Эксплуатационные параметры", this);
    QFormLayout *operationalLayout = new QFormLayout();

    annualMileageEdit = new QLineEdit("100000", this);
    averageSpeedEdit = new QLineEdit("80", this);
    enginePowerEdit = new QLineEdit("3000", this);

    QRegularExpressionValidator* numberValidator = new QRegularExpressionValidator(
        QRegularExpression("^\\d*\\.?\\d*$"), this
        );

    annualMileageEdit->setValidator(numberValidator);
    averageSpeedEdit->setValidator(numberValidator);
    enginePowerEdit->setValidator(numberValidator);

    operationalLayout->addRow("Годовой пробег (км):", annualMileageEdit);
    operationalLayout->addRow("Средняя скорость (км/ч):", averageSpeedEdit);
    operationalLayout->addRow("Мощность (кВт):", enginePowerEdit);

    operationalGroup->setLayout(operationalLayout);

    QGroupBox *economicGroup = new QGroupBox("Экономические параметры", this);
    QFormLayout *economicLayout = new QFormLayout();

    dieselPriceEdit = new QLineEdit("60000", this);
    lngPriceEdit = new QLineEdit("40000", this);
    modernizationCostEdit = new QLineEdit("5000000", this);

    dieselPriceEdit->setValidator(numberValidator);
    lngPriceEdit->setValidator(numberValidator);
    modernizationCostEdit->setValidator(numberValidator);

    economicLayout->addRow("Стоимость 1 т дизеля (руб):", dieselPriceEdit);
    economicLayout->addRow("Стоимость 1 т СПГ (руб):", lngPriceEdit);
    economicLayout->addRow("Стоимость модернизации (руб):", modernizationCostEdit);

    economicGroup->setLayout(economicLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *calculateButton = new QPushButton("Рассчитать", this);
    QPushButton *cancelButton = new QPushButton("Отмена", this);

    buttonLayout->addWidget(calculateButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addWidget(operationalGroup);
    mainLayout->addWidget(economicGroup);
    mainLayout->addLayout(buttonLayout);

    connect(calculateButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

CalculationData ParametersDialog::getData() const {
    CalculationData data;
    data.annualMileage = annualMileageEdit->text().toDouble();
    data.averageSpeed = averageSpeedEdit->text().toDouble();
    data.enginePower = enginePowerEdit->text().toDouble();
    data.dieselPricePerTon = dieselPriceEdit->text().toDouble();
    data.lngPricePerTon = lngPriceEdit->text().toDouble();
    data.modernizationCost = modernizationCostEdit->text().toDouble();
    return data;
}
