Fraxinus User Manual {#fx_manual}
===========================================================

# Fraxinus User Manual {#fx_manual_second_heading}

Table of contents:

* \ref about_user_manual
* \ref description_Fraxinus
* \ref description_workflow
  * \ref description_newloadpatient
  * \ref description_import
  * \ref description_process
  * \ref description_pinpoint
  * \ref description_vbflythrough
  * \ref description_vbcutplanes
* \ref fx_troubleshooting
* \ref fx_advancedusers

## This document {#about_user_manual}

The Fraxinus procedure and setup is described in this document.

The help pages are context sensitive, meaning that the current page might change when the user clicks different places in the program.
Since these help documents are based on the underlying CustusX, much of the content might not be relevant for Fraxinus users.
To come back to this page, use the back/forward arrows at the top, or use the leftmost button
to bring up the contents. The Fraxinus spesific help pages will be at the top of the list.

Use the question mark at the top of the Fraxinus window to bring up the help pages in the different workflow steps.

## Description of the Fraxinus procedure {#description_Fraxinus}

Here is a short description of the workflow in Fraxinus. The workflow in Fraxinus is separated in workflow steps
which most of the time should be followed in order. Many actions will automatically take you to the next workflow step.
A more detailed description of each workflow step is given below.

At the top of the Fraxinus window you see the workflow toolbar. You can see which step you are currently at,
and you can change step by clicking on a given step. Hold the mouse over each step to see the name.

* New/Load Patient - When you start Fraxinus, you will be in the New/Load Patient step. You must create new patient the first time you use Fraxinus.
When you have selected a name and location, you will be taken to the next workflow step.

* Import - In the Import step you should first open your folder with the Dicom images. Then expand and select the series which you want to
import as a volume into your patient. When the importer is finished you are taken to the nest step.

* Process - For a new patient, the Process step should be fully automatic and you should see a timer which shows the progress on the calculations.
The Process step does a segmentation of the lung airways, creating a surface model, and creates a centerline through the branches.
Tests show that most lung models should be processed within a couple of minutes. When the calculations are done, you are taken to the next step.

* Pinpoint - In this step you can navigate in the 2D images of your volume to find a target for the virtual bronchoscopy.
Use a combination of clicking, draging and zooming to find your target with the crosshair. Click the *Set Target*
button and a route to the target will be created and you will be taken to the next step.

* Virtual Bronchoscopy Fly Through - This is the interesting step. Here you use the slider to move the camera
along the centerline from the entry of the trachea to your target. The camera is on the tip of the endoscope.
You can also see the tip in the 2D images and in relation to the surface model. There are two wheels
to rotate the endoscope and to tilt it left or right. From this step you can go back to the Pinpoint step
to look at new target paths or you can go to the last step for a different view. You can also go
to any of the other steps.

* Virtual Bronchoscopy Cut Planes - This step has the same slider and wheels as the previous step,
but it has another main 3D view. Here you can see the surface model of the lungs together with the
imported volume. The volume has been cut with a plane at the tip of the endoscope. From this
step you can go to any of the other steps.

## Detailed description of workflow steps {#description_workflow}

### New/Load Patient {#description_newloadpatient}

In this step you can create a new or load an old patient. There is also a button to *Restore factory settings*.
If you change some settings in the program which you are not able to undo, or the program for some reason
starts to behave badly, you can use this button to return the program to its original state.
This button deletes the folder *Fraxinus_settings* in your home folder. If anything seems to be wrong with
the program, deleting this folder and starting it again might help.


### Import {#description_import}

Import Dicom.
Mhd - toolbars.

### Process {#description_process}

manual seed
lung sack

### Pinpoint {#description_pinpoint}

blubb

### Virtual bronchoscopy fly through {#description_vbflythrough}

Manglende linje - vinkelrett på kamera.
piltaster

### Virtual Bronchoscopy Cut Planes {#description_vbcutplanes}

blblb

## Troubleshooting {#fx_troubleshooting}

Slette folder
gå litt fram og tilbake ved svart skjem etter load pat.
Sentrere bildet.

## For advanced users {#fx_advancedusers}

Remove fil menu
