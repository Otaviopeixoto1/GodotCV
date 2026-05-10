#pragma once
#include "loaders/colmap/colmap_importer.h"
#include "godot_cpp/classes/editor_plugin.hpp"

using namespace godot;

//TODO: Extend to also enable other colmap utilities
class ColmapEditorPlugin : public EditorPlugin
{
	GDCLASS(ColmapEditorPlugin, EditorPlugin)

	Ref<ColmapImporter> importer;
public:
	void _enter_tree() override;
	void _exit_tree() override;

protected:
	static void _bind_methods();
};

