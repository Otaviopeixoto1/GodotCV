#pragma once
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/variant/packed_float64_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/packed_color_array.hpp"
#include "godot_cpp/variant/packed_int64_array.hpp"

using namespace godot;

class ColmapPointCloud : public Resource
{
	GDCLASS(ColmapPointCloud, Resource)

	PackedInt64Array ids;
	PackedVector3Array positions;
	PackedColorArray colors;
	PackedFloat64Array errors;

public:
	void set_ids(const PackedInt64Array &v) { ids = v; }
	PackedInt64Array get_ids() const { return ids; }
	void set_positions(const PackedVector3Array &v) { positions = v; }
	PackedVector3Array get_positions() const { return positions; }
	void set_colors(const PackedColorArray &v) { colors = v; }
	PackedColorArray get_colors() const { return colors; }
	void set_errors(const PackedFloat64Array &v) { errors = v; }
	PackedFloat64Array get_errors() const { return errors; }

protected:
	static void _bind_methods();
};
