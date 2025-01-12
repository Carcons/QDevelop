#pragma once

#include "Project.h"

#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTextEdit>


namespace qdev {
	class ProjectBuilder;
}


class qdev::ProjectBuilder : public QTextEdit {

	Q_OBJECT

	private:

		bool _archive = false;
		bool _shared = false;

		buildconstants::BuildMode _mode;
		QProcess* _cxxProcess = 0;
		QProcess* _secondaryProcess = 0;

		Project* _project = 0;

		QStringList constructArguments();

		void buildEnded(bool success, QProcess* process = 0, int exitCode = 0, QProcess::ExitStatus exitStatus = QProcess::ExitStatus::NormalExit);

	public:

		void CMakeGenerator(Project* project, bool debug = false);

		ProjectBuilder();

		void start();
		
		void setBuildMode(buildconstants::BuildMode mode = buildconstants::BuildMode::NORMAL) { _mode = mode; }
		void setProject(Project* project) { _project = project; }
		void mouseDoubleClickEvent(QMouseEvent* event) override;

		Project* project() { return _project; }

	protected slots:

		void cxxFinished(int exitCode, QProcess::ExitStatus exitStatus);
		void collectingObjectsFinished(int exitCode, QProcess::ExitStatus exitStatus);

	signals:

		void buildReadyError(int exitCode, QProcess::ExitStatus exitStatus, QString content);
		void updateMainWindowUI();


};

