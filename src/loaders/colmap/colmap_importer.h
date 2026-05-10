#pragma once
#include "godot_cpp/classes/editor_import_plugin.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/classes/file_access.hpp"

using namespace godot;

class ColmapImporter : public EditorImportPlugin
{
	GDCLASS(ColmapImporter, EditorImportPlugin)

	Error import_cameras(Ref<FileAccess> &f, const String &save_path) const;
	Error import_images(Ref<FileAccess> &f, const String &save_path) const;
	Error import_point_cloud(Ref<FileAccess> &f, const String &save_path) const;

public:
	enum class ColmapFileType {
		CAMERAS,
		IMAGES,
		POINT_CLOUD,
		UNKNOWN
	};

	String _get_importer_name() const override { return "colmap.bin"; }
	String _get_visible_name() const override { return "COLMAP Binary"; }
	String _get_save_extension() const override { return "res"; }
	String _get_resource_type() const override { return "Resource"; }
	float _get_priority() const override { return 1.0f; }
	int _get_import_order() const override { return 0; }

	PackedStringArray _get_recognized_extensions() const override;
	TypedArray<Dictionary> _get_import_options(const String &path, int preset) const override;
	bool _get_option_visibility(const String &path, const StringName &option, const Dictionary &options) const override;
	Error _import(const String &source_file, const String &save_path, const Dictionary &options, const TypedArray<String> &platform_variants, const TypedArray<String> &gen_files) const override;

	static ColmapFileType detect_type(const String &filename);

protected:
	static void _bind_methods();
};
