DSView as a Flatpak
===================

Builds the current working tree with the KDE 6.11 runtime (Qt 6). The
`justfile` at the repository root wraps every step.

Setup
-----

    sudo dnf install flatpak flatpak-builder just      # or your distro's
    flatpak remote-add --if-not-exists flathub \
        https://dl.flathub.org/repo/flathub.flatpakrepo
    flatpak install flathub org.kde.Platform//6.11 org.kde.Sdk//6.11

USB access needs the udev rule on the host (the sandbox cannot install it):

    just udev

Then unplug and replug the instrument.

Everyday use
------------

    just              # list the recipes
    just deploy       # build, write DSView.flatpak, install it
    just run          # start it (or use the app menu)
    just uninstall    # remove the app and runtimes nothing else uses

Other recipes: `build`, `bundle`, `install`, `clean`, `clean-all`.
The app goes into the system installation (sudo asks for your password);
`just scope=user deploy` puts it in your user installation instead. Build state lives in
`/var/tmp/dsview-fp`, not in your home folder.

Building in a distrobox
-----------------------

To keep the SDK and build cache off the host, build inside a distrobox
and install on the host. Set up flatpak, flatpak-builder, just and the
SDK inside the box as above, then, from inside the box:

    just box             # list the recipes
    just box deploy      # build in the box, install on the host
    just box run         # start it on the host

`just box` runs the same recipes; install, run, uninstall and udev go to
the host through `distrobox-host-exec`. The host only needs Flathub as a
remote, for the runtime.

Without just
------------

    flatpak-builder --disable-rofiles-fuse --force-clean \
        --state-dir=/var/tmp/dsview-fp/state --repo=/var/tmp/dsview-fp/repo \
        /var/tmp/dsview-fp/build flatpak/io.github.dog1027.DSView.yml
    flatpak build-bundle /var/tmp/dsview-fp/repo DSView.flatpak \
        io.github.dog1027.DSView
    flatpak install --system DSView.flatpak

Notes
-----

- Settings live in `~/.var/app/io.github.dog1027.DSView/config`, not in
  `~/.config/DreamSourceLab` (the native build's settings stay separate).
  `just uninstall` keeps them; delete that folder to remove them too.
- `--filesystem=home` lets the file dialogs open and save anywhere in
  your home folder. Narrow it (e.g. `xdg-documents`) if you prefer.
