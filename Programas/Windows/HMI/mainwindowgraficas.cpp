#include "mainwindowgraficas.h"
#include "ui_mainwindowgraficas.h"

MainWindowGraficas::MainWindowGraficas(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowGraficas)
{
    ui->setupUi(this);

    createClapetaChart();
    createTempSalidaChart();
    createCaudalChart();
}

MainWindowGraficas::~MainWindowGraficas()
{
    // Clapeta
    delete clapetaSpline;
    delete clapetaChart;
    delete clapetaChartView;
    delete clapetaLayout;

    // Temperatura salida
    delete tempSalidaSpline;
    delete tempSalidaChart;
    delete tempSalidaChartView;
    delete tempSalidaLayout;

    // Caudal
    delete caudalSpline;
    delete caudalChart;
    delete caudalChartView;
    delete caudalLayout;

    delete ui;
}

void MainWindowGraficas::closeEvent(QCloseEvent *)
{
    emit(disconectMainWindowGraficasSignals());
}

void MainWindowGraficas::createClapetaChart()
{
    clapetaSpline = new QSplineSeries();

    for (int i = 0 ; i <= 20 ; i++)
    {
        clapetaDatos.append(QPointF(i, 0));
    }

    clapetaSpline->append(clapetaDatos);

    clapetaChart = new QChart();

    clapetaChart->setTitle("Posicion de la clapeta");
    clapetaChart->legend()->hide();
    clapetaChart->addSeries(clapetaSpline);
    clapetaChart->createDefaultAxes();
    clapetaChart->axes(Qt::Vertical).first()->setRange(0, 90);
    clapetaChart->axes(Qt::Horizontal).first()->setRange(0, 20);

    clapetaChartView = new QChartView(clapetaChart);

    clapetaChartView->setRenderHint(QPainter::Antialiasing);

    clapetaLayout = new QGridLayout();

    clapetaLayout->addWidget(clapetaChartView, 0, 0);

    ui->widgetClapeta->setLayout(clapetaLayout);
}

void MainWindowGraficas::createTempSalidaChart()
{
    tempSalidaSpline = new QSplineSeries();

    for (int i = 0 ; i <= 20 ; i++)
    {
        tempSalidaDatos.append(QPointF(i, 0));
    }

    tempSalidaSpline->append(tempSalidaDatos);

    tempSalidaChart = new QChart();

    tempSalidaChart->setTitle("Temperatura de salida");
    tempSalidaChart->legend()->hide();
    tempSalidaChart->addSeries(tempSalidaSpline);
    tempSalidaChart->createDefaultAxes();
    tempSalidaChart->axes(Qt::Vertical).first()->setRange(0, 80);
    tempSalidaChart->axes(Qt::Horizontal).first()->setRange(0, 20);

    tempSalidaChartView = new QChartView(tempSalidaChart);

    tempSalidaChartView->setRenderHint(QPainter::Antialiasing);

    tempSalidaLayout = new QGridLayout();

    tempSalidaLayout->addWidget(tempSalidaChartView, 0, 0);

    ui->widgetTemperaturaSalida->setLayout(tempSalidaLayout);
}

void MainWindowGraficas::createCaudalChart()
{
    caudalSpline = new QSplineSeries();

    for (int i = 0 ; i <= 20 ; i++)
    {
        caudalDatos.append(QPointF(i, 0));
    }

    caudalSpline->append(caudalDatos);

    caudalChart = new QChart();

    caudalChart->setTitle("Caudal de entrada");
    caudalChart->legend()->hide();
    caudalChart->addSeries(caudalSpline);
    caudalChart->createDefaultAxes();
    caudalChart->axes(Qt::Vertical).first()->setRange(0, 100);
    caudalChart->axes(Qt::Horizontal).first()->setRange(0, 20);

    caudalChartView = new QChartView(caudalChart);

    caudalChartView->setRenderHint(QPainter::Antialiasing);

    caudalLayout = new QGridLayout();

    caudalLayout->addWidget(caudalChartView, 0, 0);

    ui->widgetCaudal->setLayout(caudalLayout);
}

void MainWindowGraficas::addPointClapetaChart(double point)
{
    // Se borra la memoria asignada
    delete clapetaSpline;
    delete clapetaChart;
    delete clapetaChartView;
    delete clapetaLayout;

    // Se añade el dato recibido
    clapetaSpline = new QSplineSeries();

    for (int i = 0 ; i < 20 ; i++)
    {
        clapetaDatos.replace(i, QPointF(i, clapetaDatos.value(i + 1).ry()));
    }

    clapetaDatos.removeLast();

    clapetaDatos.append(QPointF(20, point));

    clapetaSpline->append(clapetaDatos);

    clapetaChart = new QChart();

    clapetaChart->setTitle("Posicion de la clapeta");
    clapetaChart->legend()->hide();
    clapetaChart->addSeries(clapetaSpline);
    clapetaChart->createDefaultAxes();
    clapetaChart->axes(Qt::Vertical).first()->setRange(0, 90);
    clapetaChart->axes(Qt::Horizontal).first()->setRange(0, 20);

    clapetaChartView = new QChartView(clapetaChart);

    clapetaChartView->setRenderHint(QPainter::Antialiasing);

    clapetaLayout = new QGridLayout();

    clapetaLayout->addWidget(clapetaChartView, 0, 0);

    ui->widgetClapeta->setLayout(clapetaLayout);
}

void MainWindowGraficas::addPointTempSalidaChart(double point)
{
    // Se borra la memoria asignada
    delete tempSalidaSpline;
    delete tempSalidaChart;
    delete tempSalidaChartView;
    delete tempSalidaLayout;

    // Se añade el dato recibido
    tempSalidaSpline = new QSplineSeries();

    for (int i = 0 ; i < 20 ; i++)
    {
        tempSalidaDatos.replace(i, QPointF(i, tempSalidaDatos.value(i + 1).ry()));
    }

    tempSalidaDatos.removeLast();

    tempSalidaDatos.append(QPointF(20, point));

    tempSalidaSpline->append(tempSalidaDatos);

    tempSalidaChart = new QChart();

    tempSalidaChart->setTitle("Temperatura de salida");
    tempSalidaChart->legend()->hide();
    tempSalidaChart->addSeries(tempSalidaSpline);
    tempSalidaChart->createDefaultAxes();
    tempSalidaChart->axes(Qt::Vertical).first()->setRange(0, 80);
    tempSalidaChart->axes(Qt::Horizontal).first()->setRange(0, 20);

    tempSalidaChartView = new QChartView(tempSalidaChart);

    tempSalidaChartView->setRenderHint(QPainter::Antialiasing);

    tempSalidaLayout = new QGridLayout();

    tempSalidaLayout->addWidget(tempSalidaChartView, 0, 0);

    ui->widgetTemperaturaSalida->setLayout(tempSalidaLayout);
}

void MainWindowGraficas::addPointCaudalChart(double point)
{
    // Se borra la memoria asignada
    delete caudalSpline;
    delete caudalChart;
    delete caudalChartView;
    delete caudalLayout;

    // Se añade el dato recibido
    caudalSpline = new QSplineSeries();

    for (int i = 0 ; i < 20 ; i++)
    {
        caudalDatos.replace(i, QPointF(i, caudalDatos.value(i + 1).ry()));
    }

    caudalDatos.removeLast();

    caudalDatos.append(QPointF(20, point));

    caudalSpline->append(caudalDatos);

    caudalChart = new QChart();

    caudalChart->setTitle("Caudal de entrada");
    caudalChart->legend()->hide();
    caudalChart->addSeries(caudalSpline);
    caudalChart->createDefaultAxes();
    caudalChart->axes(Qt::Vertical).first()->setRange(0, 100);
    caudalChart->axes(Qt::Horizontal).first()->setRange(0, 20);

    caudalChartView = new QChartView(caudalChart);

    caudalChartView->setRenderHint(QPainter::Antialiasing);

    caudalLayout = new QGridLayout();

    caudalLayout->addWidget(caudalChartView, 0, 0);

    ui->widgetCaudal->setLayout(caudalLayout);
}
