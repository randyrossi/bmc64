Another emulation choice for your Pi - BMC64

Hi all.

I've been working on a bare metal port of VICE (x64) for the Raspberry Pi called BMC64. You get true 50hz/60hz smooth scrolling (no horizontal tearing!), low input to audio/video latency and better audio/video synchronization. I encourage you to give it a try and compare the performance to Combian, PiLizard and Chameleon.

One of the first things I noticed when running the C64 emulators on my Pi is the jitteriness or horizontal tearing that occurs when graphics are scrolled.  With BMC64, frames are timed to the vertical blanking signal so you never get tearing or jitteriness. The quality of video 'feels' much more like the original machine.

For a summary of the issue/complaint, see this post: https://www.lemon64.com/forum/viewtopic.php?t=65259&start=0

Some platforms VICE is compiled for (like Win32, AmigaOS) have had this feature for a while.  Hoxs64 (not VICE, for Windows only) has smooth video. However, it looks like the Linux based distributions for the Pi don't which is why you get the tearing effect when graphics are scrolled left/right.  PiLizard is a bit better but is still noticeably jittery.  Chameleon performed the best but still gets choppy. (These are all VICE so the only difference I think are settings.) 

In addition to the horizontal tearing, there's also the issue of input latency.  I watched the 8-bit guy and _ kieferkn/i:/stern _ on YouTube testing the latency between input and audio/video response on the C64 mini.  The 8-bit guy measured audio latency (from key press to bleep) at around 360ms.  Video latency was about 130ms.  _ kieferkn/i:/stern _ measured a frame latency of 6-7 frames @50 hz which would put it in agreement with the 8-bit guy's measurements.  (I think the mini was updated since then but according to _ kieferkn/i:/stern _, this did not improve the latency).

I noticed that all three Pi distributions that are/or include a Commodore 64 emulator (Combian, PiLizard, Chameleon) have audio/video sync issues as well. If you run the 8-Bit guy's delay test (delaytst.crt), you can notice the latency introduced by the emulator.

I believe the latency in BMC64 is down to about 50ms.  However, I'm working on actually measuring it. That would put it around 2-3 frames.  You can try running the 8-bit guy's delay test (included in github link below) and compare for yourself.  Also compare BMC64 to the real thing!

I *think* I have input to video latency down to one frame which would put in on par with the Ultimate64. See the chart in the README for measurements from other emulators.

Boot time is down to 6 seconds (compared to 13 for Combian and a whopping 52 for PiLizard).  Also, since there is no O/S that could be writing to your sdcard at any time, I believe there is no reason to go through a shutdown sequence like you do on Combian/PiLizard.  People who know hardware better can correct me if I'm wrong on this but you should be able to avoid having to use on of those 'power blocks' that signals a shutdown when you press power (rather than actually cutting power).  With BCM64, you should be able to just power off the device without worry of sdcard corruption.

What do you mean by bare metal?

  There is no operating system! An O/S like Linux adds layers of abstraction between the application (VICE) and the privileged kernel code that has access to hardware.  That adds latency in processing events.  User space programs can't get notified directly from interrupts, for example.  The interrupt has to be serviced by the kernel, into user space, into your program (at the mercy of the scheduler) before they can get processed.  For sound production, the user space program must fill buffers to deliver to a sound driver in the kernel before it can pass it on to the hardware.  With Combian/PiLizard and Chameleon installations, you are effectively installing a full blown linux distribution (1.5Gb image files!).

   With BMC64, VICE -IS- the kernel and has direct access to the hardware.  As soon as you power on the device, it goes straight into the emulator.  The entire kernel is ~2MB.

Why use this?

  1. fast boot time (~6 seconds as opposed to Combian's ~13, PiLizard ~52)
  2. true 50hz/60hz smooth scrolling (frames timed to vertical blank) so no horizontal tearing!
  3. just power off the device (rather than initiating a shutdown)
  4. high compatibility (thanks to VICE)
  5. low input to audio/video latency

Why not use this?

  1. sdcard support is limited and read-only (for now)
  2. sub-directories are not supported (for now)
  3. not every VICE feature/option is available (i.e. snapshots)
  4. usb gamepad support is limited

  The emulator 'feels' much more like a real machine thanks to the vsync hooks (especially on a CRT).  My first test case was GhostBusters since it has smooth scrolling text at the bottom and I notice right away if it's not true 50hz/60hz.  Also, the bouncing ball helped show audio/sync issues.  But just about any game/demo that has scrolling will look much better.

Gamepads

  Generic USB gamepads are (mostly) supported.  Circle's support for USB gamepads is limited. I will address issues as they arise.

Keyboard

  USB keyboards should just work. The keyboard mapping is positional.  Sorry, no way to change it yet.  Use Escape for RUN/STOP and PageUp for RESTORE.  This layout is compatible with the keyrah.

Menu

  F12 brings up the menu.  The menu is minimal. I only included the bare essentialls for attaching disks, tapes and cartridges.

Where do I get it?

  Source is available at: https://github.com/randyrossi/bmc64

  Precompiled available images at: http://accentual.com/bmc64

I know this isn't the first attempt at a bare metal C64 emulator for the Pi.  However, I think this might be the most complete and it was a very fun and interesting project to take on.

Try it out and let me know your experiences.  Thanks!
