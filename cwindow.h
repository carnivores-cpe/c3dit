
#ifndef __cwindow_h__
#define __cwindow_h__

#include <gtk/gtk.h>
#include <map>
#include <string>

class cwindow
{
public:

	cwindow();
	~cwindow();

	void addWidget( std::string name, GtkWidget* widget );
	GtkWidget* getWidget( std::string name );
	void addListItem( std::string name );

private:

	GtkWidget*								m_window;
	std::map<std::string, GtkWidget*>		m_widgets;

};

#endif //__cwindow_h__
