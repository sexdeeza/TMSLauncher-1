# TMSLauncher
TW MapleStory old version launcher

## Feature
- Fix domain resolve error
- Skip ads window
- Remove locale check
- Redirect server ip and port
- Send connect key to server(option)
- Remove Anti-Cheat(NGS/HS/X3)
- Mount img file(option)
- Fix window mode error(BeforeBB)
- Customize game window title
- Multiple game windows
- Remove CRC

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

## How to use
### Debug
1. Set `IS_DEV` is true
2. Set `REMOTE_DIR` to MapleStory directory
3. Set options in `config.h`
4. Build solution with `Release x86` mode
5. Run it in Visual Studio will inject hook by remote
6. Recommend using [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview) see debug log
### Release
1. Set `IS_DEV` is false
2. Build solution with `Release x86` mode
3. Put `Launcher.exe` and `Hook.dll` into MapleStory directory
4. Run `Launcher.exe` to connect login server
### IMG
1. Recommend using [HaSuite](https://github.com/Arnuh/HaSuite) convert all wz files except `List.wz` to `Data` folder
2. Put `MoveImg.bat` into `Data` folder
3. Run the bat will quickly format folder name

## Thanks
- [Tools](https://github.com/Riremito/tools) shared by Riremito
- [RemoveAntiCheat_EasyVer](https://github.com/rage123450/EmuClient/blob/develop/EmuMain/EmuMain.cpp) shared by rage123450
- [CIL](https://github.com/MapleMyth/ClientImageLoader) shared by MapleMyth