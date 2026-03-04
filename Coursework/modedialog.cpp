#include "ModeDialog.h"

ModeDialog::ModeDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Выбор режима");
    setModal(true);
    setFixedSize(300, 150);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *label = new QLabel("Выберите начальный режим работы:", this);
    modeCombo = new QComboBox(this);
    modeCombo->addItem("Дизельное топливо");
    modeCombo->addItem("Газомоторное топливо (СПГ)");

    QPushButton *okButton = new QPushButton("Продолжить", this);
    QPushButton *cancelButton = new QPushButton("Отмена", this);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    layout->addWidget(label);
    layout->addWidget(modeCombo);
    layout->addLayout(buttonLayout);

    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

QString ModeDialog::getSelectedMode() const {
    return modeCombo->currentText();
}
