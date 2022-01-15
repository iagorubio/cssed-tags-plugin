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
 * ------------------------------------------------------------------------------
 * Portions of code Copyright (c) 1996-2003, Darren Hiebert ( taken from readtags.c )
 */


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <plugin.h>
#include <stdlib.h>
#include <string.h>

#include "tagplugin.h"
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "readtags.h"

gchar*
add_to_allocated_string(gchar* old_string, gchar* string_to_add);

/* */
void
on_tp_button_create_tag_file_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget* dialog;
	CssedTagPlugin* plugin_data;

	plugin_data = TAG_PLUGIN( user_data );
	dialog = create_tag_file_dialog(plugin_data);
	gtk_widget_show( dialog );
}


void
on_tp_button_open_tag_file_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *entry;
	CssedTagPlugin *tp;
	gchar *curtag, *filename;

	tp = TAG_PLUGIN( user_data );
	entry = tp->entry_current_tag_file;	
	curtag = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);

	if( strlen(curtag) == 0 ) curtag = NULL;

	if( (filename = cssed_plugin_prompt_for_file_to_open(tp->plugin,  _("Select tag file to open"), curtag)) != NULL ){
		gtk_entry_set_text( GTK_ENTRY(entry), filename );
		g_free (filename);
	}
	g_free( curtag );
}


void
on_tp_button_query_find_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	CssedTagPlugin* tp;
	GtkListStore* store;
	GtkTreeIter iter;
	gchar* tagfile_name = NULL;
	gchar* name;
	tagFile* file;
    tagFileInfo info;
    tagEntry entry;
	gint options = 0;
	
	tp = TAG_PLUGIN( user_data );

	store = (GtkListStore*) gtk_tree_view_get_model( GTK_TREE_VIEW(tp->treeview_tags) );
	tagfile_name = gtk_editable_get_chars( GTK_EDITABLE(tp->entry_current_tag_file), 0, -1);

	if( tagfile_name == NULL || strlen( tagfile_name ) == 0 ){
		cssed_plugin_error_message(_("No open tag file"), _("First open or create a tag file to search"));
		return;
	}

    file = tagsOpen (tagfile_name, &info);
    if (file == NULL)
    {
		cssed_plugin_error_message( _("Error opening tag file"), _("Cannot open tag file\n%s\n\nError: %s"),  tagfile_name, strerror (info.status.error_number) );
		if( tagfile_name ) g_free( tagfile_name );
		return;
    }
    else
    {
		name = gtk_editable_get_chars (GTK_EDITABLE(tp->entry_tag_query), 0, -1);
		if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(tp->checkbutton_partial_match)) ){
			options |= TAG_PARTIALMATCH;
		}
		if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(tp->radiobutton_case_insensitive)) ){
			options |= TAG_IGNORECASE;
		}		
		gtk_list_store_clear( GTK_LIST_STORE(store) );	
		if (tagsFind (file, &entry, name, options) == TagSuccess)
		{
			do
			{
				gtk_list_store_append( store, &iter );
				gtk_list_store_set (store, &iter, 0, entry.name, 1, entry.file, 2, entry.address.pattern, -1 );
			} while (tagsFindNext (file, &entry) == TagSuccess);
		}
		tagsClose (file);
		g_free(name);
    }
	if( tagfile_name ) g_free( tagfile_name );
}


void
on_tp_button_query_show_all_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
	CssedTagPlugin* plugin_data;
	GtkListStore* store;
	GtkTreeIter iter;
	gchar* tagfile_name = NULL;
	tagFile* file;
    tagFileInfo info;
    tagEntry entry;
	
	plugin_data = TAG_PLUGIN( user_data );

	store = (GtkListStore*) gtk_tree_view_get_model( GTK_TREE_VIEW(plugin_data->treeview_tags) );
	tagfile_name = gtk_editable_get_chars( GTK_EDITABLE(plugin_data->entry_current_tag_file), 0, -1);

	if( tagfile_name == NULL || strlen( tagfile_name ) == 0 ){
		cssed_plugin_error_message(_("No open tag file"),_("First open or create a tag file to search"));
		return;
	}

    file = tagsOpen (tagfile_name, &info);
    if (file == NULL)
    {
		cssed_plugin_error_message(_("Error opening tag file"), _("Cannot open tag file\n%s\n\nError: %s"),  tagfile_name, strerror (info.status.error_number));
		if( tagfile_name ) g_free( tagfile_name );
		return;
    }
    else
    {
		gtk_list_store_clear( GTK_LIST_STORE(store) );		
		while (tagsNext (file, &entry) == TagSuccess){
			gtk_list_store_append( store, &iter );
			gtk_list_store_set (store, &iter, 0, entry.name, 1, entry.file, 2, entry.address.pattern, -1 );
		}
		tagsClose (file);
    }
	if( tagfile_name ) g_free( tagfile_name );
}


void
on_tp_treeview_tags_row_activated      (GtkTreeView     	*treeview,
                                        GtkTreePath     	*path,
                                        GtkTreeViewColumn 	*column,
                                        gpointer         	user_data)
{
	GtkTreeIter iter;
	GtkListStore* store;
	CssedTagPlugin* tp;
	gchar* filename;
	gchar* linenum;
	
	tp = TAG_PLUGIN(user_data);
	store = (GtkListStore*) gtk_tree_view_get_model( treeview );

	if (gtk_tree_model_get_iter (GTK_TREE_MODEL(store), &iter, path))
	{
		gtk_tree_model_get (GTK_TREE_MODEL(store), &iter, 1, &filename, 2, &linenum, -1);

		if( cssed_plugin_is_file_opened( tp->plugin, filename ) ){
			cssed_plugin_set_arrow_marker_at_line (tp->plugin, atoi(linenum));
		}else{
			cssed_plugin_open_file(tp->plugin, filename );
			if( cssed_plugin_is_file_opened( tp->plugin, filename ) ){
				cssed_plugin_set_arrow_marker_at_line ( tp->plugin, atoi(linenum) );			
			}else{
				cssed_plugin_output_write( tp->plugin, filename );
			}
		}
		g_free( filename );		
		g_free( linenum );
	} 
}

/* create-tag dialog callbacks from here */
void
on_tp_button_open_base_dir_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
	CssedTagPlugin *tp;
	GtkWidget* entry;
	GtkWidget* tag_file_dialog;
	gchar* filename;
	gchar* dirname;
	gchar* basedir_str;
	gchar* full_path;

	tp = (CssedTagPlugin*) user_data;

	tag_file_dialog = lookup_widget(GTK_WIDGET(button), "tag_file_dialog");
	entry = lookup_widget(GTK_WIDGET(button), "entry_basedir");
	basedir_str = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);

	if( strlen(basedir_str) > 0 ){
		if( !g_str_has_suffix (basedir_str,G_DIR_SEPARATOR_S) ){
			full_path = basedir_str;
			basedir_str = g_strdup_printf("%s%s", full_path, G_DIR_SEPARATOR_S);			
			g_free( full_path );
		}
	}else{
		g_free( basedir_str );
		basedir_str = NULL;
	}

	if( (filename = cssed_plugin_prompt_for_directory_to_open(tp->plugin, _("Select a directory"), basedir_str)) != NULL ){
		if( !g_file_test(  filename, G_FILE_TEST_IS_DIR ) ){
			dirname = g_path_get_dirname ( filename );
			if( !g_str_has_suffix (dirname, G_DIR_SEPARATOR_S) ){
				full_path = g_strdup_printf("%s%s", dirname, G_DIR_SEPARATOR_S);
				gtk_entry_set_text( GTK_ENTRY(entry),  full_path);
				g_free( full_path );
			}else{
				gtk_entry_set_text( GTK_ENTRY(entry),  dirname);
			}		
			g_free( dirname );
		}else{
			if( !g_str_has_suffix (filename,G_DIR_SEPARATOR_S) ){
				full_path = g_strdup_printf("%s%s", filename, G_DIR_SEPARATOR_S);
				gtk_entry_set_text( GTK_ENTRY(entry),  full_path);
				g_free( full_path );
			}else{
				gtk_entry_set_text( GTK_ENTRY(entry),  filename);
			}	
		}
	}
	if( basedir_str ) g_free( basedir_str );
	
	if( !gtk_window_is_active(GTK_WINDOW(tag_file_dialog)) ){
		gtk_window_present(GTK_WINDOW(tag_file_dialog));
	}
}


void
on_tp_create_tag_cancelbutton_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_destroy( GTK_WIDGET(user_data));
}


void
on_tp_create_tag_okbutton_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkTextBuffer* output_buffer;
	GtkWidget* basedir;
	GtkWidget* recursive;
	GtkWidget* force;
	GtkWidget* lang;
	GtkWidget* exclude;
	GtkWidget* change_langmap;
	GtkWidget* langmap;
	GtkWidget* output;
	GtkWidget* label_command;
	GtkTextIter enditer;
	gchar* cmd_line;
	gchar* output_str;
	gchar* error_str;
	gchar* exclude_str;
	gchar* basedir_str;
	gchar* utf8_str;
	gchar* tag_file_name;
	//gint status;
	GError* error = NULL;
	G_CONST_RETURN gchar* force_lang;
	G_CONST_RETURN gchar* langmap_str;
//#ifdef WIN32
//	gchar *old_ptr; 
//#endif
	basedir = lookup_widget(GTK_WIDGET(button), "entry_basedir");
	basedir_str = gtk_editable_get_chars( GTK_EDITABLE(basedir), 0, -1); 
	output = lookup_widget(GTK_WIDGET(button), "textview_output");
	output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output));
	
	if( strlen(basedir_str) <= 0 ){
		cssed_plugin_error_message(_("No base directory set."), _("You must set a base directory first"));
		g_free( basedir_str );
		return;
	}
	
	recursive = lookup_widget(GTK_WIDGET(button), "checkbutton_recursive");
	force = lookup_widget(GTK_WIDGET(button), "checkbutton_force");
	lang = lookup_widget(GTK_WIDGET(button), "combo_entry_lang");
	exclude = lookup_widget(GTK_WIDGET(button), "entry_exclude");
	change_langmap = lookup_widget(GTK_WIDGET(button), "checkbutton_change_langmap");
	langmap = lookup_widget(GTK_WIDGET(button), "entry_langmap");
	label_command = lookup_widget(GTK_WIDGET(button), "label_cmd");

#ifdef WIN32
	cmd_line = g_strdup("ctags --excmd=number "); 
#else
	cmd_line = g_strdup("ctags -V --excmd=number ");
#endif

	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(recursive)) ){
		cmd_line = add_to_allocated_string(cmd_line, "-R ");
	}

	if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(force) ) ){
		force_lang = gtk_entry_get_text( GTK_ENTRY(lang)) ;
		cmd_line = add_to_allocated_string(cmd_line, "--language-force=");
		cmd_line = add_to_allocated_string(cmd_line, (gchar*) force_lang);
		cmd_line = add_to_allocated_string(cmd_line, " ");
	}	

	exclude_str = gtk_editable_get_chars( GTK_EDITABLE(exclude), 0, -1); 
	if( strlen(exclude_str) > 0 ){
		cmd_line = add_to_allocated_string(cmd_line, "--exclude=");
		cmd_line = add_to_allocated_string(cmd_line, exclude_str);
		cmd_line = add_to_allocated_string(cmd_line, " ");
	}	
	g_free(exclude_str);

	if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(change_langmap) ) ){
		langmap_str = gtk_entry_get_text( GTK_ENTRY(langmap)) ;
		cmd_line = add_to_allocated_string(cmd_line, "--langmap=");
		cmd_line = add_to_allocated_string(cmd_line, (gchar*) langmap_str);
		cmd_line = add_to_allocated_string(cmd_line, " ");
	}

	if( !g_str_has_suffix (basedir_str,  G_DIR_SEPARATOR_S) ){
		tag_file_name = g_strdup_printf("-f '%s%sptags' ", basedir_str, G_DIR_SEPARATOR_S);
		cmd_line = add_to_allocated_string(cmd_line, tag_file_name);
		basedir_str = add_to_allocated_string(basedir_str, G_DIR_SEPARATOR_S);
#ifdef WIN32
		basedir_str = add_to_allocated_string(basedir_str, "*");
#endif
		g_free( tag_file_name );
	}else{
		tag_file_name = g_strdup_printf("-f '%sptags' ", basedir_str);
		cmd_line = add_to_allocated_string(cmd_line, tag_file_name);
#ifdef WIN32
		basedir_str = add_to_allocated_string(basedir_str, "*");
#endif
		g_free( tag_file_name );
	}	

	cmd_line = add_to_allocated_string(cmd_line, "'");
	cmd_line = add_to_allocated_string(cmd_line, basedir_str);
	cmd_line = add_to_allocated_string(cmd_line, "'");

	g_free( basedir_str );

	// UNCOMMENT FOR DEBUGGING
	// gtk_label_set_text( GTK_LABEL(label_command), cmd_line );

	if(g_spawn_command_line_sync (cmd_line, &output_str, &error_str, NULL, &error)){
		utf8_str = g_locale_to_utf8( (gchar*) output_str, -1, NULL, NULL, &error);
		gtk_text_buffer_set_text (output_buffer, utf8_str, -1);
		g_free( utf8_str );
		utf8_str = g_locale_to_utf8( (gchar*) error_str, -1, NULL, NULL, &error);
		gtk_text_buffer_insert_at_cursor (output_buffer, utf8_str, -1);
		g_free( utf8_str );

#ifdef WIN32
		gtk_text_buffer_insert_at_cursor (output_buffer, "command finished ok", -1);
#endif
		// move to end
		gtk_text_buffer_get_end_iter(output_buffer, &enditer);
		gtk_text_buffer_move_mark_by_name(output_buffer, "insert", &enditer);
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(output),
		                             gtk_text_buffer_get_mark(output_buffer, "insert"),
		                             0.0, FALSE, 0.0, 0.0);
		
	}else{
		gtk_text_buffer_set_text (output_buffer, error->message, -1);		
	}

	g_free( cmd_line );
}

void
on_tp_create_tag_pushbutton_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  CssedTagPlugin* tp;
  GtkWidget* entry_out;
  GtkWidget* entry_in;
  GtkWidget* dialog;
  gchar* str_in;
  gchar* str_out;

  tp = (CssedTagPlugin*) user_data;
  entry_in = lookup_widget(GTK_WIDGET(button), "entry_basedir");
  entry_out = tp->entry_current_tag_file;
  dialog = lookup_widget(GTK_WIDGET(button), "tag_file_dialog");
  str_in = gtk_editable_get_chars(GTK_EDITABLE(entry_in), 0, -1);

  if( strlen(str_in) > 0 ){
	if( !g_str_has_suffix (str_in, G_DIR_SEPARATOR_S) ){
		str_out = g_build_filename(str_in,"ptags", NULL);
		gtk_entry_set_text(GTK_ENTRY(entry_out), str_out);
	}else{
		str_out = g_strdup_printf("%sptags", str_in);
		gtk_entry_set_text(GTK_ENTRY(entry_out), str_out);
	}	
	g_free(str_in);
	g_free(str_out);
  }else{
	g_free(str_in);
  }
  gtk_widget_destroy(dialog);
}

void
on_tp_popmenu_find_tag_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	CssedTagPlugin* tp;
	GtkWidget* entry_tag_query;
	gchar* selection;

	tp = TAG_PLUGIN(user_data);	
	entry_tag_query = tp->entry_tag_query;
	
	selection = cssed_plugin_get_selected_text( tp->plugin );
	if( selection != NULL  ){
		cssed_plugin_select_page_with_widget_in_footer( tp->plugin, tp->user_interface );
		gtk_entry_set_text( GTK_ENTRY(entry_tag_query), selection );
		on_tp_button_query_find_clicked (NULL, (gpointer) tp );
		g_free(selection);
	}	
}

void
on_tp_popmenu_create_tag_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)

{
	GtkWidget* dialog;
	GtkWidget* entry_basedir;
	CssedTagPlugin* tp;
	gchar* basedir;
	gchar* filename;

	tp = TAG_PLUGIN(user_data);
	filename = cssed_plugin_document_get_filename( tp->plugin );

	if( filename == NULL ){
		cssed_plugin_error_message(_("No base directory set"), _("The file has not been saved, hence I cannot know the base directory.\nSave the file and try it again"));
	}else{
		basedir = g_path_get_dirname (filename);
		dialog = create_tag_file_dialog (tp);
		entry_basedir = lookup_widget( dialog, "entry_basedir");
		gtk_entry_set_text(GTK_ENTRY(entry_basedir), basedir);
		gtk_widget_show( dialog );
		g_free(filename);
		g_free(basedir);		
	}	
}

void
on_tp_popmenu_goto_tag_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	CssedTagPlugin* tp;
	gchar* selection;
	gchar* tagfile_name;
	tagFile* file;
    tagFileInfo info;
    tagEntry entry;

	tp = TAG_PLUGIN(user_data);	
	selection = cssed_plugin_get_selected_text( tp->plugin );

	if( selection != NULL  ){
		tagfile_name = gtk_editable_get_chars( GTK_EDITABLE(tp->entry_current_tag_file), 0, -1);
	
		if( tagfile_name == NULL || strlen( tagfile_name ) == 0 ){
			cssed_plugin_error_message(_("No open tag file"),_("First open or create a tag file to search"));
			return;
		}
	
		file = tagsOpen (tagfile_name, &info);
		if (file == NULL)
		{
			cssed_plugin_error_message(_("Error opening tag file"), _("Cannot open tag file\n%s\n\nError: %s"),  tagfile_name, strerror (info.status.error_number));
			if( tagfile_name ) g_free( tagfile_name );
			return;
		}
		else
		{
			if (tagsFind (file, &entry, selection, 0) == TagSuccess){
				if( cssed_plugin_is_file_opened(tp->plugin, (gchar*) entry.file) ){
					cssed_plugin_set_arrow_marker_at_line (tp->plugin, atoi(entry.address.pattern));					
				}else{
					cssed_plugin_open_file(tp->plugin, (gchar*) entry.file );
					if( cssed_plugin_is_file_opened( tp->plugin, (gchar*) entry.file ) ){
						cssed_plugin_set_arrow_marker_at_line (tp->plugin, atoi(entry.address.pattern) );			
					}
				}
			}
			tagsClose (file);
		}
		if( tagfile_name ) g_free( tagfile_name );
		g_free( selection );
	}
}

gchar*
add_to_allocated_string(gchar* old_string, gchar* string_to_add)
{
	gchar* new_string;
	new_string = g_strconcat(old_string, string_to_add, NULL);
	g_free( old_string );
	return new_string;
}






