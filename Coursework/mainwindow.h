#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ParametersDialog.h"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QTextEdit>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QApplication>
#include <QTextDocument>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onAnimationTimer();
    void onFuelConsumptionTimer();
    void showNameDialog();
    void showModeDialog();
    void showParametersDialog();
    void calculate();
    void saveReport();
    void saveAsPDF();
    void showHelp();
    void refuel();
    void showTechnicalData();
    void toggleAnimation();
    void resetAnimation();
    void switchToDiesel();
    void switchToLNG();

private:
    void setupUI();
    void setupScene();
    void updateFuelTankVisual();
    void performCalculations();
    void createContextMenu(const QPoint &pos);
    void drawRailwayTrack();
    void drawFuelTank();
    void createClouds();
    void createTrees();
    void createPowerPoles();
    void createStationBuildings();
    void drawPowerWire(int startX, int endX, int startY, int endY);

    QWidget *centralWidget;
    QGraphicsView *graphicsView;
    QTextEdit *resultsText;
    QPushButton *saveButton;
    QPushButton *savePdfButton;
    QPushButton *paramsButton;

    QGraphicsScene *scene;
    QGraphicsPixmapItem *locomotiveItem;
    QGraphicsRectItem *fuelLevelItem;
    QGraphicsTextItem *modeTextItem;
    QGraphicsTextItem *fuelLevelTextItem;

    QTimer *animationTimer;
    QTimer *fuelConsumptionTimer;

    CalculationData currentData;
    bool isAnimationRunning;
    double currentFuelLevel;
    double locomotivePosition;

    const double DIESEL_ENERGY = 42.7;
    const double LNG_ENERGY = 49.0;
    const double DIESEL_DENSITY = 850;
    const double LNG_DENSITY = 420;

    QPixmap dieselLocomotiveImage;
    QPixmap lngLocomotiveImage;
    QPixmap currentLocomotiveImage;
};

#endif
