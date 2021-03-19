#include "mainwindowconectarapuerto.h"
#include "ui_mainwindowconectarapuerto.h"

MainWindowConectarAPuerto::MainWindowConectarAPuerto(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowConectarAPuerto)
{
    ui->setupUi(this);

    serialPort = new QSerialPort();

    this->serialPort->setBaudRate(QSerialPort::Baud19200);

    serialOpen = false;

    ui->pushButtonDesconectar->setEnabled(false);
}

MainWindowConectarAPuerto::~MainWindowConectarAPuerto()
{
    delete serialPort;

    delete ui;
}

void MainWindowConectarAPuerto::closeEvent(QCloseEvent *)
{
    emit(disconectMainWindowConectarASignals());
}

void MainWindowConectarAPuerto::on_pushButtonConectar_clicked()
{
    emit(conectSerial(serialPort, serialInfo));

    this->close();
}

void MainWindowConectarAPuerto::on_pushButtonDesconectar_clicked()
{
    emit(disconectSerial());

    this->close();
}

void MainWindowConectarAPuerto::on_pushButtonActualizarPuertos_clicked()
{
    ui->comboBoxPuertosDisponibles->clear();

    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
    {
        ui->comboBoxPuertosDisponibles->addItem(info.portName() + " (" + info.description() + ")");
    }
}

void MainWindowConectarAPuerto::on_comboBoxPuertosDisponibles_currentIndexChanged(int index)
{
    QList<QSerialPortInfo> info = QSerialPortInfo::availablePorts();

    serialInfo = info.value(index).description();

    serialPort->setPortName(info.value(index).portName());
}

void MainWindowConectarAPuerto::setInfo(QString portName, QString portInfo)
{
    ui->comboBoxPuertosDisponibles->addItem(portName + " (" + portInfo + ")");

    serialOpen = true;

    ui->pushButtonDesconectar->setEnabled(true);
    ui->pushButtonConectar->setEnabled(false);
    ui->pushButtonActualizarPuertos->setEnabled(false);
    ui->comboBoxPuertosDisponibles->setEnabled(false);
}
