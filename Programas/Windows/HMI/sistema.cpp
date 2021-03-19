#include "sistema.h"

Sistema::Sistema(QWidget *parent) : QWidget(parent)
{
    timerRefresh = new QTimer();

    connect(timerRefresh, &QTimer::timeout, this, &Sistema::actualizarDatos);

    timerRefresh->start(1000);
}

Sistema::~Sistema()
{
    delete timerRefresh;
}

void Sistema::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QPen pen;
    QBrush brush;

    QFont font = painter.font();

    // Colores y renderizador iniciales
    painter.setRenderHint(QPainter::Antialiasing);

    pen.setStyle(Qt::SolidLine);
    brush.setStyle(Qt::SolidPattern);

    pen.setWidth(3);

    pen.setColor(QColor(255, 255, 255, 255));
    brush.setColor(QColor(255, 255, 255, 255));

    painter.setPen(pen);
    painter.setBrush(brush);

    // Normalizacion del sistema de cordenadas
    tamanioMinimo = qMin(this->width(), this->height());
    escala = tamanioMinimo / 400.0;

    painter.translate(QPointF(this->width() / 2.0, this->height() / 2.0));
    painter.scale(escala, escala);

    // Se guarda el painter
    painter.save();

    // Se dibuja el sistema
    brush.setStyle(Qt::NoBrush);
    pen.setColor(QColor(75, 106, 209, 255));

    painter.setBrush(brush);
    painter.setPen(pen);

    // Uniones
    painter.drawArc(QRectF(QPointF(-80.0, -40), QPointF(0.0, 40.0)), -90 * 16, 180 * 16);
    painter.drawArc(QRectF(QPointF(-80.0, -40), QPointF(0.0, 40.0)), -90 * 16, 180 * 16);

    painter.drawArc(QRectF(QPointF(-120.0, -80), QPointF(40.0, 80.0)), -90 * 16, 75 * 16);
    painter.drawArc(QRectF(QPointF(-120.0, -80), QPointF(40.0, 80.0)), 15 * 16, 75 * 16);

    // Rectangulos principales
    painter.drawLine(QPointF(-180.0, -80.0), QPointF(-40.0, -80.0));
    painter.drawLine(QPointF(-180.0, -40.0), QPointF(-40.0, -40.0));

    painter.drawLine(QPointF(-180.0, 40.0), QPointF(-40.0, 40.0));
    painter.drawLine(QPointF(-180.0, 80.0), QPointF(-40.0, 80.0));

    painter.drawLine(QPointF(38.0, -20.0), QPointF(180.0, -20.0));
    painter.drawLine(QPointF(38.0, 20.0), QPointF(180.0, 20.0));

    // Salidas y entradas en perspectiva
    painter.drawEllipse(QRectF(QPointF(-190.0, -80), QPointF(-170.0, -40.0)));
    painter.drawEllipse(QRectF(QPointF(-190.0, 40), QPointF(-170.0, 80.0)));
    painter.drawEllipse(QRectF(QPointF(170.0, -20), QPointF(190.0, 20.0)));

    // Se recupera el painter
    painter.restore();
    painter.save();

    // Se dibujan los parametros
    brush.setStyle(Qt::SolidPattern);
    pen.setColor(QColor(75, 200, 100, 255));

    painter.setBrush(brush);
    painter.setPen(pen);

    font.setPixelSize(24);

    painter.setFont(font);

    painter.drawText(QPointF(80.0, -40.0), QString::asprintf("%.2f°C", tunel.tempSalida));
    painter.drawText(QPointF(80.0, 60.0), QString::asprintf("%.2f°C", tunel.gradosSalidaSet));

    painter.drawText(QPointF(-140.0, -100.0), QString::asprintf("%.2f%%", tunel.caudalEntrada));
    painter.drawText(QPointF(-140.0, -10.0), QString::asprintf("%.2f°", tunel.gradosClapeta));

    // Se recupera el painter
    painter.restore();
}

void Sistema::setData(tunel_de_viento_t tunel)
{
    this->tunel.tempSalida = tunel.tempSalida;
    this->tunel.caudalEntrada = tunel.caudalEntrada;
    this->tunel.gradosClapeta = tunel.gradosClapeta;
    this->tunel.gradosSalidaSet = tunel.gradosSalidaSet;
}

void Sistema::actualizarDatos()
{
    this->update();
}
