#ifndef USBOHCI_H__
#define USBOHCI_H__

#include <ntddk.h>
#define NDEBUG
#include <debug.h>
#include <hubbusif.h>
#include <usbbusif.h>
#include <usbioctl.h>
#include <usb100.h>

extern
"C"
{
#include <usbdlib.h>
}

//
// FIXME: 
// #include <usbprotocoldefs.h>
//
#include <usb.h>
#include <stdio.h>
#include <wdmguid.h>

//
// FIXME:
// the following includes are required to get kcom to compile
//
#include <portcls.h>
#include <dmusicks.h>
#include <kcom.h>

#include "interfaces.h"

//
// flags for handling USB_REQUEST_SET_FEATURE / USB_REQUEST_GET_FEATURE
//
#define PORT_ENABLE         1
#define PORT_SUSPEND        2
#define PORT_OVER_CURRENT   3
#define PORT_RESET          4
#define PORT_POWER          8
#define C_PORT_CONNECTION   16
#define C_PORT_ENABLE       17
#define C_PORT_SUSPEND      18
#define C_PORT_OVER_CURRENT 19
#define C_PORT_RESET        20

//
// tag for allocations
//
#define TAG_USBOHCI 'ICHO'

//
// assert for c++ - taken from portcls
//
#define PC_ASSERT(exp) \
  (VOID)((!(exp)) ? \
    RtlAssert((PVOID) #exp, (PVOID)__FILE__, __LINE__, NULL ), FALSE : TRUE)

extern "C"
{

//
// hardware.cpp
//
NTSTATUS NTAPI CreateUSBHardware(PUSBHARDWAREDEVICE *OutHardware);

//
// usb_queue.cpp
//
NTSTATUS NTAPI CreateUSBQueue(PUSBQUEUE *OutUsbQueue);

//
// usb_request.cpp
//
NTSTATUS NTAPI InternalCreateUSBRequest(PUSBREQUEST *OutRequest);
}

#endif
