#pragma once
#include "CoreMinimal.h"
#include "FicsitChat_ConfigStruct.generated.h"

USTRUCT(BlueprintType)
struct FFicsitChat_ConfigStruct_ChatMessageColor {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float Red{0.35f};

    UPROPERTY(BlueprintReadWrite)
    float Green{0.4f};

    UPROPERTY(BlueprintReadWrite)
    float Blue{0.95f};
};

USTRUCT(BlueprintType)
struct FFicsitChat_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FString BotToken{TEXT("BOT_TOKEN_HERE")};

    UPROPERTY(BlueprintReadWrite)
    bool HasJoinedMessage{true};

    UPROPERTY(BlueprintReadWrite)
    bool HasLeftMessage{true};

    UPROPERTY(BlueprintReadWrite)
    FString ChannelId{};

    UPROPERTY(BlueprintReadWrite)
    FFicsitChat_ConfigStruct_ChatMessageColor ChatMessageColor;

    /* Loads the configuration from Configs/FicsitChat.cfg, creating the file with defaults if it doesn't exist */
    static FFicsitChat_ConfigStruct LoadFromConfigFile();
};
