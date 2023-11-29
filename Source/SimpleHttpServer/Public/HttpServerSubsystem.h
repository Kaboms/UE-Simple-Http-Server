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
	UFUNCTION(BlueprintCallable)
	USimpleHttpServer* GetSimpleHttpServer(TSubclassOf<USimpleHttpServer> SimpleHttpServerClass);
};
