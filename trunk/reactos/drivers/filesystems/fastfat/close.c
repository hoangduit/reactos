/*
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          ReactOS kernel
 * FILE:             drivers/fs/vfat/close.c
 * PURPOSE:          VFAT Filesystem
 * PROGRAMMER:       Jason Filby (jasonfilby@yahoo.com)
 */

/* INCLUDES *****************************************************************/

#include "vfat.h"

#define NDEBUG
#include <debug.h>

/* FUNCTIONS ****************************************************************/

/*
 * FUNCTION: Closes a file
 */
NTSTATUS
VfatCloseFile(
    PDEVICE_EXTENSION DeviceExt,
    PFILE_OBJECT FileObject)
{
    PVFATFCB pFcb;
    PVFATCCB pCcb;
    NTSTATUS Status = STATUS_SUCCESS;

    DPRINT("VfatCloseFile(DeviceExt %p, FileObject %p)\n",
            DeviceExt, FileObject);

    /* FIXME : update entry in directory? */
    pCcb = (PVFATCCB) (FileObject->FsContext2);
    pFcb = (PVFATFCB) (FileObject->FsContext);

    if (pFcb == NULL)
    {
        return STATUS_SUCCESS;
    }

    if (pFcb->Flags & FCB_IS_VOLUME)
    {
        DPRINT1("Volume\n");
        pFcb->RefCount--;
        FileObject->FsContext2 = NULL;
    }
    else
    {
        if (FileObject->DeletePending)
        {
            if (pFcb->Flags & FCB_DELETE_PENDING)
            {
                VfatDelEntry(DeviceExt, pFcb);

                FsRtlNotifyFullReportChange(DeviceExt->NotifySync,
                                            &(DeviceExt->NotifyList),
                                            (PSTRING)&pFcb->PathNameU,
                                            pFcb->PathNameU.Length - pFcb->LongNameU.Length,
                                            NULL,
                                            NULL,
                                            ((*pFcb->Attributes & FILE_ATTRIBUTE_DIRECTORY) ?
                                            FILE_NOTIFY_CHANGE_DIR_NAME : FILE_NOTIFY_CHANGE_FILE_NAME),
                                            FILE_ACTION_REMOVED,
                                            NULL);
            }
            else
            {
                Status = STATUS_DELETE_PENDING;
            }
        }

        vfatReleaseFCB(DeviceExt, pFcb);
    }

    FileObject->FsContext2 = NULL;
    FileObject->FsContext = NULL;
    FileObject->SectionObjectPointer = NULL;

    if (pCcb)
    {
        vfatDestroyCCB(pCcb);
    }

    return Status;
}

/*
 * FUNCTION: Closes a file
 */
NTSTATUS
VfatClose(
    PVFAT_IRP_CONTEXT IrpContext)
{
    NTSTATUS Status;

    DPRINT("VfatClose(DeviceObject %p, Irp %p)\n", IrpContext->DeviceObject, IrpContext->Irp);

    if (IrpContext->DeviceObject == VfatGlobalData->DeviceObject)
    {
        DPRINT("Closing file system\n");
        Status = STATUS_SUCCESS;
        goto ByeBye;
    }
#if 0
    /* There occurs a dead look at the call to CcRosDeleteFileCache/ObDereferenceObject/VfatClose
       in CmLazyCloseThreadMain if VfatClose is execute asynchronous in a worker thread. */
    if (!ExAcquireResourceExclusiveLite(&IrpContext->DeviceExt->DirResource, IrpContext->Flags & IRPCONTEXT_CANWAIT))
#else
    if (!ExAcquireResourceExclusiveLite(&IrpContext->DeviceExt->DirResource, TRUE))
#endif
    {
        return VfatQueueRequest(IrpContext);
    }

    Status = VfatCloseFile(IrpContext->DeviceExt, IrpContext->FileObject);
    ExReleaseResourceLite(&IrpContext->DeviceExt->DirResource);

ByeBye:
    IrpContext->Irp->IoStatus.Status = Status;
    IrpContext->Irp->IoStatus.Information = 0;
    IoCompleteRequest(IrpContext->Irp, IO_NO_INCREMENT);
    VfatFreeIrpContext(IrpContext);

    return Status;
}

/* EOF */
