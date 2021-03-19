#ifndef SISTEMA_H
#define SISTEMA_H

#include <QWidget>

#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>

#include <QtMath>

#include <QTimer>

#include "datatypes.h"

class Sistema : public QWidget
{
    Q_OBJECT

    public:
        explicit Sistema(QWidget *parent = nullptr);
        ~Sistema();

        void setData(tunel_de_viento_t tunel);

    protected:
        void paintEvent(QPaintEvent *);

    private:
        tunel_de_viento_t tunel;

        QTimer *timerRefresh;

        double tamanioMinimo;
        double escala;

    private slots:
        void actualizarDatos();
};

#endif // SISTEMA_H
