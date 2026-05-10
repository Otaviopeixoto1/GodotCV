#include "colmap_camera_data.h"
#include "godot_cpp/core/class_db.hpp"

void ColmapCameraData::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_id"), &ColmapCameraData::get_id);
	ClassDB::bind_method(D_METHOD("set_id", "v"), &ColmapCameraData::set_id);
	ClassDB::bind_method(D_METHOD("get_model_id"), &ColmapCameraData::get_model_id);
	ClassDB::bind_method(D_METHOD("set_model_id", "v"), &ColmapCameraData::set_model_id);
	ClassDB::bind_method(D_METHOD("get_width"), &ColmapCameraData::get_width);
	ClassDB::bind_method(D_METHOD("set_width", "v"), &ColmapCameraData::set_width);
	ClassDB::bind_method(D_METHOD("get_height"), &ColmapCameraData::get_height);
	ClassDB::bind_method(D_METHOD("set_height", "v"), &ColmapCameraData::set_height);
	ClassDB::bind_method(D_METHOD("get_params"), &ColmapCameraData::get_params);
	ClassDB::bind_method(D_METHOD("set_params", "v"), &ColmapCameraData::set_params);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "model_id"), "set_model_id", "get_model_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "width"), "set_width", "get_width");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height"), "set_height", "get_height");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT64_ARRAY, "params"), "set_params", "get_params");
}
