#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/classes/camera3d.hpp"
#include "godot_cpp/variant/variant.hpp"

using namespace godot;

//TODO:
// -Make a camera node to export data of intrinsic and extrinsic parameters !
// -Allow the camera to be extended with gdscript to generate frames and be processed however the user might want to using opencv

class CVCamera : public Camera3D
{
	GDCLASS(CVCamera, Camera3D)

protected:
	static void _bind_methods();

public:
	CVCamera() = default;
	~CVCamera() override = default;

	void _ready() override;
};
