#!/usr/bin/env python
import os
import sys
import re
import subprocess
from pathlib import Path
from methods import print_error


libname = "GodotCV"
projectdir = "project"

localEnv = Environment(tools=["default"], PLATFORM="")

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
opts.Add(BoolVariable("build_deps",   "Build OpenCV before building the extension", False))
opts.Add(BoolVariable("use_cuda",     "Enable CUDA support in OpenCV",              False))
opts.Add(BoolVariable("use_contrib",  "Build opencv_contrib modules",               False))
opts.Add(PathVariable("cuda_path",    "Path to CUDA toolkit", "/usr/local/cuda", PathVariable.PathAccept))
opts.Add("cuda_arch_bin", "CUDA compute capabilities (e.g. '8.6;8.9')", "8.6")
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

build_deps  = localEnv["build_deps"]
use_cuda    = localEnv["use_cuda"]
use_contrib = localEnv["use_contrib"]
cuda_path   = localEnv["cuda_path"]
cuda_arch   = localEnv["cuda_arch_bin"]

# OpenCV paths:
OPENCV_SRC           = Dir("opencv").abspath
CONTRIB_SRC          = Dir("opencv_contrib").abspath
flavour              = "cuda" if use_cuda else "cpu"
OPENCV_BUILD_DIR     = Dir(f"bin/opencv_{flavour}").abspath
OPENCV_INSTALL_DIR   = Dir(f"thirdparty/opencv_{flavour}").abspath

if sys.platform == "win32":
    OPENCV_SENTINEL      = os.path.join(OPENCV_INSTALL_DIR, "OpenCVConfig.cmake")
    OPENCV_MODULES_CMAKE = os.path.join(OPENCV_INSTALL_DIR, "OpenCVModules-release.cmake")
else:
    OPENCV_SENTINEL      = os.path.join(OPENCV_INSTALL_DIR, "lib", "cmake", "opencv4", "OpenCVConfig.cmake")
    OPENCV_MODULES_CMAKE = os.path.join(OPENCV_INSTALL_DIR, "lib", "cmake", "opencv4", "OpenCVModules-release.cmake")

# OpenCV build
def build_opencv(target, source, env):
    os.makedirs(OPENCV_BUILD_DIR, exist_ok=True)

    cmake_args = [
        "cmake", OPENCV_SRC,
        f"-B{OPENCV_BUILD_DIR}",
        "-DCMAKE_BUILD_TYPE=Release",
        f"-DCMAKE_INSTALL_PREFIX={OPENCV_INSTALL_DIR}",
        "-DBUILD_LIST=core,imgproc,imgcodecs,videoio,dnn,calib3d",
        "-DBUILD_SHARED_LIBS=OFF",
        "-DBUILD_TESTS=OFF",
        "-DBUILD_PERF_TESTS=OFF",
        "-DBUILD_EXAMPLES=OFF",
        "-DBUILD_opencv_python3=OFF",
        "-DBUILD_opencv_python_bindings_generator=OFF",
        "-DWITH_GTK=OFF",
    ]

    # Platform-specific flags
    if sys.platform.startswith("linux"):
        cmake_args += ["-DWITH_FFMPEG=ON", "-DWITH_V4L=ON", "-DWITH_GSTREAMER=ON"]
    elif sys.platform == "darwin":
        cmake_args += ["-DWITH_FFMPEG=ON", "-DWITH_AVFOUNDATION=ON"]
    elif sys.platform == "win32":
        cmake_args += [
            "-DWITH_FFMPEG=ON",
            "-DWITH_MSMF=ON",
            "-DWITH_DSHOW=ON",
            "-DCMAKE_CONFIGURATION_TYPES=Release",  # configure step: restrict to Release only
        ]

    if use_contrib:
        cmake_args.append(f"-DOPENCV_EXTRA_MODULES_PATH={CONTRIB_SRC}/modules")

    if use_cuda:
        cmake_args += [
            "-DWITH_CUDA=ON", "-DWITH_CUDNN=ON",
            "-DOPENCV_DNN_CUDA=ON", "-DCUDA_FAST_MATH=ON",
            f"-DCUDA_TOOLKIT_ROOT_DIR={cuda_path}",
            f"-DCUDA_ARCH_BIN={cuda_arch}",
            "-DWITH_CUBLAS=ON",
        ]
    else:
        cmake_args += ["-DWITH_CUDA=OFF", "-DWITH_CUDNN=OFF"]

    print("Configuring OpenCV...")
    ret = subprocess.call(cmake_args)
    if ret != 0:
        print_error(f"CMake configuration failed with exit code {ret}")
        return ret

    jobs = os.cpu_count() or 4
    print(f"Building & installing OpenCV... #cores = {jobs}")

    # --config Release goes here (build step only), not in cmake_args above
    build_cmd = [
        "cmake", "--build", OPENCV_BUILD_DIR,
        "--target", "install",
        "--parallel", str(jobs),
    ]
    if sys.platform == "win32":
        build_cmd += ["--config", "Release"]

    ret = subprocess.call(build_cmd)
    if ret != 0:
        print_error(f"CMake build failed with exit code {ret}")
        return ret

    return 0

# OpenCV cmake parser
def parse_opencv_modules_cmake(cmake_file, build_type="release"):
    text = Path(cmake_file).read_text()

    location_key = "RELEASE" if build_type == "release" else "DEBUG"
    lib_paths = re.findall(
        rf'IMPORTED_LOCATION_{location_key}\s+"([^"]+)"', text)

    lib_dirs  = sorted({os.path.dirname(p) for p in lib_paths})
    lib_names = []
    for p in lib_paths:
        name = os.path.basename(p)
        name = re.sub(r'^lib', '', name)          # Linux/macOS: strip 'lib' prefix
        name = re.sub(r'\.(a|lib)$', '', name)    # strip extension
        lib_names.append(name)

    inc_raw  = re.findall(r'INTERFACE_INCLUDE_DIRECTORIES\s+"([^"]+)"', text)
    includes = sorted({inc for m in inc_raw for inc in m.split(";") if inc})

    # 3rdparty libs live alongside the opencv libs in the same dir on Windows,
    # and in a sibling '3rdparty' subdir on Linux/macOS
    extra_dirs = []
    for lib_dir in lib_dirs:
        for candidate in [
            lib_dir,                                                  # Windows: same dir
            os.path.join(lib_dir, "opencv4", "3rdparty"),            # Linux/macOS
        ]:
            if os.path.isdir(candidate) and candidate not in extra_dirs:
                extra_dirs.append(candidate)

    return includes, lib_dirs + extra_dirs, lib_names

def find_opencv_modules_cmake(install_dir):
    candidates = [
        # Linux / macOS
        os.path.join(install_dir, "lib", "cmake", "opencv4", "OpenCVModules-release.cmake"),
        os.path.join(install_dir, "share", "opencv4", "OpenCVModules-release.cmake"),
        # Windows MSVC - everything is flat in staticlib
        os.path.join(install_dir, "x64", "vc17", "staticlib", "OpenCVModules-release.cmake"),
        os.path.join(install_dir, "x64", "vc16", "staticlib", "OpenCVModules-release.cmake"),
    ]
    debug_candidates = [
        os.path.join(install_dir, "lib", "cmake", "opencv4", "OpenCVModules-debug.cmake"),
        os.path.join(install_dir, "x64", "vc17", "staticlib", "OpenCVModules-debug.cmake"),
        os.path.join(install_dir, "x64", "vc16", "staticlib", "OpenCVModules-debug.cmake"),
    ]

    for c in candidates:
        if os.path.isfile(c):
            return c, "release"
    for c in debug_candidates:
        if os.path.isfile(c):
            print("[WARN] Only a Debug OpenCV build was found.")
            print("[WARN] Rebuild with: scons build_deps=yes to get a Release build.")
            return c, "debug"

    return None, None

def get_opencv_3rdparty_libs(lib_dirs):
    opencv_mod = re.compile(r'^opencv_')
    libs = []
    for d in lib_dirs:
        if not os.path.isdir(d):
            continue
        for f in sorted(os.listdir(d)):
            # Linux/macOS: libXXX.a     Windows: XXX.lib
            m = re.match(r'^(?:lib)?(.+?)(?:\.a|\.lib)$', f)
            if m and not opencv_mod.match(m.group(1)):
                name = m.group(1)
                if name not in libs:
                    libs.append(name)
    return libs

if build_deps:
    print(f"Building OpenCV [{flavour}] into {OPENCV_INSTALL_DIR}")

    if not os.path.isdir(OPENCV_SRC) or not os.listdir(OPENCV_SRC):
        print_error(
            "opencv submodule is missing.\n"
            "Run:  git submodule update --init --recursive"
        )
        sys.exit(1)

    opencv_built = localEnv.Command(
        target = OPENCV_SENTINEL,
        source = os.path.join(OPENCV_SRC, "CMakeLists.txt"),
        action = build_opencv,
    )
    localEnv.AlwaysBuild(opencv_built)
    Default(opencv_built)

else:
    # Re-search at runtime in case it was just built
    modules_cmake, build_type = find_opencv_modules_cmake(OPENCV_INSTALL_DIR)
    if modules_cmake is None:
        print_error(
            f"OpenCV [{flavour}] has not been built yet.\n"
            "Build it first with:\n\n"
            "    scons build_deps=yes"
            + (" use_cuda=yes" if use_cuda else "")
            + (" use_contrib=yes" if use_contrib else "")
            + (f" cuda_arch_bin={cuda_arch}" if use_cuda else "")
            + "\n"
        )
        sys.exit(1)


    if not (os.path.isdir("godot-cpp") and os.listdir("godot-cpp")):
        print_error(
            "godot-cpp is not available within this folder, as Git submodules haven't been initialized.\n"
            "Run the following command to download godot-cpp:\n\n"
            "    git submodule update --init --recursive"
        )
        sys.exit(1)

    env = localEnv.Clone()
    env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

    # Link OpenCV
    cv_includes, cv_libdirs, cv_libs = parse_opencv_modules_cmake(Path(modules_cmake), build_type)
    extra_dirs  = [d for d in cv_libdirs if "3rdparty" in d or "staticlib" in d]
    cv_3rd_libs = get_opencv_3rdparty_libs(extra_dirs)
    print(f"[OpenCV] modules={len(cv_libs)}  3rdparty={len(cv_3rd_libs)}  cuda={use_cuda}")

    env.Append(CPPPATH = cv_includes)
    env.Append(LIBPATH = cv_libdirs)
    env.Append(LIBS = cv_libs + cv_3rd_libs)

    # Platform system libs
    if sys.platform.startswith("linux"):
        env["_LIBFLAGS"] = "-Wl,--start-group " + env.get("_LIBFLAGS", "") + " -Wl,--end-group"
        env.Append(LIBS=["pthread", "dl", "m", "z", "rt"])
    elif sys.platform == "darwin":
        env.Append(FRAMEWORKS=[
            "CoreFoundation", "AVFoundation",
            "CoreMedia", "CoreVideo", "Accelerate",
        ])
        env.Append(LIBS=["m", "z"])
    elif sys.platform == "win32":
        env.Append(LIBS=[
            "ws2_32", "comctl32", "gdi32",
            "ole32", "setupapi", "vfw32",
        ])

    if use_cuda:
        env.Append(CPPDEFINES=["WITH_CUDA"])
        env.Append(LIBPATH=[os.path.join(cuda_path, "lib64")])
        env.Append(LIBS=["cudart", "cublas", "cudnn"])

    # Extension sources
    env.Append(CPPPATH=["src/"])
    sources = Glob("src/*.cpp")

    if env["target"] in ["editor", "template_debug"]:
        try:
            doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
            sources.append(doc_data)
        except AttributeError:
            print("Not including class reference as we're targeting a pre-4.3 baseline.")

    suffix = env['suffix'].replace(".dev", "").replace(".universal", "")
    lib_filename = "{}{}{}{}".format(env.subst('$SHLIBPREFIX'), libname, suffix, env.subst('$SHLIBSUFFIX'))

    library = env.SharedLibrary(
        "bin/{}/{}".format(env['platform'], lib_filename),
        source=sources,
    )

    copy = env.Install("{}/bin/{}/".format(projectdir, env["platform"]), library)

    default_args = [library, copy]
    Default(*default_args)
