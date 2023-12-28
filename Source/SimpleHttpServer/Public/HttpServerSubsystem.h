#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HttpServerSubsystem.generated.h"

class USimpleHttpServer;

/**
 * 
 */
UCLASS()
class UHttpServerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Use this function to get the server to live as long as the GameInstance is alive.
	UFUNCTION(BlueprintCallable, Meta= (DeterminesOutputType = "SimpleHttpServerClass"))
	USimpleHttpServer* GetSimpleHttpServer(TSubclassOf<USimpleHttpServer> SimpleHttpServerClass);
};
