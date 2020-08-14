set PackageName=com.tx.gpuskin
adb shell settings delete global enable_gpu_debug_layers
adb shell settings delete global gpu_debug_app
adb shell settings delete global gpu_debug_layers_gles
adb shell settings delete global gpu_debug_layers_app

PAUSE