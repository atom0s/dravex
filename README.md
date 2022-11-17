<div align="center">
    <img width="200" src="https://github.com/atom0s/dravex/raw/main/repo/dravex.png" alt="dravex">
    </br>
</div>

<div align="center">
    <a href="https://discord.gg/UmXNvjq"><img src="https://img.shields.io/discord/704822642846466096.svg?style=for-the-badge" alt="Discord server" /></a>
    <a href="LICENSE"><img src="https://img.shields.io/badge/License-AGPL_v3-blue?style=for-the-badge" alt="license" /></a>
    <br/>
</div>

# dravex

**dravex**, short for `Dungeon Runners Asset Viewer and Extractor`, is an asset viewer and extractor tool for the now-discontinued MMORPG, Dungeon Runners. **dravex** allows you to open and parse the games `game.pki` and `game.pkg` files which contains the games various data files such as textures, fonts, objects, map information, configurations, and more.

**dravex** supports the following client versions:

  - **v118**
  - **v666**

_**Note:** dravex supports these two main client versions, however, support for any other version is most likely valid and works as-is without needing to edit anything in dravex. If you have a client version that is not listed here, please contact atom0s directly to have it validated and/or have support added if need be!_

## Donations & Sponsorships

**dravex** is released completely free of charge. You can say thanks by donating or via sponsorships.

  * **GitHub Sponsor:** https://github.com/sponsors/atom0s
  * **Patreon Sponsor:** https://patreon.com/atom0s
  * **PayPal Donation:** https://www.paypal.me/atom0s

## Credits

**dravex** is coded in C++ (making use of newer C++17/20 features) by @atom0s

The following tools are used to make **dravex**:

  * **Visual Studio Code (Insiders)**: https://code.visualstudio.com/insiders/
    * **C/C++ Extension**
    * **CMake Extension**
    * **CMake Tools Extension**
  * **CMake**: https://cmake.org/
  * **vcpkg**: https://vcpkg.io/en/index.html

The following third-party libraries are used to make **dravex**:

  * **Direct3D9**: https://learn.microsoft.com/en-us/windows/win32/direct3d9/dx9-graphics
  * **ImGui**: https://github.com/ocornut/imgui
    * **imgui_colortexteditor**: https://github.com/BalazsJako/ImGuiColorTextEdit
    * **imgui_fontawesome**: https://github.com/juliettef/IconFontCppHeaders
    * **imgui_memoryeditor**: https://github.com/ocornut/imgui_club
  * **zlib**: https://www.zlib.net/
  * **miniaudio**: https://miniaud.io/index.html
  * **stb_vorbis**: https://github.com/nothings/stb

Credits to the following for additional resources:

  * **Agave Font**: https://github.com/blobject/agave
  * **FontAwesome**: https://fontawesome.com/ _(For their icon fonts.)_
  * **max.icons**: Icon used for the project.

## Building dravex

**dravex** is built using **VSCode**, **CMake** and **vcpkg** as the main tooling. For third-party libraries that are maintained outside of this repository, **vcpkg** is generally used. This includes things such as `zlib`.

You will need to install the various requirements for this project with the following commands:

```
vcpkg install zlib
```

Because of issues with some versions of Windows, Powershell and vcpkg, you may need to adjust/tweak paths for your system.

You can find overrides for working with zlib and vcpkg inside of the `CMakeLists.txt` file near the top of the file:

```
# Define the toolchain to use vcpkg..
set(CMAKE_TOOLCHAIN_FILE "Z:/SourceCode/deps/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
set(Z_VCPKG_BUILTIN_POWERSHELL_PATH "C:/Program Files/Powershell/7/pwsh.exe")
```

_This is currently setup to match my personal build environment and system setup due to said bugs._

Once you have all requirements and such installed and configured, you can use the VSCode CMake toolbar at the bottom of the window to select the desired build, presets, and targets to build **dravex**.

## License

**dravex** is licensed under [GNU AGPL v3](https://github.com/atom0s/dravex/blob/main/LICENSE)

Please be sure you understand the license before making use of **dravex**. This is `AGPL`, not standard `GPL`.

## Legal

**dravex** is developed solely for educational purposes, with the intent of research into data storage, compression, and MMORPG technologies.

We (contributors) do not claim ownership of any copyright content related to, or associated with, `Dungeon Runners`.

```
© 2008 NC Interactive, Inc. All rights reserved.
```

Please note; the reverse engineering done by this repository and its contributors is entirely 'clean room'. We **DO NOT** have or use any leaked source code or other unpublished material. By contributing to this repository, you agree to the following:

  - You are not employeed by NCSoft, NC Interactive, any partner/subsidiary, or have been previously, in any capacity.
  - You do not and have never had any leaked material related to `Dungeon Runners` in any manner.
  - You do not and have never referenced any leaked or otherwise unreleased material related to `Dungeon Runners` in any manner.

We **DO NOT** claim ownership of any material or information gathered through the means of reverse engineering the client and its files for this purpose.

## Screenshots

Below are some screenshots of **dravex**:

![dravex1](/repo/dravex1.png?raw=true "Main Screen")

![dravex1](/repo/dravex2.png?raw=true "Viewing Texture Asset")

![dravex1](/repo/dravex3.png?raw=true "Viewing Font Asset")

![dravex1](/repo/dravex4.png?raw=true "Viewing Sound Asset")

![dravex1](/repo/dravex5.png?raw=true "Viewing Shader Asset")
