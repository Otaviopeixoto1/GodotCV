#include "colmap_resource_list.h"
#include "godot_cpp/core/class_db.hpp"


void ColmapResourceList::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_items"), &ColmapResourceList::get_items);
	ClassDB::bind_method(D_METHOD("set_items", "v"), &ColmapResourceList::set_items);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "items"), "set_items", "get_items");
}
