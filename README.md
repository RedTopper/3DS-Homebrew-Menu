# The Homebrew Launcher
#### Differences
So far there has been only really minor changes:
- Launcher now shows less garbage on startup (not none, but less). A splash screen will show if menuhax_imagedisplay.bin is in the same directory as the .3dsx. It is generated using Homemenuhax found here https://github.com/yellows8/3ds_homemenuhax.
- Themes can include BGM.bin, a 44100Hz mono, signed 16 bit PCM wav file and will be loaded on startup.

#### Code credits
- https://github.com/clank201/SoundExample

That's it so far.


#### Presentation

This fork of smealum's homebrew launcher adds many features including a grid layout, customisable colour schemes, wallpapers and much much more. It is intended to be a drop-in replacement for the original launcher. Big thanks to smealum for the original hax and launcher, suloku for the encouragement and help fixing bugs, and the GBATemp community for all the support and encouragement.

#### Usage

To use the grid launcher as your homebrew menu, simply rename the 3dsx executable to boot.3dsx and place it at the root of your SD card.

Select the "?" icon in the top right corner of the launcher to view help pages. Press START in hbmenu to reboot your console into home menu. Use the D-PAD, CIRCLE-PAD or the touchscreen to select an application, and press A or touch it again to start it.

#### Please see the README.md from the original launcher for more technical details