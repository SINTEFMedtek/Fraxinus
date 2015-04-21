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

#include "cxFraxinusMainWindow.h"

#include <QtWidgets>
#include "boost/bind.hpp"

#include "cxConfig.h"
#include "cxDataLocations.h"
#include "cxProfile.h"
#include "cxLogicManager.h"
#include "cxTrackingService.h"
#include "cxSettings.h"
#include "cxVideoService.h"
#include "cxStateService.h"
#include "cxNavigation.h"
#include "cxImage.h"
#include "cxPatientModelService.h"
#include "cxViewService.h"
#include "cxViewGroupData.h"
#include "cxSessionStorageService.h"
#include "cxVisServices.h"
#include "cxAudioImpl.h"
#include "cxLayoutInteractor.h"
#include "cxVLCRecorder.h"
#include "cxCameraControl.h"

#include "cxDockWidgets.h"
#include "cxStatusBar.h"
#include "cxImportDataDialog.h"
#include "cxExportDataDialog.h"
#include "cxSecondaryMainWindow.h"
#include "cxSecondaryViewLayoutWindow.h"
#include "cxSamplerWidget.h"
#include "cxHelperWidgets.h"

#include "cxStreamPropertiesWidget.h"
#include "cxVideoConnectionWidget.h"
#include "cxToolManagerWidget.h"
#include "cxFrameTreeWidget.h"
#include "cxNavigationWidget.h"
#include "cxVolumePropertiesWidget.h"
#include "cxToolPropertiesWidget.h"
#include "cxPreferencesDialog.h"
#include "cxSlicePropertiesWidget.h"
#include "cxMeshInfoWidget.h"
#include "cxTrackPadWidget.h"
#include "cxConsoleWidget.h"
#include "cxMetricWidget.h"
#include "cxPlaybackWidget.h"
#include "cxEraserWidget.h"
#include "cxAllFiltersWidget.h"
#include "cxPluginFrameworkWidget.h"


namespace cx
{

FraxinusMainWindow::FraxinusMainWindow(std::vector<GUIExtenderServicePtr> guiExtenders) :
	mFullScreenAction(NULL), mStandard3DViewActions(new QActionGroup(this)), mControlPanel(NULL), mDockWidgets(new DockWidgets(this))
{
        this->setObjectName("MainWindow");

	mServices = VisServices::create(logicManager()->getPluginContext());
	mLayoutInteractor.reset(new LayoutInteractor());

	this->setCentralWidget(viewService()->getLayoutWidget(this, 0));

	this->createActions();
	this->createMenus();
	this->createToolBars();
	this->setStatusBar(new StatusBar());

	reporter()->setAudioSource(AudioPtr(new AudioImpl()));

        connect(stateService().get(), &StateService::applicationStateChanged, this, &FraxinusMainWindow::onApplicationStateChangedSlot);
        connect(stateService().get(), &StateService::workflowStateChanged, this, &FraxinusMainWindow::onWorkflowStateChangedSlot);
        connect(stateService().get(), &StateService::workflowStateAboutToChange, this, &FraxinusMainWindow::saveDesktopSlot);

	this->updateWindowTitle();

	this->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

	this->addAsDockWidget(new PlaybackWidget(this), "Browsing");
	this->addAsDockWidget(new VideoConnectionWidget(mServices, this), "Utility");
	this->addAsDockWidget(new EraserWidget(this), "Properties");
	this->addAsDockWidget(new MetricWidget(mServices->visualizationService, mServices->patientModelService, this), "Utility");
	this->addAsDockWidget(new SlicePropertiesWidget(mServices->patientModelService, mServices->visualizationService, this), "Properties");
	this->addAsDockWidget(new VolumePropertiesWidget(mServices->patientModelService, mServices->visualizationService, this), "Properties");
	this->addAsDockWidget(new MeshInfoWidget(mServices->patientModelService, mServices->visualizationService, this), "Properties");
	this->addAsDockWidget(new StreamPropertiesWidget(mServices->patientModelService, mServices->visualizationService, this), "Properties");
	this->addAsDockWidget(new TrackPadWidget(this), "Utility");
	this->addAsDockWidget(new ToolPropertiesWidget(this), "Properties");
	this->addAsDockWidget(new NavigationWidget(this), "Properties");
	this->addAsDockWidget(new ConsoleWidget(this, "ConsoleWidget", "Console"), "Utility");
	this->addAsDockWidget(new ConsoleWidget(this, "ConsoleWidget2", "Extra Console"), "Utility");
//	this->addAsDockWidget(new ConsoleWidgetCollection(this, "ConsoleWidgets", "Consoles"), "Utility");
	this->addAsDockWidget(new FrameTreeWidget(mServices->patientModelService, this), "Browsing");
	this->addAsDockWidget(new ToolManagerWidget(this), "Debugging");
	this->addAsDockWidget(new PluginFrameworkWidget(this), "Browsing");
	this->addAsDockWidget(new AllFiltersWidget(VisServices::create(logicManager()->getPluginContext()), this), "Algorithms");


        connect(patientService().get(), &PatientModelService::patientChanged, this, &FraxinusMainWindow::patientChangedSlot);
        connect(qApp, &QApplication::focusChanged, this, &FraxinusMainWindow::focusChanged);

	// insert all widgets from all guiExtenders
	for (unsigned i = 0; i < guiExtenders.size(); ++i)
		this->addGUIExtender(guiExtenders[i].get());

	this->setupGUIExtenders();

	// window menu must be created after all dock widgets are created
	QMenu* popupMenu = this->createPopupMenu();
	popupMenu->setTitle("Window");
	this->menuBar()->insertMenu(mHelpMenuAction, popupMenu);

	// Restore saved window states
	// Must be done after all DockWidgets are created
	if (restoreGeometry(settings()->value("mainWindow/geometry").toByteArray()))
	{
		this->show();
	}
	else
	{
		this->showMaximized();
	}

	if (settings()->value("gui/fullscreen").toBool())
		this->setWindowState(this->windowState() | Qt::WindowFullScreen);
}

void FraxinusMainWindow::changeEvent(QEvent * event)
{
	QMainWindow::changeEvent(event);

	if (event->type() == QEvent::WindowStateChange)
	{
		if (mFullScreenAction)
			mFullScreenAction->setChecked(this->windowState() & Qt::WindowFullScreen);
	}
}

void FraxinusMainWindow::setupGUIExtenders()
{
	mServiceListener.reset(new ServiceTrackerListener<GUIExtenderService>(
								 LogicManager::getInstance()->getPluginContext(),
                                                           boost::bind(&FraxinusMainWindow::onPluginBaseAdded, this, _1),
                                                           boost::bind(&FraxinusMainWindow::onPluginBaseModified, this, _1),
                                                                 boost::bind(&FraxinusMainWindow::onPluginBaseRemoved, this, _1)
							   ));
	mServiceListener->open();
}

void FraxinusMainWindow::addGUIExtender(GUIExtenderService* service)
{
	std::vector<GUIExtenderService::CategorizedWidget> widgets = service->createWidgets();
	for (unsigned j = 0; j < widgets.size(); ++j)
	{
		QWidget* widget = this->addCategorizedWidget(widgets[j]);
		mWidgetsByPlugin[service].push_back(widget);
	}

	std::vector<QToolBar*> toolBars = service->createToolBars();
	for(unsigned j = 0; j < toolBars.size(); ++j)
	{
		addToolBar(toolBars[j]);
		this->registerToolBar(toolBars[j], "Toolbar");
	}
}

QWidget *FraxinusMainWindow::addCategorizedWidget(GUIExtenderService::CategorizedWidget categorizedWidget)
{
	QWidget* retval;
	retval = this->addAsDockWidget(categorizedWidget.mWidget, categorizedWidget.mCategory);
	return retval;
}

void FraxinusMainWindow::removeGUIExtender(GUIExtenderService* service)
{
	while (!mWidgetsByPlugin[service].empty())
	{
		// TODO: must remove widget from several difference data structures: simplify!
		QWidget* widget = mWidgetsByPlugin[service].back();
		mWidgetsByPlugin[service].pop_back();

		QDockWidget* dockWidget = dynamic_cast<QDockWidget*>(widget);
		this->removeDockWidget(dockWidget);

		mDockWidgets->erase(dockWidget);

		if (dockWidget)
		{
			for (std::map<QString, QActionGroup*>::iterator iter=mWidgetGroupsMap.begin(); iter!=mWidgetGroupsMap.end(); ++iter)
			{
				iter->second->removeAction(dockWidget->toggleViewAction());
			}
		}
	}
}

void FraxinusMainWindow::onPluginBaseAdded(GUIExtenderService* service)
{
	this->addGUIExtender(service);
}

void FraxinusMainWindow::onPluginBaseModified(GUIExtenderService* service)
{
}

void FraxinusMainWindow::onPluginBaseRemoved(GUIExtenderService* service)
{
	this->removeGUIExtender(service);
}

void FraxinusMainWindow::dockWidgetVisibilityChanged(bool val)
{
	if (val)
		this->focusInsideDockWidget(sender());
}

void FraxinusMainWindow::focusChanged(QWidget * old, QWidget * now)
{
	this->focusInsideDockWidget(now);
}

void FraxinusMainWindow::focusInsideDockWidget(QObject *dockWidget)
{
	// Assume structure: QDockWidget->QScrollArea->QWidget,
        // as defined in FraxinusMainWindow::addAsDockWidget()
	QDockWidget* dw = dynamic_cast<QDockWidget*>(dockWidget);
	if (!dw)
		return;
	if (dw->parent()!=this) // avoid events from other mainwindows
		return;
	QScrollArea* sa = dynamic_cast<QScrollArea*>(dw->widget());
	if (!sa)
		return;
	QTimer::singleShot(0, sa->widget(), SLOT(setFocus())); // avoid loops etc by send async event.
}


void FraxinusMainWindow::addToWidgetGroupMap(QAction* action, QString groupname)
{
	action->setMenuRole(QAction::NoRole);
	if (mWidgetGroupsMap.find(groupname) != mWidgetGroupsMap.end())
	{
		mWidgetGroupsMap[groupname]->addAction(action);
	}
	else
	{
		QActionGroup* group = new QActionGroup(this);
		group->setExclusive(false);
		mWidgetGroupsMap[groupname] = group;
		QAction* heading = new QAction(groupname, this);
		heading->setDisabled(true);
		mWidgetGroupsMap[groupname]->addAction(heading);
		mWidgetGroupsMap[groupname]->addAction(action);
	}
}

FraxinusMainWindow::~FraxinusMainWindow()
{
	reporter()->setAudioSource(AudioPtr()); // important! QSound::play fires a thread, causes segfault during shutdown
	mServiceListener.reset();
}

QMenu* FraxinusMainWindow::createPopupMenu()
{
	QMenu* popupMenu = new QMenu(this);
	std::map<QString, QActionGroup*>::iterator it = mWidgetGroupsMap.begin();
	for (; it != mWidgetGroupsMap.end(); ++it)
	{
		popupMenu->addSeparator();
		popupMenu->addActions(it->second->actions());
	}

	return popupMenu;
}

void FraxinusMainWindow::createActions()
{
	CameraControlPtr cameraControl = viewService()->getCameraControl();
	if (cameraControl)
		mStandard3DViewActions = cameraControl->createStandard3DViewActions();

	// File
	mNewPatientAction = new QAction(QIcon(":/icons/open_icon_library/document-new-8.png"), tr(
		"&New patient"), this);
	mNewPatientAction->setShortcut(tr("Ctrl+N"));
	mNewPatientAction->setStatusTip(tr("Create a new patient file"));
	mSaveFileAction = new QAction(QIcon(":/icons/open_icon_library/document-save-5.png"), tr(
		"&Save Patient"), this);
	mSaveFileAction->setShortcut(tr("Ctrl+S"));
	mSaveFileAction->setStatusTip(tr("Save patient file"));
	mLoadFileAction = new QAction(QIcon(":/icons/open_icon_library/document-open-7.png"), tr(
		"&Load Patient"), this);
	mLoadFileAction->setShortcut(tr("Ctrl+L"));
	mLoadFileAction->setStatusTip(tr("Load patient file"));
	mClearPatientAction = new QAction(tr("&Clear Patient"), this);
	mExportPatientAction = new QAction(tr("&Export Patient"), this);

        connect(mNewPatientAction, &QAction::triggered, this, &FraxinusMainWindow::newPatientSlot);
        connect(mLoadFileAction, &QAction::triggered, this, &FraxinusMainWindow::loadPatientFileSlot);
        connect(mSaveFileAction, &QAction::triggered, this, &FraxinusMainWindow::savePatientFileSlot);
        connect(mSaveFileAction, &QAction::triggered, this, &FraxinusMainWindow::saveDesktopSlot);
        connect(mExportPatientAction, &QAction::triggered, this, &FraxinusMainWindow::exportDataSlot);
        connect(mClearPatientAction, &QAction::triggered, this, &FraxinusMainWindow::clearPatientSlot);

	mShowControlPanelAction = new QAction("Show Control Panel", this);
        connect(mShowControlPanelAction, &QAction::triggered, this, &FraxinusMainWindow::showControlPanelActionSlot);
	mSecondaryViewLayoutWindowAction = new QAction("Show Secondary View Layout Window", this);
        connect(mSecondaryViewLayoutWindowAction, &QAction::triggered, this, &FraxinusMainWindow::showSecondaryViewLayoutWindowActionSlot);

	// Application
	mAboutAction = new QAction(tr("About"), this);
	mAboutAction->setStatusTip(tr("Show the application's About box"));
	mPreferencesAction = new QAction(tr("Preferences"), this);
	mPreferencesAction->setShortcut(tr("Ctrl+,"));
	mPreferencesAction->setStatusTip(tr("Show the preferences dialog"));

	mStartLogConsoleAction = new QAction(tr("&Start Log Console"), this);
	mStartLogConsoleAction->setShortcut(tr("Ctrl+D"));
	mStartLogConsoleAction->setStatusTip(tr("Open Log Console as external application"));
        connect(mStartLogConsoleAction, &QAction::triggered, this, &FraxinusMainWindow::onStartLogConsole);

	mFullScreenAction = new QAction(tr("Fullscreen"), this);
	mFullScreenAction->setShortcut(tr("F11"));
	mFullScreenAction->setStatusTip(tr("Toggle full screen"));
	mFullScreenAction->setCheckable(true);
	mFullScreenAction->setChecked(this->windowState() & Qt::WindowFullScreen);
        connect(mFullScreenAction, &QAction::triggered, this, &FraxinusMainWindow::toggleFullScreenSlot);

	mQuitAction = new QAction(tr("&Quit"), this);
	mQuitAction->setShortcut(tr("Ctrl+Q"));
	mQuitAction->setStatusTip(tr("Exit the application"));

        connect(mAboutAction, &QAction::triggered, this, &FraxinusMainWindow::aboutSlot);
        connect(mPreferencesAction, &QAction::triggered, this, &FraxinusMainWindow::preferencesSlot);
        connect(mQuitAction, &QAction::triggered, this, &FraxinusMainWindow::quitSlot);

	mShootScreenAction = new QAction(tr("Shoot Screen"), this);
	mShootScreenAction->setIcon(QIcon(":/icons/screenshot-screen.png"));
	mShootScreenAction->setShortcut(tr("Ctrl+f"));
	mShootScreenAction->setStatusTip(tr("Save a screenshot to the patient folder."));
        connect(mShootScreenAction, &QAction::triggered, this, &FraxinusMainWindow::shootScreen);

	mShootWindowAction = new QAction(tr("Shoot Window"), this);
	mShootWindowAction->setIcon(QIcon(":/icons/screenshot-window.png"));
	mShootWindowAction->setShortcut(tr("Ctrl+Shift+f"));
	mShootWindowAction->setStatusTip(tr("Save an image of the application to the patient folder."));
        connect(mShootWindowAction, &QAction::triggered, this, &FraxinusMainWindow::shootWindow);

	mRecordFullscreenAction = new QAction(tr("Record Fullscreen"), this);
	mRecordFullscreenAction->setShortcut(tr("F8"));
	mRecordFullscreenAction->setStatusTip(tr("Record a video of the full screen."));
        connect(mRecordFullscreenAction, &QAction::triggered, this, &FraxinusMainWindow::recordFullscreen);

	//data
	mImportDataAction = new QAction(QIcon(":/icons/open_icon_library/document-import-2.png"), tr("&Import data"), this);
	mImportDataAction->setShortcut(tr("Ctrl+I"));
	mImportDataAction->setStatusTip(tr("Import image data"));

	mDeleteDataAction = new QAction(tr("Delete current image"), this);
	mDeleteDataAction->setStatusTip(tr("Delete selected volume"));

        connect(mImportDataAction, &QAction::triggered, this, &FraxinusMainWindow::importDataSlot);
        connect(mDeleteDataAction, &QAction::triggered, this, &FraxinusMainWindow::deleteDataSlot);

	mShowPointPickerAction = new QAction(tr("Point Picker"), this);
	mShowPointPickerAction->setCheckable(true);
	mShowPointPickerAction->setToolTip("Activate the 3D Point Picker Probe");
	mShowPointPickerAction->setIcon(QIcon(":/icons/point_picker.png"));
        connect(mShowPointPickerAction, &QAction::triggered, this, &FraxinusMainWindow::togglePointPickerActionSlot);

	if (viewService()->getGroup(0))
                connect(viewService()->getGroup(0).get(), &ViewGroupData::optionsChanged, this, &FraxinusMainWindow::updatePointPickerActionSlot);
	this->updatePointPickerActionSlot();

	//tool
	mToolsActionGroup = new QActionGroup(this);
	mConfigureToolsAction = new QAction(tr("Tool configuration"), mToolsActionGroup);
	mInitializeToolsAction = new QAction(tr("Initialize"), mToolsActionGroup);
	mTrackingToolsAction = new QAction(tr("Start tracking"), mToolsActionGroup);
	mTrackingToolsAction->setShortcut(tr("Ctrl+T"));

	mToolsActionGroup->setExclusive(false); // must turn off to get the checkbox independent.

	mStartStreamingAction = new QAction(tr("Start Streaming"), mToolsActionGroup);
	mStartStreamingAction->setShortcut(tr("Ctrl+V"));
        connect(mStartStreamingAction, &QAction::triggered, this, &FraxinusMainWindow::toggleStreamingSlot);
        connect(videoService().get(), &VideoService::connected, this, &FraxinusMainWindow::updateStreamingActionSlot);
	this->updateStreamingActionSlot();

	mConfigureToolsAction->setChecked(true);

        connect(mConfigureToolsAction, &QAction::triggered, this, &FraxinusMainWindow::configureSlot);
	boost::function<void()> finit = boost::bind(&TrackingService::setState, trackingService(), Tool::tsINITIALIZED);
	connect(mInitializeToolsAction, &QAction::triggered, finit);
        connect(mTrackingToolsAction, &QAction::triggered, this, &FraxinusMainWindow::toggleTrackingSlot);
        connect(trackingService().get(), &TrackingService::stateChanged, this, &FraxinusMainWindow::updateTrackingActionSlot);
        connect(trackingService().get(), &TrackingService::stateChanged, this, &FraxinusMainWindow::updateTrackingActionSlot);
	this->updateTrackingActionSlot();

	mCenterToImageCenterAction = new QAction(tr("Center Image"), this);
	mCenterToImageCenterAction->setIcon(QIcon(":/icons/center_image.png"));
        connect(mCenterToImageCenterAction, &QAction::triggered, this, &FraxinusMainWindow::centerToImageCenterSlot);
	mCenterToTooltipAction = new QAction(tr("Center Tool"), this);
	mCenterToTooltipAction->setIcon(QIcon(":/icons/center_tool.png"));
        connect(mCenterToTooltipAction, &QAction::triggered, this, &FraxinusMainWindow::centerToTooltipSlot);

	mSaveDesktopAction = new QAction(QIcon(":/icons/workflow_state_save.png"), tr("Save desktop"), this);
	mSaveDesktopAction->setToolTip("Save desktop for workflow step");
        connect(mSaveDesktopAction, &QAction::triggered, this, &FraxinusMainWindow::saveDesktopSlot);
	mResetDesktopAction = new QAction(QIcon(":/icons/workflow_state_revert.png"), tr("Reset desktop"), this);
	mResetDesktopAction->setToolTip("Reset desktop for workflow step");
        connect(mResetDesktopAction, &QAction::triggered, this, &FraxinusMainWindow::resetDesktopSlot);

	mInteractorStyleActionGroup = viewService()->createInteractorStyleActionGroup();
}

void FraxinusMainWindow::toggleFullScreenSlot()
{
	this->setWindowState(this->windowState() ^ Qt::WindowFullScreen);

	settings()->setValue("gui/fullscreen", (this->windowState() & Qt::WindowFullScreen)!=0);
}

void FraxinusMainWindow::shootScreen()
{
	QDesktopWidget* desktop = qApp->desktop();
	QList<QScreen*> screens = qApp->screens();

	for (int i=0; i<desktop->screenCount(); ++i)
	{
		QWidget* screenWidget = desktop->screen(i);
		WId screenWinId = screenWidget->winId();
		QRect geo = desktop->screenGeometry(i);
		QString name = "";
		if (desktop->screenCount()>1)
			name = screens[i]->name().split(" ").join("");
		this->saveScreenShot(screens[i]->grabWindow(screenWinId, geo.left(), geo.top(), geo.width(), geo.height()), name);
	}
}

void FraxinusMainWindow::shootWindow()
{
	QScreen* screen = qApp->primaryScreen();
	this->saveScreenShot(screen->grabWindow(this->winId()));
}

void FraxinusMainWindow::recordFullscreen()
{
	QString path = patientService()->generateFilePath("Screenshots", "mp4");

	if(vlc()->isRecording())
		vlc()->stopRecording();
	else
		vlc()->startRecording(path);

}

void FraxinusMainWindow::onStartLogConsole()
{
	QString fullname = DataLocations::findExecutableInStandardLocations("LogConsole");
//	std::cout << "FraxinusMainWindow::onStartLogConsole() " << fullname << std::endl;
	mLocalVideoServerProcess.reset(new ProcessWrapper(QString("LogConsole")));
	mLocalVideoServerProcess->launchWithRelativePath(fullname, QStringList());
}

void FraxinusMainWindow::saveScreenShot(QPixmap pixmap, QString id)
{
	QString ending = "png";
	if (!id.isEmpty())
		ending = id + "." + ending;
	QString path = patientService()->generateFilePath("Screenshots", ending);
        QtConcurrent::run(boost::bind(&FraxinusMainWindow::saveScreenShotThreaded, this, pixmap.toImage(), path));
}

/**Intended to be called in a separate thread.
 * \sa saveScreenShot()
 */
void FraxinusMainWindow::saveScreenShotThreaded(QImage pixmap, QString filename)
{
	pixmap.save(filename, "png");
	report("Saved screenshot to " + filename);
	reporter()->playScreenShotSound();
}

void FraxinusMainWindow::toggleStreamingSlot()
{
	if (videoService()->isConnected())
		videoService()->closeConnection();
	else
		videoService()->openConnection();
}

void FraxinusMainWindow::updateStreamingActionSlot()
{
	if (videoService()->isConnected())
	{
		mStartStreamingAction->setIcon(QIcon(":/icons/streaming_green.png"));
		mStartStreamingAction->setText("Stop Streaming");
	}
	else
	{
		mStartStreamingAction->setIcon(QIcon(":/icons/streaming_red.png"));
		mStartStreamingAction->setText("Start Streaming");
	}
}

void FraxinusMainWindow::centerToImageCenterSlot()
{
	NavigationPtr nav = viewService()->getNavigation();

	if (patientService()->getActiveImage())
		nav->centerToData(patientService()->getActiveImage());
	else if (!viewService()->groupCount())
		nav->centerToView(viewService()->getGroup(0)->getData());
	else
		nav->centerToGlobalDataCenter();
}

void FraxinusMainWindow::centerToTooltipSlot()
{
	NavigationPtr nav = viewService()->getNavigation();
	nav->centerToTooltip();
}

void FraxinusMainWindow::togglePointPickerActionSlot()
{
	ViewGroupDataPtr data = viewService()->getGroup(0);
	ViewGroupData::Options options = data->getOptions();
	options.mShowPointPickerProbe = !options.mShowPointPickerProbe;
	data->setOptions(options);
}
void FraxinusMainWindow::updatePointPickerActionSlot()
{
	if (!viewService()->getGroup(0))
		return;
	bool show = viewService()->getGroup(0)->getOptions().mShowPointPickerProbe;
	mShowPointPickerAction->setChecked(show);
}

void FraxinusMainWindow::updateTrackingActionSlot()
{
	if (trackingService()->getState() >= Tool::tsTRACKING)
	{
		mTrackingToolsAction->setIcon(QIcon(":/icons/polaris-green.png"));
		mTrackingToolsAction->setText("Stop Tracking");
	}
	else
	{
		mTrackingToolsAction->setIcon(QIcon(":/icons/polaris-red.png"));
		mTrackingToolsAction->setText("Start Tracking");
	}
}

void FraxinusMainWindow::toggleTrackingSlot()
{
	if (trackingService()->getState() >= Tool::tsTRACKING)
		trackingService()->setState(Tool::tsINITIALIZED);
	else
		trackingService()->setState(Tool::tsTRACKING);
}

namespace
{
QString timestampFormatFolderFriendly()
{
  return QString("yyyy-MM-dd_hh-mm");
}
}

void FraxinusMainWindow::newPatientSlot()
{
	QString patientDatafolder = this->getExistingSessionFolder();

	QString timestamp = QDateTime::currentDateTime().toString(timestampFormatFolderFriendly());
	QString filename = QString("%1_%2_%3.cx3")
			.arg(timestamp)
			.arg(profile()->getName())
			.arg(settings()->value("globalPatientNumber").toString());

	QString choosenDir = patientDatafolder + "/" + filename;

	QFileDialog dialog(this, tr("Select directory to save patient in"), patientDatafolder + "/");
	dialog.setOption(QFileDialog::DontUseNativeDialog, true);
	dialog.setOption(QFileDialog::ShowDirsOnly, true);
	dialog.selectFile(filename);
	if (!dialog.exec())
		return;
	choosenDir = dialog.selectedFiles().front();

	if (!choosenDir.endsWith(".cx3"))
		choosenDir += QString(".cx3");

	// Update global patient number
	int patientNumber = settings()->value("globalPatientNumber").toInt();
	settings()->setValue("globalPatientNumber", ++patientNumber);

	mServices->getSession()->load(choosenDir);
}

QString FraxinusMainWindow::getExistingSessionFolder()
{
	QString folder = settings()->value("globalPatientDataFolder").toString();

	// Create folders
	if (!QDir().exists(folder))
	{
		QDir().mkdir(folder);
		report("Made a new patient folder: " + folder);
	}

	return folder;
}

void FraxinusMainWindow::clearPatientSlot()
{
	mServices->getSession()->clear();
}

void FraxinusMainWindow::savePatientFileSlot()
{
	if (patientService()->getActivePatientFolder().isEmpty())
	{
		reportWarning("No patient selected, select or create patient before saving!");
		this->newPatientSlot();
		return;
	}

	mServices->getSession()->save();
}

void FraxinusMainWindow::onApplicationStateChangedSlot()
{
	this->updateWindowTitle();
}

void FraxinusMainWindow::updateWindowTitle()
{
	QString profileName = stateService()->getApplicationStateName();
	QString versionName = stateService()->getVersionName();

	QString activePatientFolder = patientService()->getActivePatientFolder();
	if (activePatientFolder.endsWith('/'))
		activePatientFolder.chop(1);
	QString patientName;

	if (!activePatientFolder.isEmpty())
	{
		QFileInfo info(activePatientFolder);
		patientName = info.completeBaseName();
	}

	QString format("%1 %2 - %3 - %4  (not approved for medical use)");
	QString title = format
			.arg(qApp->applicationDisplayName())
			.arg(versionName)
			.arg(profileName)
			.arg(patientName);
	this->setWindowTitle(title);
}

void FraxinusMainWindow::onWorkflowStateChangedSlot()
{
	Desktop desktop = stateService()->getActiveDesktop();

	this->mDockWidgets->hideAll();
//	for (std::set<QToolBar*>::iterator i=mToolbars.begin(); i!=mToolbars.end(); ++i)
//		(*i)->hide();
//	for (std::set<QToolBar*>::iterator i=mToolbars.begin(); i!=mToolbars.end(); ++i)
//		this->removeToolBar(*i);
//	for (std::set<QToolBar*>::iterator i=mToolbars.begin(); i!=mToolbars.end(); ++i)
//		this->addToolBar(*i);

	viewService()->setActiveLayout(desktop.mLayoutUid, 0);
	viewService()->setActiveLayout(desktop.mSecondaryLayoutUid, 1);
	this->restoreState(desktop.mMainWindowState);
	patientService()->autoSave();

#ifdef CX_APPLE
	// HACK
	// Toolbars are not correctly refreshed on mac 10.8,
	// Cause is related to QVTKWidget (removing it removes the problem)
	// The following "force refresh by resize" solves repaint, but
	// inactive toolbars are still partly clickable.
	QSize size = this->size();
	this->resize(size.width()-1, size.height());
	this->resize(size);
#endif
}

void FraxinusMainWindow::saveDesktopSlot()
{
	Desktop desktop;
	desktop.mMainWindowState = this->saveState();
	desktop.mLayoutUid = viewService()->getActiveLayout(0);
	desktop.mSecondaryLayoutUid = viewService()->getActiveLayout(1);
	stateService()->saveDesktop(desktop);
}

void FraxinusMainWindow::resetDesktopSlot()
{
	stateService()->resetDesktop();
	this->onWorkflowStateChangedSlot();
}

void FraxinusMainWindow::showControlPanelActionSlot()
{
	if (!mControlPanel)
		mControlPanel = new SecondaryMainWindow(this);
	mControlPanel->show();
}

void FraxinusMainWindow::showSecondaryViewLayoutWindowActionSlot()
{
	if (!mSecondaryViewLayoutWindow)
		mSecondaryViewLayoutWindow = new SecondaryViewLayoutWindow(this);
	mSecondaryViewLayoutWindow->tryShowOnSecondaryScreen();
}

void FraxinusMainWindow::loadPatientFileSlot()
{
	QString patientDatafolder = this->getExistingSessionFolder();

	// Open file dialog
	QString folder = QFileDialog::getExistingDirectory(this, "Select patient", patientDatafolder, QFileDialog::ShowDirsOnly);
	if (folder.isEmpty())
		return;

	mServices->getSession()->load(folder);
}

void FraxinusMainWindow::exportDataSlot()
{
	this->savePatientFileSlot();

	ExportDataDialog* wizard = new ExportDataDialog(mServices->patientModelService, this);
	wizard->exec(); //calling exec() makes the wizard dialog modal which prevents other user interaction with the system
}

void FraxinusMainWindow::importDataSlot()
{
	this->savePatientFileSlot();

	QString folder = mLastImportDataFolder;
	if (folder.isEmpty())
		folder = settings()->value("globalPatientDataFolder").toString();

	QStringList fileName = QFileDialog::getOpenFileNames(this, QString(tr("Select data file(s) for import")),
		folder, tr("Image/Mesh (*.mhd *.mha *.stl *.vtk *.mnc)"));
	if (fileName.empty())
	{
		report("Import canceled");
		return;
	}

	mLastImportDataFolder = QFileInfo(fileName[0]).absolutePath();

	for (int i=0; i<fileName.size(); ++i)
	{
		ImportDataDialog* wizard = new ImportDataDialog(mServices->patientModelService, fileName[i], this);
		wizard->exec(); //calling exec() makes the wizard dialog modal which prevents other user interaction with the system
	}
}

void FraxinusMainWindow::patientChangedSlot()
{
	this->updateWindowTitle();
}

void FraxinusMainWindow::createMenus()
{
#ifdef CX_LINUX
    // shortcuts defined on actions in the global menubar is not reachable on Qt5+Ubuntu14.04,
    // solve by reverting to old style.
    // This will create a double menubar: remove by autohiding menubar or using
    //   sudo apt-get autoremove appmenu-gtk appmenu-gtk3 appmenu-qt
    // and reboot
    this->menuBar()->setNativeMenuBar(false);
#endif
	mFileMenu = new QMenu(tr("File"), this);
	mWorkflowMenu = new QMenu(tr("Workflow"), this);
	mToolMenu = new QMenu(tr("Tracking"), this);
	mLayoutMenu = new QMenu(tr("Layouts"), this);
	mNavigationMenu = new QMenu(tr("Navigation"), this);
	mHelpMenu = new QMenu(tr("Help"), this);

	// File
	mFileMenu->addAction(mPreferencesAction);
	this->menuBar()->addMenu(mFileMenu);
	mFileMenu->addAction(mNewPatientAction);
	mFileMenu->addAction(mSaveFileAction);
	mFileMenu->addAction(mLoadFileAction);
	mFileMenu->addAction(mClearPatientAction);
	mFileMenu->addSeparator();
	mFileMenu->addAction(mExportPatientAction);
	mFileMenu->addAction(mImportDataAction);
	mFileMenu->addAction(mDeleteDataAction);
	mFileMenu->addSeparator();
	mFileMenu->addAction(mFullScreenAction);
	mFileMenu->addAction(mStartLogConsoleAction);
	mFileMenu->addAction(mShootScreenAction);
	mFileMenu->addAction(mShootWindowAction);
	mFileMenu->addAction(mRecordFullscreenAction);
	mFileMenu->addSeparator();
	mFileMenu->addAction(mShowControlPanelAction);
	mFileMenu->addAction(mSecondaryViewLayoutWindowAction);

	mFileMenu->addAction(mQuitAction);

	//workflow
	this->menuBar()->addMenu(mWorkflowMenu);
	QList<QAction*> actions = stateService()->getWorkflowActions()->actions();
	for (int i=0; i<actions.size(); ++i)
	{
		mWorkflowMenu->addAction(actions[i]);
	}

	mWorkflowMenu->addSeparator();
	mWorkflowMenu->addAction(mSaveDesktopAction);
	mWorkflowMenu->addAction(mResetDesktopAction);

	//tool
	this->menuBar()->addMenu(mToolMenu);
	mToolMenu->addAction(mConfigureToolsAction);
	mToolMenu->addAction(mInitializeToolsAction);
	mToolMenu->addAction(mTrackingToolsAction);
	mToolMenu->addSeparator();
	mToolMenu->addAction(mStartStreamingAction);
	mToolMenu->addSeparator();

	//layout
	this->menuBar()->addMenu(mLayoutMenu);
	mLayoutInteractor->connectToMenu(mLayoutMenu);

	this->menuBar()->addMenu(mNavigationMenu);
	mNavigationMenu->addAction(mCenterToImageCenterAction);
	mNavigationMenu->addAction(mCenterToTooltipAction);
	mNavigationMenu->addAction(mShowPointPickerAction);
	mNavigationMenu->addSeparator();
	mNavigationMenu->addActions(mInteractorStyleActionGroup->actions());

	mHelpMenuAction = this->menuBar()->addMenu(mHelpMenu);
	mHelpMenu->addAction(mAboutAction);
	mHelpMenu->addAction(QWhatsThis::createAction(this));
}

void FraxinusMainWindow::createToolBars()
{
	mDataToolBar = addToolBar("Data");
	mDataToolBar->setObjectName("DataToolBar");
	mDataToolBar->addAction(mNewPatientAction);
	mDataToolBar->addAction(mLoadFileAction);
	mDataToolBar->addAction(mSaveFileAction);
	mDataToolBar->addAction(mImportDataAction);
	this->registerToolBar(mDataToolBar, "Toolbar");

	mToolToolBar = addToolBar("Tools");
	mToolToolBar->setObjectName("ToolToolBar");
	mToolToolBar->addAction(mTrackingToolsAction);
	mToolToolBar->addAction(mStartStreamingAction);
	this->registerToolBar(mToolToolBar, "Toolbar");

	mNavigationToolBar = addToolBar("Navigation");
	mNavigationToolBar->setObjectName("NavigationToolBar");
	mNavigationToolBar->addAction(mCenterToImageCenterAction);
	mNavigationToolBar->addAction(mCenterToTooltipAction);
	mNavigationToolBar->addAction(mShowPointPickerAction);
	this->registerToolBar(mNavigationToolBar, "Toolbar");

	mInteractorStyleToolBar = addToolBar("InteractorStyle");
	mInteractorStyleToolBar->setObjectName("InteractorStyleToolBar");

	mInteractorStyleToolBar->addActions(mInteractorStyleActionGroup->actions());
	this->registerToolBar(mInteractorStyleToolBar, "Toolbar");

	mWorkflowToolBar = addToolBar("Workflow");
	mWorkflowToolBar->setObjectName("WorkflowToolBar");

	QList<QAction*> actions = stateService()->getWorkflowActions()->actions();
	for (int i=0; i<actions.size(); ++i)
	{
		mWorkflowToolBar->addAction(actions[i]);
	}

	this->registerToolBar(mWorkflowToolBar, "Toolbar");

	mDesktopToolBar = addToolBar("Desktop");
	mDesktopToolBar->setObjectName("DesktopToolBar");
	mDesktopToolBar->addAction(mSaveDesktopAction);
	mDesktopToolBar->addAction(mResetDesktopAction);
	this->registerToolBar(mDesktopToolBar, "Toolbar");

	mHelpToolBar = addToolBar("Help");
	mHelpToolBar->setObjectName("HelpToolBar");
	mHelpToolBar->addAction(QWhatsThis::createAction(this));
	this->registerToolBar(mHelpToolBar, "Toolbar");

	mScreenshotToolBar = addToolBar("Screenshot");
	mScreenshotToolBar->setObjectName("ScreenshotToolBar");
	mScreenshotToolBar->addAction(mShootScreenAction);
	this->registerToolBar(mScreenshotToolBar, "Toolbar");

	QToolBar* camera3DViewToolBar = addToolBar("Camera 3D Views");
	camera3DViewToolBar->setObjectName("Camera3DViewToolBar");
	camera3DViewToolBar->addActions(mStandard3DViewActions->actions());
	this->registerToolBar(camera3DViewToolBar, "Toolbar");

	QToolBar* samplerWidgetToolBar = addToolBar("Sampler");
	samplerWidgetToolBar->setObjectName("SamplerToolBar");
	samplerWidgetToolBar->addWidget(new SamplerWidget(this));
	this->registerToolBar(samplerWidgetToolBar, "Toolbar");

	QToolBar* toolOffsetToolBar = addToolBar("Tool Offset");
	toolOffsetToolBar->setObjectName("ToolOffsetToolBar");
	toolOffsetToolBar->addWidget(createDataWidget(mServices->visualizationService, mServices->patientModelService, this, DoublePropertyActiveToolOffset::create()));
	this->registerToolBar(toolOffsetToolBar, "Toolbar");
}

void FraxinusMainWindow::registerToolBar(QToolBar* toolbar, QString groupname)
{
	this->addToWidgetGroupMap(toolbar->toggleViewAction(), groupname);
	// this avoids overpopulation of gui at startup, and is the same functionality as for dockwidgets.
	// also gives correct size of mainwindow at startup.
	mToolbars.insert(toolbar);
	if (!mToolbars.empty())
		toolbar->hide();
}

void FraxinusMainWindow::aboutSlot()
{
	QString doc_path = DataLocations::getDocPath();
	QString appName = qApp->applicationDisplayName();
	QString url_github("https://github.com/SINTEFMedtek/CustusX");
	QString url_license = QString("file://%1/license.txt").arg(doc_path);
	QString url_config = QString("file://%1/cxConfigDescription.txt").arg(doc_path);

	QString text(""
	"<h2>%1</h2>"
	"<h4>%2</h4>"
	"<p>A Research Platform for Image-Guided Therapy<p>"
	"<p>%1 is NOT approved for medical use.<p>"
	""
	"<p><a href=%3> website </a><p>"
	"<p><a href=%4> license </a><p>"
	"<p><a href=%5> configuration </a><p>");

	QMessageBox::about(this, tr("About %1").arg(appName), text
			.arg(appName)
			.arg(CustusX_VERSION_STRING)
			.arg(url_github)
			.arg(url_license)
			.arg(url_config)
			);
}

void FraxinusMainWindow::preferencesSlot()
{
	PreferencesDialog prefDialog(mServices->visualizationService, mServices->patientModelService, this);
	prefDialog.exec();
}

void FraxinusMainWindow::quitSlot()
{
	report("Shutting down CustusX");
	viewService()->deactivateLayout();

	patientService()->autoSave();

	settings()->setValue("mainWindow/geometry", saveGeometry());
	settings()->setValue("mainWindow/windowState", saveState());
	settings()->sync();
	report("Closing: Save geometry and window state");

	qApp->quit();
}

void FraxinusMainWindow::deleteDataSlot()
{
	if (!patientService()->getActiveImage())
		return;
	QString text = QString("Do you really want to delete data %1?").arg(patientService()->getActiveImage()->getName());
	if (QMessageBox::question(this, "Data delete", text, QMessageBox::StandardButtons(QMessageBox::Ok | QMessageBox::Cancel))!=QMessageBox::Ok)
		return;
	mServices->patientModelService->removeData(patientService()->getActiveImageUid());
}

void FraxinusMainWindow::configureSlot()
{
	trackingService()->setState(Tool::tsCONFIGURED);
}

void FraxinusMainWindow::closeEvent(QCloseEvent *event)
{
	QMainWindow::closeEvent(event);
	this->quitSlot();
}

QDockWidget* FraxinusMainWindow::addAsDockWidget(QWidget* widget, QString groupname)
{
	QDockWidget* dockWidget = mDockWidgets->addAsDockWidget(widget, groupname);
	this->addToWidgetGroupMap(dockWidget->toggleViewAction(), groupname);
	QMainWindow::addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
	this->restoreDockWidget(dockWidget); // restore if added after construction
	return dockWidget;
}

}//namespace cx
