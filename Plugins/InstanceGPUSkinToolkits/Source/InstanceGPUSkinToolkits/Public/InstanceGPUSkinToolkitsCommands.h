// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "InstanceGPUSkinToolkitsStyle.h"

class FInstanceGPUSkinToolkitsCommands : public TCommands<FInstanceGPUSkinToolkitsCommands>
{
public:

	FInstanceGPUSkinToolkitsCommands()
		: TCommands<FInstanceGPUSkinToolkitsCommands>(TEXT("InstanceGPUSkinToolkits"), NSLOCTEXT("Contexts", "InstanceGPUSkinToolkits", "InstanceGPUSkinToolkits Plugin"), NAME_None, FInstanceGPUSkinToolkitsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};

#endif