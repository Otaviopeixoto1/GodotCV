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
opts.Add(BoolVariable("build_deps",  "Build OpenCV before building the extension", False))
opts.Add(BoolVariable("use_cuda",    "Enable CUDA support in OpenCV",              False))
opts.Add(BoolVariable("use_contrib", "Build opencv_contrib modules",               False))
opts.Add(BoolVariable("compiledb",   "Generate compile_commands.json for IDE",     False))
opts.Add(PathVariable("cuda_path",   "Path to CUDA toolkit", "/usr/local/cuda",    PathVariable.PathAccept))
opts.Add("cuda_arch_bin", "CUDA compute capabilities (e.g. '8.6;8.9')", "8.6")
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

build_deps  = localEnv["build_deps"]
use_cuda    = localEnv["use_cuda"]
use_contrib = localEnv["use_contrib"]
cuda_path   = localEnv["cuda_path"]
cuda_arch   = localEnv["cuda_arch_bin"]

# OpenCV paths
OPENCV_SRC         = Dir("opencv").abspath
CONTRIB_SRC        = Dir("opencv_contrib").abspath
flavour            = "cuda" if use_cuda else "cpu"
OPENCV_BUILD_DIR   = Dir(f"bin/opencv_{flavour}").abspath
OPENCV_INSTALL_DIR = Dir(f"thirdparty/opencv_{flavour}").abspath

#
# TODO: ALL OPEMCV BUILD AND LINKING SHOULD BE OFFLOADED TO ANOTHER PYTHON MODULE THAT WE IMPORT HERE
#

# Sentinel: just needs to be a file cmake produces. Exact path doesn't matter
# for correctness. It's only used as a SCons build target.
OPENCV_SENTINEL = os.path.join(OPENCV_INSTALL_DIR, "OpenCVConfig.cmake")

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

    if sys.platform.startswith("linux"):
        cmake_args += ["-DWITH_FFMPEG=ON", "-DWITH_V4L=ON", "-DWITH_GSTREAMER=ON"]
    elif sys.platform == "darwin":
        cmake_args += ["-DWITH_FFMPEG=ON", "-DWITH_AVFOUNDATION=ON"]
    elif sys.platform == "win32":
        cmake_args += [
            "-DWITH_FFMPEG=ON",
            "-DWITH_MSMF=ON",
            "-DWITH_DSHOW=ON",
            "-DCMAKE_CONFIGURATION_TYPES=Release",
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
def find_opencv_modules_cmake(install_dir):
    """Find OpenCVModules-release.cmake across all known platform layouts."""
    candidates = [
        # Linux / macOS
        os.path.join(install_dir, "lib", "cmake", "opencv4", "OpenCVModules-release.cmake"),
        os.path.join(install_dir, "share", "opencv4", "OpenCVModules-release.cmake"),
        # Windows MSVC
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


def parse_opencv_modules_cmake(cmake_file, build_type="release"):
    """
    Parse OpenCVModules-release.cmake to extract include dirs, lib dirs,
    and the authoritative ordered lib list from INTERFACE_LINK_LIBRARIES.
    Resolves CMake's ${_IMPORT_PREFIX} variable to absolute paths.
    """
    text = Path(cmake_file).read_text()
    cmake_dir = os.path.dirname(str(cmake_file))

    # _IMPORT_PREFIX is relative to the cmake file.
    # Windows path: staticlib/OpenCVModules.cmake
    # Linux path: lib/cmake/opencv4/OpenCVModules.cmake
    import_prefix = os.path.normpath(os.path.join(cmake_dir, "..", "..", ".."))

    def resolve(s):
        s = s.replace("${_IMPORT_PREFIX}", import_prefix)
        s = re.sub(r'\$<[^>]+>', '', s)  # strip any CMake generator expressions
        return s.strip()

    # Lib dirs from IMPORTED_LOCATION (where the opencv .lib/.a files actually are)
    location_key = "RELEASE" if build_type == "release" else "DEBUG"
    lib_paths = [resolve(p) for p in re.findall(
        rf'IMPORTED_LOCATION_{location_key}\s+"([^"]+)"', text)]
    lib_dirs = sorted({os.path.dirname(p) for p in lib_paths if p})

    # Include dirs
    includes = []
    for m in re.findall(r'INTERFACE_INCLUDE_DIRECTORIES\s+"([^"]+)"', text):
        for inc in m.split(";"):
            inc = resolve(inc)
            if inc and inc not in includes:
                includes.append(inc)

    # INTERFACE_LINK_LIBRARIES is the authoritative ordered list of everything
    # that needs to be linked: opencv modules + 3rdparty libs
    iface_libs = []
    for m in re.findall(r'INTERFACE_LINK_LIBRARIES\s+"([^"]+)"', text):
        for lib in m.split(";"):
            lib = resolve(lib)
            if lib and lib not in iface_libs:
                iface_libs.append(lib)

    # Extra dirs: 3rdparty lives alongside opencv libs on Windows (same dir),
    # or in a sibling subdir on Linux/macOS
    extra_dirs = []
    for lib_dir in lib_dirs:
        for candidate in [
            lib_dir,
            os.path.join(lib_dir, "opencv4", "3rdparty"),
        ]:
            candidate = os.path.normpath(candidate)
            if os.path.isdir(candidate) and candidate not in extra_dirs:
                extra_dirs.append(candidate)

    all_dirs = lib_dirs + [d for d in extra_dirs if d not in lib_dirs]
    return includes, all_dirs, iface_libs


def resolve_lib_names(lib_names, lib_dirs):
    """
    For each name from INTERFACE_LINK_LIBRARIES, find the actual file on disk
    and return the name SCons should pass to the linker.
    Handles inconsistent lib prefix conventions across platforms.
    """
    resolved = []
    for name in lib_names:
        # Absolute path: just extract the base name
        if os.path.isabs(name):
            base = os.path.basename(name)
            base = re.sub(r'\.(a|lib)$', '', base)
            if sys.platform != "win32":
                base = re.sub(r'^lib', '', base)
            if base and base not in resolved:
                resolved.append(base)
            continue

        # Try variations until we find the file on disk
        found = False
        for d in lib_dirs:
            for candidate_file in [
                f"{name}.lib",
                f"lib{name}.lib",
                f"{name}.a",
                f"lib{name}.a",
            ]:
                if os.path.isfile(os.path.join(d, candidate_file)):
                    actual = re.sub(r'\.(a|lib)$', '', candidate_file)
                    if sys.platform == "win32":
                        actual = re.sub(r'^lib', '', actual)
                    if actual not in resolved:
                        resolved.append(actual)
                    found = True
                    break
            if found:
                break

        if not found:
            print(f"[WARN] Could not find lib file for: {name} — passing as-is")
            if name not in resolved:
                resolved.append(name)

    return resolved


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
    cv_includes, cv_libdirs, cv_libs = parse_opencv_modules_cmake(
        Path(modules_cmake), build_type)
    cv_libs_resolved = resolve_lib_names(cv_libs, cv_libdirs)

    print(f"[OpenCV] libs={len(cv_libs_resolved)}  cuda={use_cuda}  build_type={build_type}")

    env.Append(CPPPATH = cv_includes)
    env.Append(LIBPATH = cv_libdirs)
    env.Append(LIBS    = cv_libs_resolved)

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
            doc_data = env.GodotCPPDocData(
                "src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
            sources.append(doc_data)
        except AttributeError:
            print("Not including class reference as we're targeting a pre-4.3 baseline.")

    suffix = env['suffix'].replace(".dev", "").replace(".universal", "")
    lib_filename = "{}{}{}{}".format(
        env.subst('$SHLIBPREFIX'), libname, suffix, env.subst('$SHLIBSUFFIX'))

    library = env.SharedLibrary(
        "bin/{}/{}".format(env['platform'], lib_filename),
        source=sources,
    )
    copy = env.Install("{}/bin/{}/".format(projectdir, env["platform"]), library)

    default_args = [library, copy]

    # compile_commands.json for IDE integration
    if localEnv["compiledb"]:
        env.Tool("compilation_db")
        cdb = env.CompilationDatabase("compile_commands.json")
        env.Alias("compiledb", cdb)
        default_args.append(cdb)

    Default(*default_args)
