
 @ stdcall ClassInitialize(ptr ptr ptr)
 @ stdcall ClassInitializeEx(ptr ptr ptr)
 @ stdcall ClassGetDescriptor(ptr ptr ptr)
 @ stdcall ClassReadDriveCapacity(ptr)
 @ stdcall ClassReleaseQueue(ptr)
 @ stdcall ClassAsynchronousCompletion(ptr ptr ptr)
 @ stdcall ClassSplitRequest(ptr ptr long)
 @ stdcall ClassDeviceControl(ptr ptr)
 @ stdcall ClassIoComplete(ptr ptr ptr)
 @ stdcall ClassIoCompleteAssociated(ptr ptr ptr)
 @ stdcall ClassInterpretSenseInfo(ptr ptr long long long ptr ptr)
 @ stdcall ClassSendDeviceIoControlSynchronous(long ptr ptr long long long ptr)
 @ stdcall ClassSendIrpSynchronous(ptr ptr)
 @ stdcall ClassForwardIrpSynchronous(ptr ptr)
 @ stdcall ClassSendSrbSynchronous(ptr ptr ptr long long)
 @ stdcall ClassSendSrbAsynchronous(ptr ptr ptr ptr long long)
 @ stdcall ClassBuildRequest(ptr ptr)
 @ stdcall ClassModeSense(ptr ptr long long)
 @ stdcall ClassFindModePage(ptr long long long)
 @ stdcall ClassClaimDevice(ptr long)
 @ stdcall ClassInternalIoControl(ptr ptr)
 @ stdcall ClassCreateDeviceObject(ptr ptr ptr long ptr)
 @ stdcall ClassRemoveDevice(ptr long)
 @ stdcall ClassInitializeSrbLookasideList(ptr long)
 @ stdcall ClassDeleteSrbLookasideList(ptr)
 @ stdcall ClassQueryTimeOutRegistryValue(ptr)
 @ stdcall ClassInvalidateBusRelations(ptr)
 @ stdcall ClassMarkChildrenMissing(ptr)
 @ stdcall ClassMarkChildMissing(ptr long)
 @ varargs ClassDebugPrint(long long)
 @ stdcall ClassGetDriverExtension(ptr)
 @ stdcall ClassCompleteRequest(ptr ptr long)
 @ stdcall ClassReleaseRemoveLock(ptr ptr)
 @ stdcall ClassAcquireRemoveLockEx(ptr ptr ptr long)
 @ stdcall ClassUpdateInformationInRegistry(ptr ptr long ptr long)
 @ stdcall ClassWmiCompleteRequest(ptr ptr long long long)
 @ stdcall ClassWmiFireEvent(ptr ptr long long ptr)
 @ stdcall ClassGetVpb(ptr)
 @ stdcall ClassSetFailurePredictionPoll(ptr long long)
 @ stdcall ClassNotifyFailurePredicted(ptr ptr long long long long long long)
 @ stdcall ClassInitializeTestUnitPolling(ptr long)
 @ stdcall ClassSignalCompletion(ptr ptr ptr)
 @ stdcall ClassSendStartUnit(ptr)
 @ stdcall ClassSetMediaChangeState(ptr long long)
 @ stdcall ClassResetMediaChangeTimer(ptr)
 @ stdcall ClassCheckMediaState(ptr)
 @ stdcall ClassInitializeMediaChangeDetection(ptr ptr)
 @ stdcall ClassCleanupMediaChangeDetection(ptr)
 @ stdcall ClassEnableMediaChangeDetection(ptr)
 @ stdcall ClassDisableMediaChangeDetection(ptr)
 @ stdcall ClassSpinDownPowerHandler(ptr ptr)
 @ stdcall ClassStopUnitPowerHandler(ptr ptr)
 @ stdcall ClassAcquireChildLock(ptr)
 @ stdcall ClassReleaseChildLock(ptr)
 @ stdcall ClassScanForSpecial(ptr ptr ptr)
 @ stdcall ClassSetDeviceParameter(ptr ptr ptr long)
 @ stdcall ClassGetDeviceParameter(ptr ptr ptr ptr)

