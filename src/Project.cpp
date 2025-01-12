
#include "Project.h"
#include "MainWindow.h"

#include <QDir>
#include <QSysInfo>
#include <iostream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QDirIterator>
#include <QFileDialog>



using namespace qdev;

ProjectsTree::ProjectsTree(QWidget* parent) : QTreeWidget(parent) {

	setHeaderHidden(true);
	setIconSize(_PROJ_ICON_SIZE);
	setContextMenuPolicy(Qt::CustomContextMenu);
	
	
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), \
		this, SLOT(doubleClickReceiver(QTreeWidgetItem*)));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), \
		this, SLOT(contextualMenu(QPoint)));
	connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), \
		this, SLOT(projectDirectoryExpanded(QTreeWidgetItem*)));
	connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem*)), \
		this, SLOT(projectDirectoryCollapsed(QTreeWidgetItem*)));


}

void ProjectsTree::projectDirectoryExpanded(QTreeWidgetItem* item) {

	ProjectDirectory* dir = dynamic_cast<ProjectDirectory*>(item);
	if (!dir)
		return;
	dir->setIcon(0, _openFolderIcon);

}

void ProjectsTree::projectDirectoryCollapsed(QTreeWidgetItem* item) {

	ProjectDirectory* dir = dynamic_cast<ProjectDirectory*>(item);
	if (!dir)
		return;
	dir->setIcon(0, _folderIcon);

}

void ProjectsTree::doubleClickReceiver(QTreeWidgetItem* item) {

	ProjectSource* src = dynamic_cast<ProjectSource*>(item);
	if (src) {
		ProjectDirectory* parentDir = src->parentDirectory();
		ProjectDirectory* binDir = src->project()->binDirectory();

		if (binDir != parentDir && binDir != parentDir->findParentDirectory(binDir))
			MainWindow::instance()->openSource(src);
	}
	
}


void ProjectsTree::contextualMenu(const QPoint& pos) {

	QTreeWidgetItem* clicked = itemAt(pos);
	if (!clicked)
		return;

	QMenu projectMenu;
	Project* project = dynamic_cast<Project*>(clicked);
	ProjectSource* source = dynamic_cast<ProjectSource*>(clicked);
	ProjectDirectory* directory = dynamic_cast<ProjectDirectory*>(clicked);

	/* Editing the data of some actions to distinguish them in the MainWindow slots
	   If the specified menu will show, then those action's data will set to true */
	QAction* projectSettingsAct = MainWindow::instance()->actionProjectSettings();
	QAction* updateContentsAct = MainWindow::instance()->actionUpdateContents();
	QAction* cleanProjectAct = MainWindow::instance()->actionCleanProject();

	// Every item has a specific menu 
	if (project){

		projectSettingsAct->setData(true);
		updateContentsAct->setData(true);
		cleanProjectAct->setData(true);

		projectMenu.addAction(projectSettingsAct);
		projectMenu.addSeparator();
		projectMenu.addAction(MainWindow::instance()->actionActiveProject());
		projectMenu.addSeparator();
		projectMenu.addAction(updateContentsAct);
		projectMenu.addSeparator();
		projectMenu.addAction(cleanProjectAct);
		projectMenu.addAction(MainWindow::instance()->actionCloseProject());
		projectMenu.exec(mapToGlobal(pos));

	}
	else if (directory) {

		if (directory == directory->project()->binDirectory() || \
			directory->findParentDirectory(directory->project()->binDirectory()))
			return;

		projectMenu.addAction(MainWindow::instance()->actionAddSource());
		projectMenu.addSeparator();
		projectMenu.addAction(MainWindow::instance()->actionAddDirectory());
		projectMenu.addSeparator();
		if (directory != directory->project()->srcDirectory()) {
			projectMenu.addAction(MainWindow::instance()->actionRenameDirectory());
			projectMenu.addSeparator();
			projectMenu.addAction(MainWindow::instance()->actionDeleteDirectory());
		}
		projectMenu.addAction(updateContentsAct);
		projectMenu.addSeparator();
		projectMenu.exec(mapToGlobal(pos));

	}
	else if (source) {

		if (source->parentDirectory() == source->project()->binDirectory() || \
			source->parentDirectory()->findParentDirectory(source->project()->binDirectory()))
			return;

		projectMenu.addAction(MainWindow::instance()->actionRenameSource());
		projectMenu.addSeparator();
		projectMenu.addAction(MainWindow::instance()->actionRemoveSource());
		projectMenu.exec(mapToGlobal(pos));

	}
	
	// When the menu is closed then reset the actions data
	projectSettingsAct->setData(false);
	updateContentsAct->setData(false);
	cleanProjectAct->setData(false);

}

void ProjectsTree::setAsActive(Project *proj){


	if (!proj) {
		_activeProject = 0;
		return;
	}

	if (_activeProject == proj)
		return;

	
	QFont font;
	font.setBold(true);
	proj->setFont(0, font);
	proj->setTextColor(0, _ACTIVE_COLOR);

	// Check if other projects are active
	if (_activeProject) {
		font.setBold(false);
		_activeProject->setFont(0, font);
		_activeProject->setTextColor(0, _DEFAULT_COLOR);
	}

	_activeProject = proj;
	

}





// constructor 1: load existing project or import from folder
Project::Project(QString path)
	: QTreeWidgetItem(0) {

	_folderPath = path;

	if (!read()) {
		QMessageBox::critical(MainWindow::instance(), "Failed to open project", "Can't open this project, can't read project file.");
		return;
	}

	init();

}

// constructor 2: create new (empty) project
Project::Project(QString path, ProjectType type)
	: QTreeWidgetItem(0) {

	_folderPath = path;
	_type = type;
	
	if (type == ProjectType::CMAKE) {

		// Fetch the CMAKE_SOURCE_DIR variable's value from generated cmake Makefile
		QFile f(_folderPath + "/Makefile");
		if (!f.open(QIODevice::ReadOnly)) {
			QMessageBox::critical(MainWindow::instance(), "Cannot open project", \
				"Cannot import CMake project, please check your (CMake) Makefile generated build");
			return;
		}

		QString content = f.readAll();
		f.close();

		QString cmakeSrcDir = "", cmakeBinDir = "";
		const unsigned int srcPosInLine = 2;
		QTextStream stream(&content);
		QString contentLine;
		while (stream.readLineInto(&contentLine)) {
			if (contentLine.startsWith("CMAKE_SOURCE_DIR = ")) 
				cmakeSrcDir = contentLine.mid(contentLine.indexOf("= ") + srcPosInLine).replace("\\", "/");
			if (contentLine.startsWith("CMAKE_BINARY_DIR = "))
				cmakeBinDir = contentLine.mid(contentLine.indexOf("= ") + srcPosInLine).replace("\\","/");
		}

		_sourcesPath = cmakeSrcDir;
		_binPath = cmakeBinDir;

	}
	else {

		_sourcesPath = path + "/src";
		_binPath = path + "/bin";


		QDir().mkpath(path);
		QDir().mkpath(_sourcesPath);
		QDir().mkpath(_binPath);

		if (_type == ProjectType::MAKEFILE) {
			QFile makefile(_sourcesPath + "/Makefile");
			makefile.open(QIODevice::WriteOnly);
			makefile.close();
		}

	}


	init();

	write();

	
}


void Project::init() {

	setText(0, name());

	if (_type == ProjectType::EXE)
		setIcon(0, QIcon(icons::EXE));
	else if (_type == ProjectType::STATIC)
		setIcon(0, QIcon(icons::STATIC_LIB));
	else if (_type == ProjectType::MAKEFILE)
		setIcon(0, QIcon(icons::MAKEFILE_PROJ));
	else if (_type == ProjectType::SHARED)
		setIcon(0, QIcon(icons::SHARED_LIB));
	else if (_type == ProjectType::CMAKE)
		setIcon(0, QIcon(icons::CMAKE_PROJ));


	_srcProjDir = new ProjectDirectory(QFileInfo(_sourcesPath), this);
	_srcProjDir->setExpanded(true);
	addChild(_srcProjDir);

	_binProjDir = new ProjectDirectory(QFileInfo(_binPath), this);
	addChild(_binProjDir);

	
}

QStringList Project::objectFiles() {

	QStringList cpps = cppFiles();
	QStringList objs;

	for (QString& cpp : cpps) {

		if (cpp.endsWith(".c"))
			objs << cpp.mid(0, cpp.lastIndexOf(".c")) + ".o";
		else if (cpp.endsWith(".cc"))
			objs << cpp.mid(0, cpp.lastIndexOf(".cc")) + ".o";
		else
			objs << cpp.mid(0, cpp.lastIndexOf(".cpp")) + ".o";

	}

	return objs;


}


QStringList Project::cppFiles() { 

	QStringList sources;

	sources << _srcProjDir->sources();
	for (int i = 0; i < _srcProjDir->childCount(); i++) {
		ProjectDirectory* dir = dynamic_cast<ProjectDirectory*>(_srcProjDir->child(i));
		if (dir) 
			sources << dir->sources();
		
	}


	return sources;

}

QString Project::name()
{
	QFileInfo info(_folderPath);
	return info.baseName();
}

bool Project::read()
{
	QFile file(_folderPath + "/" + _PROJECT_FILE);
	if (!file.open(QIODevice::ReadOnly))
		return false;

	QByteArray file_content = file.readAll();
	QJsonDocument json_doc(QJsonDocument::fromJson(file_content));
	QJsonObject json_obj = json_doc.object();

	if (json_obj.contains("_compiler_options") && json_obj["_compiler_options"].isString())
		_compilerOptions = json_obj["_compiler_options"].toString();
	if (json_obj.contains("_linker_options") && json_obj["_linker_options"].isString())
		_linkerOptions = json_obj["_linker_options"].toString();
	if (json_obj.contains("_type") && json_obj["_type"].isDouble())
		_type = ProjectType(json_obj["_type"].toInt());
	if (json_obj.contains("_src_dir") && json_obj["_src_dir"].isString())
		_sourcesPath = json_obj["_src_dir"].toString();
	if (json_obj.contains("_bin_dir") && json_obj["_bin_dir"].isString())
		_binPath = json_obj["_bin_dir"].toString();
	if (json_obj.contains("_include_dirs") && json_obj["_include_dirs"].isString())
		_includeDirs = json_obj["_include_dirs"].toString();
	if (json_obj.contains("_library_dependencies") && json_obj["_library_dependencies"].isString())
		_libraryDependencies = json_obj["_library_dependencies"].toString();
	if (json_obj.contains("_makeArgs") && json_obj["_makeArgs"].isString())
		_makeFileArguments = json_obj["_makeArgs"].toString();
	if (json_obj.contains("_runPath") && json_obj["_runPath"].isString())
		_runMakeFile = json_obj["_runPath"].toString();
	if (json_obj.contains("_preprocDefs") && json_obj["_preprocDefs"].isString())
		_preprocDefinitions = json_obj["_preprocDefs"].toString();
	

	return true;
}

void Project::write(){

	QFile qdevFile(_folderPath + "/" + _PROJECT_FILE);
	QJsonObject qdevJson;

	if (!qdevFile.exists()) {
		if (!qdevFile.open(QIODevice::WriteOnly))
			return;

		if (_type == ProjectType::MAKEFILE || _type == ProjectType::CMAKE) {
			qdevJson["_makeArgs"] = "";
			qdevJson["_runPath"] = "";
			qdevJson["_type"] = (int)_type;
			qdevJson["_src_dir"] = _sourcesPath;
			qdevJson["_bin_dir"] = _binPath;
			qdevJson["_sources"] = "";
		}
		else {
			qdevJson["_compiler_options"] = _compilerOptions;
			if (_type != ProjectType::STATIC) qdevJson["_linker_options"] = _linkerOptions;
			qdevJson["_type"] = (int)_type;
			qdevJson["_src_dir"] = _sourcesPath;
			qdevJson["_bin_dir"] = _binPath;
			qdevJson["_include_dirs"] = "";
			if (_type != ProjectType::STATIC) qdevJson["_library_dependencies"] = "";
			qdevJson["_sources"] = "";
			qdevJson["_preprocDefs"] = "";
		}
		
		qdevFile.write(QJsonDocument(qdevJson).toJson());
		qdevFile.close();

		return;
	}

	if (!qdevFile.open(QIODevice::ReadOnly))
		return;

	qdevJson = QJsonDocument::fromJson(qdevFile.readAll()).object();
	qdevFile.close();

	qdevFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

	if (_type == ProjectType::MAKEFILE || _type == ProjectType::CMAKE) {
		qdevJson["_makeArgs"] = _makeFileArguments;
		qdevJson["_runPath"] = _runMakeFile;
		qdevJson["_type"] = (int)_type;
	}
	else {
		qdevJson["_compiler_options"] = _compilerOptions;
		if (_type != ProjectType::STATIC) qdevJson["_linker_options"] = _linkerOptions;
		qdevJson["_type"] = (int)_type;
		qdevJson["_include_dirs"] = _includeDirs;
		if (_type != ProjectType::STATIC) qdevJson["_library_dependencies"] = _libraryDependencies;
		qdevJson["_preprocDefs"] = _preprocDefinitions;
	}
	

	qdevFile.write(QJsonDocument(qdevJson).toJson());
	qdevFile.close();
	

}

void Project::cleanQdev() {

	QFile qdevFile(_folderPath + "/" + _PROJECT_FILE);
	qdevFile.open(QIODevice::ReadOnly);
	QJsonObject qdevJson = QJsonDocument::fromJson(qdevFile.readAll()).object();
	qdevFile.close();
	qdevFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
	qdevJson["_sources"] = "";
	qdevFile.write(QJsonDocument(qdevJson).toJson());
	qdevFile.close();

}


void Project::writeBreakpoints(QString path, QVector<int> sourceBreakpoints) {

	QFile qdevFile(_folderPath + "/" + _PROJECT_FILE);
	// Load json contents
	qdevFile.open(QIODevice::ReadOnly);
	QJsonObject qdevJson = QJsonDocument::fromJson(qdevFile.readAll()).object();
	qdevFile.close();

	// Write new one
	qdevFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

	QJsonArray breakpointsArray;
	QJsonObject sourcesKey = qdevJson["_sources"].toObject();


	for (int i = 0; i < sourceBreakpoints.size(); i++)
		breakpointsArray.append(sourceBreakpoints.at(i));

	if (!sourcesKey.contains(path))
		sourcesKey.insert(path, "_breakpoints");

	QJsonObject sourceObject = sourcesKey[path].toObject();
	if (sourceObject.contains("_breakpoints"))
		sourceObject.remove("_breakpoints");

	sourceObject.insert("_breakpoints", breakpointsArray);
	sourcesKey[path] = sourceObject;
	qdevJson["_sources"] = sourcesKey;

	qdevFile.write(QJsonDocument(qdevJson).toJson());
	qdevFile.close();

}

void Project::removeSourceFromQdev(QString path) {

	QFile qdevFile(_folderPath + "/" + _PROJECT_FILE);

	qdevFile.open(QIODevice::ReadOnly);
	QJsonObject qdevJson = QJsonDocument::fromJson(qdevFile.readAll()).object();
	qdevFile.close();

	QJsonObject sourcesKey = qdevJson["_sources"].toObject();

	sourcesKey.remove(path);
	qdevJson["_sources"] = sourcesKey;

	qdevFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
	qdevFile.write(QJsonDocument(qdevJson).toJson());
	qdevFile.close();

}

void Project::renameSourceFromQdev(QString oldPath, QString newPath) {


	QFile qdevFile(_folderPath + "/" + _PROJECT_FILE);

	qdevFile.open(QIODevice::ReadOnly);
	QJsonObject qdevJson = QJsonDocument::fromJson(qdevFile.readAll()).object();
	qdevFile.close();


	qdevFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

	QJsonObject sourcesKey = qdevJson["_sources"].toObject();
	if (sourcesKey.contains(oldPath)) {
		sourcesKey.insert(newPath, sourcesKey[oldPath]);
		sourcesKey.remove(oldPath);
		qdevJson["_sources"] = sourcesKey;
	}
	
	qdevFile.write(QJsonDocument(qdevJson).toJson());
	qdevFile.close();

}

QList<QString> Project::readQdevSources() {

	QFile qdevFile(_folderPath + "/" + _PROJECT_FILE);

	qdevFile.open(QIODevice::ReadOnly);
	QJsonObject qdevJson = QJsonDocument::fromJson(qdevFile.readAll()).object();
	qdevFile.close();

	QJsonObject sourcesKey = qdevJson["_sources"].toObject();
	
	QList<QString> sourcesPath;
	for (auto names : sourcesKey) 
		sourcesPath.append(names.toString());
	
	return sourcesPath;
	

}

QVector<int> Project::readBreakpoints(QString path) {
	

	QFile qdevFile(_folderPath + "/" + _PROJECT_FILE);
	qdevFile.open(QIODevice::ReadOnly);

	QJsonObject qdevJson = QJsonDocument::fromJson(qdevFile.readAll()).object();
	qdevFile.close();

	QVector<int> breakpointsVector;

	if (qdevJson.contains("_sources") && qdevJson["_sources"].toObject().contains(path)) {

		QJsonObject sourcesKey = qdevJson["_sources"].toObject();
		QJsonObject thisSourceKey = sourcesKey[path].toObject();

		if (thisSourceKey.contains("_breakpoints")) {
			QJsonArray breakpointsKey = thisSourceKey.value("_breakpoints").toArray();
			for (auto b : breakpointsKey)
				breakpointsVector.append(b.toInt());
		}
	}

	return breakpointsVector;
}


QString Project::outputPath() {

	QString output = "";

	if (_type == ProjectType::CMAKE || _type == ProjectType::MAKEFILE) {
		output = _runMakeFile;
	}
	else {

		output = _binPath + "/" + name();
		QString os = QSysInfo::productType();

		if (os == "windows") {

			if (_type == ProjectType::EXE)
				output += ".exe";
			else if (_type == ProjectType::STATIC)
				output += ".lib";
			else if (_type == ProjectType::SHARED)
				output += ".dll";

		}
		else if (os == "osx") {

			if (_type == ProjectType::EXE)
				output += ".app";
			else if (_type == ProjectType::STATIC)
				output += ".a";
			else if (_type == ProjectType::SHARED)
				output += ".dylib";
		}
		else { // Linux based system
			if (_type == ProjectType::STATIC)
				output += ".a";
			else if (_type == ProjectType::SHARED)
				output += ".so";
		}
	}

	return output;

}

bool Project::outputExists()
{
	return QFile::exists(outputPath());
}

void Project::clearOutput(){

	QString output = outputPath();
	if (!output.isEmpty())
		QFile::remove(output);

}

ProjectSource* Project::findProjectSource(QString relativePath) {

	ProjectSource* src = 0;
	for (int i = 0; i < _srcProjDir->childCount() && src == 0; i++) {
		ProjectSource* findSrc = dynamic_cast<ProjectSource*>(_srcProjDir->child(i));
		ProjectDirectory* findDir = dynamic_cast<ProjectDirectory*>(_srcProjDir->child(i));

		if (findSrc && findSrc->relativePath() == relativePath)
			src = findSrc;
		else if (findDir) {
			QSet<ProjectSource*> sources = findDir->allDirectoryProjectSources();
			for (ProjectSource* source : sources) 
				if (source->relativePath() == relativePath) {
					src = source;
					break;
				}
		}

	}

	return src;

}




ProjectSource::ProjectSource(QFileInfo sourceInfo, ProjectDirectory* projectDirectory) : QTreeWidgetItem() {

	_project = projectDirectory->project();
	_parentDirectory = projectDirectory;

	init(sourceInfo);

}

void ProjectSource::updateSourceInfo(QFileInfo sourceInfo) {


	QString oldRelPath = _relativePath;
	init(sourceInfo);

	_project->renameSourceFromQdev(oldRelPath, \
		_relativePath);


}

void ProjectSource::init(QFileInfo sourceInfo) {

	_sourceName = sourceInfo.fileName();
	_sourceFullPath = sourceInfo.absoluteFilePath();


	if (!_parentDirectory->findParentDirectory(_project->binDirectory())) {
		QString srcPath = QFileInfo(_project->sourcesPath() + "/").dir().dirName();
		_relativePath = srcPath + sourceInfo.absoluteFilePath().remove(_project->sourcesPath());
	}
	else
		_relativePath = _sourceFullPath;

	if (_sourceName.endsWith(".cpp") || _sourceName.endsWith(".cc"))
		setIcon(0, QIcon(icons::CPP_FILE));
	else if (_sourceName.endsWith(".hpp") || _sourceName.endsWith(".h"))
		setIcon(0, QIcon(icons::H_FILE));
	else if (_sourceName.endsWith(".c"))
		setIcon(0, QIcon(icons::C_FILE));
	else if (_sourceName.endsWith("Makefile"))
		setIcon(0, QIcon(icons::MAKEFILE_FILE));
	else if (_sourceName.endsWith("CMakeLists.txt"))
		setIcon(0, QIcon(icons::CMAKE_FILE));
	else
		setIcon(0, QIcon(icons::GENERIC_FILE));

	setText(0, _sourceName);

}





ProjectDirectory::ProjectDirectory(QFileInfo dirInfo, Project* project) : QTreeWidgetItem() {
	
	_project = project;

	init(dirInfo);

	scanSubDirs();

}

void ProjectDirectory::init(QFileInfo dirInfo){

	_dirName = dirInfo.baseName();
	_dirFullPath = dirInfo.absoluteFilePath();

	if (_dirFullPath != _project->binaryPath() && !findParentDirectory(_project->binDirectory())) {
		QString srcPath = QFileInfo(_project->sourcesPath() + "/").dir().dirName();
		_relativePath = srcPath + dirInfo.absoluteFilePath().remove(_project->sourcesPath());// sourceInfo.dir().dirName() + "/" + _sourceName;
	}
	else
		_relativePath = _dirFullPath;
	//_relativePath = dirInfo.dir().dirName() + "/" + _dirName;


	setIcon(0, QIcon(icons::DIR));
	setText(0, _dirName);

}

ProjectDirectory* ProjectDirectory::findParentDirectory(ProjectDirectory* tofind) {

	ProjectDirectory* parentDir = dynamic_cast<ProjectDirectory*>(parent());
	if (!parentDir)
		return 0;
	
	if (parentDir == tofind)
		return parentDir;

	ProjectDirectory* foundDir = 0;
	while ((parentDir = dynamic_cast<ProjectDirectory*>(parentDir->parent())) && foundDir == 0) {
		if (parentDir == tofind)
			foundDir = parentDir;
	}

	return foundDir;


}


QStringList ProjectDirectory::sources() {

	QStringList sources;

	// Find all sources in the directory, if is a cpp or c file then remove the '/' and then add it to the source's list
	for (int i = 0; i < childCount(); i++) {
		ProjectSource* src = dynamic_cast<ProjectSource*>(child(i));
		if (src && (src->relativePath().endsWith(".cpp") || src->relativePath().endsWith(".c") || \
			src->relativePath().endsWith(".cc")))
			sources << src->relativePath();// .remove(0, 1); da togliere!
	}

	return sources;

}


QStringList ProjectDirectory::allRelativeFilePaths() {

	QStringList files;

	for (int i = 0; i < childCount(); i++) {
		ProjectDirectory* subDirectory = dynamic_cast<ProjectDirectory*>(child(i));
		ProjectSource* file = dynamic_cast<ProjectSource*>(child(i));

		if (subDirectory == _project->binDirectory()) continue;

		if (file)
			files << file->relativePath();
		else if (subDirectory)
			files << subDirectory->allRelativeFilePaths();
	}


	return files;

}

QSet<ProjectSource*> ProjectDirectory::allDirectoryProjectSources() {

	QSet<ProjectSource*> projSources;

	for (int i = 0; i < childCount(); i++) {
		ProjectDirectory* subDirectory = dynamic_cast<ProjectDirectory*>(child(i));
		ProjectSource* file = dynamic_cast<ProjectSource*>(child(i));

		if (subDirectory == _project->binDirectory()) continue;

		if (file)
			projSources << file;
		else if (subDirectory)
			projSources.unite(subDirectory->allDirectoryProjectSources());
	}

	return projSources;
}

void ProjectDirectory::updateDirInfo(QFileInfo dirInfo) {

	init(dirInfo);

	for (int i = 0; i < childCount(); i++) {
		ProjectSource* src = dynamic_cast<ProjectSource*>(child(i));
		ProjectDirectory* dir = dynamic_cast<ProjectDirectory*>(child(i));
		if (src)
			src->updateSourceInfo(QFileInfo(_dirFullPath + "/" + src->sourceName()));
		else if (dir)
			dir->updateDirInfo(QFileInfo(_dirFullPath + "/" + dir->dirName()));
	}

}

void ProjectDirectory::newDirectory(QString dirName) {

	QDir directory(_dirFullPath + "/" + dirName);

	if (!directory.exists())
		directory.mkpath(_dirFullPath + "/" + dirName);
	else {
		QMessageBox::critical(MainWindow::instance(), \
			"Invalid directory name", "Directory with same name already exists");
		return;
	}
	
	QFileInfo dirInfo(_dirFullPath + "/" + dirName);

	ProjectDirectory* newProjectDir = new ProjectDirectory(dirInfo, _project);
	addChild(newProjectDir);
	setExpanded(true);

}

void ProjectDirectory::renameDirectory(QString newName) {
	
	QString newDirBaseName = newName;
	
	int dirBaseNamePos = _dirFullPath.lastIndexOf(_dirName);
	QString parentPath = _dirFullPath.left(dirBaseNamePos);

	newName = parentPath + newName;

	if (!QDir().rename(_dirFullPath, newName)) {
		QMessageBox::critical(MainWindow::instance(), "Invalid directory name",
			"Can't rename directory, maybe a directory with same name already exists");
		return;
	}

	init(QFileInfo(newName));

	for (int i = 0; i < childCount(); i++) {
		QTreeWidgetItem* item = child(i);
		ProjectSource* src = dynamic_cast<ProjectSource*>(item);
		if (src) {
			QString newSrcPath = _relativePath + "/" + src->sourceName();
			QString newAbsoluteSrcPath = _dirFullPath + "/" + src->sourceName();
			// Update source item infos
			src->updateSourceInfo(QFileInfo(newAbsoluteSrcPath));
		}
		else {
			ProjectDirectory* dir = dynamic_cast<ProjectDirectory*>(item);
			if (dir) 
				dir->updateDirInfo(QFileInfo(_dirFullPath + "/" + dir->dirName()));
			
		}
		
	}

	setExpanded(true);

}

void ProjectDirectory::deleteDirectory() {

	QDir directory(_dirFullPath);
	if (!directory.removeRecursively()) {
		QMessageBox::critical(MainWindow::instance(), "Can't delete directory",
			"Can't delete directory, check filesystem permissions");
		return;
	}

	// For every source remove corrispondence in qdev project file
	for (int i = 0; i < childCount(); i++) {
		QTreeWidgetItem* item = child(i);
		ProjectSource* src = dynamic_cast<ProjectSource*>(item);
		if (src) 
			_project->removeSourceFromQdev(src->relativePath());
		
	}

	parent()->removeChild(this);

}

void ProjectDirectory::scanSubDirs(bool files, bool dirs) {

	QDir directory(_dirFullPath);

	if (files) {
		// Scan files
		QFileInfoList sourceFilesInfo = directory.entryInfoList(QDir::Files);

		for (QFileInfo& srcInfo : sourceFilesInfo) {
			ProjectSource* src = new ProjectSource(srcInfo, this);
			addChild(src);
		}
	}

	if (dirs) {
		// Scan dirs
		QFileInfoList subDirsInfo = directory.entryInfoList(QDir::NoDot | QDir::NoDotDot | QDir::Dirs);

		for (QFileInfo& subDirInfo : subDirsInfo) {
			ProjectDirectory* projDir = new ProjectDirectory(subDirInfo, _project);
			addChild(projDir);
		}
	}

}

void ProjectDirectory::updateContents(bool files, bool dirs) {

	if (files || dirs) {

		// Avoid multiple allocations for the same item
		for (int i = childCount() - 1; i >= 0; i--) {

			QTreeWidgetItem* item = child(i); 

			if (files && dynamic_cast<ProjectSource*>(item)) 
				removeChild(item);
			else if (dirs && dynamic_cast<ProjectDirectory*>(item)) 
				removeChild(item);
			
		}
	}else 
		return;
	
	scanSubDirs(files, dirs);


}





ProjectOptionsDialog::ProjectOptionsDialog(Project* project, QWidget* parent) : QDialog(parent) {

	_project = project;

	bool isMakeFileProject = false;
	if (_project->getType() == ProjectType::MAKEFILE || _project->getType() == ProjectType::CMAKE)
		isMakeFileProject = true;

	setMinimumWidth(_MIN_WIDTH);

	_compilerToolLine = new QLineEdit((isMakeFileProject ? "Makefile" : "g++"), this);
	_compilerToolLine->setReadOnly(true);

	_binDirLine = new QLineEdit(_project->binaryPath(), this);
	_binDirLine->setReadOnly(true);

	_srcDirLine = new QLineEdit(_project->sourcesPath(), this);
	_srcDirLine->setReadOnly(true);

	if (isMakeFileProject) {
		_makeFileArgsLine = new QLineEdit(_project->makeFileArguments(), this);
		_makeFileRunLine = new QLineEdit(_project->runMakeFile(), this);
	}

	_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
		| QDialogButtonBox::Cancel, this);

	QFormLayout* formLayout = new QFormLayout();
	formLayout->addRow("Build tool", _compilerToolLine);

	if (isMakeFileProject) {

		QHBoxLayout* pathLayout = new QHBoxLayout();
		_btnSelectPath = new QPushButton("Open file");
		pathLayout->setContentsMargins(0, 0, 0, 0);
		pathLayout->addWidget(_makeFileRunLine);
		pathLayout->addWidget(_btnSelectPath);

		QWidget* pathContainer = new QWidget(this);
		pathContainer->setLayout(pathLayout);

		formLayout->addRow("Make args", _makeFileArgsLine);
		formLayout->addRow("Run button\nInsert full path of \nthe file to launch", pathContainer);

		connect(_btnSelectPath, SIGNAL(released()), this, SLOT(selectPath()));

	}
	else {

		bool staticproj = _project->getType() == ProjectType::STATIC ? true : false;

		_includeDirsText = new QTextEdit(this);
		QStringList includes(_project->includeDirs().split(" "));
		for (QString& item : includes)
			_includeDirsText->append(item);

		if (!staticproj) {
			_libsDependenciesText = new QTextEdit(this);
			QStringList libs(_project->libraryDependencies().split(" "));
			for (QString& item : libs)
				_libsDependenciesText->append(item);
		}

		_preprocessorDefsText = new QLineEdit(_project->preprocDefinitions(), this);

		_compilerOptionsLine = new QLineEdit(_project->compilerOptions(), this);
		
		if (!staticproj)
			_linkerOptionsLine = new QLineEdit(_project->linkerOptions(), this);


		formLayout->addRow("Additional Compiler options\nSeparate them with spaces", _compilerOptionsLine);
		formLayout->addRow("Additional Preprocessor\ndefinitions\nSeparate them with spaces", _preprocessorDefsText);
		if (!staticproj) formLayout->addRow("Additional Linker options\nSeparate them with spaces", _linkerOptionsLine);

		formLayout->addRow("Additional Include directories\nInsert one directory each line", _includeDirsText);
		if (!staticproj) formLayout->addRow("Additional Library dependencies\nInsert one library each line", _libsDependenciesText);
	}

	
	formLayout->addRow("Sources directory", _srcDirLine);
	formLayout->addRow("Binary directory", _binDirLine);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(formLayout);
	mainLayout->addWidget(_buttonBox);

	setLayout(mainLayout);

	connect(_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	setWindowTitle("Project settings");

}

void ProjectOptionsDialog::selectPath() {

	QFileDialog folderDialog;
	QString path = folderDialog.getOpenFileName(this, "Select the launch file");
	if (path.isEmpty())
		return;
	if (path.contains(" ")) {
		QMessageBox::critical(this, "Invalid path", "Can't select a path that contains spaces!", QMessageBox::Cancel);
		return;
	}

	_makeFileRunLine->setText(path);

}

void ProjectOptionsDialog::accept() {

	if (_project->getType() != ProjectType::MAKEFILE && _project->getType() != ProjectType::CMAKE) {
		_project->setCompilerOptions(_compilerOptionsLine->text());
		if (_project->getType() != ProjectType::STATIC) _project->setLinkerOptions(_linkerOptionsLine->text());
		_project->setIncludeDirs(_includeDirsText->toPlainText().replace('\n', ' ').replace('\\', '/'));
		if (_project->getType() != ProjectType::STATIC) _project->setLibraryDependencies(_libsDependenciesText->toPlainText().replace('\n', ' ').replace('\\', '/'));
		_project->setPreprocDefs(_preprocessorDefsText->text());
	}
	else {
		_project->setMakeFileArguments(_makeFileArgsLine->text());
		_project->setRunMakeFile(_makeFileRunLine->text());
	}
	


	QDialog::accept();

}

