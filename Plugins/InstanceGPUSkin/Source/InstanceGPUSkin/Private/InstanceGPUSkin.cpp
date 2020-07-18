// Copyright Epic Games, Inc. All Rights Reserved.

#include "InstanceGPUSkin.h"

#define LOCTEXT_NAMESPACE "FInstanceGPUSkinModule"

void FInstanceGPUSkinModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

#if (ENGINE_MINOR_VERSION >= 21)    
	FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping("/Project", ShaderDirectory);
#endif

}

void FInstanceGPUSkinModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInstanceGPUSkinModule, InstanceGPUSkin)