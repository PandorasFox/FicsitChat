# FICSIT.chat

![icon](assets/ficsit_chat_icon_128x128.png)

Satisfactory to Discord chat bridge mod with lots of configurability.

<details open>
<summary>Image gallery</summary>

<br>

![promo banner, feat. DJMalachite, my FICSIT.chat bot, and shenanigans](assets/ficsit_chat_promo_banner.png)

</details>

## Usage

1. Create a Discord bot on Discord's [developer portal](https://discord.com/developers/applications) (make sure to copy the bot token as you will need it in the next few steps - it only appears once and you have to revoke the old one to generate a new one)
    - Example application name: `FicsitChat`
    - Example description: `Satisfactory to Discord chat bridge.`
    - Example bot username: `FICSIT.chat`
    - Example icon: [`assets\ficsit_chat_icon_512x512.png`](assets\ficsit_chat_icon_512x512.png)
2. Enable message content intent
3. Invite the bot to your server
4. Enter the bot token into FICSIT.chat's [configuration](#configuration)
5. Enable [developer mode in your Discord client](https://discord.com/developers/docs/activities/building-an-activity#step-0-enable-developer-mode)
6. Copy the ID of the channel (`Hover over channel->Right click->Copy Channel ID`) you want the bot to use to post Satisfactory messages and send Discord messages back to Satisfactory.
7. Enter the channel ID into FICSIT.chat's configuration (see next section)
8. Modify the other options in FICSIT.chat's configuration to your heart's content

Have fun!

### Configuration

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
	"ChannelId": "CHANNEL_ID_GOES_HERE",
	"ChatMessageColor":
	{
		"Red": 0.34999999403953552,
		"Green": 0.40000000596046448,
		"Blue": 0.94999998807907104
	},
	"SML_ModVersion_DoNotChange": "1.0.0"
}
```

## Contributing

To report bugs/crashes, or give suggestions, head over to the repository's [issues tab](https://github.com/Steveplays28/FicsitChat/issues).  

### Know bugs

If you add your discord bot to multiple discord servers, it's gonna send messages to channels with the same id if they exist.  

### TODO

- [ ] Add support for multiple discord servers (adding server id and channel id pair to configuration)
- [ ] Build target and DPP for Linux

## Development

- Satisfactory version: `1.1`
- Satisfactory Mod Loader (SML) version: `3.11.3`

### Prerequisites

This mod requires the **D++ Discord library** to be set up before building. The current version used is **10.1.4**.

1. Download the Visual Studio 2022 version for Windows from the [website](https://dl.dpp.dev/latest/win64-release-vs2022)
2. Extract the contents to: `Source/ThirdParty/DPPLibrary/`
   - The final structure should look like:
     ```
     Source/ThirdParty/DPPLibrary/
     ├── DPPLibrary.Build.cs
     └── libdpp-10.1.4-win64/      (or your version)
         ├── bin/
         ├── include/
         └── lib/
     ```

> **Note:** The DPP library files are not included in this repository due to their size. Each developer must download and extract them locally.

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
