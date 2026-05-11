#!/usr/bin/env python
import os
import sys
import subprocess
from methods import print_error


libname = "GodotCV"
projectdir = "project"

localEnv = Environment(tools=["default"], PLATFORM="")

# Build profiles can be used to decrease compile times.
# You can either specify "disabled_classes", OR
# explicitly specify "enabled_classes" which disables all other classes.
# Modify the example file as needed and uncomment the line below or
# manually specify the build_profile parameter when running SCons.

# localEnv["build_profile"] = "build_profile.json"

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
# OpenCV optional build args:
opts.Add(BoolVariable("use_cuda", "Enable CUDA support in OpenCV",  False))
opts.Add(BoolVariable("use_contrib", "Build opencv_contrib modules",   False))
opts.Add(PathVariable("cuda_path", "Path to CUDA toolkit", "/usr/local/cuda", PathVariable.PathIsDir))
opts.Add("cuda_arch_bin", "CUDA compute capabilities (e.g. '8.6;8.9')", "8.6")
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

#
# Setup dependencies ---------------> TODO: make it optional to use these. Allow the user to set the path to dependencies
#

# OpenCV:
use_cuda    = localEnv["use_cuda"]
use_contrib = localEnv["use_contrib"]
cuda_path   = localEnv["cuda_path"]
cuda_arch   = localEnv["cuda_arch_bin"] # check if this HAS TO match exactly the installed cuda version...


OPENCV_SRC = Dir("opencv").abspath
CONTRIB_SRC = Dir("opencv_contrib").abspath

flavour = "cuda" if use_cuda else "cpu"
OPENCV_BUILD_DIR = Dir(f"bin/opencv_{flavour}").abspath
OPENCV_INSTALL_DIR = Dir(f"thirdparty/opencv_{flavour}").abspath

def build_opencv(target, source, env):
    os.makedirs(OPENCV_BUILD_DIR, exist_ok=True)

    cmake_args = [
        "cmake", OPENCV_SRC,
        f"-B{OPENCV_BUILD_DIR}",
        "-DCMAKE_BUILD_TYPE=Release",
        f"-DCMAKE_INSTALL_PREFIX={OPENCV_INSTALL_DIR}",

        # Keep the build small, only install what the extension needs
        "-DBUILD_LIST=core,imgproc,imgcodecs,videoio,dnn,calib3d",
        "-DBUILD_SHARED_LIBS=OFF", # Build static lib
        "-DBUILD_TESTS=OFF",
        "-DBUILD_PERF_TESTS=OFF",
        "-DBUILD_EXAMPLES=OFF",
        "-DBUILD_opencv_python3=OFF",
        "-DBUILD_opencv_python_bindings_generator=OFF",
        "-DWITH_GTK=OFF", # Headless. Godot handles the window
        "-DWITH_FFMPEG=ON",
        "-DWITH_V4L=ON",
    ]

    if use_contrib:
        cmake_args.append(f"-DOPENCV_EXTRA_MODULES_PATH={CONTRIB_SRC}/modules")

    if use_cuda:
        cmake_args += [
            "-DWITH_CUDA=ON",
            "-DWITH_CUDNN=ON",
            "-DOPENCV_DNN_CUDA=ON",
            "-DCUDA_FAST_MATH=ON",
            f"-DCUDA_TOOLKIT_ROOT_DIR={cuda_path}",
            f"-DCUDA_ARCH_BIN={cuda_arch}",
            "-DWITH_CUBLAS=ON",
        ]
    else:
        cmake_args += [
            "-DWITH_CUDA=OFF",
            "-DWITH_CUDNN=OFF",
        ]

    print("Configuring OpenCV...")
    subprocess.check_call(cmake_args)

    jobs = os.cpu_count() or 4
    print("Building & installing OpenCV... #cores = " + jobs)
    subprocess.check_call([
        "cmake",
        "--build", OPENCV_BUILD_DIR,
        "--target", "install",
        "--parallel", str(jobs),
    ])


# Sentinel file: SCons only re-runs build_opencv when it is missing.
opencv_sentinel = os.path.join(OPENCV_INSTALL_DIR, "lib", "cmake", "opencv4", "OpenCVConfig.cmake")
opencv_built = env.Command(
    target = opencv_sentinel,
    source = [os.path.join(OPENCV_SRC, "CMakeLists.txt")],
    action = build_opencv,
)
env.AlwaysBuild(opencv_built)   # let SCons track the sentinel for diffs

opencv_inc  = os.path.join(OPENCV_INSTALL_DIR, "include", "opencv4")
opencv_lib_dir = os.path.join(OPENCV_INSTALL_DIR, "lib")
opencv_libs_3rdparty = os.path.join(OPENCV_INSTALL_DIR, "lib", "opencv4", "3rdparty")

env = localEnv.Clone()

#ADD LIB IMPORT HERE

if not (os.path.isdir("godot-cpp") and os.listdir("godot-cpp")):
    print_error("""godot-cpp is not available within this folder, as Git submodules haven't been initialized.
Run the following command to download godot-cpp:

    git submodule update --init --recursive""")
    sys.exit(1)

env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference as we're targeting a pre-4.3 baseline.")

# .dev doesn't inhibit compatibility, so we don't need to key it.
# .universal just means "compatible with all relevant arches" so we don't need to key it.
suffix = env['suffix'].replace(".dev", "").replace(".universal", "")

lib_filename = "{}{}{}{}".format(env.subst('$SHLIBPREFIX'), libname, suffix, env.subst('$SHLIBSUFFIX'))

library = env.SharedLibrary(
    "bin/{}/{}".format(env['platform'], lib_filename),
    source=sources,
)

copy = env.Install("{}/bin/{}/".format(projectdir, env["platform"]), library)

default_args = [library, copy]
Default(*default_args)
