#include "FicsitChat_ConfigStruct.h"
#include "Dom/JsonObject.h"
#include "FicsitChatModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

// The config file lives in the same folder and uses the same format as when the mod used SML's
// config system, so configs from older versions keep working unchanged.
static FString GetConfigFilePath() {
	return FPaths::ProjectDir() + TEXT("Configs/FicsitChat.cfg");
}

FFicsitChat_ConfigStruct FFicsitChat_ConfigStruct::LoadFromConfigFile() {
	FFicsitChat_ConfigStruct config{};

	FString configText;
	if (!FFileHelper::LoadFileToString(configText, *GetConfigFilePath())) {
		UE_LOG(LogFicsitChat, Log, TEXT("No config file found, creating %s with default values. Enter your Telegram bot token and chat ID there and reload your save."), *GetConfigFilePath());

		const TSharedRef<FJsonObject> defaults = MakeShared<FJsonObject>();
		defaults->SetStringField(TEXT("BotToken"), config.BotToken);
		defaults->SetBoolField(TEXT("HasJoinedMessage"), config.HasJoinedMessage);
		defaults->SetBoolField(TEXT("HasLeftMessage"), config.HasLeftMessage);
		defaults->SetStringField(TEXT("ChannelId"), TEXT("CHAT_ID_HERE"));
		const TSharedRef<FJsonObject> color = MakeShared<FJsonObject>();
		color->SetNumberField(TEXT("Red"), config.ChatMessageColor.Red);
		color->SetNumberField(TEXT("Green"), config.ChatMessageColor.Green);
		color->SetNumberField(TEXT("Blue"), config.ChatMessageColor.Blue);
		defaults->SetObjectField(TEXT("ChatMessageColor"), color);

		FString output;
		const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&output);
		FJsonSerializer::Serialize(defaults, writer);
		FFileHelper::SaveStringToFile(output, *GetConfigFilePath());
		return config;
	}

	TSharedPtr<FJsonObject> json;
	const TSharedRef<TJsonReader<TCHAR>> reader = TJsonReaderFactory<TCHAR>::Create(configText);
	if (!FJsonSerializer::Deserialize(reader, json) || !json.IsValid()) {
		UE_LOG(LogFicsitChat, Warning, TEXT("Failed to parse %s as JSON, using default values."), *GetConfigFilePath());
		return config;
	}

	json->TryGetStringField(TEXT("BotToken"), config.BotToken);
	json->TryGetBoolField(TEXT("HasJoinedMessage"), config.HasJoinedMessage);
	json->TryGetBoolField(TEXT("HasLeftMessage"), config.HasLeftMessage);
	json->TryGetStringField(TEXT("ChannelId"), config.ChannelId);

	const TSharedPtr<FJsonObject> *color;
	if (json->TryGetObjectField(TEXT("ChatMessageColor"), color)) {
		config.ChatMessageColor.Red = (*color)->GetNumberField(TEXT("Red"));
		config.ChatMessageColor.Green = (*color)->GetNumberField(TEXT("Green"));
		config.ChatMessageColor.Blue = (*color)->GetNumberField(TEXT("Blue"));
	}

	return config;
}
