#include "FicsitChatModule.h"
#include "FGChatManager.h"
#include "FicsitChatWorldModule.h"
#include "Module/WorldModuleManager.h"
#include "Patching/NativeHookManager.h"

#define LOCTEXT_NAMESPACE "FFicsitChatModule"

DEFINE_LOG_CATEGORY(LogFicsitChat);

void FFicsitChatModule::StartupModule() {
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Register hooks
	RegisterHooks();
}

void FFicsitChatModule::ShutdownModule() {
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FFicsitChatModule::RegisterHooks() {
#if !WITH_EDITOR
	SUBSCRIBE_METHOD_AFTER(AFGChatManager::AddChatMessageToReceived, [](AFGChatManager *self, const FChatMessageStruct &newMessage) {
		UE_LOG(LogFicsitChat, Verbose, TEXT("Chat message by %s sent to all clients: %s"), *newMessage.MessageSender.ToString(), *newMessage.MessageText.ToString());

		UWorld *world = self->GetWorld();
		if (!world) {
			return;
		}

		UFicsitChatWorldModule *worldModule = Cast<UFicsitChatWorldModule>(world->GetSubsystem<UWorldModuleManager>()->FindModule(TEXT("FicsitChat")));
		if (!worldModule || worldModule->isInjectingRemoteMessage) {
			return;
		}

		FFicsitChat_ConfigStruct config = FFicsitChat_ConfigStruct::GetActiveConfig(world);

		FString messageAuthor = newMessage.MessageSender.ToString();
		FString messageContent = newMessage.MessageText.ToString();
		// System messages contain a <PlayerName/> token that game clients substitute at display time
		messageContent.ReplaceInline(TEXT("<PlayerName/>"), *messageAuthor);
		if (messageContent.EndsWith(TEXT("has joined the game!")) && !config.HasJoinedMessage) {
			return;
		}
		if (messageContent.EndsWith(TEXT("has left the game!")) && !config.HasLeftMessage) {
			return;
		}

		const bool isSystemMessage = newMessage.MessageType != EFGChatMessageType::CMT_PlayerMessage;
		worldModule->SendMessageToTelegram(messageAuthor, messageContent, isSystemMessage);
	});
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_GAME_MODULE(FFicsitChatModule, FicsitChat)
