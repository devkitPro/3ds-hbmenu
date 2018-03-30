# The Homebrew Launcher

#### Presentation

The Homebrew Launcher (hbmenu for short) is a fairly simple (and beautiful) menu that lists homebrew applications in the 3DSX format and lets you run them. It can be used with the following entrypoints:

- ninjhax 1.x (the original 3DS homebrew exploit, originally released in November 2014)
- hax 2.x (the followup to ninjhax, originally released in July 2015)
- Rosalina (part of Luma3DS 8.0+ custom firmware, originally released in June 2017)

#### Usage

To install hbmenu, simply copy boot.3dsx to the root of your SD card.

Use the D-Pad, Circle Pad or the touchscreen to select an application, and press A or touch it again to start it. Use the C-Stick alternatively on New 3DS to scroll the list of applications.

On certain entrypoints (ninjhax 1.x and hax 2.x), it is not possible to go back to the 3DS HOME menu using the HOME button. As an alternative, you can press the START button where you can reboot your console or (in the case of hax 2.x) relaunch HOME menu.

hbmenu starts in the sdmc:/3ds/ directory for applications and it will look for 3dsx files inside it. You can navigate the directory tree and open/browse folders as you would expect. Old style application bundle folders are also detected, however this functionality may be removed in the future.

Here is an example directory structure that hbmenu will have no trouble recognizing:

- sdmc:/
  - 3ds/
    - games/
	  - Hermes.3dsx
      - cubemadness.3dsx
    - 3dscraft.3dsx
    - blargSNES.3dsx
    - gameyob.3dsx
    - 3dnes.3dsx
    - ftpd.3dsx
    - Themely.3dsx

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

3dslink uses zlib for compression and tinyxml2 for XML parsing. These libraries are provided by devkitPro through the portlibs mechanism. In order to install them, use the following command:

```shell
    pacman -S 3ds-zlib 3ds-tinyxml2
```

(Note that `dkp-pacman` is used instead on systems that do not already have pacman, such as macOS or non-Arch-based Linux distros)

Binaries of hbmenu can be downloaded from the [Releases](https://github.com/fincs/new-hbmenu/releases) page.

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
- gruetzkopf, TuxSH, AuroraWright, Soph1a7, SentientTurtle, Yami-chan, d3m3vilurr, daedreth, JixunMoe: translations
