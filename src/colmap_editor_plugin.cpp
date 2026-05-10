#include "colmap_editor_plugin.h"



void ColmapEditorPlugin::_enter_tree()
{
	print_line("COLMAP EDITOR PLUGIN: START");
	importer.instantiate();
	add_import_plugin(importer);
}

void ColmapEditorPlugin::_exit_tree()
{
	print_line("COLMAP EDITOR PLUGIN: FINISH");
	remove_import_plugin(importer);
	importer.unref();
}

void ColmapEditorPlugin::_bind_methods()
{
}
