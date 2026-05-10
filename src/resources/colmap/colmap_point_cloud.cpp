#include "colmap_point_cloud.h"
#include "godot_cpp/core/class_db.hpp"

void ColmapPointCloud::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_ids"), &ColmapPointCloud::get_ids);
	ClassDB::bind_method(D_METHOD("set_ids", "v"), &ColmapPointCloud::set_ids);
	ClassDB::bind_method(D_METHOD("get_positions"), &ColmapPointCloud::get_positions);
	ClassDB::bind_method(D_METHOD("set_positions", "v"), &ColmapPointCloud::set_positions);
	ClassDB::bind_method(D_METHOD("get_colors"), &ColmapPointCloud::get_colors);
	ClassDB::bind_method(D_METHOD("set_colors", "v"), &ColmapPointCloud::set_colors);
	ClassDB::bind_method(D_METHOD("get_errors"), &ColmapPointCloud::get_errors);
	ClassDB::bind_method(D_METHOD("set_errors", "v"), &ColmapPointCloud::set_errors);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "ids"), "set_ids", "get_ids");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "positions"), "set_positions", "get_positions");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_COLOR_ARRAY, "colors"), "set_colors", "get_colors");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT64_ARRAY, "errors"), "set_errors", "get_errors");
}
