@ stdcall AcquireCredentialsHandleW(wstr wstr long ptr ptr ptr ptr ptr ptr)
@ stdcall AddAtomW(wstr)
@ stdcall AddFontResourceW(wstr)
@ stdcall AddJobW(long long ptr long ptr)
@ stdcall AddMonitorW(wstr long ptr)
@ stdcall AddPortW(wstr long wstr)
@ stdcall AddPrintProcessorW(wstr wstr wstr wstr)
@ stdcall AddPrintProvidorW(wstr long ptr)
@ stdcall AddPrinterDriverW(wstr long ptr)
@ stdcall AddPrinterW(wstr long ptr)
@ stdcall AdvancedDocumentPropertiesW(long long wstr ptr ptr)
@ stdcall AppendMenuW(long long long ptr)
@ stdcall BeginUpdateResourceA(str long)
@ stdcall BeginUpdateResourceW(wstr long)
@ stdcall BroadcastSystemMessageW(long ptr long long long)
@ stdcall BuildCommDCBAndTimeoutsW(wstr ptr ptr)
@ stdcall BuildCommDCBW(wstr ptr)
@ stdcall CallMsgFilterW(ptr long)
@ stdcall CallNamedPipeW(wstr ptr long ptr long ptr long)
@ stdcall CallWindowProcA(ptr long long long long)
@ stdcall CallWindowProcW(ptr long long long long)
@ stdcall ChangeDisplaySettingsExW(wstr ptr long long ptr)
@ stdcall ChangeDisplaySettingsW(ptr long)
@ stdcall ChangeMenuW(long long ptr long long)
@ stdcall CharLowerBuffW(wstr long)
@ stdcall CharLowerW(wstr)
@ stdcall CharNextW(wstr)
@ stdcall CharPrevW(wstr wstr)
@ stdcall CharToOemBuffW(wstr ptr long)
@ stdcall CharToOemW(wstr ptr)
@ stdcall CharUpperBuffW(wstr long)
@ stdcall CharUpperW(wstr)
@ stdcall ChooseColorW(ptr)
@ stdcall ChooseFontW(ptr)
@ stdcall CommConfigDialogW(wstr long ptr)
@ stdcall CompareStringW(long long wstr long wstr long)
@ stdcall ConfigurePortW(wstr long wstr)
@ stdcall CopyAcceleratorTableW(long ptr long)
@ stdcall CopyEnhMetaFileW(long wstr)
@ stdcall CopyFileExW (wstr wstr ptr ptr ptr long)
@ stdcall CopyFileW(wstr wstr long)
@ stdcall CopyMetaFileW(long wstr)
@ stdcall CreateAcceleratorTableW(ptr long)
@ stdcall CreateColorSpaceW(ptr)
@ stdcall CreateDCW(wstr wstr wstr ptr)
@ stdcall CreateDialogIndirectParamW(long ptr long ptr long)
@ stdcall CreateDialogParamW(long ptr long ptr long)
@ stdcall CreateDirectoryExW(wstr wstr ptr)
@ stdcall CreateDirectoryW(wstr ptr)
@ stdcall CreateEnhMetaFileW(long wstr ptr wstr)
@ stdcall CreateEventW(ptr long long wstr)
@ stdcall CreateFileMappingW(long ptr long long long wstr)
@ stdcall CreateFileW(wstr long long ptr long long long)
@ stdcall CreateFontIndirectW(ptr)
@ stdcall CreateFontW(long long long long long long long long long long long long long wstr)
@ stdcall CreateICW(wstr wstr wstr ptr)
@ stdcall CreateMDIWindowW(ptr ptr long long long long long long long long)
@ stdcall CreateMailslotW(ptr long long ptr)
@ stdcall CreateMetaFileW(wstr)
@ stdcall CreateMutexW(ptr long wstr)
@ stdcall CreateNamedPipeW(wstr long long long long long long ptr)
@ stdcall CreateProcessW(wstr wstr ptr ptr long long ptr wstr ptr ptr)
@ stdcall CreateScalableFontResourceW(long wstr wstr wstr)
@ stdcall CreateSemaphoreW(ptr long long wstr)
@ stub CreateStdAccessibleProxyW
@ stdcall CreateWaitableTimerW(ptr long wstr)
@ stdcall CreateWindowExW(long wstr wstr long long long long long long long long ptr)
@ stdcall CryptAcquireContextW(ptr wstr wstr long long)
@ stdcall CryptEnumProviderTypesW(long ptr long ptr ptr ptr)
@ stdcall CryptEnumProvidersW(long ptr long ptr ptr ptr)
@ stdcall CryptGetDefaultProviderW(long ptr long ptr ptr)
@ stdcall CryptSetProviderExW(wstr long ptr long)
@ stdcall CryptSetProviderW(wstr long)
@ stdcall CryptSignHashW(long long ptr long ptr ptr)
@ stdcall CryptVerifySignatureW(long ptr long long ptr long)
@ stdcall DdeConnect(long long long ptr)
@ stdcall DdeConnectList(long long long long ptr)
@ stdcall DdeCreateStringHandleW(long ptr long)
@ stdcall DdeInitializeW(ptr ptr long long)
@ stdcall DdeQueryConvInfo(long long ptr)
@ stdcall DdeQueryStringW(long long ptr long long)
@ stdcall DefDlgProcW(long long long long)
@ stdcall DefFrameProcW(long long long long long)
@ stdcall DefMDIChildProcW(long long long long)
@ stdcall DefWindowProcW(long long long long)
@ stdcall DeleteFileW(wstr)
@ stdcall DeleteMonitorW(wstr wstr wstr)
@ stdcall DeletePortW(wstr long wstr)
@ stdcall DeletePrintProcessorW(wstr wstr wstr)
@ stdcall DeletePrintProvidorW(wstr wstr wstr)
@ stdcall DeletePrinterDriverW(wstr wstr wstr)
@ stdcall DeviceCapabilitiesW(wstr wstr long wstr ptr)
@ stdcall DialogBoxIndirectParamW(long ptr long ptr long)
@ stdcall DialogBoxParamW(long wstr long ptr long)
@ stdcall DispatchMessageW(ptr)
@ stdcall DlgDirListComboBoxW(long ptr long long long)
@ stdcall DlgDirListW(long wstr long long long)
@ stdcall DlgDirSelectComboBoxExW(long ptr long long)
@ stdcall DlgDirSelectExW(long ptr long long)
@ stdcall DocumentPropertiesW(long long ptr ptr ptr long)
@ stdcall DragQueryFileW(long long ptr long)
@ stdcall DrawStateW(long long ptr long long long long long long long)
@ stdcall DrawTextExW(long wstr long ptr long ptr)
@ stdcall DrawTextW(long wstr long ptr long)
@ stdcall EnableWindow(long long)
@ stdcall EndUpdateResourceA(long long)
@ stdcall EndUpdateResourceW(long long)
@ stdcall EnumCalendarInfoExW(ptr long long long)
@ stdcall EnumCalendarInfoW(ptr long long long)
@ stdcall EnumClipboardFormats(long)
@ stdcall EnumDateFormatsExW(ptr long long)
@ stdcall EnumDateFormatsW(ptr long long)
@ stdcall EnumDisplayDevicesW(ptr long ptr long)
@ stdcall EnumDisplaySettingsExW(wstr long ptr long)
@ stdcall EnumDisplaySettingsW(wstr long ptr )
@ stdcall EnumFontFamiliesExW(long ptr ptr long long)
@ stdcall EnumFontFamiliesW(long wstr ptr long)
@ stdcall EnumFontsW(long wstr ptr long)
@ stdcall EnumICMProfilesW(long ptr long)
@ stdcall EnumMonitorsW(wstr long ptr long long long)
@ stdcall EnumPortsW(wstr long ptr long ptr ptr)
@ stdcall EnumPrintProcessorDatatypesW(wstr wstr long ptr long ptr ptr)
@ stdcall EnumPrintProcessorsW(wstr wstr long ptr long ptr ptr)
@ stdcall EnumPrinterDriversW(wstr wstr long ptr long ptr ptr)
@ stdcall EnumPrintersW(long ptr long ptr long ptr ptr)
@ stdcall EnumPropsA(long ptr)
@ stdcall EnumPropsExA(long ptr long)
@ stdcall EnumPropsExW(long ptr long)
@ stdcall EnumPropsW(long ptr)
@ stdcall EnumSystemCodePagesW(ptr long)
@ stdcall EnumSystemLocalesW(ptr long)
@ stdcall EnumTimeFormatsW(ptr long long)
@ stdcall EnumerateSecurityPackagesW(ptr ptr)
@ stdcall ExpandEnvironmentStringsW(wstr ptr long)
@ stdcall ExtTextOutW(long long long long ptr wstr long ptr)
@ stdcall ExtractIconExW(wstr long ptr ptr long)
@ stdcall ExtractIconW(long wstr long)
@ stdcall FatalAppExitW(long wstr)
@ stdcall FillConsoleOutputCharacterW(long long long long ptr)
@ stdcall FindAtomW(wstr)
@ stdcall FindExecutableW(wstr wstr wstr)
@ stdcall FindFirstChangeNotificationW(wstr long long)
@ stdcall FindFirstFileW(wstr ptr)
@ stdcall FindNextFileW(long ptr)
@ stdcall FindResourceExW(long wstr wstr long)
@ stdcall FindResourceW(long wstr wstr)
@ stdcall FindTextW(ptr)
@ stdcall FindWindowExW(long long wstr wstr)
@ stdcall FindWindowW(wstr wstr)
@ stdcall FormatMessageW(long ptr long long ptr long ptr)
@ stdcall FreeContextBuffer(ptr)
@ stdcall FreeEnvironmentStringsW(ptr)
@ stdcall GetAltTabInfoW(long long ptr ptr long)
@ stdcall GetAtomNameW(long ptr long)
@ stdcall GetCPInfo(long ptr)
@ stdcall GetCPInfoExW(long long ptr)
@ stdcall GetCalendarInfoW(long long long ptr long ptr)
@ stdcall GetCharABCWidthsFloatW(long long long ptr)
@ stdcall GetCharABCWidthsW(long long long ptr)
@ stdcall GetCharWidth32W(long long long long)
@ stdcall GetCharWidthFloatW(long long long ptr)
@ stdcall GetCharWidthW(long long long long)
@ stdcall GetCharacterPlacementW(long wstr long long ptr long)
@ stdcall GetClassInfoExW(long wstr ptr)
@ stdcall GetClassInfoW(long wstr ptr)
@ stdcall GetClassLongW(long long)
@ stdcall GetClassNameW(long ptr long)
@ stdcall GetClipboardData(long)
@ stdcall GetClipboardFormatNameW(long ptr long)
@ stdcall GetComputerNameW(ptr ptr)
@ stdcall GetConsoleTitleW(ptr long)
@ stdcall GetCurrencyFormatW(long long str ptr str long)
@ stdcall GetCurrentDirectoryW(long ptr)
@ stdcall GetCurrentHwProfileW(ptr)
@ stdcall GetDateFormatW(long long ptr wstr ptr long)
@ stdcall GetDefaultCommConfigW(wstr ptr long)
@ stdcall GetDiskFreeSpaceExW (wstr ptr ptr ptr)
@ stdcall GetDiskFreeSpaceW(wstr ptr ptr ptr ptr)
@ stdcall GetDlgItemTextW(long long ptr long)
@ stdcall GetDriveTypeW(wstr)
@ stdcall GetEnhMetaFileDescriptionW(long long ptr)
@ stdcall GetEnhMetaFileW(wstr)
@ stdcall GetEnvironmentStringsW()
@ stdcall GetEnvironmentVariableW(wstr ptr long)
@ stdcall GetFileAttributesExW(wstr long ptr)
@ stdcall GetFileAttributesW(wstr)
@ stdcall GetFileTitleW(wstr ptr long)
@ stdcall GetFileVersionInfoSizeW(wstr ptr)
@ stdcall GetFileVersionInfoW(wstr long long ptr)
@ stdcall GetFullPathNameW(wstr long ptr ptr)
@ stdcall GetGlyphOutlineW(long long long ptr long ptr ptr)
@ stdcall GetICMProfileW(long ptr wstr)
@ stdcall GetJobW(long long long ptr long ptr)
@ stdcall GetKerningPairsW(long long ptr)
@ stdcall GetKeyNameTextW(long ptr long)
@ stdcall GetKeyboardLayoutNameW(ptr)
@ stdcall GetLocaleInfoW(long long ptr long)
@ stdcall GetLogColorSpaceW(long ptr long)
@ stdcall GetLogicalDriveStringsW(long ptr)
@ stdcall GetLongPathNameW (wstr long long)
@ stdcall GetMenuItemInfoW(long long long ptr)
@ stdcall GetMenuStringW(long long ptr long long)
@ stdcall GetMessageW(ptr long long long)
@ stdcall GetMetaFileW(wstr)
@ stdcall GetModuleFileNameW(long ptr long)
@ stdcall GetModuleHandleW(wstr)
@ stdcall GetMonitorInfoW(long ptr)
@ stdcall GetNamedPipeHandleStateW(long ptr ptr ptr ptr wstr long)
@ stdcall GetNumberFormatW(long long wstr ptr ptr long)
@ stdcall GetObjectW(long long ptr)
@ stdcall GetOpenFileNamePreviewW(ptr)
@ stdcall GetOpenFileNameW(ptr)
@ stdcall GetOutlineTextMetricsW(long long ptr)
@ stdcall GetPrintProcessorDirectoryW(wstr wstr long ptr long ptr)
@ stdcall GetPrinterDataW(long wstr ptr ptr long ptr)
@ stdcall GetPrinterDriverDirectoryW(wstr wstr long ptr long ptr)
@ stdcall GetPrinterDriverW(long str long ptr long ptr)
@ stdcall GetPrinterW(long long ptr long ptr)
@ stdcall GetPrivateProfileIntW(wstr wstr long wstr)
@ stdcall GetPrivateProfileSectionNamesW(ptr long wstr)
@ stdcall GetPrivateProfileSectionW(wstr ptr long wstr)
@ stdcall GetPrivateProfileStringW(wstr wstr wstr ptr long wstr)
@ stdcall GetPrivateProfileStructW(wstr wstr ptr long wstr)
@ stdcall GetProcAddress(long str)
@ stdcall GetProfileIntW(wstr wstr long)
@ stdcall GetProfileSectionW(wstr ptr long)
@ stdcall GetProfileStringW(wstr wstr wstr ptr long)
@ stdcall GetPropA(long str)
@ stdcall GetPropW(long wstr)
@ stub GetRoleTextW
@ stdcall GetSaveFileNamePreviewW(ptr)
@ stdcall GetSaveFileNameW(ptr)
@ stdcall GetShortPathNameW(wstr ptr long)
@ stdcall GetStartupInfoW(ptr)
@ stub GetStateTextW
@ stdcall GetStringTypeExW(long long wstr long ptr)
@ stdcall GetStringTypeW(long wstr long ptr)
@ stdcall GetSystemDirectoryW(ptr long)
@ stdcall GetSystemWindowsDirectoryW(ptr long)
@ stdcall GetTabbedTextExtentW(long wstr long long ptr)
@ stdcall GetTempFileNameW(wstr wstr long ptr)
@ stdcall GetTempPathW(long ptr)
@ stdcall GetTextExtentExPointW(long wstr long long ptr ptr ptr)
@ stdcall GetTextExtentPoint32W(long wstr long ptr)
@ stdcall GetTextExtentPointW(long wstr long ptr)
@ stdcall GetTextFaceW(long long ptr)
@ stdcall GetTextMetricsW(long ptr)
@ stdcall GetTimeFormatW(long long ptr wstr ptr long)
@ stdcall GetUserNameW(ptr ptr)
@ stdcall GetVersionExW(ptr)
@ stdcall GetVolumeInformationW(wstr ptr long ptr ptr ptr ptr long)
@ stdcall GetWindowLongA(long long)
@ stdcall GetWindowLongW(long long)
@ stdcall GetWindowModuleFileNameW(long ptr long)
@ stdcall GetWindowTextLengthW(long)
@ stdcall GetWindowTextW(long ptr long)
@ stdcall GetWindowsDirectoryW(ptr long)
@ stdcall GlobalAddAtomW(wstr)
@ stdcall GlobalFindAtomW(wstr)
@ stdcall GlobalGetAtomNameW(long ptr long)
@ stdcall GrayStringW(long long ptr long long long long long long)
@ stdcall InitSecurityInterfaceW()
@ stdcall InitializeSecurityContextW(ptr ptr wstr long long long ptr long ptr ptr ptr ptr)
@ stdcall InsertMenuItemW(long long long ptr)
@ stdcall InsertMenuW(long long long long ptr)
@ stdcall IsBadStringPtrW(ptr long)
@ stdcall IsCharAlphaNumericW(long)
@ stdcall IsCharAlphaW(long)
@ stdcall IsCharLowerW(long)
@ stdcall IsCharUpperW(long)
@ stdcall IsClipboardFormatAvailable(long)
@ stdcall IsDestinationReachableW(wstr ptr)
@ stdcall IsDialogMessageW(long ptr)
@ stdcall IsTextUnicode(ptr long ptr)
@ stdcall IsValidCodePage(long)
@ stdcall IsWindowUnicode(long)
@ stdcall LCMapStringW(long long wstr long ptr long)
@ stdcall LoadAcceleratorsW(long wstr)
@ stdcall LoadBitmapW(long wstr)
@ stdcall LoadCursorFromFileW(wstr)
@ stdcall LoadCursorW(long wstr)
@ stdcall LoadIconW(long wstr)
@ stdcall LoadImageW(long wstr long long long long)
@ stdcall LoadKeyboardLayoutW(wstr long)
@ stdcall LoadLibraryExW(wstr long long)
@ stdcall LoadLibraryW(wstr)
@ stdcall LoadMenuIndirectW(ptr)
@ stdcall LoadMenuW(long wstr)
@ stdcall LoadStringW(long long ptr long)
@ cdecl   MCIWndCreateW(long long long wstr)
@ stdcall MapVirtualKeyExW(long long long)
@ stdcall MapVirtualKeyW(long long)
@ stdcall MessageBoxExW(long wstr wstr long long)
@ stdcall MessageBoxIndirectW(ptr)
@ stdcall MessageBoxW(long wstr wstr long)
@ stdcall ModifyMenuW(long long long long ptr)
@ stdcall MoveFileW(wstr wstr)
@ stdcall MultiByteToWideChar(long long str long ptr long)
@ stdcall MultinetGetConnectionPerformanceW(ptr ptr)
@ stdcall OemToCharBuffW(ptr ptr long)
@ stdcall OemToCharW(ptr ptr)
@ stdcall OleUIAddVerbMenuW(ptr wstr long long long long long long ptr)
@ stdcall OleUIBusyW(ptr)
@ stdcall OleUIChangeIconW(ptr)
@ stdcall OleUIChangeSourceW(ptr)
@ stdcall OleUIConvertW(ptr)
@ stdcall OleUIEditLinksW(ptr)
@ stdcall OleUIInsertObjectW(ptr)
@ stdcall OleUIObjectPropertiesW(ptr)
@ stdcall OleUIPasteSpecialW(ptr)
@ varargs OleUIPromptUserW(long long)
@ stdcall OleUIUpdateLinksW(ptr long wstr long)
@ stdcall OpenEventW(long long wstr)
@ stdcall OpenFileMappingW(long long wstr)
@ stdcall OpenMutexW(long long wstr)
@ stdcall OpenPrinterW(wstr ptr ptr)
@ stdcall OpenSemaphoreW(long long wstr)
@ stdcall OpenWaitableTimerW(long long wstr)
@ stdcall OutputDebugStringW(wstr)
@ stdcall PageSetupDlgW(ptr)
@ stdcall PeekConsoleInputW(ptr ptr long ptr)
@ stdcall PeekMessageW(ptr long long long long)
@ stdcall PlaySoundW(ptr long long)
@ stdcall PolyTextOutW(long ptr long)
@ stdcall PostMessageW(long long long long)
@ stdcall PostThreadMessageW(long long long long)
@ stdcall PrintDlgW(ptr)
@ stdcall QueryContextAttributesW(ptr long ptr)
@ stdcall QueryCredentialsAttributesW(ptr long ptr)
@ stdcall QueryDosDeviceW(wstr ptr long)
@ stdcall QuerySecurityPackageInfoW(wstr ptr)
@ stdcall RasConnectionNotificationW(long long long)
@ stdcall RasCreatePhonebookEntryW(long wstr)
@ stdcall RasDeleteEntryW(wstr wstr)
@ stdcall RasDeleteSubEntryW(wstr wstr long)
@ stdcall RasDialW(ptr wstr ptr long ptr ptr)
@ stdcall RasEditPhonebookEntryW(long wstr wstr)
@ stdcall RasEnumConnectionsW(ptr ptr ptr)
@ stdcall RasEnumDevicesW(ptr ptr ptr)
@ stdcall RasEnumEntriesW(str str ptr ptr ptr)
@ stdcall RasGetConnectStatusW(long ptr)
@ stdcall RasGetEntryDialParamsW(wstr ptr ptr)
@ stdcall RasGetEntryPropertiesW(wstr wstr ptr ptr ptr ptr)
@ stdcall RasGetErrorStringW(long ptr long)
@ stdcall RasGetProjectionInfoW(long long ptr ptr)
@ stdcall RasHangUpW(long)
@ stdcall RasRenameEntryW(wstr wstr wstr)
@ stdcall RasSetEntryDialParamsW(wstr ptr long)
@ stdcall RasSetEntryPropertiesW(wstr wstr ptr long ptr long)
@ stdcall RasSetSubEntryPropertiesW(wstr wstr long ptr long ptr long)
@ stdcall RasValidateEntryNameW(wstr wstr)
@ stdcall ReadConsoleInputW(long ptr long ptr)
@ stdcall ReadConsoleOutputCharacterW(long ptr long long ptr)
@ stdcall ReadConsoleOutputW(long ptr long long ptr)
@ stdcall ReadConsoleW(long ptr long ptr ptr)
@ stdcall RegConnectRegistryW(wstr long ptr)
@ stdcall RegCreateKeyExW(long wstr long ptr long long ptr ptr ptr)
@ stdcall RegCreateKeyW(long wstr ptr)
@ stdcall RegDeleteKeyW(long wstr)
@ stdcall RegDeleteValueW(long wstr)
@ stdcall RegEnumKeyExW(long long ptr ptr ptr ptr ptr ptr)
@ stdcall RegEnumKeyW(long long ptr long)
@ stdcall RegEnumValueW(long long ptr ptr ptr ptr ptr ptr)
@ stdcall RegLoadKeyW(long wstr wstr)
@ stdcall RegOpenKeyExW(long wstr long long ptr)
@ stdcall RegOpenKeyW(long wstr ptr)
@ stdcall RegQueryInfoKeyW(long ptr ptr ptr ptr ptr ptr ptr ptr ptr ptr ptr)
@ stdcall RegQueryMultipleValuesW(long ptr long ptr ptr)
@ stdcall RegQueryValueExW(long wstr ptr ptr ptr ptr)
@ stdcall RegQueryValueW(long wstr ptr ptr)
@ stdcall RegReplaceKeyW(long wstr wstr wstr)
@ stdcall RegSaveKeyW(long ptr ptr)
@ stdcall RegSetValueExW(long wstr long long ptr long)
@ stdcall RegSetValueW(long wstr long ptr long)
@ stdcall RegUnLoadKeyW(long wstr)
@ stdcall RegisterClassExW(ptr)
@ stdcall RegisterClassW(ptr)
@ stdcall RegisterClipboardFormatW(wstr)
@ stdcall RegisterDeviceNotificationW(long ptr long)
@ stdcall RegisterWindowMessageW(wstr)
@ stdcall RemoveDirectoryW(wstr)
@ stdcall RemoveFontResourceW(wstr)
@ stdcall RemovePropA(long str)
@ stdcall RemovePropW(long wstr)
@ stdcall ReplaceTextW(ptr)
@ stdcall ResetDCW(long ptr)
@ stdcall ResetPrinterW(long ptr)
@ stdcall SHBrowseForFolderW(ptr)
@ stdcall SHChangeNotify (long long ptr ptr)
@ stdcall SHFileOperationW(ptr)
@ stdcall SHGetFileInfoW(ptr long ptr long long)
@ stdcall SHGetNewLinkInfoW(wstr wstr ptr long long)
@ stdcall SHGetPathFromIDListW(ptr ptr)
@ stdcall ScrollConsoleScreenBufferW(long ptr ptr ptr ptr)
@ stdcall SearchPathW(wstr wstr wstr long ptr ptr)
@ stdcall SendDlgItemMessageW(long long long long long)
@ stdcall SendMessageCallbackW(long long long long ptr long)
@ stdcall SendMessageTimeoutW(long long long long long long ptr)
@ stdcall SendMessageW(long long long long)
@ stdcall SendNotifyMessageW(long long long long)
@ stdcall SetCalendarInfoW(long long long wstr)
@ stdcall SetClassLongW(long long long)
@ stdcall SetComputerNameW(wstr)
@ stdcall SetConsoleTitleW(wstr)
@ stdcall SetCurrentDirectoryW(wstr)
@ stdcall SetDefaultCommConfigW(wstr ptr long)
@ stdcall SetDlgItemTextW(long long wstr)
@ stdcall SetEnvironmentVariableW(wstr wstr)
@ stdcall SetFileAttributesW(wstr long)
@ stdcall SetICMProfileW(long wstr)
@ stdcall SetJobW(long long long ptr long)
@ stdcall SetLocaleInfoW(long long wstr)
@ stdcall SetMenuItemInfoW(long long long ptr)
@ stdcall SetPrinterDataW(long wstr long ptr long)
@ stdcall SetPrinterW(long long ptr long)
@ stdcall SetPropA(long str long)
@ stdcall SetPropW(long wstr long)
@ stdcall SetVolumeLabelW(wstr wstr)
@ stdcall SetWindowLongA(long long long)
@ stdcall SetWindowLongW(long long long)
@ stdcall SetWindowTextW(long wstr)
@ stdcall SetWindowsHookExW(long long long long)
@ stdcall SetWindowsHookW(long ptr)
@ stdcall ShellAboutW(long wstr wstr long)
@ stdcall ShellExecuteExW (long)
@ stdcall ShellExecuteW (long wstr wstr wstr wstr long)
@ stdcall Shell_NotifyIconW(long ptr)
@ stdcall StartDocPrinterW(long long ptr)
@ stdcall StartDocW(long ptr)
@ stdcall SystemParametersInfoW(long long ptr long)
@ stdcall TabbedTextOutW(long long long wstr long long ptr long)
@ stdcall TextOutW(long long long wstr long)
@ stdcall TranslateAcceleratorW(long long ptr)
@ stdcall UnregisterClassW(wstr long)
@ stdcall UpdateICMRegKeyW(long wstr wstr long)
@ stdcall UpdateResourceA(long str str long ptr long)
@ stdcall UpdateResourceW(long wstr wstr long ptr long)
@ stdcall VerFindFileW(long wstr wstr wstr ptr ptr ptr ptr)
@ stdcall VerInstallFileW(long wstr wstr wstr wstr wstr ptr ptr)
@ stdcall VerLanguageNameW(long wstr long)
@ stdcall VerQueryValueW(ptr wstr ptr ptr)
@ stdcall VkKeyScanExW(long long)
@ stdcall VkKeyScanW(long)
@ stdcall WNetAddConnection2W(ptr wstr wstr long)
@ stdcall WNetAddConnection3W(long ptr wstr wstr long)
@ stdcall WNetAddConnectionW(wstr wstr wstr)
@ stdcall WNetCancelConnection2W(wstr long long)
@ stdcall WNetCancelConnectionW(wstr long)
@ stdcall WNetConnectionDialog1W(ptr)
@ stdcall WNetDisconnectDialog1W(ptr)
@ stdcall WNetEnumResourceW(long ptr ptr ptr)
@ stdcall WNetGetConnectionW(wstr ptr ptr)
@ stdcall WNetGetLastErrorW(ptr ptr long ptr long)
@ stdcall WNetGetNetworkInformationW(wstr ptr)
@ stdcall WNetGetProviderNameW(long ptr ptr)
@ stdcall WNetGetResourceInformationW(ptr ptr ptr ptr)
@ stdcall WNetGetResourceParentW(ptr ptr ptr)
@ stdcall WNetGetUniversalNameW (wstr long ptr ptr)
@ stdcall WNetGetUserW(wstr wstr ptr)
@ stdcall WNetOpenEnumW(long long long ptr ptr)
@ stdcall WNetUseConnectionW(long ptr wstr wstr long wstr ptr ptr)
@ stdcall WaitNamedPipeW (wstr long)
@ stdcall WideCharToMultiByte(long long wstr long ptr long ptr ptr)
@ stdcall WinHelpW(long wstr long long)
@ stdcall WriteConsoleInputW(long ptr long ptr)
@ stdcall WriteConsoleOutputCharacterW(long ptr long long ptr)
@ stdcall WriteConsoleOutputW(long ptr long long ptr)
@ stdcall WriteConsoleW(long ptr long ptr ptr)
@ stdcall WritePrivateProfileSectionW(wstr wstr wstr)
@ stdcall WritePrivateProfileStringW(wstr wstr wstr wstr)
@ stdcall WritePrivateProfileStructW(wstr wstr ptr long wstr)
@ stdcall WriteProfileSectionW(str str)
@ stdcall WriteProfileStringW(wstr wstr wstr)
@ stub __FreeAllLibrariesInMsluDll
@ stdcall auxGetDevCapsW(long ptr long)
@ stdcall capCreateCaptureWindowW(wstr long long long long long long long)
@ stdcall capGetDriverDescriptionW(long ptr long ptr long)
@ stdcall joyGetDevCapsW(long ptr long)
@ stdcall lstrcatW(wstr wstr)
@ stdcall lstrcmpW(wstr wstr)
@ stdcall lstrcmpiW(wstr wstr)
@ stdcall lstrcpyW(ptr wstr)
@ stdcall lstrcpynW(ptr wstr long)
@ stdcall lstrlenW(wstr)
@ stdcall mciGetDeviceIDW(str)
@ stdcall mciGetErrorStringW(long ptr long)
@ stdcall mciSendCommandW(long long long long)
@ stdcall mciSendStringW(wstr ptr long long)
@ stdcall midiInGetDevCapsW(long ptr long)
@ stdcall midiInGetErrorTextW(long ptr long)
@ stdcall midiOutGetDevCapsW(long ptr long)
@ stdcall midiOutGetErrorTextW(long ptr long)
@ stdcall mixerGetControlDetailsW(long ptr long)
@ stdcall mixerGetDevCapsW(long ptr long)
@ stdcall mixerGetLineControlsW(long ptr long)
@ stdcall mixerGetLineInfoW(long ptr long)
@ stdcall mmioInstallIOProcW(long ptr long)
@ stdcall mmioOpenW(wstr ptr long)
@ stdcall mmioRenameW(wstr wstr ptr long)
@ stdcall mmioStringToFOURCCW(wstr long)
@ stdcall sndPlaySoundW(ptr long)
@ stdcall waveInGetDevCapsW(long ptr long)
@ stdcall waveInGetErrorTextW(long ptr long)
@ stdcall waveOutGetDevCapsW(long ptr long)
@ stdcall waveOutGetErrorTextW(long ptr long)
@ varargs wsprintfW(wstr wstr)
@ stdcall wvsprintfW(ptr wstr ptr)
