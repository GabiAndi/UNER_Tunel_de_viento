#ifndef DATATYPES_H
#define DATATYPES_H

#include <QTimer>

#include <inttypes.h>

struct ring_buffer
{
    uint8_t datos[256];

    uint8_t indiceLectura;
    uint8_t indiceEscritura;
}typedef ring_buffer_t;

union datos
{
    uint8_t u8[4];
    uint16_t u16[2];
    uint32_t u32;

    int8_t i8[4];
    int16_t i16[2];
    int32_t i32;
}typedef datos_u;

struct serial_manager
{
    QTimer refresh;
    QTimer timeOut;

    ring_buffer bufferEntrada;
    ring_buffer bufferSalida;

    uint8_t readState;
    uint8_t payloadInit;
    uint8_t payloadLengh;

    datos_u conversor;
}typedef serial_manager_t;

struct tunel_de_viento
{
    double gradosClapeta;
    double tempSalida;
    double caudalEntrada;
    double gradosSalidaSet;
}typedef tunel_de_viento_t;

#endif // DATATYPES_H
