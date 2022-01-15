/*  tag file support plugin for cssed (c) Iago Rubio 2004
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#ifndef WIN32
#  include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <plugin.h>
#include <cssedwindow.h>

#include "tagplugin.h"
#include "callbacks.h"
#include "interface.h"
#include "support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_tagplugin_ui (CssedTagPlugin* tp)
{
	GtkWidget *window_tab;
	GtkWidget *vbox_tags;
	GtkWidget *hbox_create_open;
	GtkWidget *button_create_tag_file;
	GtkWidget *button_open_tag_file;
	GtkWidget *label_current_tag;
	GtkWidget *entry_current_tag_file;
	GtkWidget *hbox_query;
	GtkWidget *entry_tag_query;
	GtkWidget *checkbutton_query_partial_match;
	GSList *radiobutton_query_case_group = NULL;
	GtkWidget *radiobutton_query_case_sensitive;
	GtkWidget *radiobutton_query_case_insensitive;
	GtkWidget *button_query_find;
	GtkWidget *button_query_show_all;
	GtkWidget *scrolledwindow_tag_result;
	GtkWidget *treeview_tags;
	GtkTooltips *tooltips;
	GtkListStore* store;
	GtkTreeViewColumn* first_col;
	GtkTreeViewColumn* second_col;	
	GtkTreeViewColumn* third_col;
	GtkCellRenderer* text_renderer;

	tooltips = gtk_tooltips_new ();
	
	window_tab = gtk_frame_new( NULL );
	
	vbox_tags = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox_tags);
	gtk_container_add (GTK_CONTAINER (window_tab), vbox_tags);
	
	hbox_create_open = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox_create_open);
	gtk_box_pack_start (GTK_BOX (vbox_tags), hbox_create_open, FALSE, FALSE, 0);
	
	button_create_tag_file = gtk_button_new_with_mnemonic (_("Create tag file"));
	gtk_widget_show (button_create_tag_file);
	gtk_box_pack_start (GTK_BOX (hbox_create_open), button_create_tag_file, FALSE, FALSE, 2);
	
	button_open_tag_file = gtk_button_new_with_mnemonic (_("Open tag file"));
	gtk_widget_show (button_open_tag_file);
	gtk_box_pack_start (GTK_BOX (hbox_create_open), button_open_tag_file, FALSE, FALSE, 2);
	
	label_current_tag = gtk_label_new (_("Current tag file"));
	gtk_widget_show (label_current_tag);
	gtk_box_pack_start (GTK_BOX (hbox_create_open), label_current_tag, FALSE, FALSE, 2);
	
	entry_current_tag_file = gtk_entry_new ();
	gtk_widget_show (entry_current_tag_file);
	gtk_box_pack_start (GTK_BOX (hbox_create_open), entry_current_tag_file, TRUE, TRUE, 0);
	gtk_widget_set_sensitive (entry_current_tag_file, FALSE);
	gtk_tooltips_set_tip (tooltips, entry_current_tag_file, _("This is the tag file in use if any"), NULL);
	gtk_editable_set_editable (GTK_EDITABLE (entry_current_tag_file), FALSE);
	
	hbox_query = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox_query);
	gtk_box_pack_start (GTK_BOX (vbox_tags), hbox_query, FALSE, FALSE, 0);
	
	entry_tag_query = gtk_entry_new ();
	gtk_widget_show (entry_tag_query);
	gtk_box_pack_start (GTK_BOX (hbox_query), entry_tag_query, TRUE, TRUE, 2);
	gtk_tooltips_set_tip (tooltips, entry_tag_query, _("Enter tag name"), NULL);
	
	checkbutton_query_partial_match = gtk_check_button_new_with_mnemonic (_("Partial match"));
	gtk_widget_show (checkbutton_query_partial_match);
	gtk_box_pack_start (GTK_BOX (hbox_query), checkbutton_query_partial_match, FALSE, FALSE, 0);
	gtk_tooltips_set_tip (tooltips, checkbutton_query_partial_match, _("Performs a partial match search"), NULL);

	radiobutton_query_case_sensitive = gtk_radio_button_new_with_mnemonic (NULL, _("Case sensitive"));
	gtk_widget_show (radiobutton_query_case_sensitive);
	gtk_box_pack_start (GTK_BOX (hbox_query), radiobutton_query_case_sensitive, FALSE, FALSE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton_query_case_sensitive), radiobutton_query_case_group);
	radiobutton_query_case_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton_query_case_sensitive));
	
	radiobutton_query_case_insensitive = gtk_radio_button_new_with_mnemonic (NULL, _("Case insensitive"));
	gtk_widget_show (radiobutton_query_case_insensitive);
	gtk_box_pack_start (GTK_BOX (hbox_query), radiobutton_query_case_insensitive, FALSE, FALSE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton_query_case_insensitive), radiobutton_query_case_group);
	radiobutton_query_case_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton_query_case_insensitive));
	
	button_query_find = gtk_button_new_from_stock ("gtk-find");
	gtk_widget_show (button_query_find);
	gtk_box_pack_start (GTK_BOX (hbox_query), button_query_find, FALSE, FALSE, 2);
	gtk_tooltips_set_tip (tooltips, button_query_find, _("Click to search tag"), NULL);
	
	button_query_show_all = gtk_button_new_with_mnemonic (_("Show all"));
	gtk_widget_show (button_query_show_all);
	gtk_box_pack_start (GTK_BOX (hbox_query), button_query_show_all, FALSE, FALSE, 0);
	
	scrolledwindow_tag_result = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow_tag_result);
	gtk_box_pack_start (GTK_BOX (vbox_tags), scrolledwindow_tag_result, TRUE, TRUE, 0);
	
	treeview_tags = gtk_tree_view_new ();
	gtk_widget_show (treeview_tags);
	gtk_container_add (GTK_CONTAINER (scrolledwindow_tag_result), treeview_tags);

	text_renderer = gtk_cell_renderer_text_new ();	
	first_col =
		gtk_tree_view_column_new_with_attributes (_("Tag"),  text_renderer,  "text", 0, NULL);
	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(first_col),   TRUE);


	text_renderer = gtk_cell_renderer_text_new ();	
	second_col =
		gtk_tree_view_column_new_with_attributes (_("File"),  text_renderer,  "text", 1, NULL);
	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(second_col),   TRUE);
							 
	text_renderer = gtk_cell_renderer_text_new ();	
	third_col = 
		gtk_tree_view_column_new_with_attributes(_("Line"),  text_renderer,  "text", 2, NULL);
	gtk_tree_view_column_set_resizable(	GTK_TREE_VIEW_COLUMN(third_col),  TRUE);	

	gtk_tree_view_insert_column (GTK_TREE_VIEW (treeview_tags),	first_col, 0);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (treeview_tags),	second_col, 1);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (treeview_tags),	third_col, 2);

	store =
		gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
		
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview_tags),	 GTK_TREE_MODEL (store));
	g_object_unref(store);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview_tags), TRUE);
	
	tp->entry_current_tag_file = entry_current_tag_file;
	tp->entry_tag_query = entry_tag_query;
	tp->checkbutton_partial_match = checkbutton_query_partial_match;
	tp->radiobutton_case_sensitive = radiobutton_query_case_sensitive;
	tp->radiobutton_case_insensitive = radiobutton_query_case_insensitive;
	tp->treeview_tags = treeview_tags;
	
	g_signal_connect ((gpointer) button_create_tag_file, "clicked",
					G_CALLBACK (on_tp_button_create_tag_file_clicked),  tp);
	g_signal_connect ((gpointer) button_open_tag_file, "clicked",
					G_CALLBACK (on_tp_button_open_tag_file_clicked), tp);
	g_signal_connect ((gpointer) button_query_find, "clicked",
					G_CALLBACK (on_tp_button_query_find_clicked), tp);
	g_signal_connect ((gpointer) button_query_show_all, "clicked",
					G_CALLBACK (on_tp_button_query_show_all_clicked), tp);
	g_signal_connect ((gpointer) treeview_tags, "row_activated",
					G_CALLBACK (on_tp_treeview_tags_row_activated), tp);
	
	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (window_tab, window_tab, "window_tab");
	GLADE_HOOKUP_OBJECT (window_tab, entry_current_tag_file, "entry_current_tag_file");
	GLADE_HOOKUP_OBJECT (window_tab, entry_tag_query, "entry_tag_query");
	GLADE_HOOKUP_OBJECT (window_tab, checkbutton_query_partial_match, "checkbutton_query_partial_match");
	GLADE_HOOKUP_OBJECT (window_tab, radiobutton_query_case_sensitive, "radiobutton_query_case_sensitive");
	GLADE_HOOKUP_OBJECT (window_tab, radiobutton_query_case_insensitive, "radiobutton_query_case_insensitive");
	GLADE_HOOKUP_OBJECT (window_tab, treeview_tags, "treeview_tags");
	
	return window_tab;
}

GtkWidget*
create_tag_file_dialog (CssedTagPlugin* tp)
{
  GtkWidget *tag_file_dialog;
  GtkWidget *dialog_vbox;
  GtkWidget *vbox;
  GtkWidget *hbox_basedir;
  GtkWidget *label_baedir;
  GtkWidget *entry_basedir;
  GtkWidget *button_open_base_dir;
  GtkWidget *hbox_settings;
  GtkWidget *checkbutton_recursive;
  GtkWidget *checkbutton_force;
  GtkWidget *combo_lang;
  GList *combo_lang_items = NULL;
  GtkWidget *combo_entry_lang;
  GtkWidget *label_exclude;
  GtkWidget *entry_exclude;
  GtkWidget *hbox_langmap;
  GtkWidget *checkbutton_change_langmap;
  GtkWidget *entry_langmap;
  GtkWidget *scrolledwindow_output;
  GtkWidget *textview_output;
  GtkWidget *label_cmd;
  GtkWidget *dialog_action_area;
  GtkWidget *cancelbutton;
  GtkWidget *okbutton;
  GtkWidget *pushbutton;
  GtkTooltips *tooltips;
  GtkWidget *entry_current;
  gchar* current_tag_file;

  tooltips = gtk_tooltips_new ();

  entry_current = tp->entry_current_tag_file;

  tag_file_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (tag_file_dialog), _("Create tag file"));
  gtk_window_set_default_size(GTK_WINDOW (tag_file_dialog), -1, 400);

  dialog_vbox = GTK_DIALOG (tag_file_dialog)->vbox;
  gtk_widget_show (dialog_vbox);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (dialog_vbox), vbox, TRUE, TRUE, 0);

  hbox_basedir = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox_basedir);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_basedir, FALSE, FALSE, 2);

  label_baedir = gtk_label_new (_("Base directory"));
  gtk_widget_show (label_baedir);
  gtk_box_pack_start (GTK_BOX (hbox_basedir), label_baedir, FALSE, FALSE, 2);

  entry_basedir = gtk_entry_new ();
  gtk_widget_show (entry_basedir);
  gtk_box_pack_start (GTK_BOX (hbox_basedir), entry_basedir, TRUE, TRUE, 0);
  gtk_tooltips_set_tip (tooltips, entry_basedir, _("Enter here the base directory to scan for tags"), NULL);

  // check if exists a current tag file
  current_tag_file = gtk_editable_get_chars(GTK_EDITABLE(entry_current), 0, -1);
  if( strlen( current_tag_file ) > 0 ){
	gtk_entry_set_text( GTK_ENTRY(entry_basedir), g_dirname( current_tag_file ) );
  }
  g_free( current_tag_file );

  button_open_base_dir = gtk_button_new_from_stock ("gtk-open");
  gtk_widget_show (button_open_base_dir);
  gtk_box_pack_start (GTK_BOX (hbox_basedir), button_open_base_dir, FALSE, FALSE, 2);
  gtk_tooltips_set_tip (tooltips, button_open_base_dir, _("Opens a directory to be used as the base directory."), NULL);

  hbox_settings = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox_settings);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_settings, FALSE, FALSE, 2);

  checkbutton_recursive = gtk_check_button_new_with_mnemonic (_("Recursive"));
  gtk_widget_show (checkbutton_recursive);
  gtk_box_pack_start (GTK_BOX (hbox_settings), checkbutton_recursive, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, checkbutton_recursive, _("Descend recursively all directories"), NULL);
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(checkbutton_recursive), TRUE);

  checkbutton_force = gtk_check_button_new_with_mnemonic (_("Force language"));
  gtk_widget_show (checkbutton_force);
  gtk_box_pack_start (GTK_BOX (hbox_settings), checkbutton_force, FALSE, FALSE, 4);
  gtk_tooltips_set_tip (tooltips, checkbutton_force, _("Force file to be treated as a selected language"), NULL);

  combo_lang = gtk_combo_new ();
  g_object_set_data (G_OBJECT (GTK_COMBO (combo_lang)->popwin),
                     "GladeParentKey", combo_lang);
  gtk_widget_show (combo_lang);
  gtk_box_pack_start (GTK_BOX (hbox_settings), combo_lang, TRUE, TRUE, 2);
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Asm");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Asp");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Awk");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "BETA");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "C");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "C++");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "C#");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Cobol");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Eiffel");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Erlang");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Fortran");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "HTML");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Java");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "JavaScript");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Lisp");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Lua:");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Make");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Pascal");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Perl");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "PHP");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Python");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "REXX");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Ruby");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Scheme");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Sh");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "SLang");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "SML");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "SQL");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Tcl");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Vera");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Verilog");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "Vim");
  combo_lang_items = g_list_append (combo_lang_items, (gpointer) "YACC");
  gtk_combo_set_popdown_strings (GTK_COMBO (combo_lang), combo_lang_items);
  g_list_free (combo_lang_items);

  combo_entry_lang = GTK_COMBO (combo_lang)->entry;
  gtk_widget_show (combo_entry_lang);
  gtk_tooltips_set_tip (tooltips, combo_entry_lang, _("In case of forcing a language it will be taken from here"), NULL);
  gtk_editable_set_editable (GTK_EDITABLE (combo_entry_lang), FALSE);

  label_exclude = gtk_label_new (_("Exclude patterns"));
  gtk_widget_show (label_exclude);
  gtk_box_pack_start (GTK_BOX (hbox_settings), label_exclude, FALSE, TRUE, 3);

  entry_exclude = gtk_entry_new ();
  gtk_widget_show (entry_exclude);
  gtk_box_pack_start (GTK_BOX (hbox_settings), entry_exclude, TRUE, TRUE, 2);
  gtk_tooltips_set_tip (tooltips, entry_exclude, _("Enter patterns to exclude from the search"), NULL);

  hbox_langmap = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox_langmap);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_langmap, FALSE, FALSE, 0);

  checkbutton_change_langmap = gtk_check_button_new_with_mnemonic (_("Change language map "));
  gtk_widget_show (checkbutton_change_langmap);
  gtk_box_pack_start (GTK_BOX (hbox_langmap), checkbutton_change_langmap, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, checkbutton_change_langmap, _("Changes the language map: with \"PHP:.lib\" only *.lib files are recognized as PHP, with \"PHP:+.lib\" *.lib is added to the PHP recognized patterns."), NULL);

  entry_langmap = gtk_entry_new ();
  gtk_widget_show (entry_langmap);
  gtk_box_pack_start (GTK_BOX (hbox_langmap), entry_langmap, TRUE, TRUE, 2);
  gtk_tooltips_set_tip (tooltips, entry_langmap, _("Enter here the ctags language map (see man ctags for more info)"), NULL);

  scrolledwindow_output = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow_output);
  gtk_box_pack_start (GTK_BOX (vbox), scrolledwindow_output, TRUE, TRUE, 2);

  textview_output = gtk_text_view_new ();
  gtk_widget_show (textview_output);
  gtk_container_add (GTK_CONTAINER (scrolledwindow_output), textview_output);
  gtk_tooltips_set_tip (tooltips, textview_output, _("Command output"), NULL);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview_output), FALSE);

  /*label_cmd = gtk_label_new (_("ctags --verbose=yes --excmd=number --language-force=php `pwd"));*/
  label_cmd = gtk_label_new ("");
  gtk_widget_show (label_cmd);
  gtk_label_set_selectable (GTK_LABEL(label_cmd), TRUE);
  gtk_label_set_line_wrap (GTK_LABEL(label_cmd), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), label_cmd, FALSE, FALSE, 0);

  dialog_action_area = GTK_DIALOG (tag_file_dialog)->action_area;
  gtk_widget_show (dialog_action_area);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area), GTK_BUTTONBOX_END);

  cancelbutton = gtk_button_new_from_stock ("gtk-close");
  gtk_widget_show (cancelbutton);
  gtk_dialog_add_action_widget (GTK_DIALOG (tag_file_dialog), cancelbutton, GTK_RESPONSE_CLOSE);
  GTK_WIDGET_SET_FLAGS (cancelbutton, GTK_CAN_DEFAULT);

  okbutton = gtk_button_new_from_stock ("gtk-apply");
  gtk_widget_show (okbutton);
  gtk_dialog_add_action_widget (GTK_DIALOG (tag_file_dialog), okbutton, GTK_RESPONSE_APPLY);
  GTK_WIDGET_SET_FLAGS (okbutton, GTK_CAN_DEFAULT);

  pushbutton = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (pushbutton);
  gtk_dialog_add_action_widget (GTK_DIALOG (tag_file_dialog), pushbutton, GTK_RESPONSE_APPLY);
  GTK_WIDGET_SET_FLAGS (pushbutton, GTK_CAN_DEFAULT);

  g_signal_connect ((gpointer) button_open_base_dir, "clicked", G_CALLBACK (on_tp_button_open_base_dir_clicked), tp);
  g_signal_connect ((gpointer) cancelbutton, "clicked", G_CALLBACK (on_tp_create_tag_cancelbutton_clicked), tag_file_dialog);
  g_signal_connect ((gpointer) okbutton, "clicked", G_CALLBACK (on_tp_create_tag_okbutton_clicked), tag_file_dialog);
  g_signal_connect ((gpointer) pushbutton, "clicked", G_CALLBACK (on_tp_create_tag_pushbutton_clicked), tp);

  GLADE_HOOKUP_OBJECT_NO_REF (tag_file_dialog, tag_file_dialog, "tag_file_dialog");
  GLADE_HOOKUP_OBJECT (tag_file_dialog, entry_basedir, "entry_basedir");
  GLADE_HOOKUP_OBJECT (tag_file_dialog, checkbutton_recursive, "checkbutton_recursive");
  GLADE_HOOKUP_OBJECT (tag_file_dialog, checkbutton_force, "checkbutton_force");
  GLADE_HOOKUP_OBJECT (tag_file_dialog, combo_entry_lang, "combo_entry_lang");
  GLADE_HOOKUP_OBJECT (tag_file_dialog, entry_exclude, "entry_exclude");
  GLADE_HOOKUP_OBJECT (tag_file_dialog, checkbutton_change_langmap, "checkbutton_change_langmap");
  GLADE_HOOKUP_OBJECT (tag_file_dialog, entry_langmap, "entry_langmap");
  GLADE_HOOKUP_OBJECT (tag_file_dialog, textview_output, "textview_output");
  GLADE_HOOKUP_OBJECT (tag_file_dialog, label_cmd, "label_cmd");

  return tag_file_dialog;
}


void
create_tag_plugin_pop_menu_entry (CssedTagPlugin* tp)
{
	CssedWindow* window;
	GtkWidget* cssed_popmenu;
	GtkWidget* topmenu_item;
	GtkWidget* topmenu_menu;
	GtkWidget* find_tag;
	GtkWidget* create_tag;
	GtkWidget* goto_tag;

	window = (CssedWindow*) cssed_plugin_get_window( tp->plugin );
	cssed_popmenu = cssed_plugin_get_pop_menu( tp->plugin );

	topmenu_item = gtk_menu_item_new_with_label (_("Tags"));
	gtk_widget_show (topmenu_item);
	gtk_container_add (GTK_CONTAINER (cssed_popmenu), topmenu_item);
	
	topmenu_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (topmenu_item), topmenu_menu);

	create_tag = gtk_image_menu_item_new_with_label (_("Create/Update tag file in this file's directory"));
	gtk_widget_show (create_tag);
	gtk_container_add (GTK_CONTAINER (topmenu_menu), create_tag);	    
                                       
	find_tag = gtk_image_menu_item_new_with_label (_("Find this tag in current tag file"));
	gtk_widget_show (find_tag);
	gtk_container_add (GTK_CONTAINER (topmenu_menu), find_tag);


	goto_tag = gtk_image_menu_item_new_with_label (_("Go to this tag in current tag file"));
	gtk_widget_show (goto_tag);
	gtk_container_add (GTK_CONTAINER (topmenu_menu), goto_tag);

	
	tp->popmenu_item = topmenu_item;	

	g_signal_connect ((gpointer) create_tag, "activate", G_CALLBACK (on_tp_popmenu_create_tag_activate), tp);
	g_signal_connect ((gpointer) find_tag, "activate", G_CALLBACK (on_tp_popmenu_find_tag_activate), tp);
	g_signal_connect ((gpointer) goto_tag, "activate", G_CALLBACK (on_tp_popmenu_goto_tag_activate), tp);

}






