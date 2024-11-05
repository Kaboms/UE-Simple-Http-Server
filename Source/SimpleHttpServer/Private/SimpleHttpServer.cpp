#include "SimpleHttpServer.h"
#include "HttpPath.h"
#include "IHttpRouter.h"
#include "HttpServerHttpVersion.h"
#include "HttpServerModule.h"
#include "HttpServerResponse.h"
#include "Misc/EngineVersionComparison.h"

void USimpleHttpServer::BeginDestroy()
{
	Super::BeginDestroy();

	StopServer();
}

void USimpleHttpServer::StartServer(int32 ServerPort)
{
	if (ServerPort <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not start HttpServer, port number must be greater than zero!"));
		return;
	}

	CurrentServerPort = ServerPort;

	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();

	HttpRouter = HttpServerModule.GetHttpRouter(CurrentServerPort);

	if (HttpRouter.IsValid())
	{
		BindRoutes();

		HttpServerModule.StartAllListeners();

		bServerStarted = true;
		UE_LOG(LogTemp, Log, TEXT("Web server started on port = %d"), CurrentServerPort);
	}
	else
	{
		bServerStarted = false;
		UE_LOG(LogTemp, Error, TEXT("Could not start web server on port = %d"), CurrentServerPort);
	}
}

void USimpleHttpServer::StopServer()
{
	FHttpServerModule& httpServerModule = FHttpServerModule::Get();
	httpServerModule.StopAllListeners();

	if (HttpRouter.IsValid())
	{
		// Editor will crash after receive request if you start game from editor, close it and start again.
		// It is because HttpRouter lived in FHttpServerModule and don't be destroyed on game ending.
		// When server stopped or being destroyed we must unbind all handlers to prevent errors on the next game start.
		for (FHttpRouteHandle HttpRouteHandle : CreatedRouteHandlers)
		{
			HttpRouter->UnbindRoute(HttpRouteHandle);
		}
	}
}

void USimpleHttpServer::BindRoute(FString HttpPath, ENativeHttpServerRequestVerbs Verbs, FHttpServerRequestDelegate OnHttpServerRequest)
{
	RouteDelegates.Add(HttpPath, OnHttpServerRequest);

	if (HttpRouter.IsValid())
	{
		FHttpRouteHandle HttpRouteHandle = HttpRouter->BindRoute(FHttpPath(HttpPath), (EHttpServerRequestVerbs)Verbs,
#if UE_VERSION_OLDER_THAN(5, 4, 0)
			[&, HttpPath](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
			{
				return HandleRequest(HttpPath, Request, OnComplete);
			});
#else
			FHttpRequestHandler::CreateLambda([&, HttpPath](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
			{
				return HandleRequest(HttpPath, Request, OnComplete);
			}));
#endif

		CreatedRouteHandlers.Add(HttpRouteHandle);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed bind to HttpRouter: router is invalid"));
	}
}

void USimpleHttpServer::BindRouteNative(FString HttpPath, ENativeHttpServerRequestVerbs Verbs, FHttpRouteHandler Handler)
{
	RouteHandlers.Add(HttpPath, Handler);

	if (HttpRouter.IsValid())
	{
		FHttpRouteHandle HttpRouteHandle = HttpRouter->BindRoute(FHttpPath(HttpPath), (EHttpServerRequestVerbs)Verbs,
#if UE_VERSION_OLDER_THAN(5, 4, 0)
			[&, HttpPath](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
			{
				return HandleRequestNative(HttpPath, Request, OnComplete);
			});
#else
			FHttpRequestHandler::CreateLambda([&, HttpPath](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
			{
				return HandleRequestNative(HttpPath, Request, OnComplete);
			}));
#endif

		CreatedRouteHandlers.Add(HttpRouteHandle);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed bind to HttpRouter: router is invalid"));
	}
}

bool USimpleHttpServer::HandleRequest(FString HttpPath, const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	FNativeHttpServerRequest NativeHttpServerRequest;
	FillNativeRequst(Request, NativeHttpServerRequest);

	if (FHttpServerRequestDelegate* HttpServerRequestDelegate = RouteDelegates.Find(HttpPath))
	{
		if ((*HttpServerRequestDelegate).IsBound())
		{
			FNativeHttpServerResponse HttpServerResponse = (*HttpServerRequestDelegate).Execute(NativeHttpServerRequest);
			TUniquePtr<FHttpServerResponse> Response = MakeUnique<FHttpServerResponse>();
			Response->Body = HttpServerResponse.HttpServerResponse.Body;
			Response->Code = HttpServerResponse.HttpServerResponse.Code;
			Response->Headers = HttpServerResponse.HttpServerResponse.Headers;
			Response->HttpVersion = HttpServerResponse.HttpServerResponse.HttpVersion;

			OnComplete(MoveTemp(Response));
			return true;
		}
	}

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Error(EHttpServerResponseCodes::NotFound);
	OnComplete(MoveTemp(response));
	return true;
}

bool USimpleHttpServer::HandleRequestNative(FString HttpPath, const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	FNativeHttpServerRequest NativeHttpServerRequest;
	FillNativeRequst(Request, NativeHttpServerRequest);

	if (FHttpRouteHandler* HttpServerRequestDelegate = RouteHandlers.Find(HttpPath))
	{
		(*HttpServerRequestDelegate)(NativeHttpServerRequest);
		return true;
	}

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Error(EHttpServerResponseCodes::NotFound);
	OnComplete(MoveTemp(response));
	return true;
}

void USimpleHttpServer::FillNativeRequst(const FHttpServerRequest& Request, FNativeHttpServerRequest& NativeRequest)
{
	NativeRequest.Verb = (ENativeHttpServerRequestVerbs)Request.Verb;
	NativeRequest.RelativePath = *Request.RelativePath.GetPath();

	for (const auto& Header : Request.Headers)
	{
		FString StrHeaderVals;
		for (const auto& val : Header.Value)
		{
			StrHeaderVals += val + TEXT(" ");
		}

		NativeRequest.Headers.Add(Header.Key, StrHeaderVals);
	}

	NativeRequest.PathParams = Request.PathParams;
	NativeRequest.QueryParams = Request.QueryParams;


	// Convert UTF8 to FString
	FUTF8ToTCHAR BodyTCHARData(reinterpret_cast<const ANSICHAR*>(Request.Body.GetData()), Request.Body.Num());
	FString StrBodyData{ BodyTCHARData.Length(), BodyTCHARData.Get() };

	NativeRequest.Body = *StrBodyData;
}

FNativeHttpServerResponse USimpleHttpServer::MakeResponse(FString Text, FString ContentType, int32 Code)
{
	FNativeHttpServerResponse HttpServerResponse;
	HttpServerResponse.HttpServerResponse.Code = (EHttpServerResponseCodes)Code;

	FTCHARToUTF8 ConvertToUtf8(*Text);
	const uint8* ConvertToUtf8Bytes = (reinterpret_cast<const uint8*>(ConvertToUtf8.Get()));
	HttpServerResponse.HttpServerResponse.Body.Append(ConvertToUtf8Bytes, ConvertToUtf8.Length());

	FString Utf8CharsetContentType = FString::Printf(TEXT("%s;charset=utf-8"), *ContentType);
	TArray<FString> ContentTypeValue = { MoveTemp(Utf8CharsetContentType) };
	HttpServerResponse.HttpServerResponse.Headers.Add(TEXT("content-type"), MoveTemp(ContentTypeValue));

	return HttpServerResponse;
}

UWorld* USimpleHttpServer::GetWorld() const
{
	if (!GetOuter())
	{
		return nullptr;
	}

	if (Cast<UPackage>(GetOuter()) != nullptr)
	{
		return Cast<UWorld>(GetOuter()->GetOuter());
	}

	return GetOuter()->GetWorld();
}

void USimpleHttpServer::BindRoutes()
{
	// You can bind any functions for your C++ class
	//BindRoute("/Test", ENativeHttpServerRequestVerbs::GET, [this](FNativeHttpServerRequest HttpServerRequest) {USimpleHttpServer::TestRoute(HttpServerRequest); });

	ReceiveBindRoutes();
}
