#ifndef _OS_SPI_H//vermeidet Mehrfachinkludierung
#define _OS_SPI_H

//Includes----------------------------------------------------------------------

//doxygen
#include <stdint.h>
#include "util.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

//Defines-----------------------------------------------------------------------

//Global Variables--------------------------------------------------------------

//Datatypes---------------------------------------------------------------------

//Functions---------------------------------------------------------------------

/*
*Konfiguriert alle relevanten I/O-Pins und aktiviert den SPI-Bus Master (Mikrocontroller)
*/
void os_spi_init(void);

/*
*Senden von Daten, Master an Slave
*/
uint8_t os_spi_send(uint8_t data);

/*
*Erhalten von Daten, Slave and Master
*/
uint8_t os_spi_receive();

#endif
