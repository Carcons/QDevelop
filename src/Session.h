#pragma once

#include <QSettings>
#include <string>
#include <list>
#include <QFont>

namespace qdev
{
	class Session;
}

class qdev::Session
{
	private:

		QSettings settings;

		std::list <QString> _recentProjects;
		QFont _font;
		// ...other options to be maintained 

		// utility methods
		void init_defaults();
		void read();
		void write();

	public:

		Session(std::string org, std::string appname);
		~Session();

		// getter
		const std::list <QString> & recentProjects(){return _recentProjects;}
		const QFont & font(){return _font;}

		// setter
		void setFont(const QFont& newFont) { _font = newFont; write(); }
		void addRecentProject(const QString & path);
};