; ================= Fraxinus post-uninstall =================
; Optional cleanup of the shared family folder (~/Fraxinus/virtualEnvironments,
; ~/Fraxinus/models) and the global Elastix install. Off by default: these
; installs are slow (can take tens of minutes) to redo, and are shared
; between Fraxinus and FraxinusExcelsior when both are installed, so removing
; them is opt-in only, never automatic.

MessageBox MB_YESNO|MB_ICONQUESTION "Also remove the downloaded AI models, Python virtual environments, and Elastix (several GB)?$\r$\n$\r$\nThese are shared with Fraxinus/FraxinusExcelsior if both are installed on this machine - removing them here removes them for both.$\r$\n$\r$\nThis cannot be undone, and re-installing later means re-running the slow setup steps again." /SD IDNO IDYES fxpu_remove_extras IDNO fxpu_keep_extras

fxpu_remove_extras:
  DetailPrint "Removing $PROFILE\Fraxinus\virtualEnvironments"
  RMDir /r "$PROFILE\Fraxinus\virtualEnvironments"
  DetailPrint "Removing $PROFILE\Fraxinus\models"
  RMDir /r "$PROFILE\Fraxinus\models"
  DetailPrint "Removing C:\Elastix"
  RMDir /r "C:\Elastix"
fxpu_keep_extras:

; ================= End Fraxinus post-uninstall =================
