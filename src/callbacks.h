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

#include <gtk/gtk.h>


void
on_tp_button_create_tag_file_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_tp_button_open_tag_file_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_tp_button_query_find_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_tp_button_query_show_all_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_tp_treeview_tags_row_activated         (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_tp_button_open_base_dir_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_tp_create_tag_cancelbutton_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_tp_create_tag_okbutton_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_tp_create_tag_pushbutton_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_tp_popmenu_find_tag_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_tp_popmenu_find_tag_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_tp_popmenu_create_tag_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_tp_popmenu_goto_tag_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
