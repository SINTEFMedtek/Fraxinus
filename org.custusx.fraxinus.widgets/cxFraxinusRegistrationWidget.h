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

#ifndef FRAXINUSREGISTRATIONWIDGET_H
#define FRAXINUSREGISTRATIONWIDGET_H


#include "org_custusx_fraxinus_widgets_Export.h"
#include <cxStructuresSelectionWidget.h>
#include <cxFraxinusPatientOrientationWidget.h>
#include "cxBaseWidget.h"
#include "cxForwardDeclarations.h"
#include "cxTrackerConfiguration.h"
#include "cxXmlOptionItem.h"

class QRadioButton;
class QLabel;
class QPushButton;
class QComboBox;

namespace cx {
class RecordTrackingWidget;
class WidgetObscuredListener;
typedef boost::shared_ptr<class StringPropertySelectMesh> StringPropertySelectMeshPtr;
typedef boost::shared_ptr<class BronchoscopyRegistration> BronchoscopyRegistrationPtr;

class org_custusx_fraxinus_widgets_EXPORT FraxinusRegistrationWidget : public BaseWidget
{
    Q_OBJECT

    BronchoscopyRegistrationPtr mBronchoscopyRegistration;

public:
    //FraxinusRegistrationWidget(VisServicesPtr services, RegServicesPtr regServices, QWidget *parent = 0);
    FraxinusRegistrationWidget(QWidget* parent, QString objectName, QString windowTitle):
        BaseWidget(parent, objectName, windowTitle){};
    virtual ~FraxinusRegistrationWidget();

    static QString getWidgetName(){return "fraxinus_registration_widget";};
    virtual void setDefaultCenterlineMesh(MeshPtr mesh) = 0;

private slots:
    virtual void setOrientationClickedSlot() = 0;
    virtual void registrationClickedSlot() = 0;
    virtual void rejectRegistrationClickedSlot() = 0;

private:
    VisServicesPtr mServices;
    RegServicesPtr mRegServices;
    XmlOptionFile mOptions;
    QComboBox* mCenterlineComboBox;
    QComboBox* mToolsComboBox;
    QPushButton* mSetOrientationButton;
    QPushButton* mRegistrationButton;
    QPushButton* mRejectRegistrationButton;
    StringPropertySelectMeshPtr mSelectCenterlineWidget;
    RecordTrackingWidget* mRecordTrackingWidget;

    virtual void processCenterline() = 0;
};


} //namespace cx


#endif //FRAXINUSREGISTRATIONWIDGET_H
