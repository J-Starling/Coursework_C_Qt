#ifndef NAMEDIALOG_H
#define NAMEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>
#include <QHBoxLayout>

class NameDialog : public QDialog {
    Q_OBJECT
public:
    explicit NameDialog(QWidget *parent = nullptr);
    QString getName() const { return nameEdit->text(); }

private:
    QLabel *label;
    QLineEdit *nameEdit;
    QPushButton *continueButton;
    QPushButton *exitButton;

private slots:
    void enableContinueButton(const QString &);
};

#endif
