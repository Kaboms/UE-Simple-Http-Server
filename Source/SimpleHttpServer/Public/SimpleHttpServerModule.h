// Copyright (C) 2024 Mikhail Davydov (kaboms) - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SIMPLEHTTPSERVER_API FSimpleHttpServerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void GetHttpServerInstance();
};
