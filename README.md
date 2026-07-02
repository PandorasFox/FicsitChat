# FICSIT.chat

![icon](assets/ficsit_chat_icon_128x128.png)

Satisfactory to Telegram chat bridge mod with lots of configurability.

<details open>
<summary>Image gallery</summary>

<br>

![promo banner, feat. DJMalachite, my FICSIT.chat bot, and shenanigans](assets/ficsit_chat_promo_banner.png)

</details>

## Usage

1. Create a Telegram bot by messaging [@BotFather](https://t.me/BotFather) on Telegram: send `/newbot` and follow the prompts (make sure to copy the bot token as you will need it in the next few steps)
    - Example bot name: `FICSIT.chat`
    - Example bot username: `FicsitChatBot`
2. Disable the bot's privacy mode so it can read group messages: send `/setprivacy` to @BotFather, select your bot, and choose `Disable`
3. Add the bot to the Telegram group you want to bridge the game chat with
4. Get the chat ID of the group:
    - Add [@getidsbot](https://t.me/getidsbot) to the group and copy the chat ID it prints (group chat IDs are negative numbers, e.g. `-1001234567890`), or
    - Send a message in the group and open `https://api.telegram.org/bot<YOUR_BOT_TOKEN>/getUpdates` in a browser, then copy the `message.chat.id` value
5. Enter the bot token and chat ID into FICSIT.chat's [configuration](#configuration)
6. Modify the other options in FICSIT.chat's configuration to your heart's content

Have fun!

### Configuration

> **Note:** The `Channel ID` option holds the Telegram **chat ID** (or the `@username` of a public group/channel).

#### Client

See the in-game configuration screen (`Main Menu->Mods->FICSIT.chat`) for modifying the configuration.

#### Dedicated servers

Change the options in the `FicsitChat.cfg` file located in the `FactoryGame/Configs` folder of the game. Check the [Game Install Folder Documentation](https://docs.ficsit.app/satisfactory-modding/latest/faq.html#Files_GameInstall) to find where it is. Modifying this file also works for the regular game.

You need to restart the game / dedicated server for the file's content to be reloaded.

The configuration file's content looks like this:

```
{
	"BotToken": "BOT_TOKEN_GOES_HERE",
	"HasJoinedMessage": true,
	"HasLeftMessage": true,
	"ChannelId": "CHAT_ID_GOES_HERE",
	"ChatMessageColor":
	{
		"Red": 0.34999999403953552,
		"Green": 0.40000000596046448,
		"Blue": 0.94999998807907104
	},
	"SML_ModVersion_DoNotChange": "2.1.0"
}
```

#### A note on vanilla clients

Every player joining a dedicated server that runs SML needs SML installed client-side (use the [Satisfactory Mod Manager](https://smm.ficsit.app) or [ficsit-cli](https://github.com/satisfactorymodding/ficsit-cli)). FICSIT.chat itself is **not** needed on clients thanks to `AcceptsAnyRemoteVersion`. Truly vanilla clients cannot join: the game refuses modded servers client-side, and even when that check is bypassed, vanilla clients get disconnected during the join because SML replicates objects they don't have classes for (verified experimentally).

## Contributing

To report bugs/crashes, or give suggestions, head over to the repository's [issues tab](https://github.com/Steveplays28/FicsitChat/issues).  

### TODO

- [ ] Add support for bridging multiple Telegram chats
- [ ] Update the in-game configuration screen's tooltips (they still reference Discord)

## Development

- Satisfactory version: `1.2`
- Satisfactory Mod Loader (SML) version: `3.12.0`

The mod uses Telegram's [Bot API](https://core.telegram.org/bots/api) over Unreal's built-in HTTP module, so there are no third-party libraries to set up.

### Building

Visit the [Satisfactory modding documentation](https://docs.ficsit.app/satisfactory-modding/latest/Development/index.html) for information on how to set up the project for your IDE.  

## License

This project is licensed under GPLv3, see [LICENSE](LICENSE).

Copyright (c) 2023-2026 Steveplays

## Attribution

Icon created by Drew (xXdrewbaccaXx).  

## Contact info

If you want to say hi, head over to my [Discord server](https://discord.gg/KbWxgGg).  
Patreon: [steveplays28](https://patreon.com/steveplays28)
