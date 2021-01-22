/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) 2008-2014, SINTEF Department of Medical Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=========================================================================*/

#ifndef FRAXINUSVBWIDGET_H
#define FRAXINUSVBWIDGET_H


#include "org_custusx_fraxinus_widgets_Export.h"
#include <cxVBWidget.h>

class QRadioButton;
class QLabel;
class QPushButton;

namespace cx {

class org_custusx_fraxinus_widgets_EXPORT FraxinusVBWidget : public VBWidget
{
    Q_OBJECT
public:
    FraxinusVBWidget(VisServicesPtr services, QWidget *parent = 0);
    virtual ~FraxinusVBWidget();

    static QString getWidgetName();
    void setViewGroupNumber(unsigned int viewGroupNumber);
    void addObjectToVolumeView(DataPtr object);
    void addObjectToTubeView(DataPtr object);
    void addLungObject(DataPtr object);
    void addLesionObject(DataPtr object);
    void addLymphNodeObject(DataPtr object);
    void addSpineObject(DataPtr object);
    void addSmallVesselObject(DataPtr object);
    void addLargeVesselObject(DataPtr object);
    void addHeartObject(DataPtr object);
    void addEsophagusObject(DataPtr object);


private slots:
    virtual void keyPressEvent(QKeyEvent* event);
    void calculateRouteLength();
    void playbackSliderChanged(int cameraPositionInPercent);
    void viewLungsSlot();
    void viewLesionsSlot();
    void viewLymphNodesSlot();
    void viewSpineSlot();
    void viewSmallVesselsSlot();
    void viewLargeVesselsSlot();
    void viewHeartSlot();
    void viewEsophagusSlot();

private:
    void displayVolume();
    void displayTubes();
    void displayDataObjects(std::vector<DataPtr> objects);
    void hideDataObjects(std::vector<DataPtr> objects);
    void updateRttInfo(double cameraPositionInPercent);
    void updateAirwaysOpacity(double cameraPositionInPercent);
    void calculateDistanceFromRouteEndToTarget(Eigen::Vector3d routeEndpoint);
    QString createDistanceFromPathToTargetText();
    double getTargetDistance();
    double getRemainingRouteInsideAirways(double cameraPositionInPercent);

    VisServicesPtr mServices;
    QRadioButton* mVolumeButton;
    QRadioButton* mTubeButton;
    unsigned int mViewGroupNumber;
    std::vector<DataPtr> mVolumeViewObjects;
    std::vector<DataPtr> mTubeViewObjects;
    std::vector<DataPtr> mLungsObjects;
    std::vector<DataPtr> mLesionsObjects;
    std::vector<DataPtr> mLymphNodesObjects;
    std::vector<DataPtr> mSpineObjects;
    std::vector<DataPtr> mSmallVesselsObjects;
    std::vector<DataPtr> mLargeVesselsObjects;
    std::vector<DataPtr> mHeartObjects;
    std::vector<DataPtr> mEsophagusObjects;
    QLabel* mStaticTotalLegth;
    QLabel* mRemainingRttLegth;
    QLabel* mDirectDistance;
    QLabel* mDistanceToTarget;
    QLabel* mWarningLabel;
    double mRouteLength;
    double mDistanceFromPathEndToTarget;

    QPushButton* mViewLungsButton;
    QPalette mViewLungsButtonBackgroundColor;
    bool mViewLungsEnabled = false;
    QPushButton* mViewLesionsButton;
    QPalette mViewLesionsButtonBackgroundColor;
    bool mViewLesionsEnabled = false;
    QPushButton* mViewLymphNodesButton;
    QPalette mViewLymphNodesButtonBackgroundColor;
    bool mViewLymphNodesEnabled = false;
    QPushButton* mViewSpineButton;
    QPalette mViewSpineButtonBackgroundColor;
    bool mViewSpineEnabled = false;
    QPushButton* mViewSmallVesselsButton;
    QPalette mViewSmallVesselsButtonBackgroundColor;
    bool mViewSmallVesselsEnabled = false;
    QPushButton* mViewLargeVesselsButton;
    QPalette mViewLargeVesselsButtonBackgroundColor;
    bool mViewLargeVesselsEnabled = false;
    QPushButton* mViewHeartButton;
    QPalette mViewHeartButtonBackgroundColor;
    bool mViewHeartEnabled = false;
    QPushButton* mViewEsophagusButton;
    QPalette mViewEsophagusButtonBackgroundColor;
    bool mViewEsophagusEnabled = false;
};


} //namespace cx


#endif //FRAXINUSVBWIDGET_H
