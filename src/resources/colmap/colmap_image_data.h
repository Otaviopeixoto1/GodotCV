#pragma once
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"

using namespace godot;

class ColmapImageData : public Resource
{
	GDCLASS(ColmapImageData, Resource)

	int64_t id = 0;
	int64_t camera_id = 0;
	String name;
	Quaternion rotation; // (x,y,z,w) — already reordered from COLMAP's wxyz
	Vector3 translation;

public:
	void set_id(int64_t v) { id = v; }
	int64_t get_id() const { return id; }
	void set_camera_id(int64_t v) { camera_id = v; }
	int64_t get_camera_id() const { return camera_id; }
	void set_name(const String &v) { name = v; }
	String get_name() const { return name; }
	void set_rotation(const Quaternion &v) { rotation = v; }
	Quaternion get_rotation() const { return rotation; }
	void set_translation(const Vector3 &v) { translation = v; }
	Vector3 get_translation() const { return translation; }

protected:
	static void _bind_methods();
};
