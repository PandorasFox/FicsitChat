#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "FicsitChat_ConfigStruct.h"
#include "Interfaces/IHttpRequest.h"
#include "Module/GameWorldModule.h"
#include "Modules/ModuleManager.h"

#include "FicsitChatWorldModule.generated.h"

class FJsonObject;

UCLASS()
class UFicsitChatWorldModule : public UGameWorldModule {
	GENERATED_BODY()

  public:
	UFicsitChatWorldModule();

	virtual void BeginDestroy() override;
	virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;

	bool ValidateBotToken(const FString &botToken);
	void SendMessageToGame(const FString &messageContent, const FString &messageAuthor);
	void SendMessageToTelegram(const FString &messageAuthor, const FString &messageContent, bool isSystemMessage = false);

	// True while a Telegram message is being injected into the game chat, so the chat hook doesn't echo it back to Telegram
	bool isInjectingRemoteMessage = false;

	// Loaded from Configs/FicsitChat.cfg on world load
	FFicsitChat_ConfigStruct activeConfig;

  private:
	void RequestBotInfo();
	void PollUpdates();
	void OnUpdatesReceived(FHttpRequestPtr request, FHttpResponsePtr response, bool connectedSuccessfully);
	void SchedulePoll(float delaySeconds);
	void ProcessUpdate(const TSharedPtr<FJsonObject> &update);
	bool MatchesConfiguredChat(const TSharedPtr<FJsonObject> &chat) const;
	FString GetApiUrl(const FString &method) const;

	FString botToken;
	FString chatId;
	FString botUsername;
	int64 lastUpdateId = 0;
	bool isShuttingDown = false;
	FHttpRequestPtr activePollRequest;
	FTimerHandle pollTimerHandle;
};
