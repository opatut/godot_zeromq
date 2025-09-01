#!/usr/bin/env python
from pathlib import Path

env = SConscript("godot-cpp/SConstruct")

env.Append(LIBS=['libzmq'])

env.Append(CXXFLAGS=[f'-I{inc}' for inc in env['CPPPATH']])

# Add source files.
env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")
addon_path = Path("project/addons/zeromq")
project_name = "godot_zeromq"

# Create the library target (e.g. libexample.linux.debug.x86_64.so).
debug_or_release = "release" if env["target"] == "template_release" else "debug"
if env["platform"] == "macos":
    library = env.SharedLibrary(
        "{0}/bin/lib{1}.{2}.{3}.framework/{1}.{2}.{3}".format(
            addon_path,
            project_name,
            env["platform"],
            debug_or_release,
        ),
        source=sources,
    )
else:
    filename = "lib{}.{}.{}.{}{}".format(
        project_name,
        env["platform"],
        debug_or_release,
        env["arch"],
        env["SHLIBSUFFIX"],
    )

    library = env.SharedLibrary(
        addon_path/"bin"/filename,
        source=sources,
    )

Default(library)
