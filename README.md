# The Homebrew Launcher

#### Presentation

The Homebrew Launcher (hbmenu for short) is the main menu used to list and launch homebrew applications. It is essentially a graphical shell around an existing homebrew loading mechanism. The following entrypoints are supported:

- [Luma3DS Rosalina](https://github.com/LumaTeam/Luma3DS) **(recommended)**: Works on all system versions from 4.0 onwards; it provides unrestricted access to 3DS system resources as well as nice extra features such as remote debugging (GDB). For more information read the [Rosalina documentation](https://github.com/LumaTeam/Luma3DS/wiki/Rosalina).
- [Legacy \*hax 2.x](https://smealum.github.io/3ds/): This is a now-obsolete homebrew loading system that only provides limited access to 3DS system resources, as it only attacks a low privilege level. Support for this entrypoint is deprecated and may be removed in a future release.

3DS homebrew is built and distributed as executables with the `.3dsx` extension. Note that you may encounter files with the `.cia` extension - these are **not** homebrew executables that can be loaded using hbmenu.

#### Usage

To install hbmenu, simply copy `boot.3dsx` to the root of your SD card. If you are using a recent version of [Luma3DS](https://github.com/LumaTeam/Luma3DS) you probably already have a copy of hbmenu installed, as it comes bundled with it.

Use the D-Pad, Circle Pad or the touchscreen to select an application, and press A or touch it again to start it. Use the C-Stick alternatively on New 3DS to scroll the list of applications.

hbmenu supports starring applications, so that they are shown at the beginning of the list. The SELECT button stars/unstars the currently selected homebrew application.

On \*hax 2.x, it is not possible to go back to the 3DS HOME menu using the HOME button. As an alternative, you can press the START button where you can reboot your console or relaunch HOME menu.

hbmenu starts in the sdmc:/3ds/ directory for applications and it will look for 3dsx files inside it. You can navigate the directory tree and open/browse folders as you would expect. Additionally, folders containing a 3dsx file with the same name as the folder (or alternatively `boot.3dsx`) will be detected as an application bundle folder, and it will be presented as a single icon that can directly launch the application.

Here is an example directory structure that hbmenu will have no trouble recognizing:

- sdmc:/
  - 3ds/
    - games/
	    - Hermes.3dsx
      - cubemadness.3dsx
    - Checkpoint/ *(this folder will be detected as an application bundle)*
      - Checkpoint.3dsx
      - ...
    - ftpd.3dsx
    - mgba.3dsx
    - 3dscraft.3dsx
    - blargSNES.3dsx
    - gameyob.3dsx
    - 3dnes.3dsx

If hbmenu does not find an icon file (either embedded in the executable or provided separately) to associate with a given 3dsx, it will display a default icon and the path to the executable as a fallback.

hbmenu also allows you to create "shortcuts" which are xml files containing a path to a 3dsx file and optional arguments to pass to the .3dsx. This file can also include a path to icon data as well as name, description and author text using tags as follows:

    <shortcut>
        <executable>The path to the 3dsx file goes here.</executable>
        <icon>path to smdh icon data</icon>
        <arg>Place arguments to be passed to 3dsx here.</arg>
        <name>Name to display</name>
        <description>Description of homebrew app</description>
        <author>Name of the author</author>
    </shortcut>

Arguments are space or tab separated but can use single or double quotes to contain whitespace.

Name, description and author will be read from the .3dsx if it has embedded SMDH data or from the supplied icon path. The fields in the xml file will then override their respective entries.

You should not hotswap the SD card while hbmenu is running since it compromises the 3DS OS's stability amongst other things. It is recommended that you instead use a file transfer homebrew application such as ftpd to transfer files without rebooting.

#### Technical notes

hbmenu does all its rendering in hardware thanks to the [citro3d](https://github.com/fincs/citro3d) library. The 3DS system font is also used to render all text.

hbmenu uses some funky mechanisms to launch 3dsx files. If you're interested in launching 3dsx files from your own application, you should look here; although these mechanisms may change in the future.

#### Netloader

hbmenu contains support for the 3dslink protocol, which allows you to remotely load applications.
Press Y to activate as usual then run `3dslink <3dsxfile>` if your network can cope with UDP broadcast messages.
If 3dslink says 3DS not found then you can use `-a <ip address>` to tell it where to send the file.

All the other arguments you give 3dslink will be passed as arguments to the launched 3dsx file. You can also specify argv[0] with `-0 <argument>` which is useful for
setting the current working directory if you already have data files in a particular place, i.e. `3dslink myfile.3dsx -0 sdmc:/3ds/mydata/`

3dslink is provided with devkitARM or you can download binaries from [WinterMute's website](http://davejmurphy.com/3dslink/).

#### Building

hbmenu uses zlib for compression and tinyxml2 for XML parsing. These libraries are provided by devkitPro through the portlibs mechanism. In order to install them, use the following command:

```shell
    pacman -S 3ds-zlib 3ds-tinyxml2 3ds-libconfig
```

(Note that `dkp-pacman` is used instead on systems that do not distribute pacman, such as macOS or Debian-based Linux distros)

Binaries of hbmenu can be downloaded from the [Releases](https://github.com/fincs/new-hbmenu/releases) page.

#### File Associations

This is a feature backported from [nx-hbmenu](https://switchbrew.org/wiki/Homebrew_Menu#File_Associations). However, there is one notable difference: icons must be a 48x48 t3x-generated file with GPU_RGB565 as its color format.

#### Contributing

hbmenu is looking for contributors! We're making this repository public so that you, the community, can make hbmenu into the menu of your dreams. Or show you how to make your own, better menu! Of course we'd rather you improved hbmenu rather than went off and started fragmenting the userbase, but any contributions to the homebrew scene are welcome. Feel free to use code from hbmenu for your own projects, so long as you give credit to its original authors.

#### Credits

- smea: code & original hbmenu version
- fincs: code & rewrite
- GEMISIS: code
- mtheall: code
- WinterMute: netloader code
- Fluto: graphics
- Arkhandar: graphics
- dotjasp: graphics (regionfree icon)
- gruetzkopf, TuxSH, AuroraWright, Soph1a7, SentientTurtle, Yami-chan, d3m3vilurr, daedreth, JixunMoe, yy-codes, MCPE-PC: translations
