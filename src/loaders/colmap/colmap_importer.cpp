#include "colmap_importer.h"
#include "resources/colmap/colmap_camera_data.h"
#include "resources/colmap/colmap_image_data.h"
#include "resources/colmap/colmap_resource_list.h"

#include "godot_cpp/classes/resource_saver.hpp"

//
// Binary helpers -> TODO: Move to another file
//
static double read_f64(Ref<FileAccess> &f)
{
	uint64_t bits = f->get_64();
	double v;
	memcpy(&v, &bits, 8);
	return v;
}

static String read_nt_string(Ref<FileAccess> &f)
{
	String s;
	while (true)
	{
		uint8_t c = f->get_8();
		if (!c)
			break;
		s += String::chr(c);
	}
	return s;
}

static int colmap_param_count(int model_id)
{
	switch (model_id)
	{
		case 0:
			return 3; // SIMPLE_PINHOLE
		case 1:
			return 4; // PINHOLE
		case 2:
			return 4; // SIMPLE_RADIAL
		case 3:
			return 5; // RADIAL
		case 4:
			return 8; // OPENCV
		case 5:
			return 8; // OPENCV_FISHEYE
		default:
			return 0;
	}
}



//
// Plugin Interface
//
ColmapImporter::ColmapFileType ColmapImporter::detect_type(const String &filename)
{
	// Match the stem only, case-insensitive, ignoring path
	String stem = filename.get_file().get_basename().to_lower();
	if (stem == "cameras")
		return ColmapFileType::CAMERAS;
	if (stem == "images")
		return ColmapFileType::IMAGES;
	return ColmapFileType::UNKNOWN;
}

PackedStringArray ColmapImporter::_get_recognized_extensions() const
{
	PackedStringArray e;
	e.push_back("bin");
	return e;
}

TypedArray<Dictionary> ColmapImporter::_get_import_options(const String &, int) const
{
	return TypedArray<Dictionary>();
}

bool ColmapImporter::_get_option_visibility(const String &path, const StringName &option, const Dictionary &options) const
{
	return true;
}

Error ColmapImporter::_import(const String &source_file, const String &save_path, const Dictionary &options, const TypedArray<String> &platform_variants, const TypedArray<String> &gen_files) const
{
	ColmapFileType type = detect_type(source_file);

	//Skip non-colmap files
	if (type == ColmapFileType::UNKNOWN)
	{
		return ERR_UNAVAILABLE;
	}

	Ref<FileAccess> f = FileAccess::open(source_file, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(!f.is_valid(), ERR_CANT_OPEN, "ColmapImporter: cannot open " + source_file);

	switch (type)
	{
		case ColmapFileType::CAMERAS:
			return import_cameras(f, save_path);
		case ColmapFileType::IMAGES:
			return import_images(f, save_path);
		default:
			return ERR_BUG; // unreachable
	}
}


Error ColmapImporter::import_cameras(Ref<FileAccess> &f, const String &save_path) const
{
	uint64_t count = f->get_64();
	Array items;
	items.resize((int)count);

	for (uint64_t i = 0; i < count; i++)
	{
		Ref<ColmapCameraData> cam;
		cam.instantiate();

		cam->set_id((int64_t)f->get_32());
		int model_id = (int32_t)f->get_32();
		cam->set_model_id(model_id);
		cam->set_width((int64_t)f->get_64());
		cam->set_height((int64_t)f->get_64());

		int n = colmap_param_count(model_id);
		PackedFloat64Array params;
		params.resize(n);
		for (int p = 0; p < n; p++)
			params.set(p, read_f64(f));
		cam->set_params(params);

		items[(int)i] = cam;
	}

	Ref<ColmapResourceList> list;
	list.instantiate();
	list->set_items(items);

	return ResourceSaver::get_singleton()->save(list, save_path + String(".") + _get_save_extension());
}

Error ColmapImporter::import_images(Ref<FileAccess> &f, const String &save_path) const
{
	uint64_t count = f->get_64();
	Array items;
	items.resize((int)count);

	for (uint64_t i = 0; i < count; i++)
	{
		Ref<ColmapImageData> img;
		img.instantiate();

		img->set_id((int64_t)f->get_32());

		// COLMAP: qw qx qy qz -> Godot Quaternion(x, y, z, w)
		double qw = read_f64(f);
		double qx = read_f64(f);
		double qy = read_f64(f);
		double qz = read_f64(f);

		img->set_rotation(Quaternion(
				(real_t)qx, (real_t)qy, (real_t)qz, (real_t)qw));

		double tx = read_f64(f), ty = read_f64(f), tz = read_f64(f);
		img->set_translation(Vector3(
				(real_t)tx, (real_t)ty, (real_t)tz));

		img->set_camera_id((int64_t)f->get_32());
		img->set_name(read_nt_string(f));

		// Skip points2D for now — 24 bytes each (x: f64, y: f64, id: i64)
		uint64_t num_pts = f->get_64();
		f->seek(f->get_position() + num_pts * 24);

		items[(int)i] = img;
	}

	Ref<ColmapResourceList> list;
	list.instantiate();
	list->set_items(items);

	return ResourceSaver::get_singleton()->save(list, save_path + String(".") + _get_save_extension());
}


void ColmapImporter::_bind_methods() {}
