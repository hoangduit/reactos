/*
 *  FreeLoader
 *
 *  Copyright (C) 2003, 2004  Eric Kohl
 *  Copyright (C) 2009  Herv� Poussineau
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <freeldr.h>
#include <cportlib/cportlib.h>

#define NDEBUG
#include <debug.h>

#define MILLISEC     (10)
#define PRECISION    (8)

#define HZ (100)
#define CLOCK_TICK_RATE (1193182)
#define LATCH (CLOCK_TICK_RATE / HZ)


/* Maximum number of COM and LPT ports */
#define MAX_COM_PORTS   4
#define MAX_LPT_PORTS   3

/* No Mouse */
#define MOUSE_TYPE_NONE            0
/* Microsoft Mouse with 2 buttons */
#define MOUSE_TYPE_MICROSOFT       1
/* Logitech Mouse with 3 buttons */
#define MOUSE_TYPE_LOGITECH        2
/* Microsoft Wheel Mouse (aka Z Mouse) */
#define MOUSE_TYPE_WHEELZ          3
/* Mouse Systems Mouse */
#define MOUSE_TYPE_MOUSESYSTEMS    4


/* PS2 stuff */

/* Controller registers. */
#define CONTROLLER_REGISTER_STATUS                      0x64
#define CONTROLLER_REGISTER_CONTROL                     0x64
#define CONTROLLER_REGISTER_DATA                        0x60

/* Controller commands. */
#define CONTROLLER_COMMAND_READ_MODE                    0x20
#define CONTROLLER_COMMAND_WRITE_MODE                   0x60
#define CONTROLLER_COMMAND_GET_VERSION                  0xA1
#define CONTROLLER_COMMAND_MOUSE_DISABLE                0xA7
#define CONTROLLER_COMMAND_MOUSE_ENABLE                 0xA8
#define CONTROLLER_COMMAND_TEST_MOUSE                   0xA9
#define CONTROLLER_COMMAND_SELF_TEST                    0xAA
#define CONTROLLER_COMMAND_KEYBOARD_TEST                0xAB
#define CONTROLLER_COMMAND_KEYBOARD_DISABLE             0xAD
#define CONTROLLER_COMMAND_KEYBOARD_ENABLE              0xAE
#define CONTROLLER_COMMAND_WRITE_MOUSE_OUTPUT_BUFFER    0xD3
#define CONTROLLER_COMMAND_WRITE_MOUSE                  0xD4

/* Controller status */
#define CONTROLLER_STATUS_OUTPUT_BUFFER_FULL            0x01
#define CONTROLLER_STATUS_INPUT_BUFFER_FULL             0x02
#define CONTROLLER_STATUS_SELF_TEST                     0x04
#define CONTROLLER_STATUS_COMMAND                       0x08
#define CONTROLLER_STATUS_UNLOCKED                      0x10
#define CONTROLLER_STATUS_MOUSE_OUTPUT_BUFFER_FULL      0x20
#define CONTROLLER_STATUS_GENERAL_TIMEOUT               0x40
#define CONTROLLER_STATUS_PARITY_ERROR                  0x80
#define AUX_STATUS_OUTPUT_BUFFER_FULL                   (CONTROLLER_STATUS_OUTPUT_BUFFER_FULL | \
                                                         CONTROLLER_STATUS_MOUSE_OUTPUT_BUFFER_FULL)

/* Timeout in ms for sending to keyboard controller. */
#define CONTROLLER_TIMEOUT                              250

DBG_DEFAULT_CHANNEL(HWDETECT);

static unsigned int delay_count = 1;

extern UCHAR PcBiosDiskCount;

PCHAR
GetHarddiskIdentifier(
    UCHAR DriveNumber);

BOOLEAN
HwInitializeBiosDisks(VOID);

/* FUNCTIONS ****************************************************************/


static
VOID
__StallExecutionProcessor(ULONG Loops)
{
    register volatile unsigned int i;
    for (i = 0; i < Loops; i++);
}


VOID StallExecutionProcessor(ULONG Microseconds)
{
    ULONGLONG LoopCount = ((ULONGLONG)delay_count * (ULONGLONG)Microseconds) / 1000ULL;
    __StallExecutionProcessor((ULONG)LoopCount);
}


static
ULONG
Read8254Timer(VOID)
{
    ULONG Count;

    WRITE_PORT_UCHAR((PUCHAR)0x43, 0x00);
    Count = READ_PORT_UCHAR((PUCHAR)0x40);
    Count |= READ_PORT_UCHAR((PUCHAR)0x40) << 8;

    return Count;
}


static
VOID
WaitFor8254Wraparound(VOID)
{
    ULONG CurCount;
    ULONG PrevCount = ~0;
    LONG Delta;

    CurCount = Read8254Timer();

    do
    {
        PrevCount = CurCount;
        CurCount = Read8254Timer();
        Delta = CurCount - PrevCount;

        /*
         * This limit for delta seems arbitrary, but it isn't, it's
         * slightly above the level of error a buggy Mercury/Neptune
         * chipset timer can cause.
         */
    }
    while (Delta < 300);
}


VOID
HalpCalibrateStallExecution(VOID)
{
    ULONG i;
    ULONG calib_bit;
    ULONG CurCount;

    /* Initialise timer interrupt with MILLISECOND ms interval        */
    WRITE_PORT_UCHAR((PUCHAR)0x43, 0x34);  /* binary, mode 2, LSB/MSB, ch 0 */
    WRITE_PORT_UCHAR((PUCHAR)0x40, LATCH & 0xff); /* LSB */
    WRITE_PORT_UCHAR((PUCHAR)0x40, LATCH >> 8); /* MSB */

    /* Stage 1:  Coarse calibration                                   */

    WaitFor8254Wraparound();

    delay_count = 1;

    do
    {
        /* Next delay count to try */
        delay_count <<= 1;

        WaitFor8254Wraparound();

        /* Do the delay */
        __StallExecutionProcessor(delay_count);

        CurCount = Read8254Timer();
    }
    while (CurCount > LATCH / 2);

    /* Get bottom value for delay */
    delay_count >>= 1;

    /* Stage 2:  Fine calibration                                     */

    /* Which bit are we going to test */
    calib_bit = delay_count;

    for (i = 0; i < PRECISION; i++)
    {
        /* Next bit to calibrate */
        calib_bit >>= 1;

        /* If we have done all bits, stop */
        if (!calib_bit) break;

        /* Set the bit in delay_count */
        delay_count |= calib_bit;

        WaitFor8254Wraparound();

        /* Do the delay */
        __StallExecutionProcessor(delay_count);

        CurCount = Read8254Timer();
        /* If a tick has passed, turn the calibrated bit back off */
        if (CurCount <= LATCH / 2)
            delay_count &= ~calib_bit;
    }

    /* We're finished:  Do the finishing touches */

    /* Calculate delay_count for 1ms */
    delay_count /= (MILLISEC / 2);
}

static
VOID
DetectPnpBios(PCONFIGURATION_COMPONENT_DATA SystemKey, ULONG *BusNumber)
{
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PCM_PNP_BIOS_DEVICE_NODE DeviceNode;
    PCM_PNP_BIOS_INSTALLATION_CHECK InstData;
    PCONFIGURATION_COMPONENT_DATA BusKey;
    ULONG x;
    ULONG NodeSize = 0;
    ULONG NodeCount = 0;
    UCHAR NodeNumber;
    ULONG FoundNodeCount;
    int i;
    ULONG PnpBufferSize;
    ULONG Size;
    char *Ptr;

    InstData = (PCM_PNP_BIOS_INSTALLATION_CHECK)PnpBiosSupported();
    if (InstData == NULL || strncmp((CHAR*)InstData->Signature, "$PnP", 4))
    {
        TRACE("PnP-BIOS not supported\n");
        return;
    }

    TRACE("PnP-BIOS supported\n");
    TRACE("Signature '%c%c%c%c'\n",
          InstData->Signature[0], InstData->Signature[1],
          InstData->Signature[2], InstData->Signature[3]);

    x = PnpBiosGetDeviceNodeCount(&NodeSize, &NodeCount);
    if (x == 0x82)
    {
        TRACE("PnP-BIOS function 'Get Number of System Device Nodes' not supported\n");
        return;
    }

    NodeCount &= 0xFF; // needed since some fscked up BIOSes return
    // wrong info (e.g. Mac Virtual PC)
    // e.g. look: http://my.execpc.com/~geezer/osd/pnp/pnp16.c
    if (x != 0 || NodeSize == 0 || NodeCount == 0)
    {
        ERR("PnP-BIOS failed to enumerate device nodes\n");
        return;
    }
    TRACE("MaxNodeSize %u  NodeCount %u\n", NodeSize, NodeCount);
    TRACE("Estimated buffer size %u\n", NodeSize * NodeCount);

    /* Set 'Configuration Data' value */
    Size = sizeof(CM_PARTIAL_RESOURCE_LIST)
           + sizeof(CM_PNP_BIOS_INSTALLATION_CHECK) + (NodeSize * NodeCount);
    PartialResourceList = FrLdrHeapAlloc(Size, TAG_HW_RESOURCE_LIST);
    if (PartialResourceList == NULL)
    {
        ERR("Failed to allocate resource descriptor\n");
        return;
    }
    memset(PartialResourceList, 0, Size);

    /* Initialize resource descriptor */
    PartialResourceList->Version = 1;
    PartialResourceList->Revision = 1;
    PartialResourceList->Count = 1;
    PartialResourceList->PartialDescriptors[0].Type =
        CmResourceTypeDeviceSpecific;
    PartialResourceList->PartialDescriptors[0].ShareDisposition =
        CmResourceShareUndetermined;

    /* The buffer starts after PartialResourceList->PartialDescriptors[0] */
    Ptr = (char *)(PartialResourceList + 1);

    /* Set instalation check data */
    memcpy (Ptr, InstData, sizeof(CM_PNP_BIOS_INSTALLATION_CHECK));
    Ptr += sizeof(CM_PNP_BIOS_INSTALLATION_CHECK);

    /* Copy device nodes */
    FoundNodeCount = 0;
    PnpBufferSize = sizeof(CM_PNP_BIOS_INSTALLATION_CHECK);
    for (i = 0; i < 0xFF; i++)
    {
        NodeNumber = (UCHAR)i;

        x = PnpBiosGetDeviceNode(&NodeNumber, (PVOID)DISKREADBUFFER);
        if (x == 0)
        {
            DeviceNode = (PCM_PNP_BIOS_DEVICE_NODE)DISKREADBUFFER;

            TRACE("Node: %u  Size %u (0x%x)\n",
                  DeviceNode->Node,
                  DeviceNode->Size,
                  DeviceNode->Size);

            if (PnpBufferSize + DeviceNode->Size > Size)
            {
                ERR("Buffer too small!\n");
                break;
            }

            memcpy(Ptr, DeviceNode, DeviceNode->Size);

            Ptr += DeviceNode->Size;
            PnpBufferSize += DeviceNode->Size;

            FoundNodeCount++;
            if (FoundNodeCount >= NodeCount)
                break;
        }
    }

    /* Set real data size */
    PartialResourceList->PartialDescriptors[0].u.DeviceSpecificData.DataSize =
        PnpBufferSize;
    Size = sizeof(CM_PARTIAL_RESOURCE_LIST) + PnpBufferSize;

    TRACE("Real buffer size: %u\n", PnpBufferSize);
    TRACE("Resource size: %u\n", Size);

    /* Create component key */
    FldrCreateComponentKey(SystemKey,
                           AdapterClass,
                           MultiFunctionAdapter,
                           0x0,
                           0x0,
                           0xFFFFFFFF,
                           "PNP BIOS",
                           PartialResourceList,
                           Size,
                           &BusKey);

    (*BusNumber)++;
}



static PCM_PARTIAL_RESOURCE_LIST
GetHarddiskConfigurationData(UCHAR DriveNumber, ULONG* pSize)
{
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PCM_DISK_GEOMETRY_DEVICE_DATA DiskGeometry;
    EXTENDED_GEOMETRY ExtGeometry;
    GEOMETRY Geometry;
    ULONG Size;

    //
    // Initialize returned size
    //
    *pSize = 0;

    /* Set 'Configuration Data' value */
    Size = sizeof(CM_PARTIAL_RESOURCE_LIST) +
           sizeof(CM_DISK_GEOMETRY_DEVICE_DATA);
    PartialResourceList = FrLdrHeapAlloc(Size, TAG_HW_RESOURCE_LIST);
    if (PartialResourceList == NULL)
    {
        ERR("Failed to allocate a full resource descriptor\n");
        return NULL;
    }

    memset(PartialResourceList, 0, Size);
    PartialResourceList->Version = 1;
    PartialResourceList->Revision = 1;
    PartialResourceList->Count = 1;
    PartialResourceList->PartialDescriptors[0].Type =
        CmResourceTypeDeviceSpecific;
//  PartialResourceList->PartialDescriptors[0].ShareDisposition =
//  PartialResourceList->PartialDescriptors[0].Flags =
    PartialResourceList->PartialDescriptors[0].u.DeviceSpecificData.DataSize =
        sizeof(CM_DISK_GEOMETRY_DEVICE_DATA);

    /* Get pointer to geometry data */
    DiskGeometry = (PVOID)(((ULONG_PTR)PartialResourceList) + sizeof(CM_PARTIAL_RESOURCE_LIST));

    /* Get the disk geometry */
    ExtGeometry.Size = sizeof(EXTENDED_GEOMETRY);
    if (DiskGetExtendedDriveParameters(DriveNumber, &ExtGeometry, ExtGeometry.Size))
    {
        DiskGeometry->BytesPerSector = ExtGeometry.BytesPerSector;
        DiskGeometry->NumberOfCylinders = ExtGeometry.Cylinders;
        DiskGeometry->SectorsPerTrack = ExtGeometry.SectorsPerTrack;
        DiskGeometry->NumberOfHeads = ExtGeometry.Heads;
    }
    else if(MachDiskGetDriveGeometry(DriveNumber, &Geometry))
    {
        DiskGeometry->BytesPerSector = Geometry.BytesPerSector;
        DiskGeometry->NumberOfCylinders = Geometry.Cylinders;
        DiskGeometry->SectorsPerTrack = Geometry.Sectors;
        DiskGeometry->NumberOfHeads = Geometry.Heads;
    }
    else
    {
        TRACE("Reading disk geometry failed\n");
        FrLdrHeapFree(PartialResourceList, TAG_HW_RESOURCE_LIST);
        return NULL;
    }
    TRACE("Disk %x: %u Cylinders  %u Heads  %u Sectors  %u Bytes\n",
          DriveNumber,
          DiskGeometry->NumberOfCylinders,
          DiskGeometry->NumberOfHeads,
          DiskGeometry->SectorsPerTrack,
          DiskGeometry->BytesPerSector);

    //
    // Return configuration data
    //
    *pSize = Size;
    return PartialResourceList;
}

static
UCHAR
GetFloppyCount(VOID)
{
    UCHAR Data;

    WRITE_PORT_UCHAR((PUCHAR)0x70, 0x10);
    Data = READ_PORT_UCHAR((PUCHAR)0x71);

    return ((Data & 0xF0) ? 1 : 0) + ((Data & 0x0F) ? 1 : 0);
}


static
UCHAR
GetFloppyType(UCHAR DriveNumber)
{
    UCHAR Data;

    WRITE_PORT_UCHAR((PUCHAR)0x70, 0x10);
    Data = READ_PORT_UCHAR((PUCHAR)0x71);

    if (DriveNumber == 0)
        return Data >> 4;
    else if (DriveNumber == 1)
        return Data & 0x0F;

    return 0;
}


static
PVOID
GetInt1eTable(VOID)
{
    PUSHORT SegPtr = (PUSHORT)0x7A;
    PUSHORT OfsPtr = (PUSHORT)0x78;

    return (PVOID)((ULONG_PTR)(((ULONG)(*SegPtr)) << 4) + (ULONG)(*OfsPtr));
}


static
VOID
DetectBiosFloppyPeripheral(PCONFIGURATION_COMPONENT_DATA ControllerKey)
{
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptor;
    PCM_FLOPPY_DEVICE_DATA FloppyData;
    CHAR Identifier[20];
    PCONFIGURATION_COMPONENT_DATA PeripheralKey;
    ULONG Size;
    UCHAR FloppyNumber;
    UCHAR FloppyType;
    ULONG MaxDensity[6] = {0, 360, 1200, 720, 1440, 2880};
    PUCHAR Ptr;

    for (FloppyNumber = 0; FloppyNumber < 2; FloppyNumber++)
    {
        FloppyType = GetFloppyType(FloppyNumber);

        if ((FloppyType > 5) || (FloppyType == 0))
            continue;

        if (!DiskResetController(FloppyNumber))
            continue;

        Ptr = GetInt1eTable();

        /* Set 'Identifier' value */
        sprintf(Identifier, "FLOPPY%d", FloppyNumber + 1);

        Size = sizeof(CM_PARTIAL_RESOURCE_LIST) +
               sizeof(CM_FLOPPY_DEVICE_DATA);
        PartialResourceList = FrLdrHeapAlloc(Size, TAG_HW_RESOURCE_LIST);
        if (PartialResourceList == NULL)
        {
            ERR("Failed to allocate resource descriptor\n");
            return;
        }

        memset(PartialResourceList, 0, Size);
        PartialResourceList->Version = 1;
        PartialResourceList->Revision = 1;
        PartialResourceList->Count = 1;

        PartialDescriptor = &PartialResourceList->PartialDescriptors[0];
        PartialDescriptor->Type = CmResourceTypeDeviceSpecific;
        PartialDescriptor->ShareDisposition = CmResourceShareUndetermined;
        PartialDescriptor->u.DeviceSpecificData.DataSize = sizeof(CM_FLOPPY_DEVICE_DATA);

        FloppyData = (PVOID)(((ULONG_PTR)PartialResourceList) + sizeof(CM_PARTIAL_RESOURCE_LIST));
        FloppyData->Version = 2;
        FloppyData->Revision = 0;
        FloppyData->MaxDensity = MaxDensity[FloppyType];
        FloppyData->MountDensity = 0;
        RtlCopyMemory(&FloppyData->StepRateHeadUnloadTime, Ptr, 11);
        FloppyData->MaximumTrackValue = (FloppyType == 1) ? 39 : 79;
        FloppyData->DataTransferRate = 0;

        FldrCreateComponentKey(ControllerKey,
                               PeripheralClass,
                               FloppyDiskPeripheral,
                               Input | Output,
                               FloppyNumber,
                               0xFFFFFFFF,
                               Identifier,
                               PartialResourceList,
                               Size,
                               &PeripheralKey);
    }
}


static
VOID
DetectBiosFloppyController(PCONFIGURATION_COMPONENT_DATA BusKey)
{
    PCONFIGURATION_COMPONENT_DATA ControllerKey;
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptor;
    ULONG Size;
    ULONG FloppyCount;

    FloppyCount = GetFloppyCount();
    TRACE("Floppy count: %u\n",
          FloppyCount);

    Size = sizeof(CM_PARTIAL_RESOURCE_LIST) +
           2 * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
    PartialResourceList = FrLdrHeapAlloc(Size, TAG_HW_RESOURCE_LIST);
    if (PartialResourceList == NULL)
    {
        ERR("Failed to allocate resource descriptor\n");
        return;
    }
    memset(PartialResourceList, 0, Size);

    /* Initialize resource descriptor */
    PartialResourceList->Version = 1;
    PartialResourceList->Revision = 1;
    PartialResourceList->Count = 3;

    /* Set IO Port */
    PartialDescriptor = &PartialResourceList->PartialDescriptors[0];
    PartialDescriptor->Type = CmResourceTypePort;
    PartialDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    PartialDescriptor->Flags = CM_RESOURCE_PORT_IO;
    PartialDescriptor->u.Port.Start.LowPart = 0x03F0;
    PartialDescriptor->u.Port.Start.HighPart = 0x0;
    PartialDescriptor->u.Port.Length = 8;

    /* Set Interrupt */
    PartialDescriptor = &PartialResourceList->PartialDescriptors[1];
    PartialDescriptor->Type = CmResourceTypeInterrupt;
    PartialDescriptor->ShareDisposition = CmResourceShareUndetermined;
    PartialDescriptor->Flags = CM_RESOURCE_INTERRUPT_LATCHED;
    PartialDescriptor->u.Interrupt.Level = 6;
    PartialDescriptor->u.Interrupt.Vector = 6;
    PartialDescriptor->u.Interrupt.Affinity = 0xFFFFFFFF;

    /* Set DMA channel */
    PartialDescriptor = &PartialResourceList->PartialDescriptors[2];
    PartialDescriptor->Type = CmResourceTypeDma;
    PartialDescriptor->ShareDisposition = CmResourceShareUndetermined;
    PartialDescriptor->Flags = 0;
    PartialDescriptor->u.Dma.Channel = 2;
    PartialDescriptor->u.Dma.Port = 0;

    /* Create floppy disk controller */
    FldrCreateComponentKey(BusKey,
                           ControllerClass,
                           DiskController,
                           Output | Input,
                           0x0,
                           0xFFFFFFFF,
                           NULL,
                           PartialResourceList,
                           Size,
                           &ControllerKey);
    TRACE("Created key: DiskController\\0\n");

    if (FloppyCount) DetectBiosFloppyPeripheral(ControllerKey);
}

static
PCONFIGURATION_COMPONENT_DATA
DetectSystem(VOID)
{
    PCONFIGURATION_COMPONENT_DATA SystemKey;
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PCM_INT13_DRIVE_PARAMETER Int13Drives;
    GEOMETRY Geometry;
    UCHAR DiskCount;
    USHORT i;
    ULONG Size;

    DiskCount = PcBiosDiskCount;

    /* Allocate resource descriptor */
    Size = sizeof(CM_PARTIAL_RESOURCE_LIST) +
           sizeof(CM_INT13_DRIVE_PARAMETER) * DiskCount;
    PartialResourceList = FrLdrHeapAlloc(Size, TAG_HW_RESOURCE_LIST);
    if (PartialResourceList == NULL)
    {
        ERR("Failed to allocate resource descriptor\n");
        return NULL;
    }

    /* Initialize resource descriptor */
    memset(PartialResourceList, 0, Size);
    PartialResourceList->Version = 1;
    PartialResourceList->Revision = 1;
    PartialResourceList->Count = 1;
    PartialResourceList->PartialDescriptors[0].Type = CmResourceTypeDeviceSpecific;
    PartialResourceList->PartialDescriptors[0].ShareDisposition = 0;
    PartialResourceList->PartialDescriptors[0].Flags = 0;
    PartialResourceList->PartialDescriptors[0].u.DeviceSpecificData.DataSize =
        sizeof(CM_INT13_DRIVE_PARAMETER) * DiskCount;

    /* Get harddisk Int13 geometry data */
    Int13Drives = (PVOID)(((ULONG_PTR)PartialResourceList) + sizeof(CM_PARTIAL_RESOURCE_LIST));
    for (i = 0; i < DiskCount; i++)
    {
        if (MachDiskGetDriveGeometry(0x80 + i, &Geometry))
        {
            Int13Drives[i].DriveSelect = 0x80 + i;
            Int13Drives[i].MaxCylinders = Geometry.Cylinders - 1;
            Int13Drives[i].SectorsPerTrack = (USHORT)Geometry.Sectors;
            Int13Drives[i].MaxHeads = (USHORT)Geometry.Heads - 1;
            Int13Drives[i].NumberDrives = DiskCount;

            TRACE("Disk %x: %u Cylinders  %u Heads  %u Sectors  %u Bytes\n",
                  0x80 + i,
                  Geometry.Cylinders - 1,
                  Geometry.Heads - 1,
                  Geometry.Sectors,
                  Geometry.BytesPerSector);
        }
    }

    FldrCreateComponentKey(NULL,
                           SystemClass,
                           MaximumType,
                           0x0,
                           0x0,
                           0xFFFFFFFF,
                           NULL,
                           PartialResourceList,
                           Size,
                           &SystemKey);

    return SystemKey;
}

static
VOID
DetectBiosDisks(PCONFIGURATION_COMPONENT_DATA BusKey)
{
    PCONFIGURATION_COMPONENT_DATA DiskKey, ControllerKey;
    ULONG i;

    FldrCreateComponentKey(BusKey,
                           ControllerClass,
                           DiskController,
                           Output | Input,
                           0x0,
                           0xFFFFFFFF,
                           NULL,
                           NULL,
                           0,
                           &ControllerKey);
    TRACE("Created key: DiskController\\0\n");

    /* Create and fill subkey for each harddisk */
    for (i = 0; i < PcBiosDiskCount; i++)
    {
        PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
        ULONG Size;
        PCHAR Identifier;
        UCHAR DriveNumber = 0x80 + (UCHAR)i;

        /* Get disk values */
        PartialResourceList = GetHarddiskConfigurationData(DriveNumber, &Size);
        Identifier = GetHarddiskIdentifier(DriveNumber);

        /* Create disk key */
        FldrCreateComponentKey(ControllerKey,
                               PeripheralClass,
                               DiskPeripheral,
                               Output | Input,
                               0x0,
                               0xFFFFFFFF,
                               Identifier,
                               PartialResourceList,
                               Size,
                               &DiskKey);
    }
}

static
VOID
InitializeSerialPort(PUCHAR Port,
                     UCHAR LineControl)
{
    WRITE_PORT_UCHAR(Port + 3, 0x80);  /* set DLAB on   */
    WRITE_PORT_UCHAR(Port,     0x60);  /* speed LO byte */
    WRITE_PORT_UCHAR(Port + 1, 0);     /* speed HI byte */
    WRITE_PORT_UCHAR(Port + 3, LineControl);
    WRITE_PORT_UCHAR(Port + 1, 0);     /* set comm and DLAB to 0 */
    WRITE_PORT_UCHAR(Port + 4, 0x09);  /* DR int enable */
    READ_PORT_UCHAR(Port + 5);  /* clear error bits */
}


static
ULONG
DetectSerialMouse(PUCHAR Port)
{
    CHAR Buffer[4];
    ULONG i;
    ULONG TimeOut;
    UCHAR LineControl;

    /* Shutdown mouse or something like that */
    LineControl = READ_PORT_UCHAR(Port + 4);
    WRITE_PORT_UCHAR(Port + 4, (LineControl & ~0x02) | 0x01);
    StallExecutionProcessor(100000);

    /*
     * Clear buffer
     * Maybe there is no serial port although BIOS reported one (this
     * is the case on Apple hardware), or the serial port is misbehaving,
     * therefore we must give up after some time.
     */
    TimeOut = 200;
    while (READ_PORT_UCHAR(Port + 5) & 0x01)
    {
        if (--TimeOut == 0)
            return MOUSE_TYPE_NONE;
        READ_PORT_UCHAR(Port);
    }

    /*
     * Send modem control with 'Data Terminal Ready', 'Request To Send' and
     * 'Output Line 2' message. This enables mouse to identify.
     */
    WRITE_PORT_UCHAR(Port + 4, 0x0b);

    /* Wait 10 milliseconds for the mouse getting ready */
    StallExecutionProcessor(10000);

    /* Read first four bytes, which contains Microsoft Mouse signs */
    TimeOut = 200;
    for (i = 0; i < 4; i++)
    {
        while (((READ_PORT_UCHAR(Port + 5) & 1) == 0) && (TimeOut > 0))
        {
            StallExecutionProcessor(1000);
            --TimeOut;
            if (TimeOut == 0)
                return MOUSE_TYPE_NONE;
        }
        Buffer[i] = READ_PORT_UCHAR(Port);
    }

    TRACE("Mouse data: %x %x %x %x\n",
          Buffer[0], Buffer[1], Buffer[2], Buffer[3]);

    /* Check that four bytes for signs */
    for (i = 0; i < 4; ++i)
    {
        if (Buffer[i] == 'B')
        {
            /* Sign for Microsoft Ballpoint */
//      DbgPrint("Microsoft Ballpoint device detected\n");
//      DbgPrint("THIS DEVICE IS NOT SUPPORTED, YET\n");
            return MOUSE_TYPE_NONE;
        }
        else if (Buffer[i] == 'M')
        {
            /* Sign for Microsoft Mouse protocol followed by button specifier */
            if (i == 3)
            {
                /* Overflow Error */
                return MOUSE_TYPE_NONE;
            }

            switch (Buffer[i + 1])
            {
            case '3':
                TRACE("Microsoft Mouse with 3-buttons detected\n");
                return MOUSE_TYPE_LOGITECH;

            case 'Z':
                TRACE("Microsoft Wheel Mouse detected\n");
                return MOUSE_TYPE_WHEELZ;

                /* case '2': */
            default:
                TRACE("Microsoft Mouse with 2-buttons detected\n");
                return MOUSE_TYPE_MICROSOFT;
            }
        }
    }

    return MOUSE_TYPE_NONE;
}


static ULONG
GetSerialMousePnpId(PUCHAR Port, char *Buffer)
{
    ULONG TimeOut;
    ULONG i = 0;
    char c;
    char x;

    WRITE_PORT_UCHAR(Port + 4, 0x09);

    /* Wait 10 milliseconds for the mouse getting ready */
    StallExecutionProcessor(10000);

    WRITE_PORT_UCHAR(Port + 4, 0x0b);

    StallExecutionProcessor(10000);

    for (;;)
    {
        TimeOut = 200;
        while (((READ_PORT_UCHAR(Port + 5) & 1) == 0) && (TimeOut > 0))
        {
            StallExecutionProcessor(1000);
            --TimeOut;
            if (TimeOut == 0)
            {
                return 0;
            }
        }

        c = READ_PORT_UCHAR(Port);
        if (c == 0x08 || c == 0x28)
            break;
    }

    Buffer[i++] = c;
    x = c + 1;

    for (;;)
    {
        TimeOut = 200;
        while (((READ_PORT_UCHAR(Port + 5) & 1) == 0) && (TimeOut > 0))
        {
            StallExecutionProcessor(1000);
            --TimeOut;
            if (TimeOut == 0)
                return 0;
        }
        c = READ_PORT_UCHAR(Port);
        Buffer[i++] = c;
        if (c == x)
            break;
        if (i >= 256)
            break;
    }

    return i;
}


static
VOID
DetectSerialPointerPeripheral(PCONFIGURATION_COMPONENT_DATA ControllerKey,
                              PUCHAR Base)
{
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    char Buffer[256];
    CHAR Identifier[256];
    PCONFIGURATION_COMPONENT_DATA PeripheralKey;
    ULONG MouseType;
    ULONG Size, Length;
    ULONG i;
    ULONG j;
    ULONG k;

    TRACE("DetectSerialPointerPeripheral()\n");

    Identifier[0] = 0;

    InitializeSerialPort(Base, 2);
    MouseType = DetectSerialMouse(Base);

    if (MouseType != MOUSE_TYPE_NONE)
    {
        Length = GetSerialMousePnpId(Base, Buffer);
        TRACE( "PnP ID length: %u\n", Length);

        if (Length != 0)
        {
            /* Convert PnP sting to ASCII */
            if (Buffer[0] == 0x08)
            {
                for (i = 0; i < Length; i++)
                    Buffer[i] += 0x20;
            }
            Buffer[Length] = 0;

            TRACE("PnP ID string: %s\n", Buffer);

            /* Copy PnpId string */
            for (i = 0; i < 7; i++)
            {
                Identifier[i] = Buffer[3 + i];
            }
            memcpy(&Identifier[7],
                   L" - ",
                   3 * sizeof(WCHAR));

            /* Skip device serial number */
            i = 10;
            if (Buffer[i] == '\\')
            {
                for (j = ++i; i < Length; ++i)
                {
                    if (Buffer[i] == '\\')
                        break;
                }
                if (i >= Length)
                    i -= 3;
            }

            /* Skip PnP class */
            if (Buffer[i] == '\\')
            {
                for (j = ++i; i < Length; ++i)
                {
                    if (Buffer[i] == '\\')
                        break;
                }

                if (i >= Length)
                    i -= 3;
            }

            /* Skip compatible PnP Id */
            if (Buffer[i] == '\\')
            {
                for (j = ++i; i < Length; ++i)
                {
                    if (Buffer[i] == '\\')
                        break;
                }
                if (Buffer[j] == '*')
                    ++j;
                if (i >= Length)
                    i -= 3;
            }

            /* Get product description */
            if (Buffer[i] == '\\')
            {
                for (j = ++i; i < Length; ++i)
                {
                    if (Buffer[i] == ';')
                        break;
                }
                if (i >= Length)
                    i -= 3;
                if (i > j + 1)
                {
                    for (k = 0; k < i - j; k++)
                    {
                        Identifier[k + 10] = Buffer[k + j];
                    }
                    Identifier[10 + (i - j)] = 0;
                }
            }

            TRACE("Identifier string: %s\n", Identifier);
        }

        if (Length == 0 || strlen(Identifier) < 11)
        {
            switch (MouseType)
            {
            case MOUSE_TYPE_LOGITECH:
                strcpy(Identifier, "LOGITECH SERIAL MOUSE");
                break;

            case MOUSE_TYPE_WHEELZ:
                strcpy(Identifier, "MICROSOFT SERIAL MOUSE WITH WHEEL");
                break;

            case MOUSE_TYPE_MICROSOFT:
            default:
                strcpy(Identifier, "MICROSOFT SERIAL MOUSE");
                break;
            }
        }

        /* Set 'Configuration Data' value */
        Size = sizeof(CM_PARTIAL_RESOURCE_LIST) -
               sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
        PartialResourceList = FrLdrHeapAlloc(Size, TAG_HW_RESOURCE_LIST);
        memset(PartialResourceList, 0, Size);
        PartialResourceList->Version = 1;
        PartialResourceList->Revision = 1;
        PartialResourceList->Count = 0;

        /* Create 'PointerPeripheral' key */
        FldrCreateComponentKey(ControllerKey,
                               PeripheralClass,
                               PointerPeripheral,
                               Input,
                               0x0,
                               0xFFFFFFFF,
                               Identifier,
                               PartialResourceList,
                               Size,
                               &PeripheralKey);

        TRACE("Created key: PointerPeripheral\\0\n");
    }
}


static
VOID
DetectSerialPorts(PCONFIGURATION_COMPONENT_DATA BusKey)
{
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptor;
    PCM_SERIAL_DEVICE_DATA SerialDeviceData;
    ULONG Irq[MAX_COM_PORTS] = {4, 3, 4, 3};
    ULONG Base;
    CHAR Buffer[80];
    PUSHORT BasePtr;
    ULONG ControllerNumber = 0;
    PCONFIGURATION_COMPONENT_DATA ControllerKey;
    ULONG i;
    ULONG Size;

    TRACE("DetectSerialPorts()\n");

    /*
     * The BIOS data area 0x400 holds the address of the first valid COM port.
     * Each COM port address is stored in a 2-byte field.
     * Infos at: http://www.bioscentral.com/misc/bda.htm
     */
    BasePtr = (PUSHORT)0x400;

    for (i = 0; i < MAX_COM_PORTS; i++, BasePtr++)
    {
        Base = (ULONG) * BasePtr;
        if (Base == 0 || !CpDoesPortExist((PUCHAR)Base))
            continue;

        TRACE("Found COM%u port at 0x%x\n", i + 1, Base);

        /* Set 'Identifier' value */
        sprintf(Buffer, "COM%ld", i + 1);

        /* Build full device descriptor */
        Size = sizeof(CM_PARTIAL_RESOURCE_LIST) +
               2 * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) +
               sizeof(CM_SERIAL_DEVICE_DATA);
        PartialResourceList = FrLdrHeapAlloc(Size, TAG_HW_RESOURCE_LIST);
        if (PartialResourceList == NULL)
        {
            ERR("Failed to allocate resource descriptor\n");
            continue;
        }
        memset(PartialResourceList, 0, Size);

        /* Initialize resource descriptor */
        PartialResourceList->Version = 1;
        PartialResourceList->Revision = 1;
        PartialResourceList->Count = 3;

        /* Set IO Port */
        PartialDescriptor = &PartialResourceList->PartialDescriptors[0];
        PartialDescriptor->Type = CmResourceTypePort;
        PartialDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
        PartialDescriptor->Flags = CM_RESOURCE_PORT_IO;
        PartialDescriptor->u.Port.Start.LowPart = Base;
        PartialDescriptor->u.Port.Start.HighPart = 0x0;
        PartialDescriptor->u.Port.Length = 7;

        /* Set Interrupt */
        PartialDescriptor = &PartialResourceList->PartialDescriptors[1];
        PartialDescriptor->Type = CmResourceTypeInterrupt;
        PartialDescriptor->ShareDisposition = CmResourceShareUndetermined;
        PartialDescriptor->Flags = CM_RESOURCE_INTERRUPT_LATCHED;
        PartialDescriptor->u.Interrupt.Level = Irq[i];
        PartialDescriptor->u.Interrupt.Vector = Irq[i];
        PartialDescriptor->u.Interrupt.Affinity = 0xFFFFFFFF;

        /* Set serial data (device specific) */
        PartialDescriptor = &PartialResourceList->PartialDescriptors[2];
        PartialDescriptor->Type = CmResourceTypeDeviceSpecific;
        PartialDescriptor->ShareDisposition = CmResourceShareUndetermined;
        PartialDescriptor->Flags = 0;
        PartialDescriptor->u.DeviceSpecificData.DataSize = sizeof(CM_SERIAL_DEVICE_DATA);

        SerialDeviceData =
            (PCM_SERIAL_DEVICE_DATA)&PartialResourceList->PartialDescriptors[3];
        SerialDeviceData->BaudClock = 1843200; /* UART Clock frequency (Hertz) */

        /* Create controller key */
        FldrCreateComponentKey(BusKey,
                               ControllerClass,
                               SerialController,
                               Output | Input | ConsoleIn | ConsoleOut,
                               ControllerNumber,
                               0xFFFFFFFF,
                               Buffer,
                               PartialResourceList,
                               Size,
                               &ControllerKey);

        if (!Rs232PortInUse(UlongToPtr(Base)))
        {
            /* Detect serial mouse */
            DetectSerialPointerPeripheral(ControllerKey, UlongToPtr(Base));
        }

        ControllerNumber++;
    }
}


static VOID
DetectParallelPorts(PCONFIGURATION_COMPONENT_DATA BusKey)
{
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptor;
    ULONG Irq[MAX_LPT_PORTS] = {7, 5, (ULONG) - 1};
    CHAR Buffer[80];
    PCONFIGURATION_COMPONENT_DATA ControllerKey;
    PUSHORT BasePtr;
    ULONG Base;
    ULONG ControllerNumber = 0;
    ULONG i;
    ULONG Size;

    TRACE("DetectParallelPorts() called\n");

    /*
     * The BIOS data area 0x408 holds the address of the first valid LPT port.
     * Each LPT port address is stored in a 2-byte field.
     * Infos at: http://www.bioscentral.com/misc/bda.htm
     */
    BasePtr = (PUSHORT)0x408;

    for (i = 0; i < MAX_LPT_PORTS; i++, BasePtr++)
    {
        Base = (ULONG) * BasePtr;
        if (Base == 0)
            continue;

        TRACE("Parallel port %u: %x\n", ControllerNumber, Base);

        /* Set 'Identifier' value */
        sprintf(Buffer, "PARALLEL%ld", i + 1);

        /* Build full device descriptor */
        Size = sizeof(CM_PARTIAL_RESOURCE_LIST);
        if (Irq[i] != (ULONG) - 1)
            Size += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

        PartialResourceList = FrLdrHeapAlloc(Size, TAG_HW_RESOURCE_LIST);
        if (PartialResourceList == NULL)
        {
            ERR("Failed to allocate resource descriptor\n");
            continue;
        }
        memset(PartialResourceList, 0, Size);

        /* Initialize resource descriptor */
        PartialResourceList->Version = 1;
        PartialResourceList->Revision = 1;
        PartialResourceList->Count = (Irq[i] != (ULONG) - 1) ? 2 : 1;

        /* Set IO Port */
        PartialDescriptor = &PartialResourceList->PartialDescriptors[0];
        PartialDescriptor->Type = CmResourceTypePort;
        PartialDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
        PartialDescriptor->Flags = CM_RESOURCE_PORT_IO;
        PartialDescriptor->u.Port.Start.LowPart = Base;
        PartialDescriptor->u.Port.Start.HighPart = 0x0;
        PartialDescriptor->u.Port.Length = 3;

        /* Set Interrupt */
        if (Irq[i] != (ULONG) - 1)
        {
            PartialDescriptor = &PartialResourceList->PartialDescriptors[1];
            PartialDescriptor->Type = CmResourceTypeInterrupt;
            PartialDescriptor->ShareDisposition = CmResourceShareUndetermined;
            PartialDescriptor->Flags = CM_RESOURCE_INTERRUPT_LATCHED;
            PartialDescriptor->u.Interrupt.Level = Irq[i];
            PartialDescriptor->u.Interrupt.Vector = Irq[i];
            PartialDescriptor->u.Interrupt.Affinity = 0xFFFFFFFF;
        }

        /* Create controller key */
        FldrCreateComponentKey(BusKey,
                               ControllerClass,
                               ParallelController,
                               Output,
                               ControllerNumber,
                               0xFFFFFFFF,
                               Buffer,
                               PartialResourceList,
                               Size,
                               &ControllerKey);

        ControllerNumber++;
    }

    TRACE("DetectParallelPorts() done\n");
}


//static
BOOLEAN
DetectKeyboardDevice(VOID)
{
    UCHAR Status;
    UCHAR Scancode;
    ULONG Loops;
    BOOLEAN Result = TRUE;

    /* Identify device */
    WRITE_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_DATA, 0xF2);

    /* Wait for reply */
    for (Loops = 0; Loops < 100; Loops++)
    {
        StallExecutionProcessor(10000);
        Status = READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_STATUS);
        if ((Status & CONTROLLER_STATUS_OUTPUT_BUFFER_FULL) != 0)
            break;
    }

    if ((Status & CONTROLLER_STATUS_OUTPUT_BUFFER_FULL) == 0)
    {
        /* PC/XT keyboard or no keyboard */
        Result = FALSE;
    }

    Scancode = READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_DATA);
    if (Scancode != 0xFA)
    {
        /* No ACK received */
        Result = FALSE;
    }

    StallExecutionProcessor(10000);

    Status = READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_STATUS);
    if ((Status & CONTROLLER_STATUS_OUTPUT_BUFFER_FULL) == 0)
    {
        /* Found AT keyboard */
        return Result;
    }

    Scancode = READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_DATA);
    if (Scancode != 0xAB)
    {
        /* No 0xAB received */
        Result = FALSE;
    }

    StallExecutionProcessor(10000);

    Status = READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_STATUS);
    if ((Status & CONTROLLER_STATUS_OUTPUT_BUFFER_FULL) == 0)
    {
        /* No byte in buffer */
        Result = FALSE;
    }

    Scancode = READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_DATA);
    if (Scancode != 0x41)
    {
        /* No 0x41 received */
        Result = FALSE;
    }

    /* Found MF-II keyboard */
    return Result;
}


static VOID
DetectKeyboardPeripheral(PCONFIGURATION_COMPONENT_DATA ControllerKey)
{
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptor;
    PCM_KEYBOARD_DEVICE_DATA KeyboardData;
    PCONFIGURATION_COMPONENT_DATA PeripheralKey;
    ULONG Size;

    /* HACK: don't call DetectKeyboardDevice() as it fails in Qemu 0.8.2
    if (DetectKeyboardDevice()) */
    {
        /* Set 'Configuration Data' value */
        Size = sizeof(CM_PARTIAL_RESOURCE_LIST) +
               sizeof(CM_KEYBOARD_DEVICE_DATA);
        PartialResourceList = FrLdrHeapAlloc(Size, TAG_HW_RESOURCE_LIST);
        if (PartialResourceList == NULL)
        {
            ERR("Failed to allocate resource descriptor\n");
            return;
        }

        /* Initialize resource descriptor */
        memset(PartialResourceList, 0, Size);
        PartialResourceList->Version = 1;
        PartialResourceList->Revision = 1;
        PartialResourceList->Count = 1;

        PartialDescriptor = &PartialResourceList->PartialDescriptors[0];
        PartialDescriptor->Type = CmResourceTypeDeviceSpecific;
        PartialDescriptor->ShareDisposition = CmResourceShareUndetermined;
        PartialDescriptor->u.DeviceSpecificData.DataSize = sizeof(CM_KEYBOARD_DEVICE_DATA);

        KeyboardData = (PCM_KEYBOARD_DEVICE_DATA)(PartialDescriptor + 1);
        KeyboardData->Version = 1;
        KeyboardData->Revision = 1;
        KeyboardData->Type = 4;
        KeyboardData->Subtype = 0;
        KeyboardData->KeyboardFlags = 0x20;

        /* Create controller key */
        FldrCreateComponentKey(ControllerKey,
                               PeripheralClass,
                               KeyboardPeripheral,
                               Input | ConsoleIn,
                               0x0,
                               0xFFFFFFFF,
                               "PCAT_ENHANCED",
                               PartialResourceList,
                               Size,
                               &PeripheralKey);
        TRACE("Created key: KeyboardPeripheral\\0\n");
    }
}


static
VOID
DetectKeyboardController(PCONFIGURATION_COMPONENT_DATA BusKey)
{
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptor;
    PCONFIGURATION_COMPONENT_DATA ControllerKey;
    ULONG Size;

    /* Set 'Configuration Data' value */
    Size = sizeof(CM_PARTIAL_RESOURCE_LIST) +
           2 * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
    PartialResourceList = FrLdrHeapAlloc(Size, TAG_HW_RESOURCE_LIST);
    if (PartialResourceList == NULL)
    {
        ERR("Failed to allocate resource descriptor\n");
        return;
    }

    /* Initialize resource descriptor */
    memset(PartialResourceList, 0, Size);
    PartialResourceList->Version = 1;
    PartialResourceList->Revision = 1;
    PartialResourceList->Count = 3;

    /* Set Interrupt */
    PartialDescriptor = &PartialResourceList->PartialDescriptors[0];
    PartialDescriptor->Type = CmResourceTypeInterrupt;
    PartialDescriptor->ShareDisposition = CmResourceShareUndetermined;
    PartialDescriptor->Flags = CM_RESOURCE_INTERRUPT_LATCHED;
    PartialDescriptor->u.Interrupt.Level = 1;
    PartialDescriptor->u.Interrupt.Vector = 1;
    PartialDescriptor->u.Interrupt.Affinity = 0xFFFFFFFF;

    /* Set IO Port 0x60 */
    PartialDescriptor = &PartialResourceList->PartialDescriptors[1];
    PartialDescriptor->Type = CmResourceTypePort;
    PartialDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    PartialDescriptor->Flags = CM_RESOURCE_PORT_IO;
    PartialDescriptor->u.Port.Start.LowPart = 0x60;
    PartialDescriptor->u.Port.Start.HighPart = 0x0;
    PartialDescriptor->u.Port.Length = 1;

    /* Set IO Port 0x64 */
    PartialDescriptor = &PartialResourceList->PartialDescriptors[2];
    PartialDescriptor->Type = CmResourceTypePort;
    PartialDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    PartialDescriptor->Flags = CM_RESOURCE_PORT_IO;
    PartialDescriptor->u.Port.Start.LowPart = 0x64;
    PartialDescriptor->u.Port.Start.HighPart = 0x0;
    PartialDescriptor->u.Port.Length = 1;

    /* Create controller key */
    FldrCreateComponentKey(BusKey,
                           ControllerClass,
                           KeyboardController,
                           Input | ConsoleIn,
                           0x0,
                           0xFFFFFFFF,
                           NULL,
                           PartialResourceList,
                           Size,
                           &ControllerKey);
    TRACE("Created key: KeyboardController\\0\n");

    DetectKeyboardPeripheral(ControllerKey);
}


static
VOID
PS2ControllerWait(VOID)
{
    ULONG Timeout;
    UCHAR Status;

    for (Timeout = 0; Timeout < CONTROLLER_TIMEOUT; Timeout++)
    {
        Status = READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_STATUS);
        if ((Status & CONTROLLER_STATUS_INPUT_BUFFER_FULL) == 0)
            return;

        /* Sleep for one millisecond */
        StallExecutionProcessor(1000);
    }
}


static
BOOLEAN
DetectPS2AuxPort(VOID)
{
#if 1
    /* Current detection is too unreliable. Just do as if
     * the PS/2 aux port is always present
     */
    return TRUE;
#else
    ULONG Loops;
    UCHAR Status;

    /* Put the value 0x5A in the output buffer using the
     * "WriteAuxiliary Device Output Buffer" command (0xD3).
     * Poll the Status Register for a while to see if the value really turns up
     * in the Data Register. If the KEYBOARD_STATUS_MOUSE_OBF bit is also set
     * to 1 in the Status Register, we assume this controller has an
     *  Auxiliary Port (a.k.a. Mouse Port).
     */
    PS2ControllerWait();
    WRITE_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_CONTROL,
                     CONTROLLER_COMMAND_WRITE_MOUSE_OUTPUT_BUFFER);
    PS2ControllerWait();

    /* 0x5A is a random dummy value */
    WRITE_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_DATA,
                     0x5A);

    for (Loops = 0; Loops < 10; Loops++)
    {
        StallExecutionProcessor(10000);
        Status = READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_STATUS);
        if ((Status & CONTROLLER_STATUS_OUTPUT_BUFFER_FULL) != 0)
            break;
    }

    READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_DATA);

    return (Status & CONTROLLER_STATUS_MOUSE_OUTPUT_BUFFER_FULL);
#endif
}


static
BOOLEAN
DetectPS2AuxDevice(VOID)
{
    UCHAR Scancode;
    UCHAR Status;
    ULONG Loops;
    BOOLEAN Result = TRUE;

    PS2ControllerWait();
    WRITE_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_CONTROL,
                     CONTROLLER_COMMAND_WRITE_MOUSE);
    PS2ControllerWait();

    /* Identify device */
    WRITE_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_DATA, 0xF2);

    /* Wait for reply */
    for (Loops = 0; Loops < 100; Loops++)
    {
        StallExecutionProcessor(10000);
        Status = READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_STATUS);
        if ((Status & CONTROLLER_STATUS_OUTPUT_BUFFER_FULL) != 0)
            break;
    }

    Status = READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_STATUS);
    if ((Status & CONTROLLER_STATUS_MOUSE_OUTPUT_BUFFER_FULL) == 0)
        Result = FALSE;

    Scancode = READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_DATA);
    if (Scancode != 0xFA)
        Result = FALSE;

    StallExecutionProcessor(10000);

    Status = READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_STATUS);
    if ((Status & CONTROLLER_STATUS_MOUSE_OUTPUT_BUFFER_FULL) == 0)
        Result = FALSE;

    Scancode = READ_PORT_UCHAR((PUCHAR)CONTROLLER_REGISTER_DATA);
    if (Scancode != 0x00)
        Result = FALSE;

    return Result;
}


static
VOID
DetectPS2Mouse(PCONFIGURATION_COMPONENT_DATA BusKey)
{
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PCONFIGURATION_COMPONENT_DATA ControllerKey;
    PCONFIGURATION_COMPONENT_DATA PeripheralKey;
    ULONG Size;

    if (DetectPS2AuxPort())
    {
        TRACE("Detected PS2 port\n");

        PartialResourceList = FrLdrHeapAlloc(sizeof(CM_PARTIAL_RESOURCE_LIST), TAG_HW_RESOURCE_LIST);
        memset(PartialResourceList, 0, sizeof(CM_PARTIAL_RESOURCE_LIST));

        /* Initialize resource descriptor */
        PartialResourceList->Version = 1;
        PartialResourceList->Revision = 1;
        PartialResourceList->Count = 1;

        /* Set Interrupt */
        PartialResourceList->PartialDescriptors[0].Type = CmResourceTypeInterrupt;
        PartialResourceList->PartialDescriptors[0].ShareDisposition = CmResourceShareUndetermined;
        PartialResourceList->PartialDescriptors[0].Flags = CM_RESOURCE_INTERRUPT_LATCHED;
        PartialResourceList->PartialDescriptors[0].u.Interrupt.Level = 12;
        PartialResourceList->PartialDescriptors[0].u.Interrupt.Vector = 12;
        PartialResourceList->PartialDescriptors[0].u.Interrupt.Affinity = 0xFFFFFFFF;

        /* Create controller key */
        FldrCreateComponentKey(BusKey,
                               ControllerClass,
                               PointerController,
                               Input,
                               0x0,
                               0xFFFFFFFF,
                               NULL,
                               PartialResourceList,
                               sizeof(CM_PARTIAL_RESOURCE_LIST),
                               &ControllerKey);
        TRACE("Created key: PointerController\\0\n");

        if (DetectPS2AuxDevice())
        {
            TRACE("Detected PS2 mouse\n");

            /* Initialize resource descriptor */
            Size = sizeof(CM_PARTIAL_RESOURCE_LIST) -
                   sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
            PartialResourceList = FrLdrHeapAlloc(Size, TAG_HW_RESOURCE_LIST);
            memset(PartialResourceList, 0, Size);
            PartialResourceList->Version = 1;
            PartialResourceList->Revision = 1;
            PartialResourceList->Count = 0;

            /* Create peripheral key */
            FldrCreateComponentKey(ControllerKey,
                                   ControllerClass,
                                   PointerPeripheral,
                                   Input,
                                   0x0,
                                   0xFFFFFFFF,
                                   "MICROSOFT PS2 MOUSE",
                                   PartialResourceList,
                                   Size,
                                   &PeripheralKey);
            TRACE("Created key: PointerPeripheral\\0\n");
        }
    }
}


static VOID
DetectDisplayController(PCONFIGURATION_COMPONENT_DATA BusKey)
{
    CHAR Buffer[80];
    PCONFIGURATION_COMPONENT_DATA ControllerKey;
    USHORT VesaVersion;

    /* FIXME: Set 'ComponentInformation' value */

    VesaVersion = BiosIsVesaSupported();
    if (VesaVersion != 0)
    {
        TRACE("VESA version %c.%c\n",
              (VesaVersion >> 8) + '0',
              (VesaVersion & 0xFF) + '0');
    }
    else
    {
        TRACE("VESA not supported\n");
    }

    if (VesaVersion >= 0x0200)
    {
        strcpy(Buffer, "VBE Display");
    }
    else
    {
        strcpy(Buffer, "VGA Display");
    }

    FldrCreateComponentKey(BusKey,
                           ControllerClass,
                           DisplayController,
                           0x0,
                           0x0,
                           0xFFFFFFFF,
                           Buffer,
                           NULL,
                           0,
                           &ControllerKey);
    TRACE("Created key: DisplayController\\0\n");

    /* FIXME: Add display peripheral (monitor) data */
}


static
VOID
DetectIsaBios(PCONFIGURATION_COMPONENT_DATA SystemKey, ULONG *BusNumber)
{
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PCONFIGURATION_COMPONENT_DATA BusKey;
    ULONG Size;

    /* Increment bus number */
    (*BusNumber)++;

    /* Set 'Configuration Data' value */
    Size = sizeof(CM_PARTIAL_RESOURCE_LIST) -
           sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
    PartialResourceList = FrLdrHeapAlloc(Size, TAG_HW_RESOURCE_LIST);
    if (PartialResourceList == NULL)
    {
        ERR("Failed to allocate resource descriptor\n");
        return;
    }

    /* Initialize resource descriptor */
    memset(PartialResourceList, 0, Size);
    PartialResourceList->Version = 1;
    PartialResourceList->Revision = 1;
    PartialResourceList->Count = 0;

    /* Create new bus key */
    FldrCreateComponentKey(SystemKey,
                           AdapterClass,
                           MultiFunctionAdapter,
                           0x0,
                           0x0,
                           0xFFFFFFFF,
                           "ISA",
                           PartialResourceList,
                           Size,
                           &BusKey);

    /* Detect ISA/BIOS devices */
    DetectBiosDisks(BusKey);

    DetectBiosFloppyController(BusKey);

    DetectSerialPorts(BusKey);

    DetectParallelPorts(BusKey);

    DetectKeyboardController(BusKey);

    DetectPS2Mouse(BusKey);

    DetectDisplayController(BusKey);

    /* FIXME: Detect more ISA devices */
}

BOOLEAN
PcInitializeBootDevices(VOID)
{
    return HwInitializeBiosDisks();
}

PCONFIGURATION_COMPONENT_DATA
PcHwDetect(VOID)
{
    PCONFIGURATION_COMPONENT_DATA SystemKey;
    ULONG BusNumber = 0;

    TRACE("DetectHardware()\n");

    /* Create the 'System' key */
    SystemKey = DetectSystem();

    /* Detect buses */
    DetectPciBios(SystemKey, &BusNumber);
    DetectApmBios(SystemKey, &BusNumber);
    DetectPnpBios(SystemKey, &BusNumber);
    DetectIsaBios(SystemKey, &BusNumber);
    DetectAcpiBios(SystemKey, &BusNumber);

    TRACE("DetectHardware() Done\n");

    return SystemKey;
}

VOID
PcHwIdle(VOID)
{
    REGS Regs;

    /* Select APM 1.0+ function */
    Regs.b.ah = 0x53;

    /* Function 05h: CPU idle */
    Regs.b.al = 0x05;

    /* Call INT 15h */
    Int386(0x15, &Regs, &Regs);

    /* Check if successfull (CF set on error) */
    if (INT386_SUCCESS(Regs))
        return;

    /*
     * No futher processing here.
     * Optionally implement HLT instruction handling.
     */
}

/* EOF */
