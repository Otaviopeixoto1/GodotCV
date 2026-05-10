#pragma once
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/variant/packed_float64_array.hpp"

using namespace godot;

class ColmapCameraData : public Resource
{
	GDCLASS(ColmapCameraData, Resource)

	int64_t id = 0;
	int32_t model_id = 0;
	int64_t width = 0;
	int64_t height = 0;
	PackedFloat64Array params;

public:
	void set_id(int64_t v) { id = v; }
	int64_t get_id() const { return id; }
	void set_model_id(int32_t v) { model_id = v; }
	int32_t get_model_id() const { return model_id; }
	void set_width(int64_t v) { width = v; }
	int64_t get_width() const { return width; }
	void set_height(int64_t v) { height = v; }
	int64_t get_height() const { return height; }
	void set_params(const PackedFloat64Array &v) { params = v; }
	PackedFloat64Array get_params() const { return params; }

protected:
	static void _bind_methods();
};
