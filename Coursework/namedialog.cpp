#include "NameDialog.h"

NameDialog::NameDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Ввод имени");
    setModal(true);
    setFixedSize(300, 150);

    QVBoxLayout *layout = new QVBoxLayout(this);

    label = new QLabel("Введите ваше имя для отчета:", this);
    nameEdit = new QLineEdit(this);
    QRegularExpressionValidator* v = new QRegularExpressionValidator(
        QRegularExpression("^[А-Яа-яЁё][А-Яа-яЁё\\s\\-]*$"), this
        );
    nameEdit->setValidator(v);
    nameEdit->setPlaceholderText("Иван Петров");

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    continueButton = new QPushButton("Продолжить", this);
    exitButton = new QPushButton("Выход", this);

    continueButton->setEnabled(false);

    buttonLayout->addWidget(continueButton);
    buttonLayout->addWidget(exitButton);

    layout->addWidget(label);
    layout->addWidget(nameEdit);
    layout->addLayout(buttonLayout);

    connect(continueButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(exitButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(nameEdit, SIGNAL(textChanged(QString)), this, SLOT(enableContinueButton(QString)));
}

void NameDialog::enableContinueButton(const QString &text) {
    continueButton->setEnabled(!text.trimmed().isEmpty());
}
