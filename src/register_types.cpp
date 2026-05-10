#include "register_types.h"

#include <gdextension_interface.h>
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/defs.hpp"
#include "godot_cpp/godot.hpp"
#include "godot_cpp/classes/editor_plugin_registration.hpp"

#include "cv_camera.h"
#include "resources/colmap/colmap_camera_data.h"
#include "resources/colmap/colmap_point_cloud.h"
#include "resources/colmap/colmap_image_data.h"
#include "resources/colmap/colmap_resource_list.h"
#include "loaders/colmap/colmap_importer.h"
#include "colmap_editor_plugin.h"

using namespace godot;

void initialize_gdextension_types(ModuleInitializationLevel p_level)
{
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		GDREGISTER_CLASS(CVCamera);
		GDREGISTER_CLASS(ColmapPointCloud);
		GDREGISTER_CLASS(ColmapCameraData);
		GDREGISTER_CLASS(ColmapImageData);
		GDREGISTER_CLASS(ColmapResourceList);
	}
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR)
	{
		ClassDB::register_class<ColmapImporter>();
		ClassDB::register_class<ColmapEditorPlugin>();

		EditorPlugins::add_by_type<ColmapEditorPlugin>();
	}
}

void uninitialize_gdextension_types(ModuleInitializationLevel p_level)
{
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		return;
	}
}

extern "C"
{
	// Initialization
	GDExtensionBool GDE_EXPORT godotcv_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
	{
		GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
		init_obj.register_initializer(initialize_gdextension_types);
		init_obj.register_terminator(uninitialize_gdextension_types);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}
