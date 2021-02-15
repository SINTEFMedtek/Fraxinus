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

#ifndef STRUCTURESSELECTIONWIDGET_H
#define STRUCTURESSELECTIONWIDGET_H


#include "org_custusx_fraxinus_widgets_Export.h"
#include <cxStructuresSelectionWidget.h>
#include "cxBaseWidget.h"
#include "cxForwardDeclarations.h"

class QPushButton;

namespace cx {

class org_custusx_fraxinus_widgets_EXPORT StructuresSelectionWidget : public BaseWidget
{
    Q_OBJECT
public:
    StructuresSelectionWidget(VisServicesPtr services, QWidget *parent = 0);
    virtual ~StructuresSelectionWidget();

    static QString getWidgetName();
    void setViewGroupNumber(unsigned int viewGroupNumber);
    void addLungObject(DataPtr object);
    void addLesionObject(DataPtr object);
    void addLymphNodeObject(DataPtr object);
    void addSpineObject(DataPtr object);
    void addSmallVesselObject(DataPtr object);
    void addVenaCavaObject(DataPtr object);
    void addAzygosObject(DataPtr object);
    void addAortaObject(DataPtr object);
    void addSubclavianObject(DataPtr object);
    void addHeartObject(DataPtr object);
    void addEsophagusObject(DataPtr object);
    void onEntry();


private slots:
    void viewLungsSlot();
    void viewLesionsSlot();
    void viewLymphNodesSlot();
    void viewSpineSlot();
    void viewSmallVesselsSlot();
    void viewVenaCavaSlot();
    void viewAzygosSlot();
    void viewAortaSlot();
    void viewSubclavianSlot();
    void viewHeartSlot();
    void viewEsophagusSlot();

private:
    void displayDataObjects(std::vector<DataPtr> objects);
    void hideDataObjects(std::vector<DataPtr> objects);

    VisServicesPtr mServices;
    unsigned int mViewGroupNumber;
    std::vector<DataPtr> mLungsObjects;
    std::vector<DataPtr> mLesionsObjects;
    std::vector<DataPtr> mLymphNodesObjects;
    std::vector<DataPtr> mSpineObjects;
    std::vector<DataPtr> mSmallVesselsObjects;
    std::vector<DataPtr> mVenaCavaObjects;
    std::vector<DataPtr> mAzygosObjects;
    std::vector<DataPtr> mAortaObjects;
    std::vector<DataPtr> mSubclavianObjects;
    std::vector<DataPtr> mHeartObjects;
    std::vector<DataPtr> mEsophagusObjects;

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
    QPushButton* mViewVenaCavaButton;
    QPalette mViewVenaCavaButtonBackgroundColor;
    bool mViewVenaCavaEnabled = false;
    QPushButton* mViewAzygosButton;
    QPalette mViewAzygosButtonBackgroundColor;
    bool mViewAzygosEnabled = false;
    QPushButton* mViewAortaButton;
    QPalette mViewAortaButtonBackgroundColor;
    bool mViewAortaEnabled = false;
    QPushButton* mViewSubclavianButton;
    QPalette mViewSubclavianButtonBackgroundColor;
    bool mViewSubclavianEnabled = false;
    QPushButton* mViewHeartButton;
    QPalette mViewHeartButtonBackgroundColor;
    bool mViewHeartEnabled = false;
    QPushButton* mViewEsophagusButton;
    QPalette mViewEsophagusButtonBackgroundColor;
    bool mViewEsophagusEnabled = false;
};


} //namespace cx


#endif //STRUCTURESSELECTIONWIDGET_H
