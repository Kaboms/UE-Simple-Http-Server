// Copyright (C) 2024 Mikhail Davydov (kaboms) - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SimpleHttpServer.h"
#include "Templates/SubclassOf.h"
#include "HttpServerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UHttpServerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Use this function to get the server to live as long as the GameInstance is alive.
	UFUNCTION(BlueprintCallable, Meta= (DeterminesOutputType = "SimpleHttpServerClass"), Category = "Simple HTTP Server")
	USimpleHttpServer* GetSimpleHttpServer(TSubclassOf<USimpleHttpServer> SimpleHttpServerClass);

private:
	UPROPERTY()
	USimpleHttpServer* Singleton;
};
