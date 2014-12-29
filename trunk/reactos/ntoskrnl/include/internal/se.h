#pragma once

typedef struct _KNOWN_ACE
{
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
} KNOWN_ACE, *PKNOWN_ACE;

typedef struct _KNOWN_OBJECT_ACE
{
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG Flags;
    ULONG SidStart;
} KNOWN_OBJECT_ACE, *PKNOWN_OBJECT_ACE;

typedef struct _KNOWN_COMPOUND_ACE
{
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    USHORT CompoundAceType;
    USHORT Reserved;
    ULONG SidStart;
} KNOWN_COMPOUND_ACE, *PKNOWN_COMPOUND_ACE;

FORCEINLINE
PSID
SepGetGroupFromDescriptor(PVOID _Descriptor)
{
    PISECURITY_DESCRIPTOR Descriptor = (PISECURITY_DESCRIPTOR)_Descriptor;
    PISECURITY_DESCRIPTOR_RELATIVE SdRel;

    if (Descriptor->Control & SE_SELF_RELATIVE)
    {
        SdRel = (PISECURITY_DESCRIPTOR_RELATIVE)Descriptor;
        if (!SdRel->Group) return NULL;
        return (PSID)((ULONG_PTR)Descriptor + SdRel->Group);
    }
    else
    {
        return Descriptor->Group;
    }
}

FORCEINLINE
PSID
SepGetOwnerFromDescriptor(PVOID _Descriptor)
{
    PISECURITY_DESCRIPTOR Descriptor = (PISECURITY_DESCRIPTOR)_Descriptor;
    PISECURITY_DESCRIPTOR_RELATIVE SdRel;

    if (Descriptor->Control & SE_SELF_RELATIVE)
    {
        SdRel = (PISECURITY_DESCRIPTOR_RELATIVE)Descriptor;
        if (!SdRel->Owner) return NULL;
        return (PSID)((ULONG_PTR)Descriptor + SdRel->Owner);
    }
    else
    {
        return Descriptor->Owner;
    }
}

FORCEINLINE
PACL
SepGetDaclFromDescriptor(PVOID _Descriptor)
{
    PISECURITY_DESCRIPTOR Descriptor = (PISECURITY_DESCRIPTOR)_Descriptor;
    PISECURITY_DESCRIPTOR_RELATIVE SdRel;

    if (!(Descriptor->Control & SE_DACL_PRESENT)) return NULL;

    if (Descriptor->Control & SE_SELF_RELATIVE)
    {
        SdRel = (PISECURITY_DESCRIPTOR_RELATIVE)Descriptor;
        if (!SdRel->Dacl) return NULL;
        return (PACL)((ULONG_PTR)Descriptor + SdRel->Dacl);
    }
    else
    {
        return Descriptor->Dacl;
    }
}

FORCEINLINE
PACL
SepGetSaclFromDescriptor(PVOID _Descriptor)
{
    PISECURITY_DESCRIPTOR Descriptor = (PISECURITY_DESCRIPTOR)_Descriptor;
    PISECURITY_DESCRIPTOR_RELATIVE SdRel;

    if (!(Descriptor->Control & SE_SACL_PRESENT)) return NULL;

    if (Descriptor->Control & SE_SELF_RELATIVE)
    {
        SdRel = (PISECURITY_DESCRIPTOR_RELATIVE)Descriptor;
        if (!SdRel->Sacl) return NULL;
        return (PACL)((ULONG_PTR)Descriptor + SdRel->Sacl);
    }
    else
    {
        return Descriptor->Sacl;
    }
}

#ifndef RTL_H

/* SID Authorities */
extern SID_IDENTIFIER_AUTHORITY SeNullSidAuthority;
extern SID_IDENTIFIER_AUTHORITY SeWorldSidAuthority;
extern SID_IDENTIFIER_AUTHORITY SeLocalSidAuthority;
extern SID_IDENTIFIER_AUTHORITY SeCreatorSidAuthority;
extern SID_IDENTIFIER_AUTHORITY SeNtSidAuthority;

/* SIDs */
extern PSID SeNullSid;
extern PSID SeWorldSid;
extern PSID SeLocalSid;
extern PSID SeCreatorOwnerSid;
extern PSID SeCreatorGroupSid;
extern PSID SeCreatorOwnerServerSid;
extern PSID SeCreatorGroupServerSid;
extern PSID SeNtAuthoritySid;
extern PSID SeDialupSid;
extern PSID SeNetworkSid;
extern PSID SeBatchSid;
extern PSID SeInteractiveSid;
extern PSID SeServiceSid;
extern PSID SeAnonymousLogonSid;
extern PSID SePrincipalSelfSid;
extern PSID SeLocalSystemSid;
extern PSID SeAuthenticatedUserSid;
extern PSID SeRestrictedCodeSid;
extern PSID SeAliasAdminsSid;
extern PSID SeAliasUsersSid;
extern PSID SeAliasGuestsSid;
extern PSID SeAliasPowerUsersSid;
extern PSID SeAliasAccountOpsSid;
extern PSID SeAliasSystemOpsSid;
extern PSID SeAliasPrintOpsSid;
extern PSID SeAliasBackupOpsSid;
extern PSID SeAuthenticatedUsersSid;
extern PSID SeRestrictedSid;
extern PSID SeAnonymousLogonSid;
extern PSID SeLocalServiceSid;
extern PSID SeNetworkServiceSid;

/* Privileges */
extern const LUID SeCreateTokenPrivilege;
extern const LUID SeAssignPrimaryTokenPrivilege;
extern const LUID SeLockMemoryPrivilege;
extern const LUID SeIncreaseQuotaPrivilege;
extern const LUID SeUnsolicitedInputPrivilege;
extern const LUID SeTcbPrivilege;
extern const LUID SeSecurityPrivilege;
extern const LUID SeTakeOwnershipPrivilege;
extern const LUID SeLoadDriverPrivilege;
extern const LUID SeSystemProfilePrivilege;
extern const LUID SeSystemtimePrivilege;
extern const LUID SeProfileSingleProcessPrivilege;
extern const LUID SeIncreaseBasePriorityPrivilege;
extern const LUID SeCreatePagefilePrivilege;
extern const LUID SeCreatePermanentPrivilege;
extern const LUID SeBackupPrivilege;
extern const LUID SeRestorePrivilege;
extern const LUID SeShutdownPrivilege;
extern const LUID SeDebugPrivilege;
extern const LUID SeAuditPrivilege;
extern const LUID SeSystemEnvironmentPrivilege;
extern const LUID SeChangeNotifyPrivilege;
extern const LUID SeRemoteShutdownPrivilege;
extern const LUID SeUndockPrivilege;
extern const LUID SeSyncAgentPrivilege;
extern const LUID SeEnableDelegationPrivilege;
extern const LUID SeManageVolumePrivilege;
extern const LUID SeImpersonatePrivilege;
extern const LUID SeCreateGlobalPrivilege;
extern const LUID SeTrustedCredmanPrivilege;
extern const LUID SeRelabelPrivilege;
extern const LUID SeIncreaseWorkingSetPrivilege;
extern const LUID SeTimeZonePrivilege;
extern const LUID SeCreateSymbolicLinkPrivilege;

/* DACLs */
extern PACL SePublicDefaultUnrestrictedDacl;
extern PACL SePublicOpenDacl;
extern PACL SePublicOpenUnrestrictedDacl;
extern PACL SeUnrestrictedDacl;

/* SDs */
extern PSECURITY_DESCRIPTOR SePublicDefaultSd;
extern PSECURITY_DESCRIPTOR SePublicDefaultUnrestrictedSd;
extern PSECURITY_DESCRIPTOR SePublicOpenSd;
extern PSECURITY_DESCRIPTOR SePublicOpenUnrestrictedSd;
extern PSECURITY_DESCRIPTOR SeSystemDefaultSd;
extern PSECURITY_DESCRIPTOR SeUnrestrictedSd;


#define SepAcquireTokenLockExclusive(Token)                                    \
{                                                                              \
    KeEnterCriticalRegion();                                                   \
    ExAcquireResourceExclusive(((PTOKEN)Token)->TokenLock, TRUE);              \
}
#define SepAcquireTokenLockShared(Token)                                       \
{                                                                              \
    KeEnterCriticalRegion();                                                   \
    ExAcquireResourceShared(((PTOKEN)Token)->TokenLock, TRUE);                 \
}

#define SepReleaseTokenLock(Token)                                             \
{                                                                              \
    ExReleaseResource(((PTOKEN)Token)->TokenLock);                             \
    KeLeaveCriticalRegion();                                                   \
}

//
// Token Functions
//
BOOLEAN
NTAPI
SepTokenIsOwner(
    IN PACCESS_TOKEN _Token,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN TokenLocked
);

BOOLEAN
NTAPI
SepSidInToken(
    IN PACCESS_TOKEN _Token,
    IN PSID Sid
);

BOOLEAN
NTAPI
SepSidInTokenEx(
    IN PACCESS_TOKEN _Token,
    IN PSID PrincipalSelfSid,
    IN PSID _Sid,
    IN BOOLEAN Deny,
    IN BOOLEAN Restricted
);

/* Functions */
BOOLEAN
NTAPI
SeInitSystem(VOID);

VOID
NTAPI
ExpInitLuid(VOID);

VOID
NTAPI
SepInitPrivileges(VOID);

BOOLEAN
NTAPI
SepInitSecurityIDs(VOID);

BOOLEAN
NTAPI
SepInitDACLs(VOID);

BOOLEAN
NTAPI
SepInitSDs(VOID);

VOID
NTAPI
SeDeassignPrimaryToken(struct _EPROCESS *Process);

NTSTATUS
NTAPI
SeSubProcessToken(
    IN PTOKEN Parent,
    OUT PTOKEN *Token,
    IN BOOLEAN InUse,
    IN ULONG SessionId
);

NTSTATUS
NTAPI
SeInitializeProcessAuditName(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN DoAudit,
    OUT POBJECT_NAME_INFORMATION *AuditInfo
);

NTSTATUS
NTAPI
SeCreateAccessStateEx(
    IN PETHREAD Thread,
    IN PEPROCESS Process,
    IN OUT PACCESS_STATE AccessState,
    IN PAUX_ACCESS_DATA AuxData,
    IN ACCESS_MASK Access,
    IN PGENERIC_MAPPING GenericMapping
);

NTSTATUS
NTAPI
SeIsTokenChild(
    IN PTOKEN Token,
    OUT PBOOLEAN IsChild
);

NTSTATUS
NTAPI
SepCreateImpersonationTokenDacl(
    PTOKEN Token,
    PTOKEN PrimaryToken,
    PACL *Dacl
);

VOID
NTAPI
SepInitializeTokenImplementation(VOID);

PTOKEN
NTAPI
SepCreateSystemProcessToken(VOID);

BOOLEAN
NTAPI
SeDetailedAuditingWithToken(IN PTOKEN Token);

VOID
NTAPI
SeAuditProcessExit(IN PEPROCESS Process);

VOID
NTAPI
SeAuditProcessCreate(IN PEPROCESS Process);

NTSTATUS
NTAPI
SeExchangePrimaryToken(
    struct _EPROCESS* Process,
    PACCESS_TOKEN NewToken,
    PACCESS_TOKEN* OldTokenP
);

VOID
NTAPI
SeCaptureSubjectContextEx(
    IN PETHREAD Thread,
    IN PEPROCESS Process,
    OUT PSECURITY_SUBJECT_CONTEXT SubjectContext
);

NTSTATUS
NTAPI
SeCaptureLuidAndAttributesArray(
    PLUID_AND_ATTRIBUTES Src,
    ULONG PrivilegeCount,
    KPROCESSOR_MODE PreviousMode,
    PLUID_AND_ATTRIBUTES AllocatedMem,
    ULONG AllocatedLength,
    POOL_TYPE PoolType,
    BOOLEAN CaptureIfKernel,
    PLUID_AND_ATTRIBUTES* Dest,
    PULONG Length
);

VOID
NTAPI
SeReleaseLuidAndAttributesArray(
    PLUID_AND_ATTRIBUTES Privilege,
    KPROCESSOR_MODE PreviousMode,
    BOOLEAN CaptureIfKernel
);

BOOLEAN
NTAPI
SepPrivilegeCheck(
    PTOKEN Token,
    PLUID_AND_ATTRIBUTES Privileges,
    ULONG PrivilegeCount,
    ULONG PrivilegeControl,
    KPROCESSOR_MODE PreviousMode
);

NTSTATUS
NTAPI
SePrivilegePolicyCheck(
    _Inout_ PACCESS_MASK DesiredAccess,
    _Inout_ PACCESS_MASK GrantedAccess,
    _In_ PSECURITY_SUBJECT_CONTEXT SubjectContext,
    _In_ PTOKEN Token,
    _Out_opt_ PPRIVILEGE_SET *OutPrivilegeSet,
    _In_ KPROCESSOR_MODE PreviousMode);

BOOLEAN
NTAPI
SeCheckPrivilegedObject(
    IN LUID PrivilegeValue,
    IN HANDLE ObjectHandle,
    IN ACCESS_MASK DesiredAccess,
    IN KPROCESSOR_MODE PreviousMode
);

NTSTATUS
NTAPI
SepDuplicateToken(
    PTOKEN Token,
    POBJECT_ATTRIBUTES ObjectAttributes,
    BOOLEAN EffectiveOnly,
    TOKEN_TYPE TokenType,
    SECURITY_IMPERSONATION_LEVEL Level,
    KPROCESSOR_MODE PreviousMode,
    PTOKEN* NewAccessToken
);

NTSTATUS
NTAPI
SepCaptureSecurityQualityOfService(
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN KPROCESSOR_MODE AccessMode,
    IN POOL_TYPE PoolType,
    IN BOOLEAN CaptureIfKernel,
    OUT PSECURITY_QUALITY_OF_SERVICE *CapturedSecurityQualityOfService,
    OUT PBOOLEAN Present
);

VOID
NTAPI
SepReleaseSecurityQualityOfService(
    IN PSECURITY_QUALITY_OF_SERVICE CapturedSecurityQualityOfService OPTIONAL,
    IN KPROCESSOR_MODE AccessMode,
    IN BOOLEAN CaptureIfKernel
);

NTSTATUS
NTAPI
SepCaptureSid(
    IN PSID InputSid,
    IN KPROCESSOR_MODE AccessMode,
    IN POOL_TYPE PoolType,
    IN BOOLEAN CaptureIfKernel,
    OUT PSID *CapturedSid
);

VOID
NTAPI
SepReleaseSid(
    IN PSID CapturedSid,
    IN KPROCESSOR_MODE AccessMode,
    IN BOOLEAN CaptureIfKernel
);

NTSTATUS
NTAPI
SeCaptureSidAndAttributesArray(
    _In_ PSID_AND_ATTRIBUTES SrcSidAndAttributes,
    _In_ ULONG AttributeCount,
    _In_ KPROCESSOR_MODE PreviousMode,
    _In_opt_ PVOID AllocatedMem,
    _In_ ULONG AllocatedLength,
    _In_ POOL_TYPE PoolType,
    _In_ BOOLEAN CaptureIfKernel,
    _Out_ PSID_AND_ATTRIBUTES *CapturedSidAndAttributes,
    _Out_ PULONG ResultLength);

VOID
NTAPI
SeReleaseSidAndAttributesArray(
    _In_ _Post_invalid_ PSID_AND_ATTRIBUTES CapturedSidAndAttributes,
    _In_ KPROCESSOR_MODE AccessMode,
    _In_ BOOLEAN CaptureIfKernel);

NTSTATUS
NTAPI
SepCaptureAcl(
    IN PACL InputAcl,
    IN KPROCESSOR_MODE AccessMode,
    IN POOL_TYPE PoolType,
    IN BOOLEAN CaptureIfKernel,
    OUT PACL *CapturedAcl
);

VOID
NTAPI
SepReleaseAcl(
    IN PACL CapturedAcl,
    IN KPROCESSOR_MODE AccessMode,
    IN BOOLEAN CaptureIfKernel
);

NTSTATUS
SepPropagateAcl(
    _Out_writes_bytes_opt_(DaclLength) PACL AclDest,
    _Inout_ PULONG AclLength,
    _In_reads_bytes_(AclSource->AclSize) PACL AclSource,
    _In_ PSID Owner,
    _In_ PSID Group,
    _In_ BOOLEAN IsInherited,
    _In_ BOOLEAN IsDirectoryObject,
    _In_ PGENERIC_MAPPING GenericMapping);
    
PACL
SepSelectAcl(
    _In_opt_ PACL ExplicitAcl,
    _In_ BOOLEAN ExplicitPresent,
    _In_ BOOLEAN ExplicitDefaulted,
    _In_opt_ PACL ParentAcl,
    _In_opt_ PACL DefaultAcl,
    _Out_ PULONG AclLength,
    _In_ PSID Owner,
    _In_ PSID Group,
    _Out_ PBOOLEAN AclPresent,
    _Out_ PBOOLEAN IsInherited,
    _In_ BOOLEAN IsDirectoryObject,
    _In_ PGENERIC_MAPPING GenericMapping);

NTSTATUS
NTAPI
SeDefaultObjectMethod(
    PVOID Object,
    SECURITY_OPERATION_CODE OperationType,
    PSECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR NewSecurityDescriptor,
    PULONG ReturnLength,
    PSECURITY_DESCRIPTOR *OldSecurityDescriptor,
    POOL_TYPE PoolType,
    PGENERIC_MAPPING GenericMapping
);

NTSTATUS
NTAPI
SeSetWorldSecurityDescriptor(
    SECURITY_INFORMATION SecurityInformation,
    PISECURITY_DESCRIPTOR SecurityDescriptor,
    PULONG BufferLength
);

NTSTATUS
NTAPI
SeCopyClientToken(
    IN PACCESS_TOKEN Token,
    IN SECURITY_IMPERSONATION_LEVEL Level,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PACCESS_TOKEN* NewToken
);

VOID NTAPI
SeQuerySecurityAccessMask(IN SECURITY_INFORMATION SecurityInformation,
                          OUT PACCESS_MASK DesiredAccess);

VOID NTAPI
SeSetSecurityAccessMask(IN SECURITY_INFORMATION SecurityInformation,
                        OUT PACCESS_MASK DesiredAccess);

BOOLEAN
NTAPI
SeFastTraverseCheck(IN PSECURITY_DESCRIPTOR SecurityDescriptor,
                    IN PACCESS_STATE AccessState,
                    IN ACCESS_MASK DesiredAccess,
                    IN KPROCESSOR_MODE AccessMode);

BOOLEAN
NTAPI
SeCheckAuditPrivilege(
    _In_ PSECURITY_SUBJECT_CONTEXT SubjectContext,
    _In_ KPROCESSOR_MODE PreviousMode);

VOID
NTAPI
SePrivilegedServiceAuditAlarm(
    _In_opt_ PUNICODE_STRING ServiceName,
    _In_ PSECURITY_SUBJECT_CONTEXT SubjectContext,
    _In_ PPRIVILEGE_SET PrivilegeSet,
    _In_ BOOLEAN AccessGranted);

#endif

/* EOF */
