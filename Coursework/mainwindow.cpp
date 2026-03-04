#include "MainWindow.h"
#include "ModeDialog.h"
#include "NameDialog.h"
#include <QtPrintSupport/QPrinter>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QScreen>
#include <QGuiApplication>
#include <QScrollBar>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    isAnimationRunning(false),
    currentFuelLevel(1.0),
    locomotivePosition(0.0)
{
    currentData.userName = "Пользователь";
    currentData.fuelType = "Дизельное топливо";
    currentData.annualMileage = 100000;
    currentData.averageSpeed = 80;
    currentData.enginePower = 3000;
    currentData.dieselPricePerTon = 60000;
    currentData.lngPricePerTon = 40000;
    currentData.modernizationCost = 5000000;

    dieselLocomotiveImage.load("diesel_locomotive.png");
    lngLocomotiveImage.load("lng_locomotive.png");

    if (!dieselLocomotiveImage.isNull()) {
        dieselLocomotiveImage = dieselLocomotiveImage.scaled(150, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        dieselLocomotiveImage = QPixmap(150, 80);
        dieselLocomotiveImage.fill(Qt::darkBlue);
    }

    if (!lngLocomotiveImage.isNull()) {
        lngLocomotiveImage = lngLocomotiveImage.scaled(150, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        lngLocomotiveImage = QPixmap(150, 80);
        lngLocomotiveImage.fill(Qt::darkGreen);
    }

    currentLocomotiveImage = dieselLocomotiveImage;

    setupUI();
    setupScene();

    animationTimer = new QTimer(this);
    connect(animationTimer, SIGNAL(timeout()), this, SLOT(onAnimationTimer()));

    fuelConsumptionTimer = new QTimer(this);
    connect(fuelConsumptionTimer, SIGNAL(timeout()), this, SLOT(onFuelConsumptionTimer()));

    showNameDialog();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    setWindowTitle("Анализ эффективности");
    setFixedSize(1400, 800);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    QVBoxLayout *leftLayout = new QVBoxLayout();

    graphicsView = new QGraphicsView(this);
    graphicsView->setFixedSize(1000, 600);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    graphicsView->horizontalScrollBar()->setEnabled(false);
    graphicsView->verticalScrollBar()->setEnabled(false);
    graphicsView->setDragMode(QGraphicsView::NoDrag);
    graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    QGroupBox *infoBox = new QGroupBox(this);
    QHBoxLayout *infoLayout = new QHBoxLayout();

    QLabel *controlsLabel = new QLabel(
        "Управление:\n"
        "A/D - переключение топлива\n"
        "Пробел - старт/пауза\n"
        "R - сброс анимации\n"
        "I - справка\n"
        "Ctrl+S - сохранить отчет\n"
        "Ctrl+Shift+S - сохранить PDF", this);

    infoLayout->addWidget(controlsLabel);
    infoBox->setLayout(infoLayout);

    leftLayout->addWidget(graphicsView);
    leftLayout->addWidget(infoBox);

    QVBoxLayout *rightLayout = new QVBoxLayout();

    resultsText = new QTextEdit(this);
    resultsText->setMinimumSize(350, 500);
    resultsText->setReadOnly(true);

    QGroupBox *buttonBox = new QGroupBox("Действия", this);
    QVBoxLayout *buttonLayout = new QVBoxLayout();

    saveButton = new QPushButton("Сохранить отчет (TXT)", this);
    savePdfButton = new QPushButton("Сохранить отчет (PDF)", this);
    paramsButton = new QPushButton("Параметры расчета", this);

    buttonLayout->addWidget(paramsButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(savePdfButton);
    buttonLayout->addStretch();

    buttonBox->setLayout(buttonLayout);

    rightLayout->addWidget(resultsText);
    rightLayout->addWidget(buttonBox);

    mainLayout->addLayout(leftLayout, 3);
    mainLayout->addLayout(rightLayout, 1);

    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu *fileMenu = menuBar->addMenu("Файл");
    fileMenu->addAction("Параметры расчета", this, SLOT(showParametersDialog()));
    fileMenu->addAction("Сохранить отчет (TXT)", this, SLOT(saveReport()), QKeySequence::Save);
    fileMenu->addAction("Сохранить отчет (PDF)", this, SLOT(saveAsPDF()), QKeySequence("Ctrl+Shift+S"));
    fileMenu->addSeparator();
    fileMenu->addAction("Выход", qApp, SLOT(quit()), QKeySequence::Quit);

    QMenu *calcMenu = menuBar->addMenu("Расчет");
    calcMenu->addAction("Переключить на дизель", this, SLOT(switchToDiesel()), QKeySequence(Qt::Key_A));
    calcMenu->addAction("Переключить на СПГ", this, SLOT(switchToLNG()), QKeySequence(Qt::Key_D));

    QMenu *vizMenu = menuBar->addMenu("Визуализация");
    vizMenu->addAction("Старт/Пауза", this, SLOT(toggleAnimation()), QKeySequence(Qt::Key_Space));
    vizMenu->addAction("Сброс", this, SLOT(resetAnimation()), QKeySequence(Qt::Key_R));
    vizMenu->addAction("Заправить", this, SLOT(refuel()), QKeySequence("Ctrl+F"));

    QMenu *helpMenu = menuBar->addMenu("Справка");
    helpMenu->addAction("Показать справку", this, SLOT(showHelp()), QKeySequence(Qt::Key_I));
    helpMenu->addAction("Технические данные", this, SLOT(showTechnicalData()), QKeySequence("Ctrl+T"));
    helpMenu->addSeparator();

    QStatusBar *statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage("Готов к работе");

    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveReport()));
    connect(savePdfButton, SIGNAL(clicked()), this, SLOT(saveAsPDF()));
    connect(paramsButton, SIGNAL(clicked()), this, SLOT(showParametersDialog()));
}

void MainWindow::setupScene() {
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 1000, 600);
    graphicsView->setScene(scene);

    QLinearGradient skyGradient(0, 0, 0, 350);
    skyGradient.setColorAt(0, QColor(135, 206, 235));
    skyGradient.setColorAt(1, QColor(176, 224, 230));
    QGraphicsRectItem *sky = new QGraphicsRectItem(0, 0, 1000, 350);
    sky->setBrush(QBrush(skyGradient));
    sky->setPen(QPen(Qt::NoPen));
    sky->setZValue(-200);
    scene->addItem(sky);

    QLinearGradient groundGradient(0, 350, 0, 600);
    groundGradient.setColorAt(0, QColor(34, 139, 34));
    groundGradient.setColorAt(1, QColor(107, 142, 35));
    QGraphicsRectItem *ground = new QGraphicsRectItem(0, 350, 1000, 250);
    ground->setBrush(QBrush(groundGradient));
    ground->setPen(QPen(Qt::NoPen));
    ground->setZValue(-150);
    scene->addItem(ground);

    createClouds();
    drawRailwayTrack();
    createTrees();
    createPowerPoles();
    createStationBuildings();

    if (!currentLocomotiveImage.isNull()) {
        locomotiveItem = new QGraphicsPixmapItem(currentLocomotiveImage);
    } else {
        QPixmap stub(150, 80);
        stub.fill(Qt::red);
        locomotiveItem = new QGraphicsPixmapItem(stub);
    }
    scene->addItem(locomotiveItem);
    locomotiveItem->setPos(locomotivePosition, 400);
    locomotiveItem->setZValue(100);

    drawFuelTank();

    QGraphicsRectItem *infoPanel = new QGraphicsRectItem(10, 10, 300, 120);
    infoPanel->setBrush(QBrush(QColor(255, 255, 255, 220)));
    infoPanel->setPen(QPen(Qt::darkGray, 2));
    infoPanel->setZValue(150);
    scene->addItem(infoPanel);

    modeTextItem = scene->addText(currentData.fuelType);
    modeTextItem->setDefaultTextColor(Qt::darkBlue);
    modeTextItem->setFont(QFont("Arial", 14, QFont::Bold));
    modeTextItem->setPos(20, 20);
    modeTextItem->setZValue(151);

    fuelLevelTextItem = new QGraphicsTextItem("Уровень топлива: 100%");
    fuelLevelTextItem->setDefaultTextColor(Qt::darkGreen);
    fuelLevelTextItem->setFont(QFont("Arial", 10));
    fuelLevelTextItem->setPos(20, 60);
    fuelLevelTextItem->setZValue(151);
    scene->addItem(fuelLevelTextItem);

    QGraphicsEllipseItem *sun = new QGraphicsEllipseItem(900, 20, 70, 70);
    sun->setBrush(QBrush(QColor(255, 255, 0, 200)));
    sun->setPen(QPen(QColor(255, 165, 0, 180), 2));
    sun->setZValue(-100);
    scene->addItem(sun);

    for (int i = 0; i < 8; i++) {
        double angle = i * 45 * 3.14159 / 180;
        int x1 = 935 + 40 * cos(angle);
        int y1 = 55 + 40 * sin(angle);
        int x2 = 935 + 60 * cos(angle);
        int y2 = 55 + 60 * sin(angle);
        QGraphicsLineItem *ray = new QGraphicsLineItem(x1, y1, x2, y2);
        ray->setPen(QPen(QColor(255, 165, 0, 150), 3));
        ray->setZValue(-101);
        scene->addItem(ray);
    }
}

void MainWindow::drawRailwayTrack() {
    QGraphicsRectItem *ballast = new QGraphicsRectItem(0, 440, 1000, 40);
    ballast->setBrush(QBrush(QColor(139, 69, 19)));
    ballast->setPen(QPen(Qt::NoPen));
    ballast->setZValue(10);
    scene->addItem(ballast);

    QPen railPen(QColor(50, 50, 50), 12);
    railPen.setCapStyle(Qt::FlatCap);

    QGraphicsLineItem *rail1 = new QGraphicsLineItem(0, 450, 1000, 450);
    rail1->setPen(railPen);
    rail1->setZValue(20);
    scene->addItem(rail1);

    QGraphicsLineItem *rail2 = new QGraphicsLineItem(0, 470, 1000, 470);
    rail2->setPen(railPen);
    rail2->setZValue(20);
    scene->addItem(rail2);

    QPen tiePen(QColor(101, 67, 33), 8);
    for (int x = 0; x < 1000; x += 50) {
        QGraphicsLineItem *tie = new QGraphicsLineItem(x, 445, x, 475);
        tie->setPen(tiePen);
        tie->setZValue(15);
        scene->addItem(tie);
    }

    QPen clipPen(QColor(100, 100, 100), 4);
    for (int x = 25; x < 1000; x += 50) {
        QGraphicsLineItem *clip1 = new QGraphicsLineItem(x, 450, x, 445);
        clip1->setPen(clipPen);
        clip1->setZValue(25);
        scene->addItem(clip1);

        QGraphicsLineItem *clip2 = new QGraphicsLineItem(x, 470, x, 475);
        clip2->setPen(clipPen);
        clip2->setZValue(25);
        scene->addItem(clip2);
    }
}

void MainWindow::drawFuelTank() {
    QRectF tankRect(850, 150, 120, 250);
    QGraphicsRectItem *tankOutline = new QGraphicsRectItem(tankRect);

    QLinearGradient tankGradient(850, 150, 970, 400);
    tankGradient.setColorAt(0, QColor(180, 180, 180));
    tankGradient.setColorAt(1, QColor(120, 120, 120));
    tankOutline->setBrush(QBrush(tankGradient));
    tankOutline->setPen(QPen(Qt::black, 3));
    tankOutline->setZValue(50);
    scene->addItem(tankOutline);

    QGraphicsEllipseItem *tankLid = new QGraphicsEllipseItem(905, 145, 10, 10);
    tankLid->setBrush(QBrush(QColor(80, 80, 80)));
    tankLid->setPen(QPen(Qt::black, 2));
    tankLid->setZValue(51);
    scene->addItem(tankLid);

    QGraphicsRectItem *tankStand = new QGraphicsRectItem(840, 400, 140, 25);
    tankStand->setBrush(QBrush(QColor(70, 70, 70)));
    tankStand->setPen(QPen(Qt::black, 2));
    tankStand->setZValue(49);
    scene->addItem(tankStand);

    for (int i = 0; i < 3; i++) {
        QGraphicsRectItem *leg = new QGraphicsRectItem(850 + i * 50, 425, 15, 25);
        leg->setBrush(QBrush(QColor(50, 50, 50)));
        leg->setPen(QPen(Qt::black, 1));
        leg->setZValue(48);
        scene->addItem(leg);
    }

    QGraphicsRectItem *gaugeWindow = new QGraphicsRectItem(860, 160, 100, 230);
    gaugeWindow->setBrush(QBrush(Qt::white));
    gaugeWindow->setPen(QPen(Qt::black, 2));
    gaugeWindow->setZValue(52);
    scene->addItem(gaugeWindow);

    fuelLevelItem = new QGraphicsRectItem(0, 0, 100, 230 * currentFuelLevel);
    fuelLevelItem->setBrush(QBrush(QColor(0, 200, 0, 200)));
    fuelLevelItem->setPen(QPen(QColor(0, 150, 0), 1));
    fuelLevelItem->setPos(860, 160 + 230 * (1 - currentFuelLevel));
    fuelLevelItem->setZValue(53);
    scene->addItem(fuelLevelItem);

    QPen gaugePen(Qt::black, 1);
    for (int i = 0; i <= 4; i++) {
        int y = 160 + i * 57;
        QGraphicsLineItem *mark = new QGraphicsLineItem(850, y, 860, y);
        mark->setPen(gaugePen);
        mark->setZValue(54);
        scene->addItem(mark);

        QGraphicsLineItem *mark2 = new QGraphicsLineItem(960, y, 970, y);
        mark2->setPen(gaugePen);
        mark2->setZValue(54);
        scene->addItem(mark2);

        QGraphicsTextItem *label = new QGraphicsTextItem(QString::number(100 - i * 25) + "%");
        label->setDefaultTextColor(Qt::black);
        label->setFont(QFont("Arial", 8));
        label->setPos(970, y - 10);
        label->setZValue(54);
        scene->addItem(label);
    }
}

void MainWindow::createClouds() {
    QGraphicsEllipseItem *cloud1_part1 = new QGraphicsEllipseItem(50, 30, 70, 45);
    cloud1_part1->setBrush(QBrush(QColor(255, 255, 255, 180)));
    cloud1_part1->setPen(QPen(Qt::NoPen));
    cloud1_part1->setZValue(-120);
    scene->addItem(cloud1_part1);

    QGraphicsEllipseItem *cloud1_part2 = new QGraphicsEllipseItem(85, 20, 80, 55);
    cloud1_part2->setBrush(QBrush(QColor(255, 255, 255, 180)));
    cloud1_part2->setPen(QPen(Qt::NoPen));
    cloud1_part2->setZValue(-120);
    scene->addItem(cloud1_part2);

    QGraphicsEllipseItem *cloud1_part3 = new QGraphicsEllipseItem(120, 30, 70, 45);
    cloud1_part3->setBrush(QBrush(QColor(255, 255, 255, 180)));
    cloud1_part3->setPen(QPen(Qt::NoPen));
    cloud1_part3->setZValue(-120);
    scene->addItem(cloud1_part3);

    QGraphicsEllipseItem *cloud2_part1 = new QGraphicsEllipseItem(300, 50, 60, 35);
    cloud2_part1->setBrush(QBrush(QColor(255, 255, 255, 160)));
    cloud2_part1->setPen(QPen(Qt::NoPen));
    cloud2_part1->setZValue(-120);
    scene->addItem(cloud2_part1);

    QGraphicsEllipseItem *cloud2_part2 = new QGraphicsEllipseItem(330, 40, 80, 45);
    cloud2_part2->setBrush(QBrush(QColor(255, 255, 255, 160)));
    cloud2_part2->setPen(QPen(Qt::NoPen));
    cloud2_part2->setZValue(-120);
    scene->addItem(cloud2_part2);

    QGraphicsEllipseItem *cloud3_part1 = new QGraphicsEllipseItem(600, 60, 50, 30);
    cloud3_part1->setBrush(QBrush(QColor(255, 255, 255, 140)));
    cloud3_part1->setPen(QPen(Qt::NoPen));
    cloud3_part1->setZValue(-120);
    scene->addItem(cloud3_part1);

    QGraphicsEllipseItem *cloud3_part2 = new QGraphicsEllipseItem(630, 50, 60, 35);
    cloud3_part2->setBrush(QBrush(QColor(255, 255, 255, 140)));
    cloud3_part2->setPen(QPen(Qt::NoPen));
    cloud3_part2->setZValue(-120);
    scene->addItem(cloud3_part2);
}

void MainWindow::createTrees() {
    QGraphicsRectItem *tree1_trunk = new QGraphicsRectItem(80, 380, 18, 60);
    tree1_trunk->setBrush(QBrush(QColor(101, 67, 33)));
    tree1_trunk->setPen(QPen(Qt::NoPen));
    tree1_trunk->setZValue(30);
    scene->addItem(tree1_trunk);

    QGraphicsEllipseItem *tree1_crown = new QGraphicsEllipseItem(45, 340, 88, 70);
    tree1_crown->setBrush(QBrush(QColor(34, 139, 34, 200)));
    tree1_crown->setPen(QPen(Qt::NoPen));
    tree1_crown->setZValue(31);
    scene->addItem(tree1_crown);

    QGraphicsRectItem *tree2_trunk = new QGraphicsRectItem(400, 380, 15, 60);
    tree2_trunk->setBrush(QBrush(QColor(139, 69, 19)));
    tree2_trunk->setPen(QPen(Qt::NoPen));
    tree2_trunk->setZValue(30);
    scene->addItem(tree2_trunk);

    QGraphicsEllipseItem *tree2_crown = new QGraphicsEllipseItem(365, 340, 88, 70);
    tree2_crown->setBrush(QBrush(QColor(0, 100, 0, 220)));
    tree2_crown->setPen(QPen(Qt::NoPen));
    tree2_crown->setZValue(31);
    scene->addItem(tree2_crown);

    QGraphicsRectItem *tree3_trunk = new QGraphicsRectItem(750, 380, 12, 60);
    tree3_trunk->setBrush(QBrush(QColor(101, 67, 33)));
    tree3_trunk->setPen(QPen(Qt::NoPen));
    tree3_trunk->setZValue(30);
    scene->addItem(tree3_trunk);

    QGraphicsEllipseItem *tree3_crown = new QGraphicsEllipseItem(715, 350, 82, 60);
    tree3_crown->setBrush(QBrush(QColor(60, 179, 113, 180)));
    tree3_crown->setPen(QPen(Qt::NoPen));
    tree3_crown->setZValue(31);
    scene->addItem(tree3_crown);

    QGraphicsRectItem *tree4_trunk = new QGraphicsRectItem(200, 520, 14, 50);
    tree4_trunk->setBrush(QBrush(QColor(101, 67, 33)));
    tree4_trunk->setPen(QPen(Qt::NoPen));
    tree4_trunk->setZValue(30);
    scene->addItem(tree4_trunk);

    QGraphicsEllipseItem *tree4_crown = new QGraphicsEllipseItem(170, 490, 74, 50);
    tree4_crown->setBrush(QBrush(QColor(50, 205, 50, 190)));
    tree4_crown->setPen(QPen(Qt::NoPen));
    tree4_crown->setZValue(31);
    scene->addItem(tree4_crown);

    QGraphicsRectItem *tree5_trunk = new QGraphicsRectItem(650, 530, 16, 55);
    tree5_trunk->setBrush(QBrush(QColor(139, 69, 19)));
    tree5_trunk->setPen(QPen(Qt::NoPen));
    tree5_trunk->setZValue(30);
    scene->addItem(tree5_trunk);

    QGraphicsEllipseItem *tree5_crown = new QGraphicsEllipseItem(620, 500, 82, 60);
    tree5_crown->setBrush(QBrush(QColor(46, 139, 87, 200)));
    tree5_crown->setPen(QPen(Qt::NoPen));
    tree5_crown->setZValue(31);
    scene->addItem(tree5_crown);
}

void MainWindow::createPowerPoles() {
    for (int x = 200; x < 900; x += 250) {
        QGraphicsRectItem *pole = new QGraphicsRectItem(x, 270, 12, 170);
        pole->setBrush(QBrush(QColor(70, 70, 70)));
        pole->setPen(QPen(Qt::black, 1));
        pole->setZValue(-8);
        scene->addItem(pole);

        QGraphicsRectItem *crossbar = new QGraphicsRectItem(x - 50, 270, 112, 10);
        crossbar->setBrush(QBrush(QColor(50, 50, 50)));
        crossbar->setPen(QPen(Qt::black, 1));
        crossbar->setZValue(-7);
        scene->addItem(crossbar);

        for (int i = 0; i < 3; i++) {
            int insulatorX = x - 35 + i * 35;
            QGraphicsEllipseItem *insulator = new QGraphicsEllipseItem(insulatorX, 265, 10, 15);
            insulator->setBrush(QBrush(QColor(220, 220, 220)));
            insulator->setPen(QPen(Qt::black, 1));
            insulator->setZValue(-6);
            scene->addItem(insulator);
        }
    }

    for (int i = 0; i < 5; i++) {
        for (int wireNum = 0; wireNum < 3; wireNum++) {
            int startX = 250 * i + wireNum * 35 - 80;
            int endX = 250 * (i + 1) + wireNum * 35 - 80;

            drawPowerWire(startX, endX, 275, 275);
        }
    }
}

void MainWindow::drawPowerWire(int startX, int endX, int startY, int endY) {
    QPainterPath path;
    path.moveTo(startX, startY);

    int controlY = startY + 25;
    int control1X = startX + (endX - startX) / 3;
    int control2X = startX + 2 * (endX - startX) / 3;

    path.cubicTo(control1X, controlY, control2X, controlY, endX, endY);

    QGraphicsPathItem *wire = new QGraphicsPathItem(path);

    QPen wirePen(QColor(130, 130, 130), 2.5);
    wirePen.setCapStyle(Qt::RoundCap);
    wirePen.setJoinStyle(Qt::RoundJoin);
    wire->setPen(wirePen);
    wire->setZValue(50);
    scene->addItem(wire);
}

void MainWindow::createStationBuildings() {
    QGraphicsRectItem *stationMain = new QGraphicsRectItem(50, 240, 140, 120);
    QLinearGradient stationGrad(50, 240, 190, 360);
    stationGrad.setColorAt(0, QColor(210, 180, 140));
    stationGrad.setColorAt(1, QColor(180, 150, 110));
    stationMain->setBrush(QBrush(stationGrad));
    stationMain->setPen(QPen(QColor(139, 115, 85), 3));
    stationMain->setZValue(-30);
    scene->addItem(stationMain);

    QPolygonF roof;
    roof << QPointF(50, 240) << QPointF(120, 200) << QPointF(190, 240);
    QGraphicsPolygonItem *roofItem = new QGraphicsPolygonItem(roof);
    roofItem->setBrush(QBrush(QColor(139, 0, 0, 200)));
    roofItem->setPen(QPen(QColor(100, 0, 0), 2));
    roofItem->setZValue(-29);
    scene->addItem(roofItem);

    for (int i = 0; i < 3; i++) {
        QGraphicsRectItem *window = new QGraphicsRectItem(70 + i * 35, 270, 25, 35);
        window->setBrush(QBrush(QColor(173, 216, 230, 180)));
        window->setPen(QPen(QColor(70, 130, 180), 2));
        window->setZValue(-28);
        scene->addItem(window);

        QGraphicsLineItem *windowCross1 = new QGraphicsLineItem(70 + i * 35 + 12, 270, 70 + i * 35 + 12, 305);
        windowCross1->setPen(QPen(QColor(70, 130, 180), 1));
        windowCross1->setZValue(-27);
        scene->addItem(windowCross1);

        QGraphicsLineItem *windowCross2 = new QGraphicsLineItem(70 + i * 35, 287, 95 + i * 35, 287);
        windowCross2->setPen(QPen(QColor(70, 130, 180), 1));
        windowCross2->setZValue(-27);
        scene->addItem(windowCross2);
    }

    QGraphicsRectItem *door = new QGraphicsRectItem(110, 310, 30, 50);
    door->setBrush(QBrush(QColor(101, 67, 33)));
    door->setPen(QPen(QColor(70, 45, 20), 2));
    door->setZValue(-28);
    scene->addItem(door);

    QGraphicsEllipseItem *doorKnob = new QGraphicsEllipseItem(135, 330, 5, 5);
    doorKnob->setBrush(QBrush(QColor(200, 200, 0)));
    doorKnob->setPen(QPen(Qt::black, 1));
    doorKnob->setZValue(-27);
    scene->addItem(doorKnob);

    QGraphicsRectItem *sign = new QGraphicsRectItem(90, 220, 60, 20);
    sign->setBrush(QBrush(QColor(255, 255, 255, 220)));
    sign->setPen(QPen(Qt::darkRed, 2));
    sign->setZValue(-25);
    scene->addItem(sign);

    QGraphicsTextItem *signText = new QGraphicsTextItem("СТАНЦИЯ");
    signText->setDefaultTextColor(Qt::darkRed);
    signText->setFont(QFont("Comic Sans MS", 8, QFont::Bold));
    signText->setPos(88, 220);
    signText->setZValue(-24);
    scene->addItem(signText);
}

void MainWindow::updateFuelTankVisual() {
    if (currentData.fuelType == "Дизельное топливо") {
        currentLocomotiveImage = dieselLocomotiveImage;
        fuelLevelItem->setBrush(QBrush(QColor(0, 200, 0, 200)));
    } else {
        currentLocomotiveImage = lngLocomotiveImage;
        fuelLevelItem->setBrush(QBrush(QColor(0, 150, 200, 200)));
    }

    if (!currentLocomotiveImage.isNull()) {
        locomotiveItem->setPixmap(currentLocomotiveImage);
    }

    fuelLevelItem->setRect(0, 0, 100, 230 * currentFuelLevel);
    fuelLevelItem->setPos(860, 160 + 230 * (1 - currentFuelLevel));
    modeTextItem->setPlainText(currentData.fuelType);

    int fuelPercent = static_cast<int>(currentFuelLevel * 100);
    fuelLevelTextItem->setPlainText(QString("Уровень топлива: %1%").arg(fuelPercent));
}

void MainWindow::onAnimationTimer() {
    locomotivePosition += 1.5;
    if (locomotivePosition > 1000) {
        locomotivePosition = -150;
    }
    locomotiveItem->setPos(locomotivePosition, 400);

    statusBar()->showMessage(QString("Локомотив движется. Позиция: %1").arg(locomotivePosition));
}

void MainWindow::onFuelConsumptionTimer() {
    if (currentFuelLevel > 0) {
        currentFuelLevel -= 0.002;
        if (currentFuelLevel < 0) currentFuelLevel = 0;
        updateFuelTankVisual();
        if (currentFuelLevel <= 0.01 && isAnimationRunning) {
            animationTimer->stop();
            fuelConsumptionTimer->stop();
            isAnimationRunning = false;

            QMessageBox::warning(this, "Топливо закончилось",
                                 "Топливный бак пуст! Анимация остановлена.\n\n"
                                 "Заправьте топливо для продолжения.");
            statusBar()->showMessage("Топливо закончилось! Анимация остановлена");
        } else if (currentFuelLevel <= 0.01) {
            statusBar()->showMessage("Топливо на исходе! Нажмите правой кнопкой для заправки.");
        }
    }
}

void MainWindow::showNameDialog() {
    hide();
    NameDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName().trimmed();
        if (!name.isEmpty()) {
            currentData.userName = name;
            statusBar()->showMessage("Пользователь: " + name);
            showModeDialog();
        } else {
            qApp->quit();
        }
    } else {
        qApp->quit();
    }
}

void MainWindow::showModeDialog() {
    ModeDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString mode = dialog.getSelectedMode();
        currentData.fuelType = mode;
        updateFuelTankVisual();
        calculate();
        showParametersDialog();
    } else {
        qApp->quit();
    }
}

void MainWindow::showParametersDialog() {
    ParametersDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        CalculationData newData = dialog.getData();
        newData.userName = currentData.userName;
        newData.fuelType = currentData.fuelType;
        currentData = newData;
        calculate();
        updateFuelTankVisual();
        statusBar()->showMessage("Параметры обновлены");
        show();
    } else {
        qApp->quit();
    }
}

void MainWindow::calculate() {
    performCalculations();

    resultsText->clear();
    resultsText->append("<h2>Результаты расчета</h2>");
    resultsText->append("<b>Пользователь:</b> " + currentData.userName);
    resultsText->append("<b>Тип топлива:</b> " + currentData.fuelType);
    resultsText->append("<b>Годовая потребность в энергии:</b> " +
                        QString::number(currentData.annualEnergy, 'f', 2) + " МДж");
    resultsText->append("<b>Годовой расход топлива:</b> " +
                        QString::number(currentData.fuelConsumption, 'f', 2) + " тонн");
    resultsText->append("<b>Годовые затраты на топливо:</b> " +
                        QString::number(currentData.annualFuelCost, 'f', 2) + " руб");

    if (currentData.fuelType == "Газомоторное топливо (СПГ)") {
        resultsText->append("<b>Экономия по сравнению с дизелем:</b> " +
                            QString::number(currentData.savings, 'f', 2) + " руб/год");
        resultsText->append("<b>Срок окупаемости модернизации:</b> " +
                            QString::number(currentData.paybackPeriod, 'f', 2) + " лет");

        resultsText->append("<h3>Экологический эффект:</h3>");
        resultsText->append("<b>Снижение выбросов NOx:</b> " +
                            QString::number(currentData.emissionReductionNOx) + "%");
        resultsText->append("<b>Снижение выбросов CO₂:</b> " +
                            QString::number(currentData.emissionReductionCO2) + "%");
        resultsText->append("<b>Снижение выбросов твердых частиц:</b> " +
                            QString::number(currentData.emissionReductionPM) + "%");
    }

    double relativeVolume = (DIESEL_ENERGY * DIESEL_DENSITY) / (LNG_ENERGY * LNG_DENSITY);
    resultsText->append("<h3>Технические характеристики:</h3>");
    resultsText->append("<b>Энергоемкость дизельного топлива:</b> " + QString::number(DIESEL_ENERGY) + " МДж/кг");
    resultsText->append("<b>Энергоемкость СПГ:</b> " + QString::number(LNG_ENERGY) + " МДж/кг");
    resultsText->append("<b>Относительный объем топливных систем (СПГ/Дизель):</b> " +
                        QString::number(relativeVolume, 'f', 2));

    statusBar()->showMessage("Расчет завершен");
}

void MainWindow::performCalculations() {
    double operatingHours = currentData.annualMileage / currentData.averageSpeed;
    currentData.annualEnergy = currentData.enginePower * operatingHours * 3.6;

    if (currentData.fuelType == "Дизельное топливо") {
        currentData.fuelConsumption = currentData.annualEnergy / (DIESEL_ENERGY * 1000);
        currentData.annualFuelCost = currentData.fuelConsumption * currentData.dieselPricePerTon;
        currentData.savings = 0;
        currentData.paybackPeriod = 0;
        currentData.emissionReductionNOx = 0;
        currentData.emissionReductionCO2 = 0;
        currentData.emissionReductionPM = 0;
    } else {
        currentData.fuelConsumption = currentData.annualEnergy / (LNG_ENERGY * 1000);
        currentData.annualFuelCost = currentData.fuelConsumption * currentData.lngPricePerTon;

        double dieselConsumption = currentData.annualEnergy / (DIESEL_ENERGY * 1000);
        double dieselCost = dieselConsumption * currentData.dieselPricePerTon;

        currentData.savings = dieselCost - currentData.annualFuelCost;

        if (currentData.savings > 0) {
            currentData.paybackPeriod = currentData.modernizationCost / currentData.savings;
        } else {
            currentData.paybackPeriod = 999;
        }

        currentData.emissionReductionNOx = 85;
        currentData.emissionReductionCO2 = 25;
        currentData.emissionReductionPM = 92.5;
    }
}

void MainWindow::saveReport() {
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить отчет", "отчет_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm") + ".txt", "Текстовые файлы (*.txt);;Все файлы (*)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setRealNumberPrecision(2);
        out.setRealNumberNotation(QTextStream::FixedNotation);

        out << "===========================================================\n";
        out << "     ОТЧЕТ ПО РАСЧЕТУ ЭКОНОМИЧЕСКОЙ ЭФФЕКТИВНОСТИ\n";
        out << "          СРАВНЕНИЕ ДИЗЕЛЬНОГО ТОПЛИВА И СПГ\n";
        out << "===========================================================\n\n";

        out << "Дата создания: " << QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm") << "\n";
        out << "Пользователь: " << currentData.userName << "\n\n";

        out << "---------------------------\n";
        out << "ИСХОДНЫЕ ПАРАМЕТРЫ\n";
        out << "---------------------------\n";
        out << "Тип топлива: " << currentData.fuelType << "\n";
        out << "Годовой пробег: " << QString::number(currentData.annualMileage, 'f', 0) << " км\n";
        out << "Средняя скорость: " << QString::number(currentData.averageSpeed, 'f', 0) << " км/ч\n";
        out << "Мощность силовой установки: " << QString::number(currentData.enginePower, 'f', 0) << " кВт\n";
        out << "Стоимость 1 тонны дизеля: " << QString::number(currentData.dieselPricePerTon, 'f', 0) << " руб\n";
        out << "Стоимость 1 тонны СПГ: " << QString::number(currentData.lngPricePerTon, 'f', 0) << " руб\n";
        out << "Стоимость модернизации: " << QString::number(currentData.modernizationCost, 'f', 0) << " руб\n\n";

        out << "---------------------------\n";
        out << "РЕЗУЛЬТАТЫ РАСЧЕТА\n";
        out << "---------------------------\n";
        out << "Годовая потребность в энергии: " << QString::number(currentData.annualEnergy, 'f', 2) << " МДж\n";
        out << "Годовой расход топлива: " << QString::number(currentData.fuelConsumption, 'f', 2) << " тонн\n";
        out << "Годовые затраты на топливо: " << QString::number(currentData.annualFuelCost, 'f', 2) << " руб\n";

        if (currentData.fuelType == "Газомоторное топливо (СПГ)") {
            out << "Экономический эффект: " << QString::number(currentData.savings, 'f', 2) << " руб/год\n";
            out << "Срок окупаемости: " << QString::number(currentData.paybackPeriod, 'f', 2) << " лет\n\n";

            out << "---------------------------\n";
            out << "ЭКОЛОГИЧЕСКИЙ ЭФФЕКТ\n";
            out << "---------------------------\n";
            out << "Снижение выбросов NOx: " << currentData.emissionReductionNOx << "%\n";
            out << "Снижение выбросов CO₂: " << currentData.emissionReductionCO2 << "%\n";
            out << "Снижение выбросов твердых частиц: " << currentData.emissionReductionPM << "%\n";
        }

        out << "\n---------------------------\n";
        out << "ТЕХНИЧЕСКИЕ ХАРАКТЕРИСТИКИ\n";
        out << "---------------------------\n";
        out << "Энергоемкость дизельного топлива: " << QString::number(DIESEL_ENERGY, 'f', 1) << " МДж/кг\n";
        out << "Энергоемкость СПГ: " << QString::number(LNG_ENERGY, 'f', 1) << " МДж/кг\n";
        out << "Плотность дизельного топлива: " << QString::number(DIESEL_DENSITY, 'f', 0) << " кг/м³\n";
        out << "Плотность СПГ: " << QString::number(LNG_DENSITY, 'f', 0) << " кг/м³\n";

        double relativeVolume = (DIESEL_ENERGY * DIESEL_DENSITY) / (LNG_ENERGY * LNG_DENSITY);
        out << "Относительный объем топливных систем (СПГ/Дизель): " << QString::number(relativeVolume, 'f', 2) << "\n";

        file.close();

        QMessageBox::information(this, "Успех", "Отчет успешно сохранен в файл:\n" + fileName);
        statusBar()->showMessage("Отчет сохранен: " + fileName);
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл");
    }
}

void MainWindow::saveAsPDF() {
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить отчет как PDF", "отчет_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm") + ".pdf", "PDF файлы (*.pdf);;Все файлы (*)");

    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);

    QTextDocument document;
    QString html;

    html = "<html><head><style>"
            "body { font-family: Comic Sans MS, sans-serif; margin: 40px; }"
            "h1 { color: #2c3e50; text-align: center; }"
            "h2 { color: #34495e; border-bottom: 2px solid #3498db; padding-bottom: 5px; }"
            ".section { margin: 20px 0; }"
            ".param { margin: 5px 0; }"
            ".result { background-color: #f8f9fa; padding: 15px; border-radius: 5px; margin: 10px 0; }"
            ".header { text-align: center; margin-bottom: 30px; }"
            ".footer { text-align: center; margin-top: 30px; color: #7f8c8d; font-size: 12px; }"
            ".scene-title { text-align: center; margin: 40px 0 20px 0; font-size: 24px; color: #2c3e50; }"
            "</style></head><body>";

    html += "<div class='header'>"
            "<h1>ОТЧЕТ ПО РАСЧЕТУ ЭКОНОМИЧЕСКОЙ ЭФФЕКТИВНОСТИ</h1>"
            "<h2>Сравнение дизельного топлива и СПГ</h2>"
            "<p>Дата создания: " + QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm") + "</p>" "</div>";

    html += "<div class='section'>"
            "<h2>ИСХОДНЫЕ ПАРАМЕТРЫ</h2>"
            "<div class='result'>"
            "<div class='param'><strong>Пользователь:</strong> " + currentData.userName + "</div>"
            "<div class='param'><strong>Тип топлива:</strong> " + currentData.fuelType + "</div>"
            "<div class='param'><strong>Годовой пробег:</strong> " + QString::number(currentData.annualMileage, 'f', 0) + " км</div>"
            "<div class='param'><strong>Средняя скорость:</strong> " + QString::number(currentData.averageSpeed, 'f', 0) + " км/ч</div>"
            "<div class='param'><strong>Мощность силовой установки:</strong> " + QString::number(currentData.enginePower, 'f', 0) + " кВт</div>"
            "<div class='param'><strong>Стоимость 1 тонны дизеля:</strong> " + QString::number(currentData.dieselPricePerTon, 'f', 0) + " руб</div>"
            "<div class='param'><strong>Стоимость 1 тонны СПГ:</strong> " + QString::number(currentData.lngPricePerTon, 'f', 0) + " руб</div>"
            "<div class='param'><strong>Стоимость модернизации:</strong> " + QString::number(currentData.modernizationCost, 'f', 0) + " руб</div>"
            "</div></div>";

    html += "<div class='section'>"
            "<h2>РЕЗУЛЬТАТЫ РАСЧЕТА</h2>"
            "<div class='result'>"
            "<div class='param'><strong>Годовая потребность в энергии:</strong> " + QString::number(currentData.annualEnergy, 'f', 2) + " МДж</div>"
            "<div class='param'><strong>Годовой расход топлива:</strong> " + QString::number(currentData.fuelConsumption, 'f', 2) + " тонн</div>"
            "<div class='param'><strong>Годовые затраты на топливо:</strong> " + QString::number(currentData.annualFuelCost, 'f', 2) + " руб</div>";

    if (currentData.fuelType == "Газомоторное топливо (СПГ)") {
        html += "<div class='param'><strong>Экономический эффект:</strong> " + QString::number(currentData.savings, 'f', 2) + " руб/год</div>"
                "<div class='param'><strong>Срок окупаемости:</strong> " + QString::number(currentData.paybackPeriod, 'f', 2) + " лет</div>";
    }

    html += "</div></div>";

    if (currentData.fuelType == "Газомоторное топливо (СПГ)") {
        html += "<div class='section'>"
                "<h2>ЭКОЛОГИЧЕСКИЙ ЭФФЕКТ</h2>"
                "<div class='result'>"
                "<div class='param'><strong>Снижение выбросов NOx:</strong> " + QString::number(currentData.emissionReductionNOx) + "%</div>"
                "<div class='param'><strong>Снижение выбросов CO₂:</strong> " + QString::number(currentData.emissionReductionCO2) + "%</div>"
                "<div class='param'><strong>Снижение выбросов твердых частиц:</strong> " + QString::number(currentData.emissionReductionPM) + "%</div>"
                "</div></div>";
    }

    html += "<div class='section'>"
            "<h2>ТЕХНИЧЕСКИЕ ХАРАКТЕРИСТИКИ</h2>"
            "<div class='result'>"
            "<div class='param'><strong>Энергоемкость дизельного топлива:</strong> " + QString::number(DIESEL_ENERGY, 'f', 1) + " МДж/кг</div>"
            "<div class='param'><strong>Энергоемкость СПГ:</strong> " + QString::number(LNG_ENERGY, 'f', 1) + " МДж/кг</div>"
            "<div class='param'><strong>Плотность дизельного топлива:</strong> " + QString::number(DIESEL_DENSITY, 'f', 0) + " кг/м³</div>"
            "<div class='param'><strong>Плотность СПГ:</strong> " + QString::number(LNG_DENSITY, 'f', 0) + " кг/м³</div>";

    double relativeVolume = (DIESEL_ENERGY * DIESEL_DENSITY) / (LNG_ENERGY * LNG_DENSITY);
    html += "<div style='page-break-after: always;'></div>" "<div class='param'><strong>Относительный объем топливных систем (СПГ/Дизель):</strong> " + QString::number(relativeVolume, 'f', 2) + "</div>" "</div></div>";

    html += "<div class='scene-title'>Визуализация сцены</div>";

    QPixmap scenePixmap(graphicsView->viewport()->size());
    QPainter painter(&scenePixmap);
    graphicsView->render(&painter);
    painter.end();

    QString tempImagePath = QDir::tempPath() + "/scene_snapshot.png";
    scenePixmap.save(tempImagePath, "PNG");

    html += "<div style='text-align: center; margin-top: 20px;'>" "<img src='file:///" + tempImagePath + "' " "style='max-width: 90%; height: auto; border: none;'/>" "</div>";

    html += "</body></html>";

    document.setHtml(html);
    document.setPageSize(printer.pageRect(QPrinter::Point).size());
    document.print(&printer);

    QFile::remove(tempImagePath);

    QMessageBox::information(this, "Успех", "PDF отчет успешно сохранен в файл:\n" + fileName);
    statusBar()->showMessage("PDF отчет сохранен: " + fileName);
}

void MainWindow::showHelp() {
    QMessageBox::information(this, "Справка",
                            "Программа для анализа экономической эффективности\n"
                            "использования сжиженного природного газа (СПГ) вместо\n"
                            "дизельного топлива на железнодорожном транспорте.\n\n"

                            "Основные функции:\n"
                            "1. Ввод параметров эксплуатации и экономических показателей\n"
                            "2. Расчет годовых затрат на топливо\n"
                            "3. Расчет срока окупаемости модернизации\n"
                            "4. Оценка экологического эффекта\n"
                            "5. Визуализация работы локомотива и расхода топлива\n\n"

                            "Управление:\n"
                            "• A/D - переключение типа топлива\n"
                            "• Пробел - старт/пауза анимации\n"
                            "• R - сброс анимации\n"
                            "• I - эта справка\n"
                            "• Ctrl+S - сохранение отчета TXT\n"
                            "• Ctrl+Shift+S - сохранение отчета PDF\n"
                            "• Правый клик на локомотиве/баке - контекстное меню\n\n"

                            "Формулы расчета:\n"
                            "1. Годовая потребность в энергии:\n"
                            "   Энергия_год = Мощность × (Годовой_пробег / Скорость) × 3.6\n"
                            "2. Расход топлива:\n"
                            "   Расход = Энергия_год / (Энергоёмкость × 1000)\n"
                            "3. Затраты на топливо:\n"
                            "   Затраты = Расход × Стоимость_тонны\n"
                            "4. Экономия:\n"
                            "   Годовая_экономия = Затраты_дизель - Затраты_СПГ\n"
                            "5. Срок окупаемости:\n"
                            "   Срок = Стоимость_модернизации / Годовая_экономия");
}

void MainWindow::refuel() {
    currentFuelLevel = 1.0;
    updateFuelTankVisual();
    statusBar()->showMessage("Топливный бак заправлен");

    if (!isAnimationRunning) {
        statusBar()->showMessage("Топливный бак заправлен. Можно запустить анимацию.");
    }
}

void MainWindow::showTechnicalData() {
    QMessageBox::information(this, "Технические данные",
                            "Константы из технической статьи:\n\n"
                            "Энергетические характеристики:\n"
                            "• Средняя энергоемкость дизельного топлива: 42.7 МДж/кг\n"
                            "• Средняя энергоемкость СПГ: 49.0 МДж/кг\n"
                            "• Плотность дизельного топлива: 850 кг/м³\n"
                            "• Плотность СПГ: 420 кг/м³\n\n"

                            "Экологические коэффициенты снижения выбросов\n"
                            "при переходе на СПГ:\n"
                            "• Оксиды азота (NOx): 85%\n"
                            "• Диоксид углерода (CO₂): 25%\n"
                            "• Твердые частицы (сажа): 92.5%\n\n");
}

void MainWindow::toggleAnimation() {
    if (isAnimationRunning) {
        animationTimer->stop();
        fuelConsumptionTimer->stop();
        statusBar()->showMessage("Анимация приостановлена");
        isAnimationRunning = false;
    } else {
        if (currentFuelLevel <= 0.01) {
            QMessageBox::warning(this, "Нет топлива", "Топливный бак пуст! Заправьте топливо перед запуском анимации.\n\n"
            "Нажмите правой кнопкой мыши на локомотиве или топливном баке для заправки.");
            statusBar()->showMessage("Невозможно запустить анимацию: топливный бак пуст");
            return;
        }

        animationTimer->start(30);
        fuelConsumptionTimer->start(80);
        statusBar()->showMessage("Анимация запущена");
        isAnimationRunning = true;
    }
}

void MainWindow::resetAnimation() {
    locomotivePosition = 0.0;
    if (locomotiveItem) {
        locomotiveItem->setPos(locomotivePosition, 400);
    }

    if (isAnimationRunning) {
        animationTimer->stop();
        fuelConsumptionTimer->stop();
        isAnimationRunning = false;
        statusBar()->showMessage("Анимация сброшена");
    } else {
        statusBar()->showMessage("Позиция локомотива сброшена. Уровень топлива: " + QString::number(static_cast<int>(currentFuelLevel * 100)) + "%");
    }
}

void MainWindow::switchToDiesel() {
    currentData.fuelType = "Дизельное топливо";
    updateFuelTankVisual();
    calculate();
    statusBar()->showMessage("Режим: Дизельное топливо");
}

void MainWindow::switchToLNG() {
    currentData.fuelType = "Газомоторное топливо (СПГ)";
    updateFuelTankVisual();
    calculate();
    statusBar()->showMessage("Режим: Газомоторное топливо (СПГ)");
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_A:
        switchToDiesel();
        break;
    case Qt::Key_D:
        switchToLNG();
        break;
    case Qt::Key_Space:
        toggleAnimation();
        break;
    case Qt::Key_R:
        resetAnimation();
        break;
    case Qt::Key_I:
        showHelp();
        break;
    case Qt::Key_S:
        if (event->modifiers() & Qt::ControlModifier) {
            if (event->modifiers() & Qt::ShiftModifier) {
                saveAsPDF();
            } else {
                saveReport();
            }
        }
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        QPointF scenePos = graphicsView->mapToScene(event->pos());
        QGraphicsItem *item = scene->itemAt(scenePos, QTransform());

        if (item == locomotiveItem || item == fuelLevelItem ||
            (item && item->parentItem() == fuelLevelItem) ||
            (item && item->zValue() >= 50 && item->zValue() <= 54)) {
            createContextMenu(event->globalPos());
        }
    }
}

void MainWindow::createContextMenu(const QPoint &pos) {
    QMenu contextMenu;

    QString refuelText = "Заправить";
    if (currentFuelLevel <= 0.01) {
        refuelText += " (бак пуст!)";
    } else {
        refuelText += QString(" (сейчас: %1%)").arg(static_cast<int>(currentFuelLevel * 100));
    }

    contextMenu.addAction(refuelText, this, SLOT(refuel()));
    contextMenu.addSeparator();
    contextMenu.addAction("Показать технические данные", this, SLOT(showTechnicalData()));
    contextMenu.addSeparator();

    QString toggleText = isAnimationRunning ? "Пауза" : "Старт";
    if (!isAnimationRunning && currentFuelLevel <= 0.01) {
        toggleText += " (нет топлива!)";
    }

    contextMenu.addAction(toggleText, this, SLOT(toggleAnimation()));
    contextMenu.addAction("Сбросить анимацию", this, SLOT(resetAnimation()));

    contextMenu.exec(pos);
}
