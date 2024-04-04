
# Lye - a TUI display manager

![Lye in a TTY with the blizzard animation](https://github.com/wr7/lye/assets/53203261/f25890c2-fc4b-4dab-b1e2-c648a75817f8 "Lye in a TTY with the blizzard animation")

Lye (Ly Extended) is a fork of [Ly](https://github.com/fairyglade/ly), a lightweight TUI (ncurses-like) display manager for Linux and BSD.

## Fork
The upstream Ly hasn't been updated recently. Lye was made to add several bug fixes and features.

The long term plans of Lye are as follows:
 - Clean up code
   - Break up files
   - Break up functions
   - Reduce code size and complexity
 - Add new animations
 - Improve compatibility (notably with Nix)

## Animations
Currently Lye has the following animations. The ones unique to lye are denoted with a star.

 - Random - 0 (*)
 - Doom fire - 1
 - Matrix - 2
 - Blizzard - 3 (*)

## Dependencies
 - a C99 compiler (tested with tcc and gcc)
 - a C standard library
 - GNU make
 - pam
 - xcb
 - xorg
 - xorg-xauth
 - mcookie
 - tput
 - shutdown

On Debian-based distros running `apt install build-essential libpam0g-dev libxcb-xkb-dev` as root should install all the dependencies for you.
For Fedora try running `dnf install make automake gcc gcc-c++ kernel-devel pam-devel libxcb-devel`

## Support
The following desktop environments were tested with success

 - awesome
 - bspwm
 - budgie
 - cinnamon
 - deepin
 - dwm
 - enlightenment
 - gnome
 - i3
 - kde
 - labwc
 - lxde
 - lxqt
 - mate
 - maxx
 - pantheon
 - qtile
 - spectrwm
 - sway
 - windowmaker
 - xfce
 - xmonad

Lye should work with any X desktop environment, and provides
basic wayland support (sway works very well, for example).

## systemd?
Unlike what you may have heard, Lye does not require `systemd`,
and was even specifically designed not to depend on `logind`.
You should be able to make it work easily with a better init,
changing the source code won't be necessary :)

## Cloning and Compiling
Clone the repository
```
$ git clone --recurse-submodules https://github.com/wr7/lye
```

Change the directory to lye
```
$ cd lye
```

Compile
```
$ make
```

Test in the configured tty (tty2 by default)
or a terminal emulator (but desktop environments won't start)
```
# make run
```

Install Lye and the provided systemd service file
```
# make install installsystemd
```

Enable the service
```
# systemctl enable lye.service
```

If you need to switch between ttys after Lye's start you also have to
disable getty on Lye's tty to prevent "login" from spawning on top of it
```
# systemctl disable getty@tty2.service
```

### OpenRC

Clone, compile and test.

Install Lye and the provided OpenRC service
```
# make install installopenrc
```

Enable the service
```
# rc-update add lye
```

You can edit which tty Lye will start on by editing the `tty` option in the configuration file.

If you choose a tty that already has a login/getty running (has a basic login prompt), then you have to disable the getty so it doesn't respawn on top of lye
```
# rc-update del agetty.tty2
```

### runit

```
$ make
# make install installrunit
# ln -s /etc/sv/lye /var/service/
```

By default, lye will run on tty2. To change the tty it must be set in `/etc/lye/config.ini`

You should as well disable your existing display manager service if needed, e.g.:

```
# rm /var/service/lxdm
```

The agetty service for the tty console where you are running lye should be disabled. For instance, if you are running lye on tty2 (that's the default, check your `/etc/lye/config.ini`) you should disable the agetty-tty2 service like this:

```
# rm /var/service/agetty-tty2
```

## Configuration
You can find all the configuration in `/etc/lye/config.ini`.
The file is commented, and includes the default values.

## Controls
Use the up and down arrow keys to change the current field, and the
left and right arrow keys to change the target desktop environment
while on the desktop field (above the login field).

## .xinitrc
If your .xinitrc doesn't work make sure it is executable and includes a shebang.
This file is supposed to be a shell script! Quoting from xinit's man page:

> If no specific client program is given on the command line, xinit will look for a file in the user's home directory called .xinitrc to run as a shell script to start up client programs.

On Arch Linux, the example .xinitrc (/etc/X11/xinit/xinitrc) starts like this:
```
#!/bin/sh
```

## Tips
The numlock and capslock state is printed in the top-right corner.
Use the F1 and F2 keys to respectively shutdown and reboot.
Take a look at your .xsession if X doesn't start, as it can interfere
(this file is launched with X to configure the display properly).

## PSX DOOM fire animation
To enable the famous PSX DOOM fire described by [Fabien Sanglard](http://fabiensanglard.net/doom_fire_psx/index.html),
just uncomment `animate = true` in `/etc/lye/config.ini`. You may also
disable the main box borders with `hide_borders = true`.
