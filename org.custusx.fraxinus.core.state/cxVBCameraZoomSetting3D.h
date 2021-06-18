#ifndef VB3DCAMERAZOOMSETTING_H
#define VB3DCAMERAZOOMSETTING_H

/**
 * @brief The VB3DCameraZoomSetting class
 * This zoom factor was found to be good to make the camera
 * zoom inside the trachea in the virtual braonchoscopy 3D view.
 */
class VB3DCameraZoomSetting
{
public:
	VB3DCameraZoomSetting();
	static int getZoomFactor();
};

#endif // VB3DCAMERAZOOMSETTING_H
