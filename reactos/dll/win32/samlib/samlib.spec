@ stdcall SamAddMemberToAlias(ptr ptr)
@ stdcall SamAddMemberToGroup(ptr long long)
@ stdcall SamAddMultipleMembersToAlias(ptr ptr long)
@ stdcall SamChangePasswordUser2(ptr ptr ptr ptr)
@ stdcall SamChangePasswordUser3(ptr ptr ptr ptr ptr ptr)
@ stdcall SamChangePasswordUser(ptr ptr ptr)
@ stdcall SamCloseHandle(ptr)
@ stdcall SamConnect(ptr ptr long ptr)
@ stub SamConnectWithCreds
@ stdcall SamCreateAliasInDomain(ptr ptr long ptr ptr)
@ stdcall SamCreateGroupInDomain(ptr ptr long ptr ptr)
@ stdcall SamCreateUser2InDomain(ptr ptr long long ptr ptr ptr)
@ stdcall SamCreateUserInDomain(ptr ptr long ptr ptr)
@ stdcall SamDeleteAlias(ptr)
@ stdcall SamDeleteGroup(ptr)
@ stdcall SamDeleteUser(ptr)
@ stdcall SamEnumerateAliasesInDomain(ptr ptr ptr long ptr)
@ stdcall SamEnumerateDomainsInSamServer(ptr ptr ptr long ptr)
@ stdcall SamEnumerateGroupsInDomain(ptr ptr ptr long ptr)
@ stdcall SamEnumerateUsersInDomain(ptr ptr long ptr long ptr)
@ stdcall SamFreeMemory(ptr)
@ stdcall SamGetAliasMembership(ptr long ptr ptr ptr)
@ stdcall SamGetCompatibilityMode(ptr ptr)
@ stdcall SamGetDisplayEnumerationIndex(ptr long ptr ptr)
@ stdcall SamGetGroupsForUser(ptr ptr ptr)
@ stdcall SamGetMembersInAlias(ptr ptr ptr)
@ stdcall SamGetMembersInGroup(ptr ptr ptr ptr)
@ stdcall SamLookupDomainInSamServer(ptr ptr ptr)
@ stdcall SamLookupIdsInDomain(ptr long ptr ptr ptr)
@ stdcall SamLookupNamesInDomain(ptr long ptr ptr ptr)
@ stdcall SamOpenAlias(ptr long long ptr)
@ stdcall SamOpenDomain(ptr long ptr ptr)
@ stdcall SamOpenGroup(ptr long long ptr)
@ stdcall SamOpenUser(ptr long long ptr)
@ stdcall SamQueryDisplayInformation(ptr long long long long ptr ptr ptr ptr)
@ stdcall SamQueryInformationAlias(ptr long ptr)
@ stdcall SamQueryInformationDomain(ptr long ptr)
@ stdcall SamQueryInformationGroup(ptr long ptr)
@ stdcall SamQueryInformationUser(ptr long ptr)
@ stdcall SamQuerySecurityObject(ptr long ptr)
@ stdcall SamRemoveMemberFromAlias(ptr long)
@ stdcall SamRemoveMemberFromForeignDomain(ptr long)
@ stdcall SamRemoveMemberFromGroup(ptr long)
@ stdcall SamRemoveMultipleMembersFromAlias(ptr ptr long)
@ stdcall SamRidToSid(ptr long ptr)
@ stdcall SamSetInformationAlias(ptr long ptr)
@ stdcall SamSetInformationDomain(ptr long ptr)
@ stdcall SamSetInformationGroup(ptr long ptr)
@ stdcall SamSetInformationUser(ptr long ptr)
@ stdcall SamSetMemberAttributesOfGroup(ptr long long)
@ stdcall SamSetSecurityObject(ptr long ptr)
@ stdcall SamShutdownSamServer(ptr)
@ stub SamTestPrivateFunctionsDomain
@ stub SamTestPrivateFunctionsUser
@ stub SamiChangeKeys
@ stub SamiChangePasswordUser2
@ stub SamiChangePasswordUser3
@ stub SamiChangePasswordUser
@ stub SamiEncryptPasswords
@ stub SamiGetBootKeyInformation
@ stub SamiLmChangePasswordUser
@ stub SamiOemChangePasswordUser2
@ stub SamiSetBootKeyInformation
@ stub SamiSetDSRMPassword
@ stub SamiSetDSRMPasswordOWF
