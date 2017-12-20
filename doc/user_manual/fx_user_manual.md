Fraxinus user manual {#fx_manual}
===========================================================

# Fraxinus user manual {#fx_manual_second_heading}

Table of contents:

* \ref about_user_manual
* \ref hardware_Fraxinus
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
to bring up the complete help content. The Fraxinus specific help pages will be at the top of the list.

Use the question mark at the top of the Fraxinus window to bring up the help pages in the different workflow steps.

## Hardware {#hardware_Fraxinus}

Fraxinus should be installed on a computer with a relatively good graphics card.


## Description of the Fraxinus procedure {#description_Fraxinus}

Here is a short introduction to the workflow in Fraxinus. The workflow is separated in workflow steps
which most of the time should be followed in order. Many actions will automatically take you to the next workflow step.
A more detailed description of each workflow step is given after this introduction in \ref description_workflow.

At the top of the Fraxinus window you see the workflow toolbar. You can see which step you are currently at,
and you can change step by clicking on a given step. Hold the mouse over each step to see the name.

* \ref description_newloadpatient - When you start Fraxinus, you will be in the *New/Load Patient* step. You must create a new patient the first time you use Fraxinus.
When you have selected a name and location, you will be taken to the next workflow step.

* \ref description_import - In the *Import* step you should first open your folder with the Dicom images. Next, expand and select the series which you want to
import as a volume into your patient. Click *Import*, and when the importer is finished you are taken to the nest step.

* \ref description_process - For a new patient, the *Process* step should be fully automatic and you should see a timer which shows the progress on the calculations.
The Process step does a segmentation of the lung airways, creating a surface model, and creates a centerline through the branches.
From the center line, it also creates a straight centerline model with tubes around to be used for the visualization.
Tests show that most lung volumes should be processed within a couple of minutes. When the calculations are done, you are taken to the next step.

* \ref description_pinpoint - In this step you can navigate in the 2D images of your volume to find a target for the virtual bronchoscopy.
Use a combination of clicking, draging and zooming to find your target with the crosshair. Click the *Set Target*
button and a route to the target will be created and you will be taken to the next step.

* \ref description_vbflythrough - Here you use the slider to move the camera
along the centerline from the entry of the trachea to your target.
You can also see the camera position in the 2D images and in relation to the surface model. There are two wheels
to rotate the endoscope and to tilt it left or right, and a button to reset the camera.
You can select to view the original lung volume or an artificial, transparent tube model of the airways.
There is a distance metric which gives you the direct distance, i.e. the distance in a straight line, between
your position and the target point.
From this step you can go back to the Pinpoint step
to look at new target paths or you can go to the last step for a different view. You can also go back
to any of the other steps.
See \ref description_vbflythrough for a list of keyboard shortcuts.

* \ref description_vbcutplanes - This step has the same controls as the previous step,
but it has switched the main 3D view. Here you can see the surface model of the lungs together with the
imported volume. The volume is cut with a plane at the tip of the endoscope. From this
step you can go to any of the other steps.

## Detailed description of the workflow steps {#description_workflow}

\addindex new_load_patient_widget
### New/Load Patient {#description_newloadpatient}

In this step you can create a new or load an old patient. There is also a button to *Restore factory settings*.
If you change some settings in the program which you are not able to undo, or the program for some reason
starts to behave badly, you can use this button to return the program to its original state.
This button deletes the folder *Fraxinus_settings* in your home folder. If anything seems to be wrong with
the program, deleting this folder and starting it again might help.

### Import {#description_import}

There is one important limitation in Fraxinus, it only works as intended if there is one and only one volume imported per patient
at a time. If you want to try to import a different CT series, you should start over by making a new patient.

If you have more than one Dicom series to choose between, select one with preferably slice thickness 2.0 mm or less.
For Siemens CT series, look for the name *Thorax 1.0 B30*.

By pressing on the *Advanced* button, you get access to a couple of features for getting more info from your DICOM images.
See also \ref org_custusx_dicom.

### Process {#description_process}

Usually this step is fully automatic when you perform the normal Fraxinus workflow
on a new patient. However, the segmentation algorithm might not always be able to segment a given input volume. Any number of reasons
can cause this, e.g. poor quality pictures. One thing which might help is to check the
*Use manual seed point* box. You then select a point in the start of the trachea which the algorithm should work from:
- Make sure that your volume which you want to segment is visible in the 2D and 3D view by right clicking and selecting it.
- Click in the 2D views and place the crosshair freely inside the trachea.
- Press the *Play* button, and hopefully the algorithm will succeed.

Here follows a general description of the options in this step.
The algorithm which creates a surface model from the imported volume, is implemented
through a CustusX concept called *Filter*. Hence we can se that the *Airway Segmentation Filter*
is selected.

In the *Input* field one can select the volume one wants to segment. After pressing the *Play*
button, the algorithm should run and the output will appear in the boxes:
- Airway centerline is the centerline through the trachea and the branches in the lung three.
- Airway segmentation is the surface model of the lung three.
- Lung segmentation: if the *Lung segmentation* checkbox is checked, this will make the algorithm
also do a segmentation of the lung sack.
- Straight centerline: If the *Straight centerline and tubes* checkbox is checked, you will get a
centerline with straight branches between the branch points.
- Straight airway tubes: If the *Straight centerline and tubes* checkbox is checked, you will get a
tube object which fits around the straight centerline.

You can use the *Eye* button next to the boxes to show or hide the respective objects in the 3D scene.
You can also right click in the 3D or 2D views to show or hide objects.
To delete an object, select it in a box and press the *Trashcan* button twice.

\addindex pinpoint_widget
### Pinpoint {#description_pinpoint}

Use this step to set a target for the virtual bronchoscopy. If the images get away from you, you can use the button with the crosshair
next to the target name to get them back to the center. You can change the target name if you want to
save routes to different targets for the next step.

\addindex fraxinus_virtual_bronchoscopy_widget
### Virtual bronchoscopy Fly Through {#description_vbflythrough}

Here you move the camera through the airways to your target. In the box you can select which route
to move the camera along, if you have chosen different names for your targets.
The route objects are named with *rtt* and the target name. Those named with *_ext*
indicate an extension from the center of the airways perpendicular out to the target. This line
is the blue line which you can see when you approach the closest point to the target inside the center of the airways.
To get the help lines, you must select the corresponding objects in the 3D scene by right clicking.
Usually it is easier to go back to the *Pinpoint* step and set the target again. A tip is to slide
the camera all the way to the target, so that you already are in the lower part of the lungs when you
go back to the images in the *Pinpoint* step.

The *Tubes* view let you fly through an artificial, transparent tube model of the airways, created around a straight centerline.
The camera path, however is based on the real centerline. Therefore you might see that the camera goes
outside the tubes at times, especially at the start of the trachea. The tubes are meant to give an additional
overview of the layout of the airways. One should go back and forth between the views to get comfortable with
the airways.

The slider, the wheels for the endoscope orientation, the reset button and the view can be controlled from the keyboard.
Use the arrows and page up/page down for the endoscope. Reset is at the *5* key and toggle the view with *V*,*T* and *7*.
One can also use the numpad. Make sure that the *Num lock* button is in the right setting.

You can zoom in the views by using either a mouse wheel or two fingers up or down on a mouse pad. Note that
the main view is zoomed in very much already, so you need quite much scrolling to zoom out.

If you are not happy with the color and/or texture of your volume, you can edit the transfer function.
See \ref fx_advancedusers.

### Virtual Bronchoscopy Cut Planes {#description_vbcutplanes}

See the previous step.

## Troubleshooting {#fx_troubleshooting}

- Most things needed to know to use Fraxinus are explained above.
- As a general recommendation, if something seems to be very wrong, try a restart of the program.
If that doesn't help, use the *Restore factory settings* button in the first step to clean the
*Fraxinus_settings folder* and restart the program.
- Try to create a new patient if something seems to be wrong with your patient.
- The Airway segmentation in the Process step might hang on certain unusual or "bad" Dicom series.
If so, you might have to restart the computer if nothing happens after several minutes.
- It has been seen that CT images with low resolution might give some strange twists in the camera path
along the branches.
- It has been seen that the 3D view in some steps might be empty upon entering the step.
In that case, going to another step and then back should fill the view.
- To make the arrows control the camera in the fly through view, it has been necessary to override all
other keyboard input in those steps. Meaning e.g. that the "R" button can't be used to center the image
in the steps containing this view.
- If the arrow keys seems to stop working, a single or more presses on the "Tab" key on the keyboard
might solve this. This has been seen on Mac after setting a new target.
Sometimes the side view of the endoscope might also hang. Rotating and moving back/forth might make
the side view work again.
- You might experience that the tail of the line you are navigating after in the Fly through step
gets a small curl. By zooming in more you can get rid of this curl.
- To get more output in possible error situations, run Fraxinus from a
command window: use the *Windows button + R*, type *cmd* and press enter. Here navigate to the folder
with your Fraxinus.exe and run the command:

    set_run_environment.bat Fraxinus.exe

## Appendix A: For advanced users {#fx_advancedusers}

*Fraxinus* is a layer on top of *CustusX* which gives a smooth and simple workflow for doing
virtual bronchoscopy. The user interface has been made as clean and simple as possible, and
everything which is needed for the users of Fraxinus is available by default.

However, for advanced users who want to have access to more functionality,
it is possible to right click on the top of the window to enable the rest of the widgets
and toolbars of CustusX.

To e.g. edit the transfer function of the volume in the Fly through step, right click and
open the *Volume Properties* widget. Go to the *Transfer Functions* tab and drag the values
or right click to remove or set new points. Right click on the color bar to customize the colors.
Note that going between workflow steps will reset many of the widget, help and object layouts to the
Fraxinus default settings.

To enable the menu bar with all the options from CustusX: open the file

    C:\Users\your_user_name\Fraxinus_settings\profiles\Bronchoscopy\settings\settings.ini

where you must use the *your_user_name* you have on your computer. Change *showMenuBar=false*
to *showMenuBar=true*. Then start the program. One thing this enables is e.g. import of the lung volume
from .mhd files. You will find more help on the CustusX features in other parts of the help system.

