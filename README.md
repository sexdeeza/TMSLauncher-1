# TMSLauncher
TW MapleStory v113-v194 Launcher

## Feature
- Fix domain: Unable to show ads window if not set
- Skip ads window: Quick start
- Remove locale check: Still displays unknown text
- Redirect: Modify connection ip and port or resolve remote domain
- Send connection key to server(optional)：Avoid using cmd bat to connect login server
- XOR all packets from server/client(optional): The header and payload of packet will be xor
- Remove Security Client(HS/X3): Let us successfully enter the game without interruption 
- Mount IMG file(optional): Check if base.wz exists? wz default mode or img mode
- Mount Multiple.wz files(optional): Check if base.wz and multiple.wz exists? wz multiple mode or wz default mode 
- Fix window mode error: Allow older clients(BeforeBB) to run at the recent displays
- Customize game window title and allow multiple game windows
- Remove CRC: Prevent client crashes after 30 seconds or upon entering field

## Test version
The supported version range is v113-v194  
Please tell me if you find any errors during login to these versions
### Anti-Cheat
- Versions in which HS were removed: v113-v183
- Versions in which BC counldn't be removed: v160-v165
- Versions in which X3 were removed: v184-v194
### CRC
- Versions in which CWvsApp::Run CRC were removed: v113-v182
- Versions in which CWvsContext::OnEnterField CRC were removed: v183-v194
### IMG
1. Supported version range is v113-v194
2. Using `HaRepacker` to convert all wz files except `List.wz` to `Data` folder
3. Put `MoveImg.bat` into `Data` folder
4. Run the bat will quickly format folder name
5. Remove `Base.wz` file from the client root directory to another location
### MultipleWZ
1. Supported version range is v113-v158
2. Using `HaRepacker` to open list.wz
3. Separate other WZ files based on the order defined in List.wz
4. The wz file name format is as follows `Character.wz` `Character2.wz` ... `Character99.wz`
5. There is no need to add wz directory in `Base.wz`
### DamageSkin
1. Download [Effect2.wz](https://mega.nz/file/2kMUUJIS#_dfyDkpx9TAGC8lU8Wtj1MqK1cZL8RIQc5s6KtOybn8) and place it in the client root directory
2. Using `HaRepacker` to set maple version of `Effect2.wz`
3. Set the `DamageSkinID` in `Config.h`
## How to use
### Debug
1. Set `IS_DEV` is true
2. Set `REMOTE_DIR` to MapleStory directory
3. Set options in `Config.h`
4. Build solution with `Release x86` mode
5. Run it in Visual Studio will inject hook by remote
6. Recommend using [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview) to see debug log

### Release
1. Set `IS_DEV` is false
2. Build solution with `Release x86` mode
3. Put `Launcher.exe` and `Hook.dll` into MapleStory directory
4. Run `Launcher.exe` to connect login server

## Credits
- [EmuMain](https://github.com/rage123450/EmuClient/blob/develop/EmuMain/EmuMain.cpp) shared by rage123450
- [Tools](https://github.com/Riremito/tools) shared by Riremito
- [kinoko_client](https://github.com/iw2d/kinoko_client) shared by iw2d
- [HaRepacker](https://github.com/lastbattle/Harepacker-resurrected) shared by lastbattle