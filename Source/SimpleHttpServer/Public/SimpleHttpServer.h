#pragma once

#include "CoreMinimal.h"
#include "HttpServerRequest.h"
#include "HttpResultCallback.h"
#include "HttpRouteHandle.h"

#include "SimpleHttpServer.generated.h"

UENUM(BlueprintType)
enum class ENativeHttpServerRequestVerbs : uint8
{
	NONE = 0,
	GET = 1 << 0,
	POST = 1 << 1,
	PUT = 1 << 2,
	PATCH = 1 << 3,
	DELETE = 1 << 4,
	OPTIONS = 1 << 5
};

//Contains only FHttpServerResponse. Just to be used from blueprints
USTRUCT(BlueprintType)
struct FNativeHttpServerResponse
{
	GENERATED_BODY()

public:
	FHttpServerResponse HttpServerResponse;
};

USTRUCT(BlueprintType)
struct FNativeHttpServerRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FString RelativePath;

	UPROPERTY(BlueprintReadOnly)
	/** The HTTP-compliant verb  */
	ENativeHttpServerRequestVerbs Verb = ENativeHttpServerRequestVerbs::NONE;

	UPROPERTY(BlueprintReadOnly)
	/** The HTTP headers */
	TMap<FString, FString> Headers;

	UPROPERTY(BlueprintReadOnly)
	/** The query parameters */
	TMap<FString, FString> QueryParams;

	UPROPERTY(BlueprintReadOnly)
	/** The path parameters */
	TMap<FString, FString> PathParams;

	UPROPERTY(BlueprintReadOnly)
	/** The raw body contents */
	FString Body;
};

typedef TFunction<void(FNativeHttpServerRequest Response)> FHttpRouteHandler;

DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FNativeHttpServerResponse, FHttpServerRequestDelegate, FNativeHttpServerRequest, HttpServerRequest);

/**
 * HttpServer service.
 */
UCLASS(Blueprintable, BlueprintType)
class USimpleHttpServer : public UObject
{
	GENERATED_BODY()

public:
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintPure, Category = "Http")
	bool IsServerStarted() const { return bServerStarted; }

	UFUNCTION(BlueprintCallable)
	void StartServer(int32 ServerPort = 8080);

	UFUNCTION(BlueprintCallable)
	void StopServer();

	// Bind Blueprint event to route
	UFUNCTION(BlueprintCallable)
	void BindRoute(FString HttpPath, ENativeHttpServerRequestVerbs Verbs, FHttpServerRequestDelegate OnHttpServerRequest);

	// Bind C++ function to route
	void BindRouteNative(FString HttpPath, ENativeHttpServerRequestVerbs Verbs, FHttpRouteHandler Handler);

	// Handle request and pass this to blueprint event
	bool HandleRequest(FString HttpPath, const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	// Handle request and pass this to c++ function
	bool HandleRequestNative(FString HttpPath, const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	// Fill FNativeHttpServerRequest to use it from bluerpints
	void FillNativeRequst(const FHttpServerRequest& Request, FNativeHttpServerRequest& NativeRequest);

	// Make response to send this to client
	UFUNCTION(BlueprintCallable)
	FNativeHttpServerResponse MakeResponse(FString Text, FString ContentType = "application/json", int32 Code = 200);

	virtual class UWorld* GetWorld() const override;

protected:
	void BindRoutes();

	UFUNCTION(BlueprintImplementableEvent, Meta=(DisplayName="BindRoutes"))
	void ReceiveBindRoutes();

public:
	// TODO Maybe it make sence to setup a some ports range and search for open one

	UPROPERTY(BlueprintReadOnly, Category = "Http")
	int32 CurrentServerPort = 8080;

protected:
	// Usualy used for blueprints
	TMap<FString, FHttpServerRequestDelegate> RouteDelegates;

	// Usualy used for c++
	TMap<FString, FHttpRouteHandler> RouteHandlers;

	// Cached Route Handlers. We should unbing them from route on self destroy
	TArray<FHttpRouteHandle> CreatedRouteHandlers;

	TSharedPtr<class IHttpRouter> HttpRouter;

	bool bServerStarted = false;
};
