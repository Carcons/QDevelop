#pragma once

#ifdef _WIN32 
#include "Windows.h"
#endif

#include "Global.h"
#include "Project.h"
#include "Source.h"
#include "Session.h"
#include "Debugger.h" 
#include "ProjectBuilder.h"

#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QMenu>
#include <QTreeWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QProcess>
#include <QListWidget>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>



namespace qdev{

	class MainWindow;
	class CreateProjectView;
	class InfoLocalsDialog;

}



class qdev::MainWindow : public QMainWindow{
	
	Q_OBJECT 
	
	private:

		const QSize _TOOLBAR_ICON_SIZE = QSize(40, 40);
		const QSize _ACTIVE_PROJ_VIEW_SIZE = QSize(300, 34);

		// actions (buttons)
		QAction* _newProjectAct = 0;
		QAction* _cleanProjectAct = 0;
		QAction* _exitAct = 0;
		QAction* _importCMakeAct = 0;
		QAction* _openProjectAct = 0;
		QAction* _updateContentsAct = 0;
		QAction* _newDirAct = 0;
		QAction* _deleteDirAct = 0;
		QAction* _renameDirAct = 0;
		QAction* _closeProjectAct = 0;
		QAction* _saveSourceAct = 0;
		QAction* _saveAllSourcesAct = 0;
		QAction* _closeSourceAct = 0;
		QAction* _closeAllSourcesAct = 0;
		QAction* _printAct = 0;
		QAction* _undoAct = 0;
		QAction* _redoAct = 0;
		QAction* _fontAct = 0;
		QAction* _projectSettingsAct = 0;
		QAction* _addSourceAct = 0;
		QAction* _renameSourceAct = 0;
		QAction* _removeSourceAct = 0;
		QAction* _buildAct = 0;
		QAction* _runAct = 0;
		QAction* _stopAct = 0;
		QAction* _aboutAct = 0;
		QAction* _activeProjectAct = 0;
		QAction* _saveAllExitAct = 0;
		QAction* _debuggerStartAct = 0;
		QAction* _debuggerContinueAct = 0;
		QAction* _debuggerStopAct = 0;
		QAction* _debuggerNextLineAct = 0;
		QAction* _debuggerBuilderAct = 0;
		QAction* _infoLocalAct = 0;

		// Toolbar
		QToolBar* _toolBar = 0;

		// Menus
		QMenu* _fileMenu = 0;
		QMenu* _debuggerMenu = 0;
		QMenu* _recentProjectsMenu = 0;
		QMenu* _editMenu = 0;
		QMenu* _projectMenu = 0;
		QMenu* _executeMenu = 0;
		QMenu* _helpMenu = 0;

		// Views
		QDockWidget* _dock = 0;
		QSplitter*  _splitterHorz = 0;
		QSplitter*  _splitterVert = 0;
		ProjectsTree* _projectsView = 0;
		QWidget*    _sourcesView = 0;
		QTabWidget* _sourcesTabs = 0;
		QStatusBar* _statusBar = 0;
		QTabWidget* _processTabs = 0;
		ProjectBuilder*  _buildView = 0;
		QListWidget* _errorsLstView = 0;
		QComboBox* _activeProjView = 0;
		
		// Debugger
		Debugger* _debugger = 0;

		// Persistence
		Session _session;

		// Run process
		QProcess* _runProcess = 0;

		MainWindow();


	public: 
		
		// singleton
		static MainWindow* instance();

		// getters
		QAction* actionCleanProject() { return _cleanProjectAct; }
		QAction* actionImportCMake() { return _importCMakeAct; }
		QAction* actionAddSource() { return _addSourceAct; }
		QAction* actionProjectSettings() { return _projectSettingsAct; }
		QAction* actionCloseProject() { return _closeProjectAct; }
		QAction* actionRemoveSource() { return _removeSourceAct; }
		QAction* actionRenameSource() { return _renameSourceAct; }
		QAction* actionActiveProject() { return _activeProjectAct; }
		QAction* actionAddDirectory() { return _newDirAct; }
		QAction* actionUpdateContents() { return _updateContentsAct; }
		QAction* actionRenameDirectory() { return _renameDirAct; }
		QAction* actionDeleteDirectory() { return _deleteDirAct; }
		Session& session() { return _session; }
		Source* currentSource();

		// utility and setters
		void setFont(QFont& newFont);
		void openSourceError(Project* project, QString line);
		bool openSource(ProjectSource* projectSource, bool fromFile = true, bool helloworld = false);

		// event handlers
		virtual void closeEvent(QCloseEvent* e) override;


	protected slots:

		// actions slots
		void projectNew();
		void importCMakeProject();
		void projectSettings();
		void projectClean();
		void projectUpdateContents();
		void directoryNew();
		void directoryRename();
		void directoryDelete();
		void projectOpen(QString fromArgv = "");
		bool projectClose();
		void sourceSave();
		void sourceSaveAll(Project* proj = 0);
		bool sourceClose(int index = -1);
		bool sourceCloseAll(Project *toCloseProj = 0);
		void sourceAdd();
		void sourceRename();
		void sourceRemove();
		void print();
		void undo();
		void redo();
		void changeFont();
		void build(buildconstants::BuildMode buildMode = buildconstants::BuildMode::NORMAL);
		void run();
		void about();
		void saveAllExit();
	

		// user interface update slot
		void updateUI();

		// Create new project on disk slot
		void createNewProject(QString name, QString path, ProjectType projectType);
		 
		// source slots
		bool sourceTabClosed(int index);
		void sourceCursorChanged();
		void updateBreakpoints(Source* src);

		void updateRecentMenu();
		void currentProjectViewChange(int index);

		// Build logger slots
		void buildFinished(int exitCode, QProcess::ExitStatus exitStatus, QString errorContent);

		//Build list slots
		void errorsListItemDoubleClick(QListWidgetItem* item);

		//Active project slot
		void setActiveProject(Project* proj = 0);
		
		// run slots
		void runFinished(int exitCode, QProcess::ExitStatus exitStatus);
		void runStop();

		// Debug slots
		void debuggerStart();
		void debuggerStop();
		void debuggerContinue();
		void debuggerNextLine();
		bool checkDebugging();
		void debuggerFinish(QString message);
		void buildDebug();
		void debuggerGoLine(QString sourcePath, int line);
		void infoLocal();
};



class qdev::InfoLocalsDialog : public QDialog {

	Q_OBJECT

	private:

		const unsigned int _MIN_WIDTH = 500;

		QTextEdit* _textArea;
		QVBoxLayout* _mainLayout;

	public:

		InfoLocalsDialog(QString text = "");
		void setInfoLocals(QString text);

};



class qdev::CreateProjectView : public QDialog {

	Q_OBJECT

	private:

		const unsigned int _MIN_WIDTH = 600;


		QLineEdit* _projectPathView = 0;
		QLineEdit* _projectNameView = 0;
		

		QPushButton* _btnSelectPath = 0;

		QDialogButtonBox* _confirmButton = 0;

		QComboBox* _projectChooserView = 0;
		QComboBox* _projectTypeView = 0;


	public:

		CreateProjectView();

	public slots:
		
		void selectPath();
		void projectChoosed(int index);
		void accept() override;
		
	signals:

		void createProject(QString name, QString path, ProjectType projectType);


};







