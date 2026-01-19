// Copyright (c) 2025 Nick McClure (Protospatial). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintLibraries/N2CTagBlueprintLibrary.h"

/**
 * Token cache entry for efficient token estimation
 */
struct FN2CTokenCacheEntry
{
	/** Cached token estimate (JSON length / 4) */
	int32 CachedTokenCount = 0;

	/** Size of serialized JSON in characters */
	int32 CachedJsonSize = 0;

	/** Blueprint path for invalidation tracking */
	FString BlueprintPath;

	/** When the cache was created */
	FDateTime CacheTimestamp;

	FN2CTokenCacheEntry() = default;

	FN2CTokenCacheEntry(int32 InTokenCount, int32 InJsonSize, const FString& InBlueprintPath)
		: CachedTokenCount(InTokenCount)
		, CachedJsonSize(InJsonSize)
		, BlueprintPath(InBlueprintPath)
		, CacheTimestamp(FDateTime::Now())
	{
	}
};

/**
 * Centralized service for token estimation with caching and model info.
 * Provides notifications when model settings change or cache is invalidated.
 *
 * Key features:
 * - Caches token estimates per graph with Blueprint compilation invalidation
 * - Tracks current model info (context window, cost, etc.)
 * - Applies 65% rule: usable context = advertised context * 0.65
 * - Broadcasts notifications for UI updates
 */
class NODETOCODE_API FN2CTokenEstimationService
{
public:
	/**
	 * Get the singleton instance
	 */
	static FN2CTokenEstimationService& Get();

	/**
	 * Initialize the service (subscribe to delegates)
	 * Should be called during module startup
	 */
	void Initialize();

	/**
	 * Shutdown the service (unsubscribe from delegates)
	 * Should be called during module shutdown
	 */
	void Shutdown();

	// ==================== Token Estimation ====================

	/**
	 * Get the token estimate for a graph
	 * Uses caching with Blueprint compilation invalidation
	 * @param GraphInfo The graph to estimate tokens for
	 * @return Estimated token count
	 */
	int32 GetTokenEstimate(const FN2CTagInfo& GraphInfo);

	/**
	 * Invalidate cache for a specific graph (manual refresh)
	 * @param GraphGuid The GUID of the graph to invalidate
	 */
	void InvalidateCache(const FString& GraphGuid);

	/**
	 * Invalidate all cached token estimates
	 */
	void InvalidateAllCache();

	// ==================== Model Info (65% rule applied) ====================

	/**
	 * Get the current model's display name
	 */
	FString GetCurrentModelName() const { return CachedModelName; }

	/**
	 * Get the usable context window size (ContextWindow * 0.65)
	 * The advertised context window is not fully usable for input
	 */
	int32 GetUsableContextWindow() const { return CachedUsableContextWindow; }

	/**
	 * Get the full advertised context window size
	 */
	int32 GetFullContextWindow() const { return CachedFullContextWindow; }

	/**
	 * Get the input cost per 1M tokens
	 */
	float GetInputCostPer1M() const { return CachedInputCostPer1M; }

	/**
	 * Check if the current provider is a local model (free)
	 */
	bool IsLocalProvider() const { return bCachedIsLocal; }

	/**
	 * Calculate the estimated cost for a token count
	 * @param TokenCount The number of tokens
	 * @return Estimated cost in USD
	 */
	float CalculateCost(int32 TokenCount) const;

	/**
	 * Force refresh of model information from settings
	 */
	void RefreshModelInfo();

	// ==================== Notifications ====================

	/** Delegate fired when model settings change (provider or model) */
	DECLARE_MULTICAST_DELEGATE(FOnModelChanged);
	FOnModelChanged OnModelChanged;

	/** Delegate fired when token cache is invalidated */
	DECLARE_MULTICAST_DELEGATE(FOnCacheInvalidated);
	FOnCacheInvalidated OnCacheInvalidated;

private:
	FN2CTokenEstimationService();
	~FN2CTokenEstimationService();

	// Disable copy
	FN2CTokenEstimationService(const FN2CTokenEstimationService&) = delete;
	FN2CTokenEstimationService& operator=(const FN2CTokenEstimationService&) = delete;

	/**
	 * Handle settings property changes
	 */
	void OnSettingsPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);

	/**
	 * Handle Blueprint compilation - invalidates all cache entries
	 * Note: OnBlueprintCompiled delegate doesn't pass the specific Blueprint,
	 * so we invalidate all entries. Individual entries also self-invalidate
	 * on access if the Blueprint is dirty.
	 */
	void OnBlueprintCompiled();

	/**
	 * Serialize a graph to JSON for token estimation
	 * @param GraphInfo The graph to serialize
	 * @return JSON string, empty on failure
	 */
	FString SerializeGraphToJson(const FN2CTagInfo& GraphInfo);

	/**
	 * Evict oldest cache entries if over limit
	 */
	void EvictOldestCacheEntries();

	/**
	 * Format a number with comma separators
	 */
	static FString FormatNumber(int64 Number);

	// Token cache: GraphGuid string -> cache entry
	TMap<FString, FN2CTokenCacheEntry> TokenCache;

	// Maximum cache entries before LRU eviction
	static constexpr int32 MaxCacheEntries = 100;

	// Context window factor - advertised context is not fully usable and prone to hallucination past a certain amount
	static constexpr float ContextWindowFactor = 0.45f;

	// Cached model info
	FString CachedModelName;
	int32 CachedFullContextWindow = 0;
	int32 CachedUsableContextWindow = 0;
	float CachedInputCostPer1M = 0.0f;
	bool bCachedIsLocal = false;

	// Delegate handles
	FDelegateHandle SettingsChangedHandle;
	FDelegateHandle BlueprintCompiledHandle;

	// Initialization state
	bool bIsInitialized = false;
};
