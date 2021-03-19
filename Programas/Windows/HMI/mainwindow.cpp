#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Configuracion del puerto serie
    serialPort = new QSerialPort();

    connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::serialRead);

    // Configuracion de los timers
    connect(&serialManager.refresh, &QTimer::timeout, this, &MainWindow::refreshData);
    connect(&serialManager.timeOut, &QTimer::timeout, this, &MainWindow::serialReadTimeOut);

    // Timer del HMI
    hmi = new QTimer();

    connect(hmi, &QTimer::timeout, this, &MainWindow::hmiRefresh);

    // Configuracion de los buffer
    serialManager.bufferEntrada.indiceLectura = 0;
    serialManager.bufferEntrada.indiceEscritura = 0;

    serialManager.bufferSalida.indiceLectura = 0;
    serialManager.bufferSalida.indiceEscritura = 0;

    serialManager.readState = 0;

    // Configuracion de graficas
    plot = false;

    // Valores iniciales del tunel
    tunel.gradosClapeta = 45.0;
    tunel.tempSalida = 40.0;
    tunel.caudalEntrada = 50.0;
    tunel.gradosSalidaSet = 40.0;

    // Configuracion de los botones
    ui->pushButtonEnviarTemperatura->setEnabled(false);
    ui->pushButtonEnviarGradosClapeta->setEnabled(false);
    ui->pushButtonEnviarModo->setEnabled(false);

    // Valores iniciales al hmi
    hmiRefresh();
}

MainWindow::~MainWindow()
{
    delete serialPort;

    delete hmi;

    delete ui;
}

void MainWindow::on_actionSalir_triggered()
{
    this->close();
}

void MainWindow::on_actionGr_ficas_triggered()
{
    if (!plot)
    {
        mainWindowGraficas = new MainWindowGraficas(this);

        mainWindowGraficas->show();

        connect(mainWindowGraficas, &MainWindowGraficas::disconectMainWindowGraficasSignals, this, &MainWindow::disconectMainWindowGraficasSignals);

        plot = true;
    }
}

void MainWindow::on_actionConectar_triggered()
{
    mainWindowConectarAPuerto = new MainWindowConectarAPuerto(this);

    mainWindowConectarAPuerto->show();

    if (serialPort->isOpen())
    {
        mainWindowConectarAPuerto->setInfo(serialPort->portName(), serialInfo);
    }

    connect(mainWindowConectarAPuerto, &MainWindowConectarAPuerto::conectSerial, this, &MainWindow::conectSerial);
    connect(mainWindowConectarAPuerto, &MainWindowConectarAPuerto::disconectSerial, this, &MainWindow::disconectSerial);
    connect(mainWindowConectarAPuerto, &MainWindowConectarAPuerto::disconectMainWindowConectarASignals, this, &MainWindow::disconectMainWindowConectarASignals);
}

void MainWindow::conectSerial(QSerialPort *serialPort, QString serialInfo)
{
    this->serialPort->setPortName(serialPort->portName());
    this->serialPort->setBaudRate(serialPort->baudRate());

    this->serialInfo = serialInfo;

    if (this->serialPort->open(QIODevice::ReadWrite))
    {
        QMessageBox::information(this, "Conectado", "Se logro la conexión a " + this->serialPort->portName() + ".", QMessageBox::Ok);

        ui->actionConectar->setText("Desconectar");
        ui->pushButtonEnviarTemperatura->setEnabled(true);
        ui->pushButtonEnviarGradosClapeta->setEnabled(true);
        ui->pushButtonEnviarModo->setEnabled(true);

        serialManager.refresh.start(1000);
        hmi->start(1000);
    }

    else
    {
        QMessageBox::critical(this, "Error", "No se logro la conexión a " + this->serialPort->portName() + ".", QMessageBox::Ok);
    }
}

void MainWindow::disconectSerial()
{
    serialPort->close();

    if (!serialPort->isOpen())
    {
        QMessageBox::information(this, "Desconectado", "Se logro la desconexión de " + this->serialPort->portName() + ".", QMessageBox::Ok);

        ui->actionConectar->setText("Conectar");
        ui->pushButtonEnviarTemperatura->setEnabled(false);
        ui->pushButtonEnviarGradosClapeta->setEnabled(false);
        ui->pushButtonEnviarModo->setEnabled(false);

        serialManager.refresh.stop();
        hmi->stop();
    }

    else
    {
        QMessageBox::critical(this, "Error", "No se logro la desconexión de " + this->serialPort->portName() + ".", QMessageBox::Ok);
    }
}

void MainWindow::disconectMainWindowConectarASignals()
{
    disconnect(mainWindowConectarAPuerto, &MainWindowConectarAPuerto::conectSerial, this, &MainWindow::conectSerial);
    disconnect(mainWindowConectarAPuerto, &MainWindowConectarAPuerto::disconectSerial, this, &MainWindow::disconectSerial);
    disconnect(mainWindowConectarAPuerto, &MainWindowConectarAPuerto::disconectMainWindowConectarASignals, this, &MainWindow::disconectMainWindowConectarASignals);
}

void MainWindow::disconectMainWindowGraficasSignals()
{
    disconnect(mainWindowGraficas, &MainWindowGraficas::disconectMainWindowGraficasSignals, this, &MainWindow::disconectMainWindowGraficasSignals);

    plot = false;
}

void MainWindow::refreshData()
{
    if (serialPort->isOpen())
    {
        // Peticion de los datos de los sensores
        serialWrite(0x00, nullptr, 0);  // Peticion de la posicion de la clapeta
        serialWrite(0x02, nullptr, 0);  // Peticion de la temperatura de salida
        serialWrite(0x03, nullptr, 0);  // Peticion del caudal de entrada

        // Si hay algo para enviar por el puerto se envia
        while (serialManager.bufferSalida.indiceLectura != serialManager.bufferSalida.indiceEscritura)
        {
            serialPort->write(reinterpret_cast<char *>(&serialManager.bufferSalida.datos[serialManager.bufferSalida.indiceLectura]), 1);
            serialManager.bufferSalida.indiceLectura++;
        }
    }
}

void MainWindow::serialWrite(uint8_t cmd, uint8_t *payload, uint8_t lengh)
{
    // Cabecera
    serialManager.bufferSalida.datos[serialManager.bufferSalida.indiceEscritura] = 'P';
    serialManager.bufferSalida.indiceEscritura++;
    serialManager.bufferSalida.datos[serialManager.bufferSalida.indiceEscritura] = 'D';
    serialManager.bufferSalida.indiceEscritura++;
    serialManager.bufferSalida.datos[serialManager.bufferSalida.indiceEscritura] = 'C';
    serialManager.bufferSalida.indiceEscritura++;
    serialManager.bufferSalida.datos[serialManager.bufferSalida.indiceEscritura] = 'C';
    serialManager.bufferSalida.indiceEscritura++;
    serialManager.bufferSalida.datos[serialManager.bufferSalida.indiceEscritura] = lengh;
    serialManager.bufferSalida.indiceEscritura++;
    serialManager.bufferSalida.datos[serialManager.bufferSalida.indiceEscritura] = ':';
    serialManager.bufferSalida.indiceEscritura++;

    // Datos
    serialManager.bufferSalida.datos[serialManager.bufferSalida.indiceEscritura] = cmd;
    serialManager.bufferSalida.indiceEscritura++;

    for (uint8_t i = 0 ; i < lengh ; i++)
    {
        serialManager.bufferSalida.datos[serialManager.bufferSalida.indiceEscritura] = payload[i];
        serialManager.bufferSalida.indiceEscritura++;
    }

    serialManager.bufferSalida.datos[serialManager.bufferSalida.indiceEscritura] =
        checksum(&serialManager.bufferSalida.datos[serialManager.bufferSalida.indiceEscritura - lengh - 7], lengh + 7);
    serialManager.bufferSalida.indiceEscritura++;
}

uint8_t MainWindow::checksum(uint8_t *data, uint8_t lengh)
{
    uint8_t suma = 0x00;

    for (uint8_t i = 0 ; i < lengh ; i++)
    {
        suma ^= data[i];
    }

    return suma;
}

void MainWindow::serialRead()
{
    // Se capturan todos los datos
    while (serialPort->bytesAvailable())
    {
        serialPort->read(reinterpret_cast<char *>(&serialManager.bufferEntrada.datos[serialManager.bufferEntrada.indiceEscritura]), 1);
        serialManager.bufferEntrada.indiceEscritura++;
    }

    // Si hay algo para leer en el buffer se lee
    while (serialManager.bufferEntrada.indiceLectura != serialManager.bufferEntrada.indiceEscritura)
    {
        switch (serialManager.readState)
        {
            // Inicio de la cabecera
            case 0:
                if (serialManager.bufferEntrada.datos[serialManager.bufferEntrada.indiceLectura] == 'P')
                {
                    serialManager.timeOut.start(500);

                    serialManager.readState = 1;
                }

                break;

            case 1:
                if (serialManager.bufferEntrada.datos[serialManager.bufferEntrada.indiceLectura] == 'D')
                {
                    serialManager.readState = 2;
                }

                else
                {
                    serialManager.readState = 0;
                }

                break;

            case 2:
                if (serialManager.bufferEntrada.datos[serialManager.bufferEntrada.indiceLectura] == 'C')
                {
                    serialManager.readState = 3;
                }

                else
                {
                    serialManager.readState = 0;
                }

                break;

            case 3:
                if (serialManager.bufferEntrada.datos[serialManager.bufferEntrada.indiceLectura] == 'C')
                {
                    serialManager.readState = 4;
                }

                else
                {
                    serialManager.readState = 0;
                }

                break;

            case 4:
                serialManager.payloadLengh = serialManager.bufferEntrada.datos[serialManager.bufferEntrada.indiceLectura];

                serialManager.readState = 5;

                break;

            case 5:
                if (serialManager.bufferEntrada.datos[serialManager.bufferEntrada.indiceLectura] == ':')
                {
                    serialManager.readState = 6;
                }

                else
                {
                    serialManager.readState = 0;
                }

                break;

            // Inicio de la parte de comando y control
            case 6:
                serialManager.payloadInit = serialManager.bufferEntrada.indiceLectura + 1;

                serialManager.readState = 7;

                break;

            case 7:
                // Si se terminaron de recibir todos los datos
                if (serialManager.bufferEntrada.indiceLectura == (serialManager.payloadInit + serialManager.payloadLengh))
                {
                    // Se comprueba la integridad de datos
                    if (checksum(&serialManager.bufferEntrada.datos[serialManager.payloadInit - 7], serialManager.payloadLengh + 7)
                        == serialManager.bufferEntrada.datos[serialManager.bufferEntrada.indiceLectura])
                    {
                        // Analisis del comando recibido
                        switch (serialManager.bufferEntrada.datos[serialManager.payloadInit - 1])
                        {
                            case 0x00:  // Pos clapeta
                                serialManager.conversor.u8[0] = serialManager.bufferEntrada.datos[serialManager.payloadInit];
                                serialManager.conversor.u8[1] = serialManager.bufferEntrada.datos[serialManager.payloadInit + 1];

                                tunel.gradosClapeta = static_cast<double>(serialManager.conversor.u16[0]) * 180.0 / 1000.0;

                                if (plot)
                                {
                                    mainWindowGraficas->addPointClapetaChart(tunel.gradosClapeta);
                                }

                                break;

                            case 0x01:
                                serialManager.conversor.u8[0] = serialManager.bufferEntrada.datos[serialManager.payloadInit];
                                serialManager.conversor.u8[1] = serialManager.bufferEntrada.datos[serialManager.payloadInit + 1];

                                tunel.gradosSalidaSet = serialManager.conversor.i16[0] / 10.0;

                                ui->statusBar->showMessage(QString::asprintf("Temperatura seteada en %d°C",
                                                                             static_cast<int>(serialManager.conversor.i16[0] / 10.0)), 2000);
                                break;

                            case 0x02:  // Temperatura de salida
                                serialManager.conversor.u8[0] = serialManager.bufferEntrada.datos[serialManager.payloadInit];
                                serialManager.conversor.u8[1] = serialManager.bufferEntrada.datos[serialManager.payloadInit + 1];

                                tunel.tempSalida = static_cast<double>(serialManager.conversor.i16[0]) / 10.0;

                                if (plot)
                                {
                                    mainWindowGraficas->addPointTempSalidaChart(tunel.tempSalida);
                                }

                                break;

                            case 0x03:  // Caudal de entrada
                                serialManager.conversor.u8[0] = serialManager.bufferEntrada.datos[serialManager.payloadInit];
                                serialManager.conversor.u8[1] = serialManager.bufferEntrada.datos[serialManager.payloadInit + 1];

                                tunel.caudalEntrada = static_cast<double>(serialManager.conversor.i16[0]) / 10.0;

                                if (plot)
                                {
                                    mainWindowGraficas->addPointCaudalChart(tunel.caudalEntrada);
                                }

                                break;

                            case 0x04:
                                serialManager.conversor.u8[0] = serialManager.bufferEntrada.datos[serialManager.payloadInit];
                                serialManager.conversor.u8[1] = serialManager.bufferEntrada.datos[serialManager.payloadInit + 1];

                                ui->statusBar->showMessage(QString::asprintf("Clapeta seteada a %d°",
                                                                             static_cast<int>(serialManager.conversor.u16[0] * 180.0 / 1000.0)), 2000);

                                break;

                            case 0x05:
                                if (serialManager.bufferEntrada.datos[serialManager.payloadInit] == 0xFF)
                                {
                                    ui->labelModo->setText("Modo de operación automatico");

                                    ui->statusBar->showMessage("Modo automatico activado", 2000);
                                }

                                else if (serialManager.bufferEntrada.datos[serialManager.payloadInit] == 0x00)
                                {
                                    ui->labelModo->setText("Modo de operación manual");

                                    ui->statusBar->showMessage("Modo automatico desactivado", 2000);
                                }

                                break;

                            case 0xFE:	// ACK
                                ui->statusBar->showMessage("Respuesta del sistema", 2000);

                                break;

                            case 0xFF:  // Error de dato al enviar
                                ui->statusBar->showMessage("Error de dato de envio", 2000);

                                break;
                        }
                    }

                    // Corrupcion de datos al recibir
                    else
                    {
                        ui->statusBar->showMessage("Error de dato de recepcion", 2000);
                    }

                    serialManager.timeOut.stop();

                    serialManager.readState = 0;
                }

                break;
        }

        serialManager.bufferEntrada.indiceLectura++;
    }
}

void MainWindow::serialReadTimeOut()
{
    serialManager.timeOut.stop();

    ui->statusBar->showMessage("Paquete perdido", 2000);

    // Configuracion de los buffer
    serialManager.bufferEntrada.indiceLectura = serialManager.bufferEntrada.indiceEscritura;

    serialManager.readState = 0;
}

void MainWindow::on_horizontalSliderTemperaturaSalida_valueChanged(int value)
{
    ui->labelTemperaturaSeteada->setText(QString::asprintf("Temperatura seteada %d°C", value));
}

void MainWindow::hmiRefresh()
{
    tunel_de_viento_t tunelData;

    tunelData.tempSalida = this->tunel.tempSalida;
    tunelData.caudalEntrada = this->tunel.caudalEntrada;
    tunelData.gradosClapeta = this->tunel.gradosClapeta;
    tunelData.gradosSalidaSet = this->tunel.gradosSalidaSet;

    ui->widgetEstado->setData(tunelData);
}

void MainWindow::on_pushButtonEnviarTemperatura_clicked()
{
    serialManager.conversor.i16[0] = static_cast<int16_t>(ui->horizontalSliderTemperaturaSalida->value() * 10.0);

    serialWrite(0x01, &(serialManager.conversor.u8[0]), 2);
}

void MainWindow::on_pushButtonEnviarGradosClapeta_clicked()
{
    serialManager.conversor.u16[0] = static_cast<uint16_t>(ui->horizontalSliderGradosClapeta->value() * 1000.0 / 180.0);

    serialWrite(0x04, &(serialManager.conversor.u8[0]), 2);
}

void MainWindow::on_horizontalSliderGradosClapeta_valueChanged(int value)
{
    ui->labelGradosClapeta->setText(QString::asprintf("Setear posición de la clapeta a %d°", value));
}

void MainWindow::on_pushButtonEnviarModo_clicked()
{
    if (ui->checkBoxModo->isChecked())
    {
        serialManager.conversor.u8[0] = 0xFF;
    }

    else
    {
        serialManager.conversor.u8[0] = 0x00;
    }

    serialWrite(0x05, &(serialManager.conversor.u8[0]), 1);
}
