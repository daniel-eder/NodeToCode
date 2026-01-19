// Copyright (c) 2025 Nick McClure (Protospatial). All Rights Reserved.

#include "Core/Services/N2CTokenEstimationService.h"
#include "Core/N2CSettings.h"
#include "Core/N2CNodeCollector.h"
#include "Core/N2CNodeTranslator.h"
#include "Core/N2CSerializer.h"
#include "LLM/N2CLLMModelRegistry.h"
#include "LLM/N2CLLMTypes.h"
#include "Utils/N2CLogger.h"
#include "Engine/Blueprint.h"
#include "EdGraph/EdGraph.h"
#include "K2Node.h"
#include "Editor.h"

FN2CTokenEstimationService& FN2CTokenEstimationService::Get()
{
	static FN2CTokenEstimationService Instance;
	return Instance;
}

FN2CTokenEstimationService::FN2CTokenEstimationService()
{
}

FN2CTokenEstimationService::~FN2CTokenEstimationService()
{
	Shutdown();
}

void FN2CTokenEstimationService::Initialize()
{
	if (bIsInitialized)
	{
		return;
	}

	// Subscribe to settings changes
	SettingsChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(
		this, &FN2CTokenEstimationService::OnSettingsPropertyChanged);

	// Subscribe to Blueprint compilation (void delegate - no Blueprint parameter)
	if (GEditor)
	{
		BlueprintCompiledHandle = GEditor->OnBlueprintCompiled().AddLambda(
			[this]() { OnBlueprintCompiled(); });
	}

	// Initial refresh of model info
	RefreshModelInfo();

	bIsInitialized = true;
}

void FN2CTokenEstimationService::Shutdown()
{
	if (!bIsInitialized)
	{
		return;
	}

	// Unsubscribe from settings changes
	if (SettingsChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(SettingsChangedHandle);
		SettingsChangedHandle.Reset();
	}

	// Unsubscribe from Blueprint compilation
	if (BlueprintCompiledHandle.IsValid() && GEditor)
	{
		GEditor->OnBlueprintCompiled().Remove(BlueprintCompiledHandle);
		BlueprintCompiledHandle.Reset();
	}

	TokenCache.Empty();
	bIsInitialized = false;
}

int32 FN2CTokenEstimationService::GetTokenEstimate(const FN2CTagInfo& GraphInfo)
{
	// Check cache first
	if (FN2CTokenCacheEntry* CacheEntry = TokenCache.Find(GraphInfo.GraphGuid))
	{
		// Validate cache - check if Blueprint is dirty
		UBlueprint* Blueprint = Cast<UBlueprint>(FSoftObjectPath(GraphInfo.BlueprintPath).TryLoad());
		if (Blueprint && Blueprint->GetPackage())
		{
			if (!Blueprint->GetPackage()->IsDirty())
			{
				// Cache is valid
				return CacheEntry->CachedTokenCount;
			}
		}
		// Blueprint is dirty, invalidate cache entry
		TokenCache.Remove(GraphInfo.GraphGuid);
	}

	// Cache miss - serialize and calculate
	FString JsonContent = SerializeGraphToJson(GraphInfo);
	if (JsonContent.IsEmpty())
	{
		return 0;
	}

	// Estimate tokens: ~4 characters per token
	int32 TokenEstimate = FMath::CeilToInt(JsonContent.Len() / 4.0f);

	// Store in cache
	TokenCache.Add(GraphInfo.GraphGuid, FN2CTokenCacheEntry(TokenEstimate, JsonContent.Len(), GraphInfo.BlueprintPath));

	// Evict old entries if cache is too large
	EvictOldestCacheEntries();

	return TokenEstimate;
}

void FN2CTokenEstimationService::InvalidateCache(const FString& GraphGuid)
{
	TokenCache.Remove(GraphGuid);
	OnCacheInvalidated.Broadcast();
}

void FN2CTokenEstimationService::InvalidateAllCache()
{
	TokenCache.Empty();
	OnCacheInvalidated.Broadcast();
}

float FN2CTokenEstimationService::CalculateCost(int32 TokenCount) const
{
	if (bCachedIsLocal || CachedInputCostPer1M <= 0.0f)
	{
		return 0.0f;
	}

	// Calculate cost: (tokens / 1,000,000) * cost per 1M
	return (static_cast<float>(TokenCount) / 1000000.0f) * CachedInputCostPer1M;
}

void FN2CTokenEstimationService::RefreshModelInfo()
{
	const UN2CSettings* Settings = GetDefault<UN2CSettings>();
	if (!Settings)
	{
		return;
	}

	FN2CLLMModelRegistry& Registry = FN2CLLMModelRegistry::Get();
	const FN2CModelMetadata* Metadata = nullptr;

	switch (Settings->Provider)
	{
	case EN2CLLMProvider::OpenAI:
		Metadata = Registry.GetModelMetadata(Settings->OpenAI_Model);
		break;
	case EN2CLLMProvider::Anthropic:
		Metadata = Registry.GetModelMetadata(Settings->AnthropicModel);
		break;
	case EN2CLLMProvider::Gemini:
		Metadata = Registry.GetModelMetadata(Settings->Gemini_Model);
		break;
	case EN2CLLMProvider::DeepSeek:
		Metadata = Registry.GetModelMetadata(Settings->DeepSeekModel);
		break;
	case EN2CLLMProvider::Ollama:
		CachedModelName = Settings->OllamaModel;
		CachedFullContextWindow = 128000; // Default estimate for local models
		CachedUsableContextWindow = FMath::RoundToInt(CachedFullContextWindow * ContextWindowFactor);
		CachedInputCostPer1M = 0.0f;
		bCachedIsLocal = true;
		return;
	case EN2CLLMProvider::LMStudio:
		CachedModelName = Settings->LMStudioModel;
		CachedFullContextWindow = 128000; // Default estimate for local models
		CachedUsableContextWindow = FMath::RoundToInt(CachedFullContextWindow * ContextWindowFactor);
		CachedInputCostPer1M = 0.0f;
		bCachedIsLocal = true;
		return;
	default:
		break;
	}

	if (Metadata)
	{
		CachedModelName = Metadata->DisplayName;
		CachedFullContextWindow = Metadata->ContextWindowSize;
		CachedUsableContextWindow = FMath::RoundToInt(CachedFullContextWindow * ContextWindowFactor);
		CachedInputCostPer1M = Metadata->InputCostPer1M;
		bCachedIsLocal = false;
	}
	else
	{
		CachedModelName = TEXT("Unknown Model");
		CachedFullContextWindow = 128000;
		CachedUsableContextWindow = FMath::RoundToInt(CachedFullContextWindow * ContextWindowFactor);
		CachedInputCostPer1M = 0.0f;
		bCachedIsLocal = false;
	}
}

void FN2CTokenEstimationService::OnSettingsPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	// Check if this is the settings object
	if (!Object || !Object->IsA<UN2CSettings>())
	{
		return;
	}

	// Check if relevant property changed
	FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UN2CSettings, Provider) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UN2CSettings, OpenAI_Model) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UN2CSettings, AnthropicModel) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UN2CSettings, Gemini_Model) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UN2CSettings, DeepSeekModel) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UN2CSettings, OllamaModel) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UN2CSettings, LMStudioModel))
	{
		RefreshModelInfo();
		// Model change invalidates cost calculations, so clear the cache
		InvalidateAllCache();
		OnModelChanged.Broadcast();
	}
}

void FN2CTokenEstimationService::OnBlueprintCompiled()
{
	// The OnBlueprintCompiled delegate doesn't tell us which Blueprint was compiled,
	// so we invalidate all cache entries. This is conservative but safe.
	// Individual cache entries also self-invalidate on access if their Blueprint is dirty.
	if (TokenCache.Num() > 0)
	{
		InvalidateAllCache();
	}
}

FString FN2CTokenEstimationService::SerializeGraphToJson(const FN2CTagInfo& GraphInfo)
{
	// Load the Blueprint
	UBlueprint* Blueprint = Cast<UBlueprint>(FSoftObjectPath(GraphInfo.BlueprintPath).TryLoad());
	if (!Blueprint)
	{
		FN2CLogger::Get().LogWarning(
			FString::Printf(TEXT("Failed to load Blueprint: %s"), *GraphInfo.BlueprintPath),
			TEXT("TokenEstimationService"));
		return FString();
	}

	// Find the graph by GUID
	FGuid GraphGuid;
	if (!FGuid::Parse(GraphInfo.GraphGuid, GraphGuid))
	{
		FN2CLogger::Get().LogWarning(
			FString::Printf(TEXT("Invalid graph GUID: %s"), *GraphInfo.GraphGuid),
			TEXT("TokenEstimationService"));
		return FString();
	}

	UEdGraph* FoundGraph = nullptr;
	for (UEdGraph* Graph : Blueprint->UbergraphPages)
	{
		if (Graph && Graph->GraphGuid == GraphGuid)
		{
			FoundGraph = Graph;
			break;
		}
	}

	if (!FoundGraph)
	{
		for (UEdGraph* Graph : Blueprint->FunctionGraphs)
		{
			if (Graph && Graph->GraphGuid == GraphGuid)
			{
				FoundGraph = Graph;
				break;
			}
		}
	}

	if (!FoundGraph)
	{
		FN2CLogger::Get().LogWarning(
			FString::Printf(TEXT("Graph not found in Blueprint: %s"), *GraphInfo.GraphName),
			TEXT("TokenEstimationService"));
		return FString();
	}

	// Collect nodes from the graph
	TArray<UK2Node*> CollectedNodes;
	if (!FN2CNodeCollector::Get().CollectNodesFromGraph(FoundGraph, CollectedNodes))
	{
		return FString();
	}

	if (CollectedNodes.IsEmpty())
	{
		return FString();
	}

	// Translate to N2CBlueprint structure
	FN2CNodeTranslator& Translator = FN2CNodeTranslator::Get();
	if (!Translator.GenerateN2CStruct(CollectedNodes))
	{
		return FString();
	}

	// Serialize to minified JSON for accurate size calculation
	FN2CSerializer::SetPrettyPrint(false);
	return FN2CSerializer::ToJson(Translator.GetN2CBlueprint());
}

void FN2CTokenEstimationService::EvictOldestCacheEntries()
{
	if (TokenCache.Num() <= MaxCacheEntries)
	{
		return;
	}

	// Find and remove oldest entries
	while (TokenCache.Num() > MaxCacheEntries)
	{
		FString OldestKey;
		FDateTime OldestTime = FDateTime::MaxValue();

		for (const auto& Pair : TokenCache)
		{
			if (Pair.Value.CacheTimestamp < OldestTime)
			{
				OldestTime = Pair.Value.CacheTimestamp;
				OldestKey = Pair.Key;
			}
		}

		if (!OldestKey.IsEmpty())
		{
			TokenCache.Remove(OldestKey);
		}
		else
		{
			break;
		}
	}
}

FString FN2CTokenEstimationService::FormatNumber(int64 Number)
{
	FString Result = FString::Printf(TEXT("%lld"), Number);
	int32 InsertPosition = Result.Len() - 3;

	while (InsertPosition > 0)
	{
		Result.InsertAt(InsertPosition, TEXT(","));
		InsertPosition -= 3;
	}

	return Result;
}
