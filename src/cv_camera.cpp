#include "cv_camera.h"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/packed_float64_array.hpp"

void CVCamera::apply_intrinsics()
{
	if (!colmap_camera_data.is_valid())
		return;

	// COLMAP params layout per model:
	//   SIMPLE_PINHOLE (0): f, cx, cy
	//   PINHOLE        (1): fx, fy, cx, cy
	//   SIMPLE_RADIAL  (2): f, cx, cy, k
	//   RADIAL         (3): f, cx, cy, k1, k2
	//   OPENCV         (4): fx, fy, cx, cy, k1, k2, p1, p2
	//   OPENCV_FISHEYE (5): fx, fy, cx, cy, k1, k2, k3, k4

	PackedFloat64Array params = colmap_camera_data->get_params();
	if (params.size() == 0)
		return;

	int64_t height = colmap_camera_data->get_height();
	if (height == 0)
		return;

	// fx is always params[0]; for SIMPLE_PINHOLE/SIMPLE_RADIAL f=fx=fy
	double fx = params[0];

	// Vertical FOV from focal length and image height
	// fov_y = 2 * atan(height / (2 * fx))  ← uses fx as approximation;
	// if model has separate fy (PINHOLE, OPENCV) use params[1] instead
	double fy = fx;
	int model_id = colmap_camera_data->get_model_id();
	if (model_id == 1 || model_id == 4 || model_id == 5) {
		fy = params[1]; // these models have separate fy
	}

	double fov_y_rad = 2.0 * Math::atan(height / (2.0 * fy));
	double fov_y_deg = Math::rad_to_deg(fov_y_rad);

	set_fov((real_t)fov_y_deg);
	set_keep_aspect_mode(Camera3D::KEEP_HEIGHT);
}

void CVCamera::apply_extrinsics()
{
	if (!colmap_image_data.is_valid())
		return;

	// COLMAP stores world-to-camera transform: x_cam = R * x_world + t
	// We need camera-to-world for Godot's Transform3D.
	//
	// camera-to-world:
	//   R_cw = R^-1 = R^T  (rotation inverse = conjugate for unit quaternions)
	//   t_cw = -R^T * t

	Quaternion q = colmap_image_data->get_rotation();
	Vector3 t = colmap_image_data->get_translation();

	Quaternion q_inv = q.inverse();
	Basis basis = Basis(q_inv);
	Vector3 position = basis.xform(-t); // -R^T * t


	static const Basis colmap_to_godot(
			Vector3(1, 0, 0), // X stays
			Vector3(0, -1, 0), // Y flips
			Vector3(0, 0, -1) // Z flips
	);

	basis = colmap_to_godot * basis;
	position = colmap_to_godot.xform(position);

	set_global_transform(Transform3D(basis, position));
}

void CVCamera::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("set_colmap_camera_data", "camera data"),
			&CVCamera::set_colmap_camera_data);
	ClassDB::bind_method(D_METHOD("get_colmap_camera_data"),
			&CVCamera::get_colmap_camera_data);

	ClassDB::bind_method(D_METHOD("set_colmap_image_data", "image data"),
			&CVCamera::set_colmap_image_data);
	ClassDB::bind_method(D_METHOD("get_colmap_image_data"),
			&CVCamera::get_colmap_image_data);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "colmap_camera_data", PROPERTY_HINT_RESOURCE_TYPE, "ColmapCameraData"), "set_colmap_camera_data", "get_colmap_camera_data");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "colmap_image_data", PROPERTY_HINT_RESOURCE_TYPE, "ColmapImageData"), "set_colmap_image_data", "get_colmap_image_data");
}

void CVCamera::set_colmap_camera_data(const Ref<ColmapCameraData> &v)
{
	colmap_camera_data = v;
	apply_intrinsics();
}

void CVCamera::set_colmap_image_data(const Ref<ColmapImageData> &v)
{
	colmap_image_data = v;
	apply_extrinsics();
}

void CVCamera::_ready()
{
	//TODO: check if we have the properties set and if not, create them for the default camera
	apply_intrinsics();
	apply_extrinsics();
	print_line("CVCamera: READY");
}

