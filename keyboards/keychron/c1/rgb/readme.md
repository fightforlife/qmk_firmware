# Keychron C1 RGB Hot-swappable

![Keychron C1](https://i.imgur.com/oB9h56v.png)

A customizable TKL keyboard.

* Keyboard Maintainer(s): [SonixQMK](https://github.com/SonixQMK), [IsaacDynamo](https://github.com/IsaacDynamo), [noldevin](https://github.com/noldevin), [vjdato21](https://github.com/vjdato21), [NetUserGet](https://github.com/NetUserGet)
* Hardware Supported: Keychron C1 RGB Hot-swappable
* Hardware Availability: [Keychron](https://www.keychron.com/products/keychron-c1-wired-mechanical-keyboard?variant=32321246986329)

Make example for this keyboard (after setting up your build environment):

    make keychron/c1/rgb:default
Flashing example for this keyboard:
1. Download and run [Sonix Flasher](https://github.com/SonixQMK/sonix-flasher/releases/latest).
2. Click `Reboot to Bootloader [HFD]`
3. Select `SN32F24X` under 'Device'. And select `0x00` as the qmk offset.
4. Click `Flash QMK...` and open the compiled firmware.

* * *
## Keymapping
#### Windows Mode
<details>

Without Fn | With Fn
---------- | -------
F1 | Decrease PC Brightness
F2 | Increase PC Brightness
F3 | Task View
F4 | File Explorer
F5 | Decrease KB Brightness
F6 | Increase KB Brightness
F7 | Previous Track
F8 | Play/Pause Track
F9 | Next Track
F10 | Mute
F11 | Decrease Volume
F12 | Increase Volume
Print Screen | Snip & Sketch
Change RGB | Toggle RGB
Home | Increase RGB Saturation
End | Decrease RGB Saturation
Page Up | Increase RGB Hue
Page Down | Decrease RGB Hue
↑ | Increase RGB Hue
← | Decrease RGB Saturation
↓ | Decrease RGB Hue
→ | Increase RGB Saturation

</details>

#### Mac Mode
<details>

Without Fn | With Fn
---------- | -------
F1 | Decrease PC Brightness
F2 | Increase PC Brightness
F3 | Mission Control
F4 | Finder
F5 | Decrease KB Brightness
F6 | Increase KB Brightness
F7 | Previous Track
F8 | Play/Pause Track
F9 | Next Track
F10 | Mute
F11 | Decrease Volume
F12 | Increase Volume
Take a screenshot (whole screen) | Take a screenshot (specific area)
Siri (hold down) | None
Change RGB | Toggle RGB
Home | Increase RGB Saturation
End | Decrease RGB Saturation
Page Up | Increase RGB Hue
Page Down | Decrease RGB Hue
↑ | Increase RGB Hue
← | Decrease RGB Saturation
↓ | Decrease RGB Hue
→ | Increase RGB Saturation

</details>

In an occassion that the keyboard does not send keystrokes to your PC, press `FN + B`. This will restart your keyboard and should resolve the problem.
* * *
## Bootloader
To enter the bootloader, you can use any of the following methods:
- Press `FN + ESC` if the keyboard is already powered on.
- Short the boot pads below the space bar, then plug in your keyboard.
* * *
For more information and more detailed flashing instructions, please visit [SonixQMK Docs](https://sonixqmk.github.io/qmk_docs/newbs_getting_started)

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).
