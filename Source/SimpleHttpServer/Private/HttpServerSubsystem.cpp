#include "HttpServerSubsystem.h"

USimpleHttpServer* UHttpServerSubsystem::GetSimpleHttpServer(TSubclassOf<USimpleHttpServer> SimpleHttpServerClass)
{
    if (!IsValid(Singleton))
    {
        Singleton = NewObject<USimpleHttpServer>(this, SimpleHttpServerClass);
    }

    return Singleton;
}
