// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#if WITH_EDITOR

#include "InstanceGPUSkinToolkitsCommands.h"

#define LOCTEXT_NAMESPACE "FInstanceGPUSkinToolkitsModule"

void FInstanceGPUSkinToolkitsCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "GPUAnimExpoter", "Execute InstanceGPUSkinToolkits action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE


#endif