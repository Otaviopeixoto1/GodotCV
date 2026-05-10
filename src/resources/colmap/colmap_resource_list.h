#pragma once
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/variant/array.hpp"

using namespace godot;

class ColmapResourceList : public Resource
{
	GDCLASS(ColmapResourceList, Resource)

	Array items;
public:
	void set_items(const Array &v) { items = v; }
	Array get_items() const { return items; }

protected:
	static void _bind_methods();
};
