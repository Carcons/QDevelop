#pragma once

#include <QProcess>
#include <QString>
#include <QSet>
#include <QMap>


#include "Project.h"


namespace qdev {
	class Debugger;
}

class qdev::Debugger : public QObject {

	Q_OBJECT
		
	private:

		QProcess* _gdb = 0;
		Project* _project = 0;

		void gdbWrite(const QString& input);
		void insertAllBreakpoints();

		QMap<QString, int> lineNumber(QString& line);

	public:

		Debugger(QObject* parent = 0);

		void setProject(Project* project) { _project = project; }
	
		void startDebugging();
		void stopDebugging();
		void step();
		void continueRun();
		bool isDebugging() { if (_gdb) return _gdb->state() == QProcess::Running; else return false; }
		void updateBreakpoints();
		QString infoLocals();


	signals:

		void debuggingFinished(QString message);
		void debuggerGoToLine(QString relativePath, int line);

	private slots:

		void gdbRead();
		void gdbClose(QString message = "");
		 
		

};
