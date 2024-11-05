#include "HttpServerSubsystem.h"
#include "SimpleHttpServer.h"

USimpleHttpServer* UHttpServerSubsystem::GetSimpleHttpServer(TSubclassOf<USimpleHttpServer> SimpleHttpServerClass)
{
    return NewObject<USimpleHttpServer>(this, SimpleHttpServerClass);
}
