# Optimising BMC64 <!-- omit from toc -->

[Back to the User Guide](USER_GUIDE.md)

Settings and options that affect BMC64's performance, and how to recover headroom when it runs slowly or the sound or
picture glitches. Most of this information was gathered while profiling a Raspberry Pi Zero and Zero W, where these limits are reached first, 
but the same optimisations can be used on any model of Raspberry Pi.

## Contents <!-- omit from toc -->

- [Raspberry Pi Zero and Zero W: Recommended Settings](#raspberry-pi-zero-and-zero-w-recommended-settings)
  - [Drives: the biggest cost](#drives-the-biggest-cost)
  - [Networking and the Web UI](#networking-and-the-web-ui)
  - [Sound, video and other settings](#sound-video-and-other-settings)
  - [Disk writes and shutting down](#disk-writes-and-shutting-down)
  - [If the sound or picture glitches](#if-the-sound-or-picture-glitches)

## Raspberry Pi Zero and Zero W: Recommended Settings

This section is for the single-core Raspberry Pi Zero and Zero W (the `kernel.img` build, shown as "Lite" on the About
screen). The emulator, the USB stack and networking all share that one core, so this board has the least processing
headroom of any supported model. Typical programs run well within it, but demanding software can exceed the available
frame time, which shows up as audio drop-outs or missed frames. Headroom is recovered by disabling the features you do
not need, as described below.

The measurements were made on a Pi Zero W, but this section is also a general reference for optimising any BMC64 build
for performance. Emulated drives, networking, logging and display effects add load on every board. The Pi Zero reaches
its limits first, so it shows the cost of each option most clearly. Faster boards have more headroom to absorb the same
load, but these are the options to review when tuning them. 

The advice below comes from measurements on a Pi Zero W at 1000 MHz with no overclock, from previous notes in this
guide, and from limits built into the menus. The measurements are in
[perf test RESULTS.md](../tools/perftest/RESULTS.md). It was run using one demanding game, so treat the numbers as a guide,
not a guarantee. The Zero 2 W is a different, quad-core board and was not measured.

**The short version**

* Use **one emulated drive**, or two if you need them. Every extra emulated drive costs CPU even when nothing is using it.
* Set unused drive units to **None**.
* Leave **True Drive Emulation on**. Many disks do not load or run without it.
* Keep the **SID Engine on ReSID**, the default.
* Turn **networking** off if you do not use it, and let Wi-Fi join before you start a demanding game.
* Leave **Logging off**, especially File logging.
* Turn the **CRT shader off**, or use fewer of its options.
* Use a **powered OTG USB hub** if you are connecting multiple USB devices.
* If it stutters, see [If the sound or picture glitches](#if-the-sound-or-picture-glitches).

The following generally doesn't have a large impact on performance

* **Networking and the Web UI** while idle.
* **Attached IDE64 image** while idle.
* **The REU** while idle.


### Drives: the biggest cost

Each emulated drive runs its own CPU alongside the C64 whenever True Drive Emulation is on for it, even while the drive
is idle. On a Pi Zero W, in a demanding game with drive 8 always present, the busiest part of the game needed this much
of each 20 ms frame (the rest is spare time):

| Drives configured | Emulation time per frame | Result |
| --- | --- | --- |
| 8 only | about 11 ms | Comfortable |
| 8 + 9 (1541-II) | about 13 ms | Comfortable |
| 8, 9 + 10 (CMD HD) | about 14 ms | Workable, little margin |
| 8 to 11 (+ FD-4000) | about 17 ms | On the edge, can glitch |

Each extra drive added roughly 1 to 2 ms per frame, and the FD-4000 the most. Once a game needs about 17 to 18 ms per
frame the audio buffer starts to run dry and you hear drop-outs. Four drives put this game right at that point.

Drives 9 to 11 are **None** by default, so this only matters if you have added them. To remove one, use
**Drives -> Drive 9 (or 10, 11) -> Change Model... -> None**.

Do not turn off **True Drive Emulation** to save CPU. It stops the drive's CPU, but many disks do not load or run without
it.

**Advanced: idle drives.** VICE can skip the time a drive spends waiting in its idle loop. BMC64 does not show this
setting in its menus, but you can set it in `vice.ini` on the SD card, in the section for your machine (for example
`[C64]`), for the extra units only:

```
Drive9IdleMethod=2
Drive10IdleMethod=2
Drive11IdleMethod=2
```

In testing with three extra drives this saved about 1.4 ms per frame at rest and nothing while the game was busy, so it
is a small help and not a cure. It does not help the CMD HD, because VICE has no idle skip for that drive. Leave drive 8
as it is.

### Networking and the Web UI

* The Pi Zero has no Wi-Fi and no Ethernet. The Pi Zero W has Wi-Fi. 
* A Wi-Fi connection that has joined, is active, and running the WebUI has minimal performance impact.
* When Wi-Fi joins, the emulator stalls briefly, about 10 seconds after boot, once or twice. You may hear a short drop-out if a game is already running, so
  wait for the connection before starting anything demanding.
* A network that keeps failing to connect and continues to retry (e.g. A wrong password, a weak signal or DHCP retrying) can cause slow downs. 
* Turn networking **off** if you don't need it.

### Sound, video and other settings

* SID Engine: keep the default ReSID. On the Pi Zero it uses the 'Fast' sampling method, the only one available (see
  [Sound](USER_GUIDE.md#sound)). 
* The [CRT Filter](USER_GUIDE.md#crt-filter) can stutter: Curvature, MultiSample and Filter together may be too much,
  two of the three may work, and the shader may crash at higher resolutions such as 1600x900 at 60 Hz. If in doubt,
  turn the shader off first. See [CRT Filter + Performance](USER_GUIDE.md#crt-filter--performance). 
* Leave logging **off**. UART logging is slow (115200 baud), and a burst of messages can stall a frame for tens of
  milliseconds. File logging writes to the SD card. Use either only while diagnosing a problem. See
  [Logging](USER_GUIDE.md#logging).
* Use a powered OTG USB hub. Some devices can cause brownouts and shut the Pi Zero down.

### Disk writes and shutting down

BMC64 has no shutdown sequence. With True Drive Emulation the drive can hold its last written track, usually the
directory, in memory, so turning off he Pi can cause data loss. The **Prefs -> Flush disk writes** setting can be 
configured to when data is written to the SD card.

* **On detach**: only when the disk is detached or the drive moves to another track. 
* **On write** (default): shortly after the drive stops writing. Reading a disk is unaffected.
* **On write (logged)**: as **On write**, and logs each flush time. Set **Logging** to UART or File to see it.

Note: This setting does not apply with True Drive Emulation off.

BMC64 has been configured to write to the SD Card shortly after disk writes occur (**On write**). If you have glitches 
during disk write activity, try **On detach**. 

### If the sound or picture glitches

The usual signs are audio drop-outs or judder, and, with logging on, `Sound: Warning - Buffer drained` messages. Try
these in order, cheapest first:

1. Set drives 9 to 11 to **None** if you are not using them. This is the biggest saving.
2. Turn networking off (**Network -> Network Device -> Off**), especially if Wi-Fi is reconnecting or failing.
3. Turn the CRT shader off, or use fewer of its options.
4. Make sure **Logging** is **Off**.
5. If you need extra drives, try the idle drive setting above.
6. If it only glitches just after saving to a disk, set **Prefs -> Flush disk writes** to **On detach**.

If it still glitches, the game or demo may simply be too demanding for a Pi Zero. A Pi 2 or Pi 3 has more processing
power and more cores.
