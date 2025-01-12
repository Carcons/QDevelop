#pragma once


#include "Global.h"

#include <QTreeWidget>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QFileInfo>
#include <QAction>
#include <QSet>


#include <QDebug>


namespace qdev {
	class Project;
	class ProjectSource;
	class ProjectDirectory;
	class ProjectOptionsDialog;
	class ProjectsTree;
}

class qdev::ProjectsTree : public QTreeWidget {

	Q_OBJECT

	private:


		const QColor _ACTIVE_COLOR = QColor(13, 71, 161);
		const QColor _DEFAULT_COLOR = QColor(0, 0, 0);
		const QSize _PROJ_ICON_SIZE = QSize(27, 27);

		QIcon _openFolderIcon = QIcon(icons::DIR_OPEN);
		QIcon _folderIcon = QIcon(icons::DIR);


		Project* _activeProject = 0;

	public:

		ProjectsTree(QWidget* parent);

	public slots:

		void projectDirectoryExpanded(QTreeWidgetItem* item);
		void projectDirectoryCollapsed(QTreeWidgetItem* item);
		void setAsActive(Project* proj);

		//getters
		Project* activeProject() { return _activeProject; }


	public slots:

		void doubleClickReceiver(QTreeWidgetItem* item);
		void contextualMenu(const QPoint& pos);
	
	
};

class qdev::ProjectSource : public QTreeWidgetItem {

	private:

		Project* _project;
		ProjectDirectory* _parentDirectory;
		QString _sourceName;
		QString _sourceFullPath;
		QString _relativePath;

		void init(QFileInfo sourceInfo);

	public:

		ProjectSource(QFileInfo sourceInfo, ProjectDirectory* projectDirectory);

		void updateSourceInfo(QFileInfo sourceInfo);
		
		Project* project() { return _project; }
		ProjectDirectory* parentDirectory() { return _parentDirectory; }
		QString relativePath() { return _relativePath; }
		QString sourceName() { return _sourceName; }
		QString sourcePath() { return _sourceFullPath; }

};

class qdev::ProjectDirectory : public QTreeWidgetItem {

	private:

		Project* _project;
		QString _relativePath;
		QString _dirName;
		QString _dirFullPath;
		
		void scanSubDirs(bool files = true, bool dirs = true);
		void init(QFileInfo dirInfo);

	public:

		ProjectDirectory(QFileInfo dirInfo, Project* project);

		void updateDirInfo(QFileInfo dirInfo);
		void newDirectory(QString dirName);
		void renameDirectory(QString newName);
		void deleteDirectory();

		void updateContents(bool files = true, bool dirs = true);

		Project*				project() { return _project; }
		QString					dirName() { return _dirName; }
		QString					dirFullPath() { return _dirFullPath; }
		QString					relativePath() { return _relativePath; }
		QStringList				sources();
		QSet<ProjectSource*>	allDirectoryProjectSources();
		QStringList				allRelativeFilePaths();
		ProjectDirectory*		findParentDirectory(ProjectDirectory* tofind);
			
};

class qdev::Project : public QTreeWidgetItem {
	
	protected:

		const char* _PROJECT_FILE = ".qdev";

		ProjectType _type;

		ProjectDirectory* _srcProjDir;
		ProjectDirectory* _binProjDir;

		QString _compilerOptions;
		QString _preprocDefinitions;
		QString _linkerOptions;
		QString _folderPath; // Project ".qdev" folder
		QString _sourcesPath;
		QString _binPath;
		QString _includeDirs;
		QString _libraryDependencies;
		QString _makeFileArguments;
		QString _runMakeFile;

		void init();

	public:
		
	
		Project(QString path);

		
		Project(QString path, ProjectType type);

		// getters
		QString name();
		QString makeFileArguments() { return _makeFileArguments; }
		QString path() { return _folderPath; }
		QString sourcesPath() { return _sourcesPath; }
		QString binaryPath() { return _binPath; }
		QString compilerOptions() { return _compilerOptions; }
		QString preprocDefinitions() { return _preprocDefinitions; }
		QString linkerOptions() { return _linkerOptions; }
		QString includeDirs() { return _includeDirs; }
		QString libraryDependencies() { return _libraryDependencies; }
		QString runMakeFile() { return _runMakeFile; }
		ProjectType getType() { return _type; }
		ProjectDirectory* srcDirectory() { return _srcProjDir; }
		ProjectDirectory* binDirectory() { return _binProjDir; }

		// setters
		void setCompilerOptions(QString options) { _compilerOptions = options; }
		void setLinkerOptions(QString options) { _linkerOptions = options; }
		void setIncludeDirs(QString dirs) { _includeDirs = dirs; }
		void setLibraryDependencies(QString libs) { _libraryDependencies = libs; }
		void setRunMakeFile(QString path) { _runMakeFile = path; }
		void setMakeFileArguments(QString args) { _makeFileArguments = args; }
		void setPreprocDefs(QString defs) { _preprocDefinitions = defs; }

		// utility methods
		QStringList				cppFiles();		// list of all sources
		QStringList				allRelativeFilePaths() { return _srcProjDir->allRelativeFilePaths(); }
		QSet<ProjectSource*>	allProjectSources() { return _srcProjDir->allDirectoryProjectSources(); }
		QStringList				objectFiles();
		QString					outputPath();	// path of project output including OS-specific extension
		void					cleanQdev();
		bool					outputExists();	
		void					clearOutput();
		void					removeSourceFromQdev(QString path);
		void					renameSourceFromQdev(QString oldPath, QString newPath);
		void					writeBreakpoints(QString path, QVector<int> breakpoints);
		QVector<int>			readBreakpoints(QString path);
		QList<QString>			readQdevSources();
		ProjectSource*			findProjectSource(QString relativePath);
		

		// read/write
		bool read();
		void write();


		

};



class qdev::ProjectOptionsDialog : public QDialog{

	Q_OBJECT

	private:

		
		const unsigned int _MIN_WIDTH = 900;

		Project* _project = 0;
		
		QLineEdit* _makeFileArgsLine = 0;
		QLineEdit* _makeFileRunLine = 0;
		QLineEdit* _compilerToolLine = 0;
		QLineEdit* _compilerOptionsLine = 0;
		QLineEdit* _linkerOptionsLine = 0;
		QLineEdit* _binDirLine = 0;
		QLineEdit* _srcDirLine = 0;
		QTextEdit* _includeDirsText = 0;
		QTextEdit* _libsDependenciesText = 0;
		QLineEdit* _preprocessorDefsText = 0;
		QPushButton* _btnSelectPath = 0;

		QDialogButtonBox* _buttonBox = 0;

	public:

		ProjectOptionsDialog(Project* project, QWidget* parent);

	public slots:

		void accept() override;
		void selectPath();
};

