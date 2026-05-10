#include "colmap_image_data.h"
#include "godot_cpp/core/class_db.hpp"

void ColmapImageData::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_id"), &ColmapImageData::get_id);
	ClassDB::bind_method(D_METHOD("set_id", "v"), &ColmapImageData::set_id);
	ClassDB::bind_method(D_METHOD("get_camera_id"), &ColmapImageData::get_camera_id);
	ClassDB::bind_method(D_METHOD("set_camera_id", "v"), &ColmapImageData::set_camera_id);
	ClassDB::bind_method(D_METHOD("get_name"), &ColmapImageData::get_name);
	ClassDB::bind_method(D_METHOD("set_name", "v"), &ColmapImageData::set_name);
	ClassDB::bind_method(D_METHOD("get_rotation"), &ColmapImageData::get_rotation);
	ClassDB::bind_method(D_METHOD("set_rotation", "v"), &ColmapImageData::set_rotation);
	ClassDB::bind_method(D_METHOD("get_translation"), &ColmapImageData::get_translation);
	ClassDB::bind_method(D_METHOD("set_translation", "v"), &ColmapImageData::set_translation);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "camera_id"), "set_camera_id", "get_camera_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_name", "get_name");
	ADD_PROPERTY(PropertyInfo(Variant::QUATERNION, "rotation"), "set_rotation", "get_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "translation"), "set_translation", "get_translation");
}

