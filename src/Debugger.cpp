#include "Debugger.h"

#include <QSignalBlocker>
#include <QSysInfo>
#include <QFileInfo>
#include <QDir>

using namespace qdev;

// Constructor
Debugger::Debugger(QObject* parent) : QObject(parent) {}


void Debugger::gdbWrite(const QString& input) {

	if (!_gdb)
		return;

	_gdb->write(input.toStdString().c_str());
	_gdb->write("\r\n");

}


void Debugger::gdbClose(QString message) {

	if (!_gdb)
		return;

	// Lock readyRead signals
	QSignalBlocker* blocker = new QSignalBlocker(_gdb);

	// Try to send kill signal to gdb so his children threads will close
	_gdb->kill();
	// Wait for response
	_gdb->waitForFinished(-1);		

	// Delete the blocker and the process
	delete blocker; blocker = 0;
	delete _gdb; _gdb = 0;

	// If message is an error then return a signal with the message to show to the user
	if (message == "" || message.contains("The program is not being run"))
		emit debuggingFinished("finish");
	else
		emit debuggingFinished(message);

}


QMap<QString, int> Debugger::lineNumber(QString& line) {

	QMap<QString, int> source;
	QString lineStr;

	bool cmakeProject = false;
	// With CMake projects gdb works on full path sources
	if (_project->getType() == ProjectType::CMAKE)
		cmakeProject = true;

	QStringList splitted = line.split("\r\n");

	for (QString str : splitted) {

		if (str.contains("\\"))
			str.replace("\\", "/");

	
				
		if (( str.startsWith("Breakpoint") || str.contains(" at ")) && !str.contains(" file ")) {

			QString sourceStr;
			int num;

			if (cmakeProject) {
				lineStr = str.mid(str.indexOf("at ") + 3, str.lastIndexOf(":"));

				sourceStr = QFileInfo(_project->sourcesPath() + "/").dir().dirName()
					+ lineStr.remove(_project->sourcesPath());
				sourceStr = sourceStr.mid(0, sourceStr.indexOf(":"));
				num = lineStr.mid(lineStr.lastIndexOf(":") + 1).toInt();

			}
			else {
				lineStr = str.mid(str.indexOf("at ") + 3, str.indexOf(":"));
				sourceStr = lineStr.mid(0, lineStr.indexOf(":"));
				num = lineStr.mid(lineStr.indexOf(":") + 1).toInt();
			}

			source.insert(sourceStr, num);

			return source;
		}
		else {
			
			bool isNum = false;
			int num = str.mid(0, str.indexOf("\t")).toInt(&isNum);
			if (isNum) {
				source.insert("", num);
				return source;
			}
		}
		
	}


	return source;

}

QString Debugger::infoLocals() {

	QSignalBlocker* blocker = new QSignalBlocker(_gdb);

	gdbWrite(debuggerconstants::INFO_LOCALS);
	_gdb->waitForReadyRead(-1);
	QString response = _gdb->readAllStandardOutput();

	delete blocker; blocker = 0;

	return response;

}


void Debugger::gdbRead() {

	if (!_gdb)
		return;

	QString output = _gdb->readAllStandardOutput();
	if (output != "") {

		
		QMap<QString,int> source = lineNumber(output);
		if (!source.isEmpty())
			emit debuggerGoToLine(source.firstKey(), source.value(source.firstKey()));
		
		if (output.contains(debuggerconstants::NORMAL_EXIT))
			gdbClose();
	}
	
	if (_gdb) {
		QString error = _gdb->readAllStandardError();
		
		if (error != "")
			gdbClose(error);
	}
		
}

void Debugger::insertAllBreakpoints() {

	if (!_project || !_gdb)
		return;

	if (_gdb->state() == QProcess::NotRunning)
		return;

	QSignalBlocker* blocker = new QSignalBlocker(_gdb);

	// Get project sources
	QSet<ProjectSource*> sources = _project->allProjectSources();

	// For every source store breakpoints list
	for (ProjectSource* src : sources) {

		QVector<int> breakpoints = _project->readBreakpoints(src->relativePath());
		// For every breakpoint write into gdb process
		for (int b : breakpoints)
			gdbWrite(debuggerconstants::BREAK_CMD + " " + src->relativePath() + ":" + QString::number(b));

	}

	_gdb->waitForReadyRead(-1);

	// Remove blocker to gdb so the readyRead signals are enabled again
	delete blocker; blocker = 0;

}

void Debugger::updateBreakpoints() {

	// Clear all breakpoints
	gdbWrite(debuggerconstants::DELETE_CMD);
	// Insert new one(s)
	insertAllBreakpoints();

}



void Debugger::startDebugging() {

	if (!_project)
		return;

	
	if ( !_project->outputExists())
		return;

	if (_gdb)
		gdbClose();

	_gdb = new QProcess(this);
	connect(_gdb, SIGNAL(readyReadStandardOutput()), this, SLOT(gdbRead()));
	connect(_gdb, SIGNAL(readyReadStandardError()), this, SLOT(gdbRead()));


	// Start gdb
	_gdb->start(debuggerconstants::DEBUGGER, { _project->outputPath() });
	_gdb->waitForStarted(-1);
	_gdb->waitForReadyRead(-1);

	// Set confirm off
	gdbWrite(debuggerconstants::CONFIRM_OFF);
	// Show in new console (windows-only feature)
	if (QSysInfo::productType() == "windows") gdbWrite(debuggerconstants::NEW_CONSOLE_CMD);
	


	// Insert all project breakpoints
	insertAllBreakpoints();

	// run gdb
	gdbWrite(debuggerconstants::RUN_CMD);

	

	
	

}

void Debugger::step() {

	if (!_gdb || _gdb->state() != QProcess::Running)
		return;


	gdbWrite(debuggerconstants::STEP);


}

void Debugger::stopDebugging() {

	gdbClose();

}


void Debugger::continueRun() {

	if (!_gdb || _gdb->state() == QProcess::NotRunning)
		return;

	gdbWrite(debuggerconstants::CONTINUE_CMD);

}