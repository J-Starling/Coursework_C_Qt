#ifndef MODEDIALOG_H
#define MODEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ModeDialog : public QDialog {
    Q_OBJECT
public:
    explicit ModeDialog(QWidget *parent = nullptr);
    QString getSelectedMode() const;

private:
    QComboBox *modeCombo;
};

#endif
