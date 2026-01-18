// Copyright (c) 2025 Nick McClure (Protospatial). All Rights Reserved.

#include "LLM/N2CLLMModels.h"
#include "LLM/N2CLLMModelRegistry.h"

// All model data is now centralized in FN2CLLMModelRegistry.
// These functions delegate to the registry for backward compatibility.

FString FN2CLLMModelUtils::GetOpenAIModelValue(EN2COpenAIModel Model)
{
	return FN2CLLMModelRegistry::Get().GetModelId(Model);
}

FString FN2CLLMModelUtils::GetAnthropicModelValue(EN2CAnthropicModel Model)
{
	return FN2CLLMModelRegistry::Get().GetModelId(Model);
}

FString FN2CLLMModelUtils::GetGeminiModelValue(EN2CGeminiModel Model)
{
	return FN2CLLMModelRegistry::Get().GetModelId(Model);
}

FString FN2CLLMModelUtils::GetDeepSeekModelValue(EN2CDeepSeekModel Model)
{
	return FN2CLLMModelRegistry::Get().GetModelId(Model);
}

FN2COpenAIPricing FN2CLLMModelUtils::GetOpenAIPricing(EN2COpenAIModel Model)
{
	return FN2CLLMModelRegistry::Get().GetOpenAIPricing(Model);
}

FN2CAnthropicPricing FN2CLLMModelUtils::GetAnthropicPricing(EN2CAnthropicModel Model)
{
	return FN2CLLMModelRegistry::Get().GetAnthropicPricing(Model);
}

FN2CDeepSeekPricing FN2CLLMModelUtils::GetDeepSeekPricing(EN2CDeepSeekModel Model)
{
	return FN2CLLMModelRegistry::Get().GetDeepSeekPricing(Model);
}

FN2CGeminiPricing FN2CLLMModelUtils::GetGeminiPricing(EN2CGeminiModel Model)
{
	return FN2CLLMModelRegistry::Get().GetGeminiPricing(Model);
}

bool FN2CLLMModelUtils::SupportsSystemPrompts(EN2COpenAIModel Model)
{
	return FN2CLLMModelRegistry::Get().SupportsSystemPrompts(Model);
}

bool FN2CLLMModelUtils::SupportsSystemPrompts(EN2CAnthropicModel Model)
{
	return FN2CLLMModelRegistry::Get().SupportsSystemPrompts(Model);
}
