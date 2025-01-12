#include "Session.h"

using namespace qdev;

Session::Session(std::string org, std::string appname)
	: settings(org.c_str(), appname.c_str())
{
	init_defaults();
	read();
}

Session::~Session()
{
	write();
}

void Session::init_defaults()
{
	_font = QFont("Consolas", 15);
}

void Session::read()
{
	if(settings.contains("fontFamily"))
		_font.setFamily(settings.value("fontFamily").toString());

	if(settings.contains("fontSize"))
		_font.setPointSize(settings.value("fontSize").toInt());

	if(settings.contains("fontWeight"))
		_font.setWeight(settings.value("fontWeight").toInt());

	if(settings.contains("fontItalic"))
		_font.setItalic(settings.value("fontItalic").toBool());

	size_t dim = settings.beginReadArray("recentFiles");
	for(int i=0; i<dim; i++)
	{
		settings.setArrayIndex(i);
		_recentProjects.push_back(settings.value("path").toString());
	}
	settings.endArray();
}

void Session::write()
{
	settings.setValue("fontFamily", _font.family());
	settings.setValue("fontSize", _font.pointSize());
	settings.setValue("fontWeight", _font.weight());
	settings.setValue("fontItalic", _font.italic());

	settings.beginWriteArray("recentFiles");
	int i=0;
	for(auto & f : _recentProjects)
	{
		settings.setArrayIndex(i++);
		settings.setValue("path", f);
	}
	settings.endArray();
}

void Session::addRecentProject(const QString & path)
{
	if(std::find(_recentProjects.begin(), _recentProjects.end(), path) != _recentProjects.end())
		_recentProjects.remove(path);

	_recentProjects.push_front(path);

	if(_recentProjects.size() > 10)
		_recentProjects.pop_back();

	write();
}