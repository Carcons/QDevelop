#include "ProjectBuilder.h"
#include "MainWindow.h"

#include <QMessageBox>
#include <QDir>

using namespace qdev;

ProjectBuilder::ProjectBuilder() : QTextEdit(0) {

	setReadOnly(true);
	setStyleSheet("QTextEdit { background-color: rgb(30, 29, 35);}");
	setFont(QFont("Consolas", 9));

	_mode = buildconstants::BuildMode::NORMAL;
}

void ProjectBuilder::CMakeGenerator(Project* project, bool debug) {

	
	QProcess generator;
	generator.setProgram(buildconstants::CMAKE);
	QStringList args;
	args << buildconstants::CMAKE_SOURCE << project->sourcesPath();
	args << buildconstants::CMAKE_BUILD << project->binaryPath();
	if (debug) args << buildconstants::CMAKE_DEBUG << buildconstants::CMAKE_BUILD_DEBUG;
	generator.setArguments(args);
	generator.start();
	generator.waitForFinished(-1);
	
	append(generator.readAll());



}


QStringList ProjectBuilder::constructArguments() {

	QStringList args;

	// Get all project informations
	QStringList compilerOptions = _project->compilerOptions().split(" ");
	QStringList preprocDefs = _project->preprocDefinitions().split(" ");
	QStringList includeDirsList = _project->includeDirs().split(" ");
	QStringList cppFiles = _project->cppFiles();
	QStringList libraryDependencies = _project->libraryDependencies().split(" ");
	QStringList linkerOptions = _project->linkerOptions().split(" ");
	QString outputPath = _project->outputPath();

	// All the include directories and preprocessor definitions needs -I and -D 
	QStringList includeDirs;
	for (QString& dir : includeDirsList)
		if (!dir.isEmpty())
			includeDirs << dir.insert(0, buildconstants::INCLUDE);

	for (QString& def : preprocDefs)
		if (!def.isEmpty())
			compilerOptions << def.insert(0, buildconstants::PREPROCESSOR_DEFS);

	// Every project type need different arguments inserted in different orders
	if (_project->getType() == ProjectType::EXE) {
		if (_mode == buildconstants::BuildMode::DEBUG)
			args << buildconstants::DEBUGGING << compilerOptions;
		else
			args << compilerOptions;
		args << includeDirs;
		args << cppFiles;
		args << buildconstants::OUTPUT << outputPath;
		args << libraryDependencies;
		args << linkerOptions;
	}
	else if (_project->getType() == ProjectType::STATIC) {

		// if the archive flag is enabled the arguments are for the secondary process
		if (_archive)
			args << buildconstants::RVS << buildconstants::OUTPUT << outputPath << _project->objectFiles();
		else {
			if (_mode == buildconstants::BuildMode::DEBUG)
				args << buildconstants::DEBUGGING << compilerOptions;
			else
				args << compilerOptions;
			args << includeDirs;
			args << buildconstants::COMPILE << cppFiles;
			args << buildconstants::OUTPUT << _project->objectFiles();
		}

	}
	else if (_project->getType() == ProjectType::SHARED) {

		// if the shared flag is enabled then we are in the second step of compilation and we need differents args
		if (_shared)
			args << buildconstants::SHARED << buildconstants::OUTPUT << _project->outputPath() << \
			_project->objectFiles() << libraryDependencies << linkerOptions;
		else {
			if (_mode == buildconstants::BuildMode::DEBUG)
				args << buildconstants::DEBUGGING << compilerOptions;
			else
				args << compilerOptions;
			args << includeDirs;
			args << buildconstants::FPIC;
			args << buildconstants::COMPILE;
			args << cppFiles;
			args << buildconstants::OUTPUT;
			args << _project->objectFiles();
		}

	}
	else if (_project->getType() == ProjectType::MAKEFILE || _project->getType() == ProjectType::CMAKE) 
		args << _project->makeFileArguments().split(" ");;
	


	args.removeAll("");

	return args;

}


void ProjectBuilder::start() {

	clear();

	setTextColor(Qt::green);

	// Get projects info
	ProjectType projType = _project->getType();

	append("Checking g++ version...");

	// Check if g++ exists
	if (QProcess::execute(buildconstants::CXX + " " + buildconstants::CXX_VER) != 0) {
		append("g++ not found...abort");
		QMessageBox::critical(this, "g++ Required", \
			"g++ has not be found in PATH. Please download and install latest GNU g++ version and configure PATH");
		return;
	}

	_cxxProcess = new QProcess(this);
	connect(_cxxProcess, SIGNAL(finished(int, QProcess::ExitStatus)), \
		this, SLOT(cxxFinished(int, QProcess::ExitStatus)));
	_cxxProcess->setWorkingDirectory(_project->path());
	_cxxProcess->setProgram(buildconstants::CXX);

	// Build compiling process using project info
	if (projType == ProjectType::MAKEFILE || projType == ProjectType::CMAKE) {

		append("Project is using GNU Make build system");

		// Check if make (or mingw-make) exists: if true than compile the project
		if (QSysInfo::productType() == "windows") {

			append("Checking mingw32-make version...");

			if (QProcess::execute(buildconstants::MINGW32_MAKE + " " + buildconstants::CXX_VER) != 0) {
				append("mingw32-make not found...abort");
				QMessageBox::critical(this, "MinGW Make Required", \
					"mingw32-make has not been found in PATH");
				return;
			}

		}
		else {

			append("Checking make version...");

			if (QProcess::execute(buildconstants::MAKE + " " + buildconstants::CXX_VER) != 0) {
				append("make not found...abort");
				QMessageBox::critical(this, "GNU Make Required", \
					"make has not been found in PATH");
				return;
			}

		}
		if (projType == ProjectType::MAKEFILE)
			_cxxProcess->setWorkingDirectory(_project->sourcesPath());
		else {
			if (_mode == buildconstants::BuildMode::DEBUG) CMakeGenerator(_project, true);
			else CMakeGenerator(_project);
			_cxxProcess->setWorkingDirectory(_project->binaryPath());
		}
		_cxxProcess->setArguments(constructArguments());
		_cxxProcess->setProgram((QSysInfo::productType() == "windows" ? buildconstants::MINGW32_MAKE : buildconstants::MAKE));

		append("Starting make");


	}
	else if (projType == ProjectType::EXE) {

		append("Project is an Executable");
		_cxxProcess->setArguments(constructArguments());
		append("Compiling executable...");


	}
	else if (projType == ProjectType::STATIC) {

		append("Project is a Static Library");
		_cxxProcess->setArguments(constructArguments());
		append("Compiling sources into object files...");
		_archive = true;

	}
	else { // Shared

		append("Project is a Shared Library");
		_cxxProcess->setArguments(constructArguments());
		append("Compiling sources with Position Independency into object files...");
		_shared = true;

	}

	_cxxProcess->start();

}

void ProjectBuilder::mouseDoubleClickEvent(QMouseEvent* event) {

	QTextCursor textCur = textCursor();

	textCur.select(QTextCursor::LineUnderCursor);
	QString line = textCur.selectedText();


	if (!(line.contains(": error:") || line.contains(": fatal error:") || line.contains(": warning:")))
		return;


	MainWindow::instance()->openSourceError(_project, line);


}

void ProjectBuilder::buildEnded(bool success, QProcess* process, int exitCode, QProcess::ExitStatus exitStatus) {

	if (success) {
		setTextColor(Qt::green);
		append(_cxxProcess->readAllStandardOutput());
		append(QString("Build succeeded"));
	}
	else {

		QString errorContent = process->readAllStandardError();

		_project->clearOutput();
		setTextColor(Qt::red);
		append(errorContent);
		append(QString("Build error, return code = ") + QString::number(exitCode));
		append(QString("Build failed"));

		emit buildReadyError(exitCode, exitStatus, errorContent);
	}

	setEnabled(true);

	if (_project->getType() == ProjectType::STATIC || _project->getType() == ProjectType::SHARED) {
		// Clear all *.o files
		QDir dir(_project->sourcesPath());
		dir.setNameFilters(QStringList() << "*.o");
		dir.setFilter(QDir::Files);
		for (QString dirFile : dir.entryList())
			dir.remove(dirFile);
	}

	_project->srcDirectory()->updateContents();
	_project->binDirectory()->updateContents();

	_archive = false;
	_shared = false;

	if (_cxxProcess) {
		delete _cxxProcess;
		_cxxProcess = 0;
	}

	if (_secondaryProcess) {
		delete _secondaryProcess;
		_secondaryProcess = 0;
	}

	emit updateMainWindowUI();


}

void ProjectBuilder::cxxFinished(int exitCode, QProcess::ExitStatus exitStatus) {

	if (exitCode == EXIT_SUCCESS && exitStatus == QProcess::NormalExit) {

		if (_shared) {


			append("Generating shared library...");

			_secondaryProcess = new QProcess(this);

			connect(_secondaryProcess, SIGNAL(finished(int, QProcess::ExitStatus)), \
				this, SLOT(collectingObjectsFinished(int, QProcess::ExitStatus)));
			_secondaryProcess->setWorkingDirectory(_project->path());

			_secondaryProcess->setArguments(constructArguments());
			_secondaryProcess->setProgram(buildconstants::CXX);

			_secondaryProcess->start();



		}
		else if (_archive) {

			append("Generating archive...");

			_secondaryProcess = new QProcess(this);

			connect(_secondaryProcess, SIGNAL(finished(int, QProcess::ExitStatus)), \
				this, SLOT(collectingObjectsFinished(int, QProcess::ExitStatus)));
			_secondaryProcess->setWorkingDirectory(_project->path());

			_secondaryProcess->setArguments(constructArguments());
			_secondaryProcess->setProgram(buildconstants::ARCHIVE);

			_secondaryProcess->start();

		}
		else
			buildEnded(true);


	}
	else
		buildEnded(false, _cxxProcess, exitCode, exitStatus);


}

void ProjectBuilder::collectingObjectsFinished(int exitCode, QProcess::ExitStatus exitStatus) {

	if (exitCode == EXIT_SUCCESS && exitStatus == QProcess::NormalExit)
		buildEnded(true);
	else
		buildEnded(false, _secondaryProcess, exitCode, exitStatus);


}
