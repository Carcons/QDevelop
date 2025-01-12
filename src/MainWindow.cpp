
#include "Source.h"
#include "MainWindow.h"


#include <QApplication>
#include <QIcon>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QFileSystemModel>
#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QFontDialog>
#include <QTextStream>
#include <QTextCursor>
#include <QDebug>
#include <QSet>



using namespace qdev;

// Start MainWindow

static MainWindow* unique_instance = 0;

MainWindow* MainWindow::instance()
{
	if (unique_instance == 0)
		unique_instance = new MainWindow();
	return unique_instance;
}

MainWindow::MainWindow() : 
	QMainWindow(0), _session("Unicas", "QDevelop2") {

	// Window options
	setWindowIcon(QIcon(icons::APP_ICO));
	setWindowState(Qt::WindowMaximized);

	_debugger = new Debugger();

	// Create Actions
	_newProjectAct = new QAction("New Project", this);
	_newProjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
	_importCMakeAct = new QAction(QIcon(icons::CMAKE_PROJ), "Import CMake Project", this);
	_exitAct = new QAction(QIcon(icons::CANCEL), "Exit");
	_updateContentsAct = new QAction("Refresh", this);
	_cleanProjectAct = new QAction("Clear project", this);
	_newDirAct = new QAction(QIcon(icons::DIR_NEW), "New Directory", this);
	_renameDirAct = new QAction("Rename directory", this);
	_deleteDirAct = new QAction("Delete directory", this);
	_openProjectAct = new QAction("Open Project", this);
	_openProjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
	_saveSourceAct = new QAction(QIcon(icons::SAVE), "Save", this);
	_saveSourceAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	_saveAllSourcesAct = new QAction(QIcon(icons::SAVE_ALL), "Save all", this);
	_closeSourceAct = new QAction(QIcon(icons::CLOSE), "Close", this);
	_closeAllSourcesAct = new QAction(QIcon(icons::CLOSE_ALL), "Close all", this);
	_printAct = new QAction(QIcon(icons::PRINTER), "Print", this);
	_printAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
	_closeProjectAct = new QAction(QIcon(icons::CANCEL), "Close Project", this);
	_closeProjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
	_undoAct = new QAction(QIcon(icons::UNDO), "Undo", this);
	_undoAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
	_redoAct = new QAction(QIcon(icons::REDO), "Redo", this);
	_redoAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));
	_fontAct = new QAction("Font...", this);
	_projectSettingsAct = new QAction(QIcon(icons::SETTINGS), "Options", this);
	_addSourceAct = new QAction(QIcon(icons::ADD_SOURCE), "New file", this);
	_renameSourceAct = new QAction(QIcon(icons::RENAME_SOURCE), "Rename file", this);
	_removeSourceAct = new QAction(QIcon(icons::REMOVE_SOURCE), "Remove file", this);
	_buildAct = new QAction(QIcon(icons::BUILD), "Build", this);
	_runAct = new QAction(QIcon(icons::RUN), "Run", this);
	_runAct->setEnabled(false);
	_stopAct = new QAction(QIcon(icons::STOP), "Stop", this);
	_stopAct->setEnabled(false);
	_aboutAct = new QAction(QIcon(icons::ABOUT), "About", this);
	_activeProjectAct = new QAction("Set as Active");
	_saveAllExitAct = new QAction("Save All and Exit");
	_debuggerStartAct = new QAction(QIcon(icons::DEBUGGER_START), "Run Debug", this);
	_debuggerStopAct = new QAction(QIcon(icons::DEBUGGER_STOP), "Stop Debug", this);
	_debuggerNextLineAct = new QAction(QIcon(icons::DEBUGGER_NEXT_LINE), "Next line", this);
	_debuggerContinueAct = new QAction(QIcon(icons::DEBUGGER_CONTINUE), "Continue", this);
	_debuggerBuilderAct = new QAction(QIcon(icons::DEBUGGER_BUILD), "Build (Debug)", this);
	_infoLocalAct = new QAction(QIcon(icons::INFO_LOCAL), "Info Locals", this);

	// Create Menu 
	_fileMenu = new QMenu("File");
	_fileMenu->addAction(_newProjectAct);
	_fileMenu->addAction(_openProjectAct);
	_fileMenu->addAction(_importCMakeAct);
	_recentProjectsMenu = new QMenu("Recent Projects");
	_fileMenu->addMenu(_recentProjectsMenu);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_saveSourceAct);
	_fileMenu->addAction(_saveAllSourcesAct);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_closeSourceAct);
	_fileMenu->addAction(_closeAllSourcesAct);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_printAct);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_saveAllExitAct);
	_fileMenu->addAction(_exitAct);
	_editMenu = new QMenu("Edit");
	_editMenu->addAction(_undoAct);
	_editMenu->addAction(_redoAct);
	_editMenu->addSeparator();
	_editMenu->addAction(_fontAct);
	_projectMenu = new QMenu("Project");
	_projectMenu->addAction(_projectSettingsAct);
	_projectMenu->addAction(_cleanProjectAct);
	_projectMenu->addAction(_updateContentsAct);
	_executeMenu = new QMenu("Execute");
	_executeMenu->addAction(_buildAct);
	_executeMenu->addAction(_runAct);
	_executeMenu->addAction(_stopAct);
	_debuggerMenu = new QMenu("Debugger");
	_debuggerMenu->addAction(_debuggerBuilderAct);
	_debuggerMenu->addAction(_debuggerStartAct);
	_debuggerMenu->addAction(_debuggerStopAct);
	_debuggerMenu->addAction(_debuggerNextLineAct);
	_debuggerMenu->addAction(_debuggerContinueAct);
	_debuggerMenu->addAction(_infoLocalAct);
	_helpMenu = new QMenu("Help");
	_helpMenu->addAction(_aboutAct);
	menuBar()->addMenu(_fileMenu);
	menuBar()->addMenu(_editMenu);
	menuBar()->addMenu(_projectMenu);
	menuBar()->addMenu(_executeMenu);
	menuBar()->addMenu(_debuggerMenu);
	menuBar()->addMenu(_helpMenu);

	// Projects view
	_projectsView = new ProjectsTree(this);


	// Active (current) projects view
	_activeProjView = new QComboBox(this);
	_activeProjView->setMinimumSize(_ACTIVE_PROJ_VIEW_SIZE);

	// Toolbar
	_toolBar = new QToolBar(this);
	_toolBar->addAction(_saveSourceAct);
	_toolBar->addAction(_saveAllSourcesAct);
	_toolBar->addSeparator();
	_toolBar->addAction(_printAct);
	_toolBar->addSeparator();
	_toolBar->addAction(_undoAct);
	_toolBar->addAction(_redoAct);
	_toolBar->addSeparator();
	_toolBar->addAction(_addSourceAct);
	_toolBar->addAction(_closeSourceAct);
	_toolBar->addAction(_closeAllSourcesAct);
	_toolBar->addSeparator();
	_toolBar->addAction(_buildAct);
	_toolBar->addAction(_runAct);
	_toolBar->addAction(_stopAct);
	_toolBar->addSeparator();
	_toolBar->addWidget(_activeProjView);
	_toolBar->addAction(_debuggerBuilderAct);
	_toolBar->addAction(_debuggerStartAct);
	_toolBar->addAction(_debuggerContinueAct);
	_toolBar->addAction(_debuggerStopAct);
	_toolBar->addAction(_debuggerNextLineAct);
	_toolBar->addAction(_infoLocalAct);
	_toolBar->setIconSize(_TOOLBAR_ICON_SIZE);
	_toolBar->setFixedHeight(_TOOLBAR_ICON_SIZE.height());
	_toolBar->setMovable(false);
	addToolBar(_toolBar);

	// Sources view
	_sourcesView = new QWidget();
	QVBoxLayout* sourcesLayout = new QVBoxLayout();
	_sourcesTabs = new QTabWidget();
	_sourcesTabs->setTabsClosable(true);
	_sourcesTabs->setMovable(true);
	_statusBar = new QStatusBar();
	sourcesLayout->addWidget(_sourcesTabs);
	sourcesLayout->addWidget(_statusBar);
	sourcesLayout->setContentsMargins(0, 2, 2, 0);
	sourcesLayout->setSpacing(0);
	_sourcesView->setLayout(sourcesLayout);

	// Build view
	_processTabs = new QTabWidget(this);
	_buildView = new ProjectBuilder();
	_processTabs->addTab(_buildView, QIcon(icons::BUILD), "Build log");


	// Central widget
	_splitterVert = new QSplitter(Qt::Vertical);
	_splitterHorz = new QSplitter(Qt::Horizontal);
	_splitterHorz->addWidget(_sourcesView);
	_splitterHorz->setStretchFactor(0, 1);
	_splitterHorz->setStretchFactor(1, 6);
	_splitterVert->addWidget(_splitterHorz);
	_splitterVert->addWidget(_processTabs);
	_splitterVert->setStretchFactor(0, 10);
	//_splitterVert->setStretchFactor(1, 1);
	setCentralWidget(_splitterVert); 
	

	// Errors view
	_errorsLstView = new QListWidget(this);
	_errorsLstView->setWordWrap(true);
	_processTabs->addTab(_errorsLstView, "Build errors");
	_processTabs->setVisible(false);
	_processTabs->setTabVisible(_processTabs->indexOf(_errorsLstView), false);

	// Connections
	connect(_newProjectAct, SIGNAL(triggered()), this, SLOT(projectNew()));
	connect(_cleanProjectAct, SIGNAL(triggered()), this, SLOT(projectClean()));
	connect(_updateContentsAct, SIGNAL(triggered()), this, SLOT(projectUpdateContents()));
	connect(_newDirAct, SIGNAL(triggered()), this, SLOT(directoryNew()));
	connect(_renameDirAct, SIGNAL(triggered()), this, SLOT(directoryRename()));
	connect(_deleteDirAct, SIGNAL(triggered()), this, SLOT(directoryDelete()));
	connect(_openProjectAct, SIGNAL(triggered()), this, SLOT(projectOpen()));
	connect(_closeProjectAct, SIGNAL(triggered()), this, SLOT(projectClose()));
	connect(_projectSettingsAct, SIGNAL(triggered()), this, SLOT(projectSettings()));
	connect(_saveSourceAct, SIGNAL(triggered()), this, SLOT(sourceSave()));
	connect(_saveAllSourcesAct, SIGNAL(triggered()), this, SLOT(sourceSaveAll()));
	connect(_closeSourceAct, SIGNAL(triggered()), this, SLOT(sourceClose()));
	connect(_closeAllSourcesAct, SIGNAL(triggered()), this, SLOT(sourceCloseAll()));
	connect(_addSourceAct, SIGNAL(triggered()), this, SLOT(sourceAdd()));
	connect(_renameSourceAct, SIGNAL(triggered()), this, SLOT(sourceRename()));
	connect(_removeSourceAct, SIGNAL(triggered()), this, SLOT(sourceRemove()));
	connect(_printAct, SIGNAL(triggered()), this, SLOT(print()));
	connect(_buildAct, SIGNAL(triggered()), this, SLOT(build()));
	connect(_runAct, SIGNAL(triggered()), this, SLOT(run()));
	connect(_stopAct, SIGNAL(triggered()), this, SLOT(runStop()));
	connect(_undoAct, SIGNAL(triggered()), this, SLOT(undo()));
	connect(_redoAct, SIGNAL(triggered()), this, SLOT(redo()));
	connect(_fontAct, SIGNAL(triggered()), this, SLOT(changeFont()));
	connect(_aboutAct, SIGNAL(triggered()), this, SLOT(about()));
	connect(_sourcesTabs, SIGNAL(currentChanged(int)), this, SLOT(updateUI()));
	connect(_sourcesTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(sourceTabClosed(int)));
	connect(_errorsLstView, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(errorsListItemDoubleClick(QListWidgetItem*)));
	connect(_activeProjectAct, SIGNAL(triggered()), this, SLOT(setActiveProject()));
	connect(_activeProjView, SIGNAL(currentIndexChanged(int)), this, SLOT(currentProjectViewChange(int)));
	connect(_saveAllExitAct, SIGNAL(triggered()), this, SLOT(saveAllExit()));
	connect(_buildView, SIGNAL(buildReadyError(int, QProcess::ExitStatus, QString)), this, \
		SLOT(buildFinished(int, QProcess::ExitStatus, QString)));
	connect(_importCMakeAct, SIGNAL(triggered()), this, SLOT(importCMakeProject()));
	connect(_buildView, SIGNAL(updateMainWindowUI()), this, SLOT(updateUI()));
	connect(_debuggerStartAct, SIGNAL(triggered()), this, SLOT(debuggerStart()));
	connect(_debuggerStopAct, SIGNAL(triggered()), this, SLOT(debuggerStop()));
	connect(_debuggerContinueAct, SIGNAL(triggered()), this, SLOT(debuggerContinue()));
	connect(_debuggerNextLineAct, SIGNAL(triggered()), this, SLOT(debuggerNextLine()));
	connect(_debugger, SIGNAL(debuggingFinished(QString)), this, SLOT(debuggerFinish(QString)));
	connect(_debuggerBuilderAct, SIGNAL(triggered()), this, SLOT(buildDebug()));
	connect(_debugger, SIGNAL(debuggerGoToLine(QString, int)), this, SLOT(debuggerGoLine(QString, int)));
	connect(_infoLocalAct, SIGNAL(triggered()), this, SLOT(infoLocal()));
	connect(_exitAct, SIGNAL(triggered()), this, SLOT(saveAllExit()));

	// Check for project opening by args
	if (qApp->arguments().count() == 2)
		projectOpen(qApp->arguments().at(1));

	updateRecentMenu();
	updateUI();

}


void MainWindow::updateRecentMenu() {
	_recentProjectsMenu->clear();
	std::list<QString> recents = _session.recentProjects();
	for (auto& p : recents) {
		QAction* act = new QAction(p, this);
		connect(act, SIGNAL(triggered()), this, SLOT(projectOpen()));
		_recentProjectsMenu->addAction(act);
	}
}


void MainWindow::updateUI() {

	// Actions
	if (currentSource()) {
		_saveSourceAct->setEnabled(!currentSource()->isSaved());
		_undoAct->setEnabled(currentSource()->isUndoAvailable());
		_redoAct->setEnabled(currentSource()->isRedoAvailable());
	}
	else {
		_saveSourceAct->setEnabled(false);
		_undoAct->setEnabled(false);
		_redoAct->setEnabled(false);
	}

	if (_projectsView->activeProject()) {
		_runAct->setEnabled(_projectsView->activeProject()->outputExists());
		_debuggerStartAct->setEnabled( \
			_projectsView->activeProject()->outputExists() && !_debugger->isDebugging() \
		);
	}
	else {
		_debuggerStartAct->setEnabled(false);
		_runAct->setEnabled(false);
	}

	_fontAct->setEnabled(currentSource());
	_cleanProjectAct->setEnabled(_projectsView->activeProject());
	_updateContentsAct->setEnabled(_projectsView->activeProject());
	_stopAct->setEnabled(_runProcess);
	_printAct->setEnabled(currentSource());
	_saveAllSourcesAct->setEnabled(false);
	_printAct->setEnabled(currentSource());
	_addSourceAct->setEnabled(_projectsView->activeProject());
	_closeSourceAct->setEnabled(currentSource());
	_closeAllSourcesAct->setEnabled(currentSource());
	_buildAct->setEnabled(_projectsView->activeProject());
	_closeProjectAct->setEnabled(_projectsView->activeProject());
	_projectSettingsAct->setEnabled(_projectsView->activeProject());
	_addSourceAct->setEnabled(_projectsView->activeProject());

	
	_debuggerStopAct->setEnabled(_debugger->isDebugging());
	_debuggerNextLineAct->setEnabled(_debugger->isDebugging());
	_debuggerContinueAct->setEnabled(_debugger->isDebugging());
	_infoLocalAct->setEnabled(_debugger->isDebugging());
	_debuggerBuilderAct->setEnabled(_projectsView->activeProject() && !_debugger->isDebugging());
	

	for (int i = 0; i < _sourcesTabs->count(); i++)
		if (!dynamic_cast<Source*>(_sourcesTabs->widget(i))->isSaved())
			_saveAllSourcesAct->setEnabled(true);

	// Views
	_splitterVert->setVisible(_projectsView->activeProject());
	_projectsView->setVisible(_projectsView->activeProject());
	_activeProjView->setEnabled(_projectsView->activeProject());

	if (_sourcesTabs->count() == 0)
		_statusBar->showMessage("");

}


void MainWindow::projectUpdateContents() {

	QAction* action = dynamic_cast<QAction*>(sender());
	if (!action)
		return;

	checkDebugging();

	if  (QMessageBox::warning(this, "Refresh?", "Refresh will update sources from disk\nContinue?", \
		QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Abort) != QMessageBox::StandardButton::Ok)
		return;


	Project* project = 0;

	ProjectDirectory* directory = dynamic_cast<ProjectDirectory*>(_projectsView->currentItem());

	if (action->data().toBool())
		project = dynamic_cast<Project*>(_projectsView->currentItem());
	else
		project = _projectsView->activeProject();
	
	if (project) {
		project->srcDirectory()->updateContents();
		project->binDirectory()->updateContents();
	}
	else if (directory) {
		directory->updateContents();
		project = directory->project();
	}

	
	// Update the sources view
	int tabsCount = _sourcesTabs->count();
	for (int i = tabsCount; i >= 0; i--)
		if (_sourcesTabs->tabText(i).startsWith(project->name() + ": "))
			if (!dynamic_cast<Source*>(_sourcesTabs->widget(i))->read())
				delete _sourcesTabs->widget(i);

	updateUI();
		
}

void MainWindow::importCMakeProject() {

	checkDebugging();

	QFileDialog folderDialog;
	QString path = folderDialog.getExistingDirectory(this, "Select a folder where to create the project");
	if (path.isEmpty())
		return;
	if (path.contains(" ")) {
		QMessageBox::critical(this, "Invalid path", "Can't select a path that contains spaces!", QMessageBox::Cancel);
		return;
	}

	createNewProject(QFileInfo(path).baseName(), path, ProjectType::CMAKE);

}

void MainWindow::projectClean() {

	checkDebugging();

	Project* project = 0;
	QAction* action = dynamic_cast<QAction*>(sender());
	if (!action)
		return;

	if (action->data().toBool())
		project = dynamic_cast<Project*>(_projectsView->currentItem());
	else
		project = _projectsView->activeProject();

	if (!project) return;

	QMessageBox::StandardButton choice =
		QMessageBox::question(this, "Cleaning project", \
			"Notice: if you clean the project some informations about sources will be removed from .qdev project file", \
			QMessageBox::Ok | QMessageBox::Cancel);

	if (choice == QMessageBox::Ok) 
		project->cleanQdev();
	


}

void MainWindow::directoryNew() {

	checkDebugging();

	QTreeWidgetItem* item = _projectsView->currentItem();

	bool ok;
	QString dirName = QInputDialog::getText(this, "New directory",
		"Directory name:", QLineEdit::Normal, "", &ok);

	if (!(ok && !dirName.isEmpty()))
		return;

	if (dynamic_cast<ProjectDirectory*>(item))
		dynamic_cast<ProjectDirectory*>(item)->newDirectory(dirName);

}

void MainWindow::directoryRename() {

	checkDebugging();

	ProjectDirectory *projDir = dynamic_cast<ProjectDirectory*>(_projectsView->currentItem());
	if (!projDir)
		return;

	if (projDir == projDir->project()->srcDirectory() || projDir == projDir->project()->binDirectory()) {
		QMessageBox::critical(this, "Can't rename this directory", "Can't rename the src and bin directories");
		return;
	}

	bool ok;
	QString newName = QInputDialog::getText(this, "Rename directory",
		"Directory name:", QLineEdit::Normal, projDir->dirName(), &ok);

	if (!(ok && !newName.isEmpty()))
		return;

	QString oldRelativePath = projDir->relativePath();

	projDir->renameDirectory(newName);

	int oldIndex = _sourcesTabs->currentIndex();

	for (int i = 0; i < _sourcesTabs->count(); i++) {
		if (_sourcesTabs->tabText(i).startsWith(projDir->project()->name() + ": " + oldRelativePath)) {

			_sourcesTabs->setTabText(i, _sourcesTabs->tabText(i).replace(\
				projDir->project()->name() + ": " + oldRelativePath,\
				projDir->project()->name() + ": " + projDir->relativePath()));

			_sourcesTabs->setCurrentIndex(i);

			Source* src = currentSource();
			src->updateDirPath(projDir->dirFullPath());
		}
	}

	// Restore old index
	_sourcesTabs->setCurrentIndex(oldIndex);


}

void MainWindow::directoryDelete() {

	checkDebugging();

	ProjectDirectory* directory = dynamic_cast<ProjectDirectory*>(_projectsView->currentItem());

	if (!directory)
		return;

	if (QMessageBox::question(this, "Delete directory?", "Do you want to delete " + directory->dirName() + "?") != QMessageBox::Yes)
		return;

	Project* parentProj = directory->project();
	if (directory == parentProj->srcDirectory() || directory == parentProj->binDirectory()) {
		QMessageBox::critical(this, "Can't delete this directory", "The src and bin directories cannot be deleted");
		return;
	}

	directory->deleteDirectory();
	
	updateUI();
}
 
Source* MainWindow::currentSource() {

	return dynamic_cast<Source*>(_sourcesTabs->currentWidget());

}


bool MainWindow::openSource(ProjectSource* projectSource, bool fromFile, bool helloworld){

	/* If there isn't an associated projectsView source, then there is no source that can be created 
	* in the sources tabs */
	if (!projectSource)
		return false;

	QFileInfo info(projectSource->sourcePath());

	ProjectDirectory* projectDir = projectSource->parentDirectory();
	Project* parentProj = projectSource->project();

	QString projName = parentProj->name();
	QString tabText = projName + ": " + projectSource->relativePath();

	// Check whether source is already opened (if yes, select it and return true)
	for (int i = 0; i < _sourcesTabs->count(); i++)
		if (_sourcesTabs->tabText(i) == tabText) {
			_sourcesTabs->setCurrentIndex(i);
			return true;
		}

	// Open source file	
	Source* src = new Source(projectSource->sourcePath(), fromFile, helloworld);
	_sourcesTabs->addTab(src, tabText);
	_sourcesTabs->setCurrentWidget(src);
	_sourcesTabs->setFocus();

	
	// Source-connections
	connect(src, SIGNAL(textChanged()), this, SLOT(updateUI()));
	connect(src, SIGNAL(textChanged()), this, SLOT(checkDebugging()));
	connect(src, SIGNAL(breakpointChanged(Source*)), this, SLOT(updateBreakpoints(Source*)));
	connect(src, SIGNAL(cursorPositionChanged()), this, SLOT(sourceCursorChanged()));
	

	src->addBreakpoints(parentProj->readBreakpoints(projectSource->relativePath()));

	updateUI();

	return false;
}



void MainWindow::setActiveProject(Project* proj){

	checkDebugging();

	Project* currentProj;
	if (!proj)
		currentProj = dynamic_cast<Project*>(_projectsView->currentItem());
	else currentProj = proj;
	
	_activeProjView->setCurrentIndex(_activeProjView->findText(currentProj->text(0), Qt::MatchExactly));
	_projectsView->setAsActive(currentProj);
	
	updateUI();

}

void MainWindow::projectNew(){

	checkDebugging();

	CreateProjectView createProjectView;
	createProjectView.exec();

}

void MainWindow::createNewProject(QString name, QString path, ProjectType projectType) {

	checkDebugging();

	if (projectType == ProjectType::CMAKE && QFile(path + "/.qdev").exists()) {
		QMessageBox::critical(this, "Cannot create project", "A project already exists in this directory");
		return;
	}

	QString projectFolder = path + "/" + name;

	if (QDir(projectFolder).exists())
		QMessageBox::critical(this, "Cannot create project", "A project with the same name already exists");
	else {

		Project* proj = new Project(path, projectType);
		_projectsView->setAsActive(proj);
		_projectsView->insertTopLevelItem(0, proj);
		_splitterHorz->insertWidget(0, _projectsView);
		_session.addRecentProject(proj->path());
		_activeProjView->insertItem(0, proj->icon(0), proj->text(0));
		_activeProjView->setCurrentIndex(0);

		if (projectType != ProjectType::CMAKE)
			Source* src = new Source(proj->sourcesPath() + "/main.cpp", false, true);

		proj->srcDirectory()->updateContents(true, false);
		
	}

	updateRecentMenu();
	updateUI();
	
	

}

void MainWindow::projectOpen(QString fromArgv){

	checkDebugging();

	QString folderPath;

	if (QObject::sender() != _openProjectAct && fromArgv.isEmpty())
		folderPath = dynamic_cast<QAction*>(QObject::sender())->text();
	else if (!fromArgv.isEmpty())
		folderPath = fromArgv;
	else 
		folderPath = QFileDialog::getExistingDirectory(this, tr("Select a project folder"));

	if (!folderPath.isEmpty()){
		if (!QDir(folderPath).exists()){
			QMessageBox::critical(this, "Cannot load project", "Project does not exist");
			return;
		}

		QFileInfo info(folderPath);

		// Check if project is already open
		QList<QTreeWidgetItem*> allItems = _projectsView->findItems(info.baseName(), Qt::MatchExactly | Qt::MatchRecursive, 0);
		for (auto* item : allItems)
			if (dynamic_cast<Project*>(item)) {
				QMessageBox::critical(this, "Cannot load project", "Project already open");
				return;
			}

		Project* proj = new Project(folderPath);
		_projectsView->setAsActive(proj);
		if (_projectsView->topLevelItemCount() == 0)
			_splitterHorz->insertWidget(0, _projectsView);

		_projectsView->insertTopLevelItem(0, proj);
		proj->setExpanded(true);
		_activeProjView->insertItem(0, proj->icon(0), proj->text(0));
		_activeProjView->setCurrentIndex(0);


		_session.addRecentProject(proj->path());
	}

	

	updateRecentMenu();
	updateUI();
	

}

bool MainWindow::projectClose(){

	checkDebugging();

	Project* toCloseProj = dynamic_cast<Project*>(_projectsView->currentItem());

	QMessageBox::StandardButton choice =
		QMessageBox::question(this, "Close project?",
			"Do you confirm to close the current project?",
			QMessageBox::Ok | QMessageBox::Cancel);
	if (choice != QMessageBox::Ok)
		return false;

	if (!sourceCloseAll(toCloseProj))
		return false;

	_activeProjView->removeItem(_activeProjView->findText(toCloseProj->text(0), Qt::MatchExactly));

	delete toCloseProj;

	updateUI();
	
	return true;

}

void MainWindow::projectSettings(){

	checkDebugging();
	
	QAction* action = dynamic_cast<QAction*>(sender());

	if (!_projectsView->activeProject() || !action)
		return;

	Project* project = 0;
	
	if (action->data().toBool())
		project = dynamic_cast<Project*>(_projectsView->currentItem());
	else
		project = _projectsView->activeProject();


	ProjectOptionsDialog dialog(project, this);
	if (dialog.exec() == QDialog::Accepted)
		project->write();

	
}

void MainWindow::sourceSave(){

	checkDebugging();

	if (currentSource()) {
		QString tabText = _sourcesTabs->tabText(_sourcesTabs->currentIndex());
		QString projectName = tabText.mid(0, tabText.indexOf(":"));

		const unsigned int relPathStartPos = 2;
		QString currentSourcePath = tabText.mid(tabText.indexOf(": ") + relPathStartPos);

		// Find all the items and with the for loop scan them and check only for project items
		QList<QTreeWidgetItem*> itemsList =_projectsView->findItems(projectName, Qt::MatchExactly, 0);
		for (auto* item : itemsList) 
			if (dynamic_cast<Project*>(item)) {

				Project* parentProj = dynamic_cast<Project*>(item);
				ProjectSource* src = parentProj->findProjectSource(currentSourcePath);
				if (src)
					parentProj->writeBreakpoints(src->relativePath(), currentSource()->breakpoints());

				break;
			}
	}else
		return;
	
	currentSource()->write();
	updateUI();

}

void MainWindow::saveAllExit() {

	checkDebugging();

	if (dynamic_cast<QAction*>(sender()) == _exitAct)
		QApplication::exit();

	sourceSaveAll();
	QApplication::exit();

}

void MainWindow::sourceSaveAll(Project* proj){

	checkDebugging();

	for (int i = 0; i < _sourcesTabs->count(); i++)
		if (proj) {
			if (_sourcesTabs->tabText(i).startsWith(proj->name() + ": "))
				dynamic_cast<Source*>(_sourcesTabs->widget(i))->write();
		}
		else
			dynamic_cast<Source*>(_sourcesTabs->widget(i))->write();

	updateUI();

}

bool MainWindow::sourceClose(int index){

	bool isClosed;

	if (index != -1)
		isClosed = sourceTabClosed(index);
	else
		isClosed = sourceTabClosed(_sourcesTabs->currentIndex());

	updateUI();

	return isClosed;

}

bool MainWindow::sourceCloseAll(Project* toCloseProj){

	int tabsCount = _sourcesTabs->count();
	
	if (toCloseProj) 
		for (int i = tabsCount; i >= 0; i--)
			if (_sourcesTabs->tabText(i).startsWith(toCloseProj->name() + ": "))
				if (!sourceClose(i))
					return false;

	updateUI();
				
	return true;

}

void MainWindow::sourceAdd(){

	checkDebugging();

	ProjectDirectory* projectDir = dynamic_cast<ProjectDirectory*>(_projectsView->currentItem());

	if (!projectDir || projectDir->project()->binDirectory() == projectDir)
		return;

	QString srcName = QInputDialog::getText(this, "Add source", "Source name:");
	if (srcName.isEmpty())
		return;

	QString srcPath = projectDir->dirFullPath() + "/" + srcName;

	ProjectSource* prjSrc = new ProjectSource(QFileInfo(srcPath), projectDir);
	if (!openSource(prjSrc, false))
		projectDir->addChild(prjSrc);
	else {
		delete prjSrc;
		QMessageBox::critical(this, "Internal error", "Error creating new file, abort.");
		return;
	}

	updateUI();
	
}

void MainWindow::sourceRemove(){

	checkDebugging();

	ProjectSource* projSource = dynamic_cast<ProjectSource*>(_projectsView->currentItem());
	if (!projSource)
		return;
	
	QString toremoveStr = projSource->sourceName();

	QMessageBox::StandardButton choice =
		QMessageBox::question(this, "Confirm", QString("Do you want to remove ") + toremoveStr + "?", QMessageBox::Yes | QMessageBox::Cancel);

	if (choice == QMessageBox::Yes){

		int currentIndex = _sourcesTabs->currentIndex();

		openSource(projSource);
		currentSource()->remove();
		delete currentSource();

		_sourcesTabs->setCurrentIndex(currentIndex);
		ProjectDirectory* parentDirectory = projSource->parentDirectory();

		parentDirectory->removeChild(projSource);

		parentDirectory->project()->removeSourceFromQdev(projSource->relativePath());
		parentDirectory->setExpanded(true);

		updateUI();
		
	}
}

void MainWindow::sourceRename(){

	checkDebugging();

	ProjectSource* projSource = dynamic_cast<ProjectSource*>(_projectsView->currentItem());
	if (!projSource)
		return;

	ProjectDirectory* projectDir = projSource->parentDirectory();

	QString projName = projSource->project()->name();
	QString tabText = projName + ": " + projSource->relativePath();

	QString newName = QInputDialog::getText(this, "Rename file", "Rename to", QLineEdit::Normal, projSource->sourceName());
	if (!newName.isEmpty()){

		bool alreadyOpened = openSource(projSource);
		currentSource()->rename(newName);

		projSource->updateSourceInfo(QFileInfo(projectDir->dirFullPath() + "/" + newName));

		if (!alreadyOpened)
			delete currentSource();
		else {

			int currentIndex = -1;
			for (int i = 0; i < _sourcesTabs->count(); i++)
				if (_sourcesTabs->tabText(i) == tabText)
					currentIndex = i;
			_sourcesTabs->setCurrentIndex(currentIndex);

			tabText = projName + ": " + projSource->relativePath();
			_sourcesTabs->setTabText(currentIndex, tabText);
			
		}


		updateUI();
	}
}

void MainWindow::print(){
	QPrinter printer;
	QPrintDialog printDialog(&printer, this);
	if (printDialog.exec() == QDialog::Accepted)
		currentSource()->print(&printer);
}

void MainWindow::undo(){

	if (currentSource())
		currentSource()->undo();

	updateUI();
}

void MainWindow::redo(){
	if (currentSource())
		currentSource()->redo();

	updateUI();
}

void MainWindow::setFont(QFont& newFont){
	_session.setFont(newFont);

	for (int i = 0; i < _sourcesTabs->count(); i++)
		dynamic_cast<Source*>(_sourcesTabs->widget(i))->setFont(newFont);
}

void MainWindow::changeFont(){
	if (!currentSource())
		return;

	bool dialogAccepted;
	QFont font = QFontDialog::getFont(&dialogAccepted, currentSource()->font(), this);

	if (dialogAccepted)
		setFont(font);
}

void MainWindow::updateBreakpoints(Source* src) {

	bool found = false;

	for (int i = 0; i < _sourcesTabs->count() && !found; i++) {

		if (dynamic_cast<Source*>(_sourcesTabs->widget(i)) == src) {
			QString tabText = _sourcesTabs->tabText(i);
			QString projectName = tabText.mid(0, tabText.indexOf(":"));

			const unsigned int relPathStartPos = 2;
			QString currentSourcePath = tabText.mid(tabText.indexOf(": ") + relPathStartPos);

			// Find all the items and with the for loop scan them and check only for project items
			QList<QTreeWidgetItem*> itemsList = _projectsView->findItems(projectName, Qt::MatchExactly, 0);
			for (auto* item : itemsList)
				if (dynamic_cast<Project*>(item)) {

					Project* parentProj = dynamic_cast<Project*>(item);
					ProjectSource* projsrc = parentProj->findProjectSource(currentSourcePath);
					if (projsrc)
						parentProj->writeBreakpoints(projsrc->relativePath(), src->breakpoints());

					found = true;
					break;
				}

		}
	}

	if (found && _debugger->isDebugging()) 
		_debugger->updateBreakpoints();
	

}

void MainWindow::debuggerStart() {

	Project* activeProject = _projectsView->activeProject();

	if (!activeProject)
		return;

	if (QProcess::execute(debuggerconstants::DEBUGGER, { debuggerconstants::VERSION }) != 0) {
		QMessageBox::critical(this, "GDB not found.", "Please install gdb and update PATH to make debugger works");
		return;
	}

	if (activeProject->getType() == ProjectType::STATIC || \
		activeProject->getType() == ProjectType::SHARED) {

		QMessageBox::critical(this, "Can't debug this project", \
			"Only Makefile|CMake projects with specified executable path and Standard Executable projects can be debugged");
		return;

	}

	_debuggerStartAct->setEnabled(false);
	_debuggerStopAct->setEnabled(true);
	_debuggerNextLineAct->setEnabled(true);
	_debuggerContinueAct->setEnabled(true);
	_infoLocalAct->setEnabled(true);

	_debugger->setProject(activeProject);
	_debugger->startDebugging();

	
}

void MainWindow::debuggerStop() {

	_debuggerNextLineAct->setEnabled(false);
	_debuggerContinueAct->setEnabled(false);
	_debuggerBuilderAct->setEnabled(true);
	_infoLocalAct->setEnabled(false);
	_debugger->stopDebugging();


}

void MainWindow::debuggerContinue() {

	_debugger->continueRun();

}

void MainWindow::debuggerNextLine() {

	_debugger->step();

}

bool MainWindow::checkDebugging() {

	if (_debugger->isDebugging()) {
		QMessageBox::critical(this, "Debugging is running", "Debugging stopped because of external activities");
		_debugger->stopDebugging();
		return true;
	}

	return false;
}

void MainWindow::debuggerFinish(QString message) {

	if (message != "finish") 
		QMessageBox::critical(this, "Error during debugging", "Error from GDB \n" + message);
	
	_debuggerStartAct->setEnabled(true);
	_debuggerStopAct->setEnabled(false);
	_debuggerNextLineAct->setEnabled(false);
	_debuggerContinueAct->setEnabled(false);
	_debuggerBuilderAct->setEnabled(true);
	_infoLocalAct->setEnabled(false);

	for (int i = 0; i < _sourcesTabs->count(); i++)
		dynamic_cast<Source*>(_sourcesTabs->widget(i))->removeHighlight();


}

void MainWindow::debuggerGoLine(QString sourcePath, int line) {


	// If the sourcepath is empty then gdb is working on the same source as before
	if (currentSource() && sourcePath == "") {
		if (line > currentSource()->blockCount())
			return;
		currentSource()->removeHighlight();
		currentSource()->highlightLine(line, SRC_NEXT_LINE_COLOR);
		currentSource()->gotoLine(line);
	}

	// Try to search source file in tabs
	bool found = false;
	Project* project = _projectsView->activeProject();
	for (int i = 0; i < _sourcesTabs->count() && !found; i++) {

		if (_sourcesTabs->tabText(i) == project->name() + ": " + sourcePath) {
			Source* src = dynamic_cast<Source*>(_sourcesTabs->widget(i));
			if (src->blockCount() <= line)
				return;
			src->removeHighlight();
			src->highlightLine(line, SRC_NEXT_LINE_COLOR);
			src->gotoLine(line);
			_sourcesTabs->setCurrentIndex(i);
			found = true;
		}

	}

	// If source isn't open then scan project sources and find it
	if (!found && sourcePath != "") {

		ProjectSource* source = project->findProjectSource(sourcePath);
		if (!source)
			return;
		openSource(source);
		currentSource()->removeHighlight();
		currentSource()->highlightLine(line, SRC_NEXT_LINE_COLOR);
		currentSource()->gotoLine(line);

	}

}

void MainWindow::infoLocal() {

	if (!_debugger->isDebugging())
		return;

	QString response = _debugger->infoLocals();
	if (response.contains("(gdb)"))
		response.remove("(gdb)");

	InfoLocalsDialog infoDialog(response);
	infoDialog.exec();
}


void MainWindow::buildDebug() {

	if (!_projectsView->activeProject())
		return;

	if (_projectsView->activeProject()->getType() == ProjectType::MAKEFILE)
		QMessageBox::warning(this, "Not executable project", \
			"Building this project with debug symbols affects only Standard QDevelop projects\n" \
			"Configure manually Makefile projects to build sources with debug symbols.");
	

	build(buildconstants::BuildMode::DEBUG);

}

void MainWindow::about(){

	QString text("<html><h1>QDevelop v");
	text = text + QString::number(QDEVELOP_MAJOR) + "." + QString::number(QDEVELOP_MINOR) + "." + QString::number(QDEVELOP_PATCH) + "</h1>"
		"<big>A Qt-based IDE for C++ development</big><br><br>"
		"Developed by the class of <i>Tecniche di Programmazione 2021</i> "
		"under the supervision of Prof. Alessandro Bria (email: <a style=\"color: blue;\" href=\"mailto:a.bria@unicas.it\">a.bria@unicas.it</a>)</li><br><br>"
		"Version 2.0 developed by Andrea Carcone <br>(email: <a style=\"color: blue;\" href=\"mailto:andrea.carcone99@gmail.com\">andrea.carcone99@gmail.com</a>)<br><br>"
		"For further info, please visit our <a style=\"color: blue;\" href=\"https://drive.google.com/drive/folders/1fi0YWkEeuYx9H2Qa7W8AVy72Rk6pI52Wy4-8FZQJ6YRv8yNFES5Q5T2mzEjfXvzOgj4-3l-8?usp=sharing\">website</a>.<br>"
		"<br><small>(c) 2021 University of Cassino and L.M.</small></html>";

	QMessageBox::about(this, "About QDevelop", text);

}

bool MainWindow::sourceTabClosed(int index){
	Source* src = dynamic_cast<Source*>(_sourcesTabs->widget(index));
	if (src){
		if (!src->isSaved()){
			QMessageBox::StandardButton choice = 
				QMessageBox::question(this, "Confirm", QString("Save changes to ") + src->name() + "?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			if (choice == QMessageBox::Yes)
				src->write();
			else if (choice != QMessageBox::No)
				return false;
		}
		delete src;
	}

	updateUI();

	return true;
}

void MainWindow::sourceCursorChanged(){
	if (currentSource()) {
		int line = currentSource()->textCursor().blockNumber();
		int column = currentSource()->textCursor().positionInBlock();
		_statusBar->showMessage(QString("Line: " + QString::number(line+1) + "   Column: " + QString::number(column)));
	}
	
	
}

void MainWindow::closeEvent(QCloseEvent* e){

	checkDebugging();

	if (_saveAllSourcesAct->isEnabled()){
		QString message = "Save changes to the following items?\n";
		for (int i = 0; i < _sourcesTabs->count(); i++)
			if (dynamic_cast<Source*>(_sourcesTabs->widget(i))->isSaved() == false)
				message += "\n " + dynamic_cast<Source*>(_sourcesTabs->widget(i))->name();

		QMessageBox::StandardButton choice = QMessageBox::question(this, "Confirm", message, QMessageBox::SaveAll | QMessageBox::NoAll | QMessageBox::Cancel);
		if (choice == QMessageBox::SaveAll){
			sourceSaveAll();
			QMainWindow::closeEvent(e);
		}
		else if (choice == QMessageBox::NoAll)
			QMainWindow::closeEvent(e);
		else
			e->ignore();

		updateUI();

	}else
		QMainWindow::closeEvent(e);
}



void MainWindow::build(buildconstants::BuildMode buildMode) {

	checkDebugging();

	if (_projectsView->activeProject()->getType() == ProjectType::CMAKE && \
		QProcess::execute(buildconstants::CMAKE, { buildconstants::CMAKE_VERSION }) != 0) {
		QMessageBox::critical(this, "CMake required", "CMake has not been found in PATH.");
		return;
	}

	sourceSaveAll(_projectsView->activeProject());

	_processTabs->setVisible(true);
	_processTabs->setCurrentWidget(_buildView);
	_processTabs->setTabVisible(_processTabs->indexOf(_errorsLstView), false);
	_errorsLstView->clear();
	_buildAct->setEnabled(false);
	_runAct->setEnabled(false);
	_projectsView->activeProject()->clearOutput();

	_buildView->setProject(_projectsView->activeProject());
	_buildView->setBuildMode(buildMode);
	_buildView->start();

	//Clean open sources from errors
	for (int i = 0; i < _sourcesTabs->count(); i++)
		dynamic_cast<Source*>(_sourcesTabs->widget(i))->removeHighlight();

	updateUI();

	
}

void MainWindow::openSourceError(Project* project, QString line) {

	QString sourcePath;
	QString currentLine = line;

	unsigned int errorLine;

	// Some line editing in order to catch source name correctly
	if (currentLine.contains("\\"))
		currentLine = currentLine.replace("\\","/");

	if (currentLine.contains(project->sourcesPath()))
		currentLine = currentLine.replace(project->sourcesPath() + "/", "");


	QString lineSubstr;

	sourcePath = currentLine.mid(0, currentLine.indexOf(":"));
	/* 
	 *	If is a MAKEFILE project then the Makefile is inside the sources path
	 *	so the src path should be added before the source name
	 */
	if (project->getType() == ProjectType::MAKEFILE)
		sourcePath = project->srcDirectory()->relativePath() + "/" + sourcePath;
	/*
	 *	If is a CMAKE project then the Makefile is in the build directory
	 *	and the source path contains full path so the sources path should be removed
	 */
	else if (project->getType() == ProjectType::CMAKE) 
		sourcePath = QFileInfo(project->sourcesPath() + "/").dir().dirName() + "/" + \
			sourcePath.remove(project->sourcesPath());
	
	lineSubstr = currentLine.mid(currentLine.indexOf(":") + 1, currentLine.length());
	lineSubstr = lineSubstr.left(lineSubstr.indexOf(":"));
	currentLine = lineSubstr;

	// Check if the error comes from a source file or from an external file 
	QSet<ProjectSource*> allProjSrcs = project->allProjectSources();
	ProjectSource* src = 0;
	QSetIterator<ProjectSource*> i(allProjSrcs);
	while (i.hasNext() && src == 0) {
		ProjectSource* probableSrc = i.next();
		if (probableSrc->relativePath() == sourcePath || probableSrc->relativePath() == "/" + sourcePath)
			src = probableSrc;
	}

	if (!src)
		return;

	errorLine = currentLine.toUInt();

	openSource(src);
	currentSource()->setFocus();

	if (line.contains(": warning:"))
		currentSource()->highlightLine(errorLine, SRC_WARNING_COLOR);
	else if (line.contains(": error:") || line.contains(": fatal error:"))
		currentSource()->highlightLine(errorLine, SRC_ERROR_COLOR);

	currentSource()->gotoLine(errorLine);

	updateUI();

}


void MainWindow::buildFinished(int exitCode, QProcess::ExitStatus exitStatus, QString errorContent) {

	if (!(exitCode == EXIT_SUCCESS && exitStatus == QProcess::NormalExit)) {
		_runAct->setEnabled(false);
		_processTabs->setTabVisible(_processTabs->indexOf(_errorsLstView), true);
		_processTabs->setCurrentWidget(_errorsLstView);

		QString line;

		QTextStream stream(&errorContent);
		QString sourceName;
		unsigned int errorLine = 0;

		Project* buildingProject = _buildView->project();

		while (stream.readLineInto(&line)) 
			if (line.contains(": error:") || line.contains(": fatal error:") || line.contains(": warning:")) {
				openSourceError(buildingProject, line);
				_errorsLstView->addItem(line);
			}
		
	}else 
		_runAct->setEnabled(true);

	updateUI();
	
}


void MainWindow::errorsListItemDoubleClick(QListWidgetItem* item){

	openSourceError(_buildView->project(), item->text());
	
}


void MainWindow::run() {

	if (!_projectsView->activeProject())
		return;

	checkDebugging();

	if (_projectsView->activeProject()->getType() == ProjectType::SHARED || \
		_projectsView->activeProject()->getType() == ProjectType::STATIC) {
		QMessageBox::critical(this, "Can't run this project", "Shared libraries and Static libraries can't be launched\n");
		return;
	}


	_runAct->setEnabled(false);
	_stopAct->setEnabled(true);

	_runProcess = new QProcess();

	// If the project is a makefile|cmake project then use different arguments
	bool makefile = false;
	if (_projectsView->activeProject()->getType() == ProjectType::MAKEFILE || \
		_projectsView->activeProject()->getType() == ProjectType::CMAKE) {

		if (_projectsView->activeProject()->runMakeFile().isEmpty()) {
			QMessageBox::critical(this, "No run button associated.", "Go to Project->Options and set run path to specify application to launch for makefile|cmake projects");
			return;
		}

		makefile = true;


	}
	connect(_runProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(runFinished(int, QProcess::ExitStatus)));
	if (QSysInfo::productType() == "windows") {
		// This block of code works only when compiling on Windows (requires Windows.h)
		#ifdef _WIN32
			// Create process arguments settings args from lambda expression
			_runProcess->setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments* args){
					args->flags |= CREATE_NEW_CONSOLE;
					args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES;
				});
			if (makefile)
				_runProcess->start(_projectsView->activeProject()->runMakeFile());
			else
				_runProcess->start(QString("\"") + _projectsView->activeProject()->outputPath() + QString("\""));
		#endif
	}
	else if (QSysInfo::productType() == "osx") {
		// MacOS
		if (makefile)
			_runProcess->start(runconstants::MAC_OPEN, QStringList() << runconstants::MAC_OPEN_ARGS << runconstants::MAC_TERMINAL \
				<< _projectsView->activeProject()->runMakeFile().split(" "));
		else
			_runProcess->start(runconstants::MAC_OPEN, {runconstants::MAC_OPEN_ARGS, runconstants::MAC_TERMINAL, \
				QString("\"") + _projectsView->activeProject()->outputPath() + QString("\"") });

	}
	else  { // linux
		
		if (QProcess::execute(runconstants::XTERM + " " + runconstants::XTERM_VERSION) != 0) {
			QMessageBox::critical(this, "XTerm needed", \
				"XTerm is needed in order to launch programs.\nPlease install it from your package manager.");
			return;
		}

		if (makefile)
			_runProcess->start(runconstants::XTERM, \
				QStringList() << runconstants::XTERM_EXEC << _projectsView->activeProject()->runMakeFile());
		else
			_runProcess->start(runconstants::XTERM, \
				QStringList() << runconstants::XTERM_EXEC << \
				QString("\"") + _projectsView->activeProject()->outputPath() + QString("\""));
	}
	

	
}


void MainWindow::runFinished(int exitCode, QProcess::ExitStatus exitStatus){

	
	delete _runProcess;
	_runProcess = 0;
	
	updateUI();

}


void MainWindow::runStop(){
	
	if (_runProcess) 
		_runProcess->kill();

	updateUI();
	
}

void MainWindow::currentProjectViewChange(int index) {

	checkDebugging();
	 
	if (_activeProjView->count() == 0) {
		_projectsView->setAsActive(0);
		return;
	}

	Project* proj = dynamic_cast<Project*>(_projectsView->findItems(_activeProjView->itemText(index), Qt::MatchExactly).at(0));
	setActiveProject(proj);

}

// End MainWindow


InfoLocalsDialog::InfoLocalsDialog(QString text) : QDialog() {

	setMinimumWidth(_MIN_WIDTH);
	setWindowTitle("Debugger Info Locals window");
	setWindowOpacity(0.8);

	_textArea = new QTextEdit();
	setInfoLocals(text);
	_textArea->setReadOnly(true);

	_mainLayout = new QVBoxLayout();
	_mainLayout->addWidget(_textArea);
	setLayout(_mainLayout);

}

void InfoLocalsDialog::setInfoLocals(QString text) {

	QStringList textlist = text.split("\r\n");
	for (QString textparagraph : textlist)
		_textArea->setText(_textArea->toPlainText() + textparagraph + "\n");

}




CreateProjectView::CreateProjectView() : QDialog() {

	setMinimumWidth(_MIN_WIDTH);
	setWindowTitle("New Project");

	_confirmButton = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	_projectNameView = new QLineEdit(this);

	_projectPathView = new QLineEdit(this);
	_projectPathView->setReadOnly(true);


	_btnSelectPath = new QPushButton("Open Directory", this);

	_projectChooserView = new QComboBox(this);
	_projectChooserView->addItems(QStringList() << "Standard C/C++ Project" << "Makefile Project");

	_projectTypeView = new QComboBox(this);
	_projectTypeView->addItems(QStringList() << "Executable" << "Static Library" << "Shared Library");

	QHBoxLayout* pathLayout = new QHBoxLayout();
	pathLayout->setContentsMargins(0, 0, 0, 0);
	pathLayout->addWidget(_projectPathView);
	pathLayout->addWidget(_btnSelectPath);

	QWidget* pathContainer = new QWidget(this);
	pathContainer->setLayout(pathLayout);

	QFormLayout* formLayout = new QFormLayout();
	formLayout->addRow("Project Name", _projectNameView);
	formLayout->addRow("Project Path", pathContainer);
	formLayout->addRow("Project", _projectChooserView);
	formLayout->addRow("Project Type", _projectTypeView);
	

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(formLayout);
	mainLayout->addWidget(_confirmButton);

	setLayout(mainLayout);

	connect(_confirmButton, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(_projectChooserView, SIGNAL(currentIndexChanged(int)), this, SLOT(projectChoosed(int)));
	connect(_confirmButton, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(_btnSelectPath, SIGNAL(released()), this, SLOT(selectPath()));
	connect(this, SIGNAL(createProject(QString, QString, ProjectType)), MainWindow::instance(), \
		SLOT(createNewProject(QString, QString, ProjectType)));


}

void CreateProjectView::projectChoosed(int index) {

	_projectTypeView->setEnabled(true);

	if (index == 1) // MAKEFILE
		_projectTypeView->setEnabled(false);


}


void CreateProjectView::selectPath() {

	QFileDialog folderDialog;
	QString path = folderDialog.getExistingDirectory(this, "Select a folder where to create the project");
	if(path.isEmpty())
		return;
	if (path.contains(" ")) {
		QMessageBox::critical(this, "Invalid path", "Can't select a path that contains spaces!", QMessageBox::Cancel);
		return;
	}

	_projectPathView->setText(path);

}



void CreateProjectView::accept() {

	if (_projectNameView->text().contains(" ") || _projectNameView->text().isEmpty() || _projectPathView->text().isEmpty()) {
		QMessageBox::critical(this, "Invalid project specifications", \
			"Can't create a project with a Name/Path that contains spaces or is empty!", QMessageBox::Cancel);
		return;
	}

	ProjectType projType;
	if (_projectChooserView->currentIndex() == 1)
		projType = ProjectType::MAKEFILE;
	else
		projType = (ProjectType) _projectTypeView->currentIndex();

	emit createProject(_projectNameView->text(), _projectPathView->text(), projType);


	QDialog::accept();

}




