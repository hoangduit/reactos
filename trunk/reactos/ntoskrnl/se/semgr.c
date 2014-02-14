/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/se/semgr.c
 * PURPOSE:         Security manager
 *
 * PROGRAMMERS:     No programmer listed.
 */

/* INCLUDES *******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

/* GLOBALS ********************************************************************/

PSE_EXPORTS SeExports = NULL;
SE_EXPORTS SepExports;
ULONG SidInTokenCalls = 0;

extern ULONG ExpInitializationPhase;
extern ERESOURCE SepSubjectContextLock;

/* PRIVATE FUNCTIONS **********************************************************/

static BOOLEAN
INIT_FUNCTION
SepInitExports(VOID)
{
    SepExports.SeCreateTokenPrivilege = SeCreateTokenPrivilege;
    SepExports.SeAssignPrimaryTokenPrivilege = SeAssignPrimaryTokenPrivilege;
    SepExports.SeLockMemoryPrivilege = SeLockMemoryPrivilege;
    SepExports.SeIncreaseQuotaPrivilege = SeIncreaseQuotaPrivilege;
    SepExports.SeUnsolicitedInputPrivilege = SeUnsolicitedInputPrivilege;
    SepExports.SeTcbPrivilege = SeTcbPrivilege;
    SepExports.SeSecurityPrivilege = SeSecurityPrivilege;
    SepExports.SeTakeOwnershipPrivilege = SeTakeOwnershipPrivilege;
    SepExports.SeLoadDriverPrivilege = SeLoadDriverPrivilege;
    SepExports.SeCreatePagefilePrivilege = SeCreatePagefilePrivilege;
    SepExports.SeIncreaseBasePriorityPrivilege = SeIncreaseBasePriorityPrivilege;
    SepExports.SeSystemProfilePrivilege = SeSystemProfilePrivilege;
    SepExports.SeSystemtimePrivilege = SeSystemtimePrivilege;
    SepExports.SeProfileSingleProcessPrivilege = SeProfileSingleProcessPrivilege;
    SepExports.SeCreatePermanentPrivilege = SeCreatePermanentPrivilege;
    SepExports.SeBackupPrivilege = SeBackupPrivilege;
    SepExports.SeRestorePrivilege = SeRestorePrivilege;
    SepExports.SeShutdownPrivilege = SeShutdownPrivilege;
    SepExports.SeDebugPrivilege = SeDebugPrivilege;
    SepExports.SeAuditPrivilege = SeAuditPrivilege;
    SepExports.SeSystemEnvironmentPrivilege = SeSystemEnvironmentPrivilege;
    SepExports.SeChangeNotifyPrivilege = SeChangeNotifyPrivilege;
    SepExports.SeRemoteShutdownPrivilege = SeRemoteShutdownPrivilege;

    SepExports.SeNullSid = SeNullSid;
    SepExports.SeWorldSid = SeWorldSid;
    SepExports.SeLocalSid = SeLocalSid;
    SepExports.SeCreatorOwnerSid = SeCreatorOwnerSid;
    SepExports.SeCreatorGroupSid = SeCreatorGroupSid;
    SepExports.SeNtAuthoritySid = SeNtAuthoritySid;
    SepExports.SeDialupSid = SeDialupSid;
    SepExports.SeNetworkSid = SeNetworkSid;
    SepExports.SeBatchSid = SeBatchSid;
    SepExports.SeInteractiveSid = SeInteractiveSid;
    SepExports.SeLocalSystemSid = SeLocalSystemSid;
    SepExports.SeAliasAdminsSid = SeAliasAdminsSid;
    SepExports.SeAliasUsersSid = SeAliasUsersSid;
    SepExports.SeAliasGuestsSid = SeAliasGuestsSid;
    SepExports.SeAliasPowerUsersSid = SeAliasPowerUsersSid;
    SepExports.SeAliasAccountOpsSid = SeAliasAccountOpsSid;
    SepExports.SeAliasSystemOpsSid = SeAliasSystemOpsSid;
    SepExports.SeAliasPrintOpsSid = SeAliasPrintOpsSid;
    SepExports.SeAliasBackupOpsSid = SeAliasBackupOpsSid;
    SepExports.SeAuthenticatedUsersSid = SeAuthenticatedUsersSid;
    SepExports.SeRestrictedSid = SeRestrictedSid;
    SepExports.SeAnonymousLogonSid = SeAnonymousLogonSid;

    SepExports.SeUndockPrivilege = SeUndockPrivilege;
    SepExports.SeSyncAgentPrivilege = SeSyncAgentPrivilege;
    SepExports.SeEnableDelegationPrivilege = SeEnableDelegationPrivilege;

    SeExports = &SepExports;
    return TRUE;
}


BOOLEAN
NTAPI
INIT_FUNCTION
SepInitializationPhase0(VOID)
{
    PAGED_CODE();

    ExpInitLuid();
    if (!SepInitSecurityIDs()) return FALSE;
    if (!SepInitDACLs()) return FALSE;
    if (!SepInitSDs()) return FALSE;
    SepInitPrivileges();
    if (!SepInitExports()) return FALSE;

    /* Initialize the subject context lock */
    ExInitializeResource(&SepSubjectContextLock);

    /* Initialize token objects */
    SepInitializeTokenImplementation();

    /* Clear impersonation info for the idle thread */
    PsGetCurrentThread()->ImpersonationInfo = NULL;
    PspClearCrossThreadFlag(PsGetCurrentThread(),
                            CT_ACTIVE_IMPERSONATION_INFO_BIT);

    /* Initialize the boot token */
    ObInitializeFastReference(&PsGetCurrentProcess()->Token, NULL);
    ObInitializeFastReference(&PsGetCurrentProcess()->Token,
                              SepCreateSystemProcessToken());
    return TRUE;
}

BOOLEAN
NTAPI
INIT_FUNCTION
SepInitializationPhase1(VOID)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING Name;
    HANDLE SecurityHandle;
    HANDLE EventHandle;
    NTSTATUS Status;

    PAGED_CODE();

    /* Insert the system token into the tree */
    Status = ObInsertObject((PVOID)(PsGetCurrentProcess()->Token.Value &
                                    ~MAX_FAST_REFS),
                            NULL,
                            0,
                            0,
                            NULL,
                            NULL);
    ASSERT(NT_SUCCESS(Status));

    /* TODO: Create a security desscriptor for the directory */

    /* Create '\Security' directory */
    RtlInitUnicodeString(&Name, L"\\Security");
    InitializeObjectAttributes(&ObjectAttributes,
                               &Name,
                               OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                               0,
                               NULL);

    Status = ZwCreateDirectoryObject(&SecurityHandle,
                                     DIRECTORY_ALL_ACCESS,
                                     &ObjectAttributes);
    ASSERT(NT_SUCCESS(Status));

    /* Create 'LSA_AUTHENTICATION_INITIALIZED' event */
    RtlInitUnicodeString(&Name, L"LSA_AUTHENTICATION_INITIALIZED");
    InitializeObjectAttributes(&ObjectAttributes,
                               &Name,
                               OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                               SecurityHandle,
                               SePublicDefaultSd);

    Status = ZwCreateEvent(&EventHandle,
                           GENERIC_WRITE,
                           &ObjectAttributes,
                           NotificationEvent,
                           FALSE);
    ASSERT(NT_SUCCESS(Status));

    Status = ZwClose(EventHandle);
    ASSERT(NT_SUCCESS(Status));

    Status = ZwClose(SecurityHandle);
    ASSERT(NT_SUCCESS(Status));

    return TRUE;
}

BOOLEAN
NTAPI
INIT_FUNCTION
SeInitSystem(VOID)
{
    /* Check the initialization phase */
    switch (ExpInitializationPhase)
    {
        case 0:

            /* Do Phase 0 */
            return SepInitializationPhase0();

        case 1:

            /* Do Phase 1 */
            return SepInitializationPhase1();

        default:

            /* Don't know any other phase! Bugcheck! */
            KeBugCheckEx(UNEXPECTED_INITIALIZATION_CALL,
                         0,
                         ExpInitializationPhase,
                         0,
                         0);
            return FALSE;
    }
}

NTSTATUS
NTAPI
SeDefaultObjectMethod(IN PVOID Object,
                      IN SECURITY_OPERATION_CODE OperationType,
                      IN PSECURITY_INFORMATION SecurityInformation,
                      IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
                      IN OUT PULONG ReturnLength OPTIONAL,
                      IN OUT PSECURITY_DESCRIPTOR *OldSecurityDescriptor,
                      IN POOL_TYPE PoolType,
                      IN PGENERIC_MAPPING GenericMapping)
{
    PAGED_CODE();

    /* Select the operation type */
    switch (OperationType)
    {
            /* Setting a new descriptor */
        case SetSecurityDescriptor:

            /* Sanity check */
            ASSERT((PoolType == PagedPool) || (PoolType == NonPagedPool));

            /* Set the information */
            return ObSetSecurityDescriptorInfo(Object,
                                               SecurityInformation,
                                               SecurityDescriptor,
                                               OldSecurityDescriptor,
                                               PoolType,
                                               GenericMapping);

        case QuerySecurityDescriptor:

            /* Query the information */
            return ObQuerySecurityDescriptorInfo(Object,
                                                 SecurityInformation,
                                                 SecurityDescriptor,
                                                 ReturnLength,
                                                 OldSecurityDescriptor);

        case DeleteSecurityDescriptor:

            /* De-assign it */
            return ObDeassignSecurity(OldSecurityDescriptor);

        case AssignSecurityDescriptor:

            /* Assign it */
            ObAssignObjectSecurityDescriptor(Object, SecurityDescriptor, PoolType);
            return STATUS_SUCCESS;

        default:

            /* Bug check */
            KeBugCheckEx(SECURITY_SYSTEM, 0, STATUS_INVALID_PARAMETER, 0, 0);
    }

    /* Should never reach here */
    ASSERT(FALSE);
    return STATUS_SUCCESS;
}

VOID
NTAPI
SeQuerySecurityAccessMask(IN SECURITY_INFORMATION SecurityInformation,
                          OUT PACCESS_MASK DesiredAccess)
{
    *DesiredAccess = 0;

    if (SecurityInformation & (OWNER_SECURITY_INFORMATION |
                               GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION))
    {
        *DesiredAccess |= READ_CONTROL;
    }

    if (SecurityInformation & SACL_SECURITY_INFORMATION)
    {
        *DesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }
}

VOID
NTAPI
SeSetSecurityAccessMask(IN SECURITY_INFORMATION SecurityInformation,
                        OUT PACCESS_MASK DesiredAccess)
{
    *DesiredAccess = 0;

    if (SecurityInformation & (OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION))
    {
        *DesiredAccess |= WRITE_OWNER;
    }

    if (SecurityInformation & DACL_SECURITY_INFORMATION)
    {
        *DesiredAccess |= WRITE_DAC;
    }

    if (SecurityInformation & SACL_SECURITY_INFORMATION)
    {
        *DesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }
}

/* EOF */
