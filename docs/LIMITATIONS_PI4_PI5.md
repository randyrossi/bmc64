# Raspberry Pi 4 and 5 Limitations

What the Raspberry Pi 4 B, Pi 400, Pi 5, Pi 500 and Pi 500+ don't support
compared with the Pi Zero to Pi 3. See [PI4_PI5.md](PI4_PI5.md) for how
these boards work.

## Video output

- **HDMI only.** Composite and DPI (for example the VGA666 adapter) aren't
  supported, and their `machines.txt` entries aren't offered in the
  machine switch menu. (The Pi 400 has no composite socket, and the Pi 5
  has composite only on test pads.)
- **720p and 1080p.** The machine switch menu offers each machine at
  720p and 1080p (the PET at 1080p, as on the other Pis), at 50 Hz (PAL)
  or 60 Hz (NTSC), run at the machine's own
  frame rate (within CTA-861's 0.5 % pixel clock tolerance). There is no
  Resolution setting in the Video menu: the machine switch entry chooses
  it.
- **Custom HDMI modes on the Pi 4 only.** The Pi 4 and Pi 400 keep the
  `machines.txt` entries with `hdmi_timings` (such as `768x545@50.125Hz`),
  which the firmware sets up at boot. The Pi 5's firmware doesn't take
  custom modes, so the Pi 5 family doesn't offer them.
- **Pi 5: only the modes the display lists.** The Pi 5's firmware starts
  in the display's own mode; BMC64 then switches to the entry's 720p or
  1080p mode, at 50 or 60 Hz, only if the display lists it. If not, the
  display's own mode (and refresh rate) is kept, which the log reports.
  Many computer monitors don't list 720p or 1080p at 50 Hz.
- **The firmware's video settings in `config.txt` aren't applied**, since
  BMC64 drives the display itself: `overscan_*` and `scaling_kernel` (the
  Video menu's Scaling Interpolation chooses smooth or sharp scaling
  instead). `gpu_mem` has no effect.

## CRT shader

The shader is drawn by the 3D GPU, with crt-pi's default look. Its menu has
**Enable**, **Curvature** and **Reset**, and Scaling Interpolation smooths it.
These options aren't offered, and the shader settings for them are ignored:

- Curvature X and Curvature Y
- Scanlines, Multisample, Scanline Weight and Scanline Gap Brightness
- Mask and Mask Brightness
- Bloom
- Gamma, Input Gamma and Output Gamma
- Sharper

## Sound

- **No analogue sound on the Pi 400 or the Pi 5 family**, which have no
  headphone jack. Sound is over HDMI; the Pi 4 B can use its headphone jack
  (`audio_out=analog` in `cmdline.txt`).

## Emulation

- **The C64 is always VICE's cycle exact x64sc.** The faster x64 that the
  Pi 0 to 3 run isn't available. Its VICE settings are saved in the
  `[C64SC]` section of `vice.ini`, so C64 settings saved on a Pi 0 to 3
  card aren't used.
