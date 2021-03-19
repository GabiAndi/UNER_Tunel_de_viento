#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QSerialPort>
#include <QIODevice>
#include <QMessageBox>
#include <QTimer>

#include "mainwindowgraficas.h"
#include "mainwindowconectarapuerto.h"

#include "datatypes.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private:
        Ui::MainWindow *ui;

        MainWindowGraficas *mainWindowGraficas;
        MainWindowConectarAPuerto *mainWindowConectarAPuerto;

        QTimer *hmi;

        QSerialPort *serialPort;
        QString serialInfo;

        serial_manager_t serialManager;

        bool plot;

        tunel_de_viento_t tunel;

        void serialWrite(uint8_t cmd, uint8_t *payload, uint8_t lengh);
        uint8_t checksum(uint8_t *data, uint8_t lengh);

    public slots:
        void conectSerial(QSerialPort *serialPort, QString serialInfo);
        void disconectSerial();

        void disconectMainWindowConectarASignals();
        void disconectMainWindowGraficasSignals();

    private slots:
        void refreshData();
        void serialRead();

        void serialReadTimeOut();

        void hmiRefresh();

        void on_actionGr_ficas_triggered();
        void on_actionSalir_triggered();
        void on_actionConectar_triggered();
        void on_horizontalSliderTemperaturaSalida_valueChanged(int value);
        void on_pushButtonEnviarTemperatura_clicked();
        void on_pushButtonEnviarGradosClapeta_clicked();
        void on_horizontalSliderGradosClapeta_valueChanged(int value);
        void on_pushButtonEnviarModo_clicked();
};
#endif // MAINWINDOW_H
