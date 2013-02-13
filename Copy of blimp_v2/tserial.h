
#pragma once
#ifndef TSERIAL_H
#define TSERIAL_H

/* ------------------------------------------------------------------------ --
--                                                                          --
--                        PC serial port connection object                  --
--                           for non-event-driven programs                  --
--                                                                          --
--                                                                          --
--                                                                          --
--  Copyright @ 2001          Thierry Schneider                             --
--                            thierry@tetraedre.com                         --
--                                                                          --
--                                                                          --
--                                                                          --
-- ------------------------------------------------------------------------ --
--                                                                          --
--  Filename : sertest2.cpp                                                 --
--  Author   : Thierry Schneider                                            --
--  Created  : April 4th 2000                                               --
--  Modified : April 8th 2001                                               --
--  Plateform: Windows 95, 98, NT, 2000 (Win32)                             --
-- ------------------------------------------------------------------------ --
--                                                                          --
--  This software is given without any warranty. It can be distributed      --
--  free of charge as long as this header remains, unchanged.               --
--                                                                          --
-- ------------------------------------------------------------------------ */




/* ---------------------------------------------------------------------- */

#include <windows.h>
#include "stdafx.h"
#include <stdio.h>



enum serial_parity  { spNONE, spODD, spEVEN };

/*
typedef int serial_parity;
#define spNONE 0
#define spODD 1
#define spEVEN 2
*/
/* -------------------------------------------------------------------- */
/* -----------------------------  Tserial  ---------------------------- */
/* -------------------------------------------------------------------- */
class Tserial
{
    // -------------------------------------------------------- //
protected:
    char              port[10];                      // port name "com1",...
    int               rate;                          // baudrate
    serial_parity     parityMode;
    HANDLE            serial_handle;                 // ...

    // ++++++++++++++++++++++++++++++++++++++++++++++
    // .................. EXTERNAL VIEW .............
    // ++++++++++++++++++++++++++++++++++++++++++++++
public:
                  Tserial();
                 ~Tserial();
    int           connect          (char *port_arg, int rate_arg,
                                    serial_parity parity_arg, bool flowControl);
    void          sendChar         (char c);
    void          sendArray        (char *buffer, int len);
    char          getChar          (void);
    int           getArray         (char *buffer, int len);
    int           getNbrOfBytes    (void);
    void          disconnect       (void);
};
/* -------------------------------------------------------------------- */

#endif


