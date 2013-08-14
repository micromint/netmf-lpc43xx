////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Microsoft Corporation.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SPOT_Hardware_serial.h"

static const CLR_RT_MethodHandler method_lookup[] =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    Library_spot_hardware_serial_native_System_IO_Ports_SerialPort::Read___I4__SZARRAY_U1__I4__I4,
    Library_spot_hardware_serial_native_System_IO_Ports_SerialPort::Write___VOID__SZARRAY_U1__I4__I4,
    Library_spot_hardware_serial_native_System_IO_Ports_SerialPort::Flush___VOID,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    Library_spot_hardware_serial_native_System_IO_Ports_SerialPort::InternalOpen___VOID,
    Library_spot_hardware_serial_native_System_IO_Ports_SerialPort::InternalClose___VOID,
    Library_spot_hardware_serial_native_System_IO_Ports_SerialPort::InternalDispose___VOID,
    Library_spot_hardware_serial_native_System_IO_Ports_SerialPort::BytesInBuffer___I4__BOOLEAN,
    Library_spot_hardware_serial_native_System_IO_Ports_SerialPort::DiscardBuffer___VOID__BOOLEAN,
    NULL,
};

const CLR_RT_NativeAssemblyData g_CLR_AssemblyNative_Microsoft_SPOT_Hardware_SerialPort =
{
    "Microsoft.SPOT.Hardware.SerialPort", 
#if defined(PLATFORM_ARM)
    0x1C1E5B6D,
#else
    0x4E6685A9,
#endif
    method_lookup
};

