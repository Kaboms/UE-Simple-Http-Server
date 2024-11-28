// Copyright (C) 2024 Mikhail Davydov (kaboms) - All Rights Reserved

#include "SimpleHttpServerModule.h"

#define LOCTEXT_NAMESPACE "FSimpleHttpServerModule"

void FSimpleHttpServerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FSimpleHttpServerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSimpleHttpServerModule, SimpleHttpServer)