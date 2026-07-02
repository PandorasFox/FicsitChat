#include "FicsitChatWorldModule.h"
#include "Dom/JsonObject.h"
#include "FGChatManager.h"
#include "FicsitChatModule.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "TimerManager.h"

namespace {
	// Telegram's HTML parse mode only requires these three entities to be escaped
	FString EscapeHtml(const FString &text) {
		FString escaped = text;
		escaped.ReplaceInline(TEXT("&"), TEXT("&amp;"));
		escaped.ReplaceInline(TEXT("<"), TEXT("&lt;"));
		escaped.ReplaceInline(TEXT(">"), TEXT("&gt;"));
		return escaped;
	}

	FString SerializeJson(const TSharedRef<FJsonObject> &jsonObject) {
		FString output;
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&output);
		FJsonSerializer::Serialize(jsonObject, writer);
		return output;
	}

	TSharedPtr<FJsonObject> ParseJson(const FString &input) {
		TSharedPtr<FJsonObject> jsonObject;
		const TSharedRef<TJsonReader<TCHAR>> reader = TJsonReaderFactory<TCHAR>::Create(input);
		FJsonSerializer::Deserialize(reader, jsonObject);
		return jsonObject;
	}
} // namespace

UFicsitChatWorldModule::UFicsitChatWorldModule() {
#if !WITH_EDITOR
	this->bRootModule = true;
#endif
}

void UFicsitChatWorldModule::BeginDestroy() {
	Super::BeginDestroy();

	isShuttingDown = true;
	if (activePollRequest.IsValid()) {
		activePollRequest->CancelRequest();
		activePollRequest.Reset();
	}
}

// Runs on game world load
void UFicsitChatWorldModule::DispatchLifecycleEvent(ELifecyclePhase Phase) {
	Super::DispatchLifecycleEvent(Phase);

	if (Phase != ELifecyclePhase::INITIALIZATION)
		return;

	UE_LOG(LogFicsitChat, Verbose, TEXT("DispatchLifecycleEvent"));

	// Get mod config
	activeConfig = FFicsitChat_ConfigStruct::LoadFromConfigFile();
	const FFicsitChat_ConfigStruct &config = activeConfig;

	if (!ValidateBotToken(config.BotToken)) {
		return;
	}
	if (config.ChannelId.TrimStartAndEnd().IsEmpty()) {
		UE_LOG(LogFicsitChat, Warning, TEXT("No Telegram chat ID is set. Please set the chat ID in the mod's configuration and reload your save. The Telegram bot will not be started until then."));
		return;
	}

	botToken = config.BotToken.TrimStartAndEnd();
	chatId = config.ChannelId.TrimStartAndEnd();

	UE_LOG(LogFicsitChat, Log, TEXT("Starting the Telegram bot. If nothing shows up in Telegram, check if your bot token and chat ID are valid, and check that the bot is a member of the chat."));
	RequestBotInfo();
}

bool UFicsitChatWorldModule::ValidateBotToken(const FString &botTokenToValidate) {
	const FString trimmedBotToken = botTokenToValidate.TrimStartAndEnd();

	if (trimmedBotToken.Contains(TEXT("BOT_TOKEN_HERE"))) {
		UE_LOG(LogFicsitChat, Warning,
			   TEXT("Failed to validate the Telegram bot token. The Telegram bot token is set to the default value, which is invalid.\nPlease change it in the mod's configuration and reload your save. The Telegram bot will not be started "
					"until then."));
		return false;
	}

	// Telegram bot tokens look like <numeric bot ID>:<35 character secret>
	FString botId, secret;
	if (!trimmedBotToken.Split(TEXT(":"), &botId, &secret) || botId.IsEmpty() || !botId.IsNumeric() || secret.Len() < 30) {
		UE_LOG(LogFicsitChat, Warning, TEXT("Failed to validate the Telegram bot token. Telegram bot tokens look like 123456789:AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA (get one from @BotFather)."));
		return false;
	}

	return true;
}

FString UFicsitChatWorldModule::GetApiUrl(const FString &method) const {
	return FString::Printf(TEXT("https://api.telegram.org/bot%s/%s"), *botToken, *method);
}

// Calls getMe to verify the bot token against the Telegram API, then starts polling for messages
void UFicsitChatWorldModule::RequestBotInfo() {
	const FHttpRequestRef request = FHttpModule::Get().CreateRequest();
	request->SetURL(GetApiUrl(TEXT("getMe")));
	request->SetVerb(TEXT("GET"));
	request->OnProcessRequestComplete().BindWeakLambda(this, [this](FHttpRequestPtr, FHttpResponsePtr response, bool connectedSuccessfully) {
		if (isShuttingDown) {
			return;
		}
		if (!connectedSuccessfully || !response.IsValid()) {
			UE_LOG(LogFicsitChat, Warning, TEXT("Failed to reach the Telegram API, retrying in 10 seconds. Check your internet connection."));
			GetWorld()->GetTimerManager().SetTimer(pollTimerHandle, this, &UFicsitChatWorldModule::RequestBotInfo, 10.0f, false);
			return;
		}

		const TSharedPtr<FJsonObject> json = ParseJson(response->GetContentAsString());
		if (!json.IsValid() || !json->GetBoolField(TEXT("ok"))) {
			UE_LOG(LogFicsitChat, Warning, TEXT("The Telegram API rejected the bot token (HTTP %d): %s\nPlease check the bot token in the mod's configuration and reload your save."), response->GetResponseCode(),
				   *response->GetContentAsString());
			return;
		}

		const TSharedPtr<FJsonObject> botInfo = json->GetObjectField(TEXT("result"));
		botUsername = botInfo->GetStringField(TEXT("username"));
		UE_LOG(LogFicsitChat, Log, TEXT("Connected to Telegram as @%s. Bridging the game chat with chat ID %s."), *botUsername, *chatId);
		PollUpdates();
	});
	request->ProcessRequest();
}

// Long polls the Telegram API for new messages; Telegram holds the request open until a message arrives or the timeout elapses
void UFicsitChatWorldModule::PollUpdates() {
	if (isShuttingDown || !GetWorld()) {
		return;
	}

	const TSharedRef<FJsonObject> requestBody = MakeShared<FJsonObject>();
	requestBody->SetNumberField(TEXT("timeout"), 20);
	requestBody->SetNumberField(TEXT("offset"), lastUpdateId + 1);
	TArray<TSharedPtr<FJsonValue>> allowedUpdates;
	allowedUpdates.Add(MakeShared<FJsonValueString>(TEXT("message")));
	requestBody->SetArrayField(TEXT("allowed_updates"), allowedUpdates);

	const FHttpRequestRef request = FHttpModule::Get().CreateRequest();
	request->SetURL(GetApiUrl(TEXT("getUpdates")));
	request->SetVerb(TEXT("POST"));
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	request->SetContentAsString(SerializeJson(requestBody));
	request->SetTimeout(30.0f);
	request->OnProcessRequestComplete().BindUObject(this, &UFicsitChatWorldModule::OnUpdatesReceived);
	activePollRequest = request;
	request->ProcessRequest();
}

void UFicsitChatWorldModule::OnUpdatesReceived(FHttpRequestPtr request, FHttpResponsePtr response, bool connectedSuccessfully) {
	activePollRequest.Reset();
	if (isShuttingDown) {
		return;
	}

	if (!connectedSuccessfully || !response.IsValid()) {
		SchedulePoll(5.0f);
		return;
	}

	const TSharedPtr<FJsonObject> json = ParseJson(response->GetContentAsString());
	if (!json.IsValid() || !json->GetBoolField(TEXT("ok"))) {
		UE_LOG(LogFicsitChat, Warning, TEXT("Failed to get updates from the Telegram API (HTTP %d): %s"), response.IsValid() ? response->GetResponseCode() : 0, response.IsValid() ? *response->GetContentAsString() : TEXT(""));
		SchedulePoll(10.0f);
		return;
	}

	for (const TSharedPtr<FJsonValue> &updateValue : json->GetArrayField(TEXT("result"))) {
		const TSharedPtr<FJsonObject> update = updateValue->AsObject();
		if (update.IsValid()) {
			ProcessUpdate(update);
		}
	}

	SchedulePoll(0.5f);
}

void UFicsitChatWorldModule::ProcessUpdate(const TSharedPtr<FJsonObject> &update) {
	lastUpdateId = FMath::Max(lastUpdateId, static_cast<int64>(update->GetNumberField(TEXT("update_id"))));

	const TSharedPtr<FJsonObject> *message;
	if (!update->TryGetObjectField(TEXT("message"), message)) {
		return;
	}

	FString messageContent;
	if (!(*message)->TryGetStringField(TEXT("text"), messageContent) || messageContent.IsEmpty()) {
		return;
	}

	const TSharedPtr<FJsonObject> *chat;
	if (!(*message)->TryGetObjectField(TEXT("chat"), chat) || !MatchesConfiguredChat(*chat)) {
		return;
	}

	FString messageAuthor = TEXT("Telegram");
	const TSharedPtr<FJsonObject> *author;
	if ((*message)->TryGetObjectField(TEXT("from"), author)) {
		if (!(*author)->TryGetStringField(TEXT("first_name"), messageAuthor)) {
			(*author)->TryGetStringField(TEXT("username"), messageAuthor);
		}
	}

	SendMessageToGame(messageContent, messageAuthor);
}

bool UFicsitChatWorldModule::MatchesConfiguredChat(const TSharedPtr<FJsonObject> &chat) const {
	// The configured chat ID can either be a numeric chat ID or a public @username
	const FString numericChatId = FString::Printf(TEXT("%lld"), static_cast<int64>(chat->GetNumberField(TEXT("id"))));
	if (chatId == numericChatId) {
		return true;
	}

	FString chatUsername;
	if (chat->TryGetStringField(TEXT("username"), chatUsername)) {
		return chatId.Equals(chatUsername, ESearchCase::IgnoreCase) || chatId.Equals(TEXT("@") + chatUsername, ESearchCase::IgnoreCase);
	}

	return false;
}

void UFicsitChatWorldModule::SchedulePoll(float delaySeconds) {
	if (isShuttingDown || !GetWorld()) {
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(pollTimerHandle, this, &UFicsitChatWorldModule::PollUpdates, delaySeconds, false);
}

void UFicsitChatWorldModule::SendMessageToGame(const FString &messageContent, const FString &messageAuthor) {
	const FFicsitChat_ConfigStruct &config = activeConfig;

	FChatMessageStruct chatMessageStruct{};
	chatMessageStruct.MessageText = FText::FromString(messageContent);
	chatMessageStruct.MessageType = EFGChatMessageType::CMT_PlayerMessage;
	chatMessageStruct.ServerTimeStamp = GetWorld()->TimeSeconds;
	chatMessageStruct.MessageSender = FText::FromString(messageAuthor);
	chatMessageStruct.MessageSenderColor = FLinearColor(config.ChatMessageColor.Red, config.ChatMessageColor.Green, config.ChatMessageColor.Blue);

	AFGChatManager *chatManager = AFGChatManager::Get(GetWorld());
	if (!chatManager) {
		return;
	}

	isInjectingRemoteMessage = true;
	if (GetWorld()->GetNetMode() == NM_Client) {
		// Clients can't broadcast to other players, so the message is only shown locally
		chatManager->AddChatMessageToReceived(chatMessageStruct);
	} else {
		chatManager->BroadcastChatMessage(chatMessageStruct);
	}
	isInjectingRemoteMessage = false;
}

void UFicsitChatWorldModule::SendMessageToTelegram(const FString &messageAuthor, const FString &messageContent, bool isSystemMessage) {
	if (botToken.IsEmpty() || chatId.IsEmpty()) {
		return;
	}

	// System messages (joins/leaves) already contain the player name in the text
	const FString text = isSystemMessage ? FString::Printf(TEXT("<i>%s</i>"), *EscapeHtml(messageContent)) : FString::Printf(TEXT("<b>%s</b>: %s"), *EscapeHtml(messageAuthor), *EscapeHtml(messageContent));

	const TSharedRef<FJsonObject> requestBody = MakeShared<FJsonObject>();
	requestBody->SetStringField(TEXT("chat_id"), chatId);
	requestBody->SetStringField(TEXT("text"), text);
	requestBody->SetStringField(TEXT("parse_mode"), TEXT("HTML"));

	const FHttpRequestRef request = FHttpModule::Get().CreateRequest();
	request->SetURL(GetApiUrl(TEXT("sendMessage")));
	request->SetVerb(TEXT("POST"));
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	request->SetContentAsString(SerializeJson(requestBody));
	request->OnProcessRequestComplete().BindWeakLambda(this, [](FHttpRequestPtr, FHttpResponsePtr response, bool connectedSuccessfully) {
		if (!connectedSuccessfully || !response.IsValid()) {
			UE_LOG(LogFicsitChat, Warning, TEXT("Failed to send a chat message to Telegram. Check your internet connection."));
		} else if (response->GetResponseCode() != 200) {
			UE_LOG(LogFicsitChat, Warning, TEXT("The Telegram API rejected a chat message (HTTP %d): %s"), response->GetResponseCode(), *response->GetContentAsString());
		}
	});
	request->ProcessRequest();
}
