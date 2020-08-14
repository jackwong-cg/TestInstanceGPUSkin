
rem 在这里设置好要调试的程序包名
set PackageName=com.tx.gpuskin

adb shell settings put global enable_gpu_debug_layers 1
adb shell settings put global gpu_debug_app %PackageName%
adb shell settings put global gpu_debug_layers_gles libVkLayer_GLES_RenderDoc.so
adb shell setprop debug.vr.profiler 1

PAUSE