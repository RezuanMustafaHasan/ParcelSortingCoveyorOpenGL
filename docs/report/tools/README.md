# Reproduce the report captures

These two helper sources are copies of the inspected application entry point and camera implementation. Changes only add custom camera values, existing light/pause toggles, and capture-state logging. The original application source tree was not edited.

`CaptureMain.cpp` uses the existing scene, simulation, shaders and rendering passes. `CaptureCamera.cpp` accepts `REPORT_CAMERA` as `eye_x,eye_y,eye_z,target_x,target_y,target_z,field_of_view_degrees`. `REPORT_LIGHT_OFF=1` and `REPORT_PAUSED=1` apply the corresponding existing simulation toggles after advancing to the selected time.

To reproduce on the inspected macOS/arm64 CMake Makefiles build:

```bash
python3 capture_report_frames.py /absolute/path/to/application/project
```

First build the application in its `build/` folder with the Unix Makefiles generator; this helper depends on that object-file layout. Install Pillow in the Python environment (`python3 -m pip install Pillow`). Run the script from this folder or any working directory. It writes frames to `docs/report/tools/regenerated/`; it does not replace report images. Other platforms should compile these helper sources with their normal compiler/link flags, replacing the original main/camera objects and keeping all other application sources and shaders the same.

`../assets/capture_metadata.json` records all custom frames, time, camera, options and actual simulation state. `../assets/crop_manifest.json` gives exact pixel crop rectangles. Apply each crop with Pillow or any lossless crop tool; no resampling, color correction or synthetic elements are used. Figure arrows are LaTeX/TikZ overlays and are not part of the rendered scene. Original preset screenshots were captured with the application's standard command listed in the report.
