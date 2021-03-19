#ifndef MAINWINDOWGRAFICAS_H
#define MAINWINDOWGRAFICAS_H

#include <QMainWindow>

#include <QtCharts>
#include <QSplineSeries>
#include <QChartView>
#include <QGridLayout>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindowGraficas;
}
QT_END_NAMESPACE

using namespace QtCharts;

class MainWindowGraficas : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindowGraficas(QWidget *parent = nullptr);
        ~MainWindowGraficas();

        void addPointClapetaChart(double point);
        void addPointTempSalidaChart(double point);
        void addPointCaudalChart(double point);

    protected:
        void closeEvent(QCloseEvent *);

    private:
        Ui::MainWindowGraficas *ui;

        // Grafico clapeta
        QSplineSeries *clapetaSpline;
        QChart *clapetaChart;
        QChartView *clapetaChartView;
        QGridLayout *clapetaLayout;

        QList<QPointF> clapetaDatos;

        // Grafico temperatura de salida
        QSplineSeries *tempSalidaSpline;
        QChart *tempSalidaChart;
        QChartView *tempSalidaChartView;
        QGridLayout *tempSalidaLayout;

        QList<QPointF> tempSalidaDatos;

        // Grafico caudal
        QSplineSeries *caudalSpline;
        QChart *caudalChart;
        QChartView *caudalChartView;
        QGridLayout *caudalLayout;

        QList<QPointF> caudalDatos;

        void createClapetaChart();
        void createTempSalidaChart();
        void createCaudalChart();

    signals:
        void disconectMainWindowGraficasSignals();
};

#endif // MAINWINDOWGRAFICAS_H
