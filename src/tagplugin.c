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

#include <gtk/gtk.h>
#include <gmodule.h>
#include <plugin.h>

#include "tagplugin.h"
#include "interface.h"
#include "support.h"


G_MODULE_EXPORT CssedPlugin* init_plugin(void);
gboolean load_tagplugin ( CssedPlugin* );
void clean_tagplugin ( CssedPlugin* );

static CssedPlugin tagplugin;
// this will return the plugin to the caller
G_MODULE_EXPORT CssedPlugin* init_plugin()
{
    tagplugin.name = _("Tagplugin"); 				// the plugin name	
	tagplugin.description  = _("Frontend plugin for ctags");// the plugin description
	tagplugin.load_plugin = &load_tagplugin; 	// load plugin function, will build the UI
	tagplugin.clean_plugin = &clean_tagplugin;	// clean plugin function, will destroy the UI
	tagplugin.user_data = NULL;					// User data
	tagplugin.priv =  NULL;						// Private data, this is opaque and should be ignored
	
	return &tagplugin;
}


gboolean
load_tagplugin (CssedPlugin* plugin)
{
	CssedTagPlugin* tp;

#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif

    tp = g_malloc( sizeof(CssedTagPlugin));
	tp->plugin = &tagplugin;
    tp->user_interface = create_tagplugin_ui(tp);
	gtk_widget_show( tp->user_interface );
	tagplugin.user_data = tp;
	create_tag_plugin_pop_menu_entry (tp);
    cssed_plugin_add_page_with_widget_to_footer( &tagplugin, tp->user_interface,	_("Tags") );
	
	return TRUE;
}

/* could be used to post UI destroy
void g_module_unload (GModule *module)
{
	g_print(_("** Tag file support plugin unloaded\n"));	
}
*/
// to destroy UI and stuff.
void clean_tagplugin ( CssedPlugin* p )
{
	CssedTagPlugin* tp;
	GtkWidget* ui;
	
    tp =  TAG_PLUGIN( tagplugin.user_data );
	ui = tp->user_interface;
	gtk_widget_destroy(	ui );
	gtk_widget_destroy(	tp->popmenu_item );
	g_free(tp);

	return;
}



