"""
PlatformIO pre-build script: adds the project src/ directory to the
LVGL library's include path so it can find our lv_conf.h.
"""
Import("env")

# When LVGL is compiled as a library, its own build environment doesn't
# carry the project's build_flags.  We inject our lv_conf.h path here.
def add_lv_conf_path(source, target, env):
    # This runs right before the LVGL library is compiled
    env.Append(CPPPATH=[env["PROJECTSRC_DIR"]])

# Register the callback for the LVGL library build
env.AddPreAction("$BUILD_DIR/src/*", add_lv_conf_path)
