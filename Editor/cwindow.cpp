
#include "cwindow.h"
#include <cstdio>

using namespace std;


static void buttonPressed( GtkWidget* widget, gpointer data)
{
	printf( "Button 'test' was clicked!\n" );
}

static void init_list( GtkWidget *list )
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeStore *store;

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("List Items", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

	store = gtk_tree_store_new( 1, G_TYPE_STRING );

	gtk_tree_view_set_model( GTK_TREE_VIEW(list), GTK_TREE_MODEL(store) );

	g_object_unref(store);
}

cwindow::cwindow()
{
	m_window = gtk_window_new( GTK_WINDOW_TOPLEVEL );

	// -- Position the window at the center of the screen and size it to the initial size of 960x540p
	gtk_window_set_default_size( GTK_WINDOW(m_window), 960, 540 );
	gtk_window_set_position( GTK_WINDOW(m_window), GTK_WIN_POS_CENTER );

	// -- Apply the initial title
	gtk_window_set_title( GTK_WINDOW(m_window), "Carnivores 3D Editor" );

	// -- Create a container
	addWidget( "vbox", gtk_vbox_new(FALSE, 0) );
	gtk_container_add(GTK_CONTAINER(m_window), getWidget( "vbox" ) );

	// -- Create the menubar
	addWidget( "menubar", gtk_menu_bar_new() );

	// -- Menubar submenus
	addWidget( "lblFile", gtk_menu_item_new_with_label("File") );
	addWidget( "lblEdit", gtk_menu_item_new_with_label("Edit") );
	addWidget( "lblTools", gtk_menu_item_new_with_label("Tools") );
	addWidget( "lblHelp", gtk_menu_item_new_with_label("Help") );

	// -- Create the File menu items
	addWidget( "menuFile", gtk_menu_new() );
	addWidget( "lblFileNew", gtk_menu_item_new_with_label( "New" ) );
	addWidget( "lblFileOpen", gtk_menu_item_new_with_label( "Open" ) );
	addWidget( "lblFileSave", gtk_menu_item_new_with_label( "Save" ) );
	addWidget( "lblFileSaveAs", gtk_menu_item_new_with_label( "Save As..." ) );
	addWidget( "lblFileRecent", gtk_menu_item_new_with_label( "Recent Files" ) );
	addWidget( "lblFileQuit", gtk_menu_item_new_with_label( "Quit" ) );

	// -- Create the Help menu items
	addWidget( "menuHelp", gtk_menu_new() );
	addWidget( "lblHelpHelp", gtk_menu_item_new_with_label( "Help" ) );
	addWidget( "lblHelpAbout", gtk_menu_item_new_with_label( "About" ) );

	// -- Create the File menu items
	addWidget( "menuRecent", gtk_menu_new() );

	// -- Attach the labels and submenus to the menubar dropdown
	gtk_menu_item_set_submenu( GTK_MENU_ITEM( getWidget( "lblFile" ) ), getWidget( "menuFile" ) );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menuFile" )), getWidget( "lblFileNew" ) );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menuFile" )), getWidget( "lblFileOpen" ) );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menuFile" )), getWidget( "lblFileSave" ) );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menuFile" )), getWidget( "lblFileSaveAs" ) );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menuFile" )), gtk_menu_item_new() );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menuFile" )), getWidget( "lblFileRecent" ) );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menuFile" )), gtk_menu_item_new() );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menuFile" )), getWidget( "lblFileQuit" ) );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM( getWidget( "lblHelp" ) ), getWidget( "menuHelp" ) );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menuHelp" )), getWidget( "lblHelpHelp" ) );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menuHelp" )), gtk_menu_item_new() );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menuHelp" )), getWidget( "lblHelpAbout" ) );

	// -- Recent Files Submenu
	gtk_menu_item_set_submenu( GTK_MENU_ITEM( getWidget( "lblFileRecent" ) ), getWidget( "menuRecent" ) );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menuRecent" )), gtk_menu_item_new_with_label( "None..." ) );

	// -- Add the Submenus to the menubar
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menubar" )), getWidget( "lblFile" ) );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menubar" )), getWidget( "lblEdit" ) );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menubar" )), getWidget( "lblTools" ) );
	gtk_menu_shell_append( GTK_MENU_SHELL(getWidget( "menubar" )), getWidget( "lblHelp" ) );

	gtk_box_pack_start( GTK_BOX( getWidget( "vbox" ) ), getWidget( "menubar" ), FALSE, FALSE, 3 );

	// -- Tree View
	addWidget( "listRsc", gtk_tree_view_new() );
	gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( getWidget( "listRsc" ) ), TRUE );
	gtk_box_pack_start( GTK_BOX( getWidget( "vbox" ) ), getWidget( "listRsc" ), TRUE, TRUE, 3 );

	init_list( getWidget( "listRsc" ) );

	addListItem( "Dummy1" );
	addListItem( "Dummy2" );

	// -- Set up signal to quit when clicking the close button
	g_signal_connect_swapped(G_OBJECT(m_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	// -- Set up signal to quit when clicking the Quit menu item
	g_signal_connect(G_OBJECT( getWidget( "lblFileQuit" ) ), "activate", G_CALLBACK(gtk_main_quit), NULL);

	gtk_widget_show_all( m_window );

	// -- Show the window and all of its contents
	gtk_widget_show_all(window);
}

cwindow::~cwindow()
{
}


void cwindow::addWidget( string name, GtkWidget* widget )
{
	m_widgets.insert( pair<string, GtkWidget*>( name, widget ) );
}

GtkWidget* cwindow::getWidget( string name )
{
	return m_widgets.at( name );
}

void cwindow::addListItem( string name )
{
	GtkWidget* list = getWidget( "listRsc" );
	GtkTreeStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE( gtk_tree_view_get_model( GTK_TREE_VIEW( list ) ) );

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, name.c_str(), -1);
}
