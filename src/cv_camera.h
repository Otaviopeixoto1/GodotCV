#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/classes/camera3d.hpp"
#include "godot_cpp/variant/variant.hpp"

#include "resources/colmap/colmap_camera_data.h"
#include "resources/colmap/colmap_image_data.h"

using namespace godot;

//TODO:
// -Make a camera node to export data of intrinsic and extrinsic parameters !
// -Allow the camera to be extended with gdscript to generate frames and be processed however the user might want to using opencv

class CVCamera : public Camera3D
{
	GDCLASS(CVCamera, Camera3D)
private:
	Ref<ColmapCameraData> colmap_camera_data;
	Ref<ColmapImageData> colmap_image_data;

	void apply_intrinsics();
	void apply_extrinsics();
		
protected:
	static void _bind_methods();

public:
	CVCamera() = default;
	~CVCamera() override = default;

	void set_colmap_camera_data(const Ref<ColmapCameraData> &v);
	Ref<ColmapCameraData> get_colmap_camera_data() const { return colmap_camera_data; }

	void set_colmap_image_data(const Ref<ColmapImageData> &v);
	Ref<ColmapImageData> get_colmap_image_data() const { return colmap_image_data; }

	//Add the a data property: allow to bind camera data from colmap

	void _ready() override;
};
