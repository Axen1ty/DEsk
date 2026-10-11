**English** | [日本語](RELEASE-NOTES.ja.md)

# DEsk v0.1.1 (BETA)

## What changed since v0.1

- **Fixed: opening the calendar or the memo pad stopped the desk** on
  screens drawn by the GPU. On a GPU screen the desk read the size of a
  picture it had already handed over, and laid it for the whole screen.
- **Simpler install**: one line per system, `sudo sh scripts/install_debian.sh`
  (or `install_ubuntu.sh`, `install_arch.sh`), at the top of the README. Each
  installer now checks the copy against `SHA256SUMS` itself.

To update, unpack the new release and run the same line again; your
settings are kept.

## About DEsk

DEsk is a Wayland desktop for Linux in realistic
skeuomorphism -- the kind iOS 6 had -- in which the desktop is a desk:
windows are sheets of paper, the dock is a drawer, programs are filed in
binders on a bookcase, notifications arrive in a post box, and one desk lamp
lights a ray-traced room.

## What is in it

- **A whole desktop**, not only a window manager: a Wayland compositor
  (wlroots 0.18) with its own notification daemon, system tray, media
  controls, screen lock, idle and screen-off handling, display arrangement
  memory, and a session that starts the polkit agent, keyring, input method
  and network applet for you.
- **Windows on paper**, folded away, opened out to the desk, crumpled and
  thrown when closed. Folded windows lie on the desk under the others, or go
  into the drawer.
- **The drawer** (the dock), **the bookcase** with eight binders and an ALL
  APPS book you can search, **the Control Panel** (meters, Wi-Fi and
  Bluetooth levers, volume, brightness, the tray), **the post box**, a
  **desk calendar** with sticky notes, a **memo pad** and a **clock** with a
  media player page.
- **The front door** as the screen lock, in the real time of day and,
  optionally, the real weather.
- **Shell Settings**, which also opens the free programs DEsk borrows for the
  rest of the machine, each row naming the program it uses.
- **Seven languages**: English, Japanese, Chinese (simplified), Korean,
  Spanish, French and German, with calendars printed the way each country
  prints them and Chinese and Korean set in their own cut of the CJK fonts.
- **More than one screen**: the desk on one, a plain desk to put windows on
  on the others.
- **Five quality levels**, from Ultra down to a 2D Fallback that DEsk picks by
  itself when the screen is drawn on the CPU (no GPU, or a virtual machine
  without 3D).
- **Installers** for Debian 13, Ubuntu 24.04 and 26.04, and Arch Linux (and
  their derivatives), which install DEsk, what it needs and a classic set of
  GTK programs, and check the copy against `SHA256SUMS` first.

## Installing

Unpack the release (`DEsk-v0.1.1-BETA.zip` or `.tar.gz`), go into its folder
and run the line for your system:

```sh
sudo sh scripts/install_debian.sh    # Debian 13
sudo sh scripts/install_ubuntu.sh    # Ubuntu 24.04 / 26.04, Linux Mint, Pop!_OS
sudo sh scripts/install_arch.sh      # Arch Linux, EndeavourOS, Manjaro
```

Then log out and choose **DEsk** on the login screen. The README has the
details, the options (`--yes`, `--no-apps`, `--bare`, `--prefix`) and how to
build it yourself. To update later, run the same line from the new copy;
your settings and the traced room are kept.

The first start at a screen size traces the room: a few seconds on a GPU,
and longer on the processor (a letter on the desk says so). It is kept on
disk afterwards.

## Known limitations

- **Not yet supported protocols**: layer-shell (panels such as waybar, and
  slurp for picking an area), session-lock (external screen lockers),
  gamma-control (night light), input-method-v2 (on-screen keyboards),
  pointer-constraints and relative-pointer (pointer capture in games and
  virtual machines), fractional-scale.
- **High-density screens**: DEsk sizes its own things in points, but does
  not yet tell programs a scale, so their windows come out small on a HiDPI
  screen.
- **More than one screen**: screens other than the desk's show a plain desk
  for windows; the bookcase, Control Panel and post box are on the desk's
  screen only.
- **X11 programs** run through Xwayland. Ones that keep the screen awake
  through `org.freedesktop.ScreenSaver` (some video players and games) are
  not yet heard.
- **Suspend and hibernate** have no button yet (log out, restart and power
  off do).
- **Translations** other than English and Japanese have not yet been
  reviewed by native speakers.
- **No plugin API**: changing DEsk means changing its source
  ([docs/MODDING.md](docs/MODDING.md)).
- Real hardware tested so far: VMware virtual machines (SVGA3D) and an ASUS
  ZenBook Duo (two GPUs, two screens). Reports from other machines are very
  welcome.

## How it was checked

Built with no warnings in four configurations (minimal, `-Werror`,
`_FORTIFY_SOURCE=3`, AddressSanitizer + UndefinedBehaviorSanitizer), with a
strict warning set and GCC's static analyzer over every file; run under
ThreadSanitizer, AddressSanitizer and UndefinedBehaviorSanitizer, including
with deliberately malformed settings, data and `.desktop` files; every view
rendered in all seven languages; the translation tables checked by
`meson test`. Details in [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Checking your copy

The folder carries `SHA256SUMS`, which the installer checks before it builds.
The archive comes with `DEsk-v0.1.1-BETA.tar.gz.sha256`:

```sh
sha256sum -c DEsk-v0.1.1-BETA.tar.gz.sha256
```

## License

MIT. See [LICENSE](LICENSE).
