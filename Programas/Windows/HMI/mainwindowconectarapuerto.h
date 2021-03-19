#ifndef MAINWINDOWCONECTARAPUERTO_H
#define MAINWINDOWCONECTARAPUERTO_H

#include <QMainWindow>

#include <QSerialPort>
#include <QSerialPortInfo>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindowConectarAPuerto;
}
QT_END_NAMESPACE

class MainWindowConectarAPuerto : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindowConectarAPuerto(QWidget *parent = nullptr);
        ~MainWindowConectarAPuerto();

        void setInfo(QString portName, QString portInfo);

    protected:
        void closeEvent(QCloseEvent *);

    private:
        Ui::MainWindowConectarAPuerto *ui;

        QSerialPort *serialPort;
        QString serialInfo;
        bool serialOpen;

    signals:
        void conectSerial(QSerialPort *, QString);
        void disconectSerial();

        void disconectMainWindowConectarASignals();

    private slots:
        void on_pushButtonConectar_clicked();
        void on_pushButtonDesconectar_clicked();
        void on_pushButtonActualizarPuertos_clicked();
        void on_comboBoxPuertosDisponibles_currentIndexChanged(int index);
};

#endif // MAINWINDOWCONECTARAPUERTO_H
