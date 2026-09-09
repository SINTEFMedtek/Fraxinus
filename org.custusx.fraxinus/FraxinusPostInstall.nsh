; ================= Fraxinus post-install =================
; Runs the optional Raidionics/TotalSegmentator/Elastix setup scripts if the
; corresponding installer component was selected. Each script now logs its
; own output to %USERPROFILE%\Fraxinus\install_logs\ and exits
; non-zero on failure; previously a failure here was silent, since the
; console window these scripts run in closes as soon as they exit and
; nothing checked the exit code afterwards.

StrCpy $R8 ""   ; accumulates names of components whose setup script failed

SectionGetFlags ${RaidionicsSetup} $0
IntOp $0 $0 & ${SF_SELECTED}
IntCmp $0 0 fxpi_skip_raidionics
  DetailPrint "Running PowerShell: installRaidionics.ps1"
  ExecWait '"$WINDIR\System32\WindowsPowerShell\v1.0\powershell.exe" -ExecutionPolicy Bypass -NoLogo -NonInteractive -File "$INSTDIR\installRaidionics.ps1"' $1
  DetailPrint "  -> Raidionics exit code: $1"
  IntCmp $1 0 fxpi_skip_raidionics
    StrCpy $R8 "$R8- Raidionics$\r$\n"
fxpi_skip_raidionics:

SectionGetFlags ${TotalSegSetup} $2
IntOp $2 $2 & ${SF_SELECTED}
IntCmp $2 0 fxpi_skip_totalseg
  DetailPrint "Running PowerShell: installTotalsegmentator.ps1"
  ExecWait '"$WINDIR\System32\WindowsPowerShell\v1.0\powershell.exe" -ExecutionPolicy Bypass -NoLogo -NonInteractive -File "$INSTDIR\installTotalsegmentator.ps1"' $3
  DetailPrint "  -> TotalSegmentator exit code: $3"
  IntCmp $3 0 fxpi_skip_totalseg
    StrCpy $R8 "$R8- TotalSegmentator$\r$\n"
fxpi_skip_totalseg:

SectionGetFlags ${ElastixSetup} $4
IntOp $4 $4 & ${SF_SELECTED}
IntCmp $4 0 fxpi_skip_elastix
  DetailPrint "Running PowerShell: installElastix.ps1"
  ExecWait '"$WINDIR\System32\WindowsPowerShell\v1.0\powershell.exe" -ExecutionPolicy Bypass -NoLogo -NonInteractive -File "$INSTDIR\installElastix.ps1"' $5
  DetailPrint "  -> Elastix exit code: $5"
  IntCmp $5 0 fxpi_skip_elastix
    StrCpy $R8 "$R8- Elastix$\r$\n"
fxpi_skip_elastix:

; Report any failures to the user, since these console windows close
; immediately and the exit codes above are otherwise easy to miss.
StrCmp $R8 "" fxpi_no_failures
  MessageBox MB_ICONEXCLAMATION|MB_OK "The following optional Fraxinus component(s) failed to set up correctly:$\r$\n$\r$\n$R8$\r$\nFraxinus will still run, but the related feature(s) may not work until this is fixed.$\r$\n$\r$\nSee the log files under $PROFILE\${CX_FAMILY_FOLDER_NAME}\install_logs\ for details, or try re-running the corresponding install script manually from $INSTDIR."
fxpi_no_failures:

; ================= End Fraxinus post-install =================
