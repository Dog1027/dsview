# DSView Flatpak workflow.
#
#   just              list the recipes
#   just deploy       build + bundle + install (the usual loop)
#   just run          start the installed app
#
# Needs flatpak and flatpak-builder, plus org.kde.Sdk//6.11 and
# org.kde.Platform//6.11 (see flatpak/README.md).
#
# Overridable on the command line, e.g. `just scope=user install`:
#   scope    system (default) or user installation
#   flatpak  command used for install/run/uninstall
#   sudo     command used for the udev rule
#
# Building inside a distrobox and installing on the host: `just box`.

mod box "flatpak/box.just"

app      := "io.github.dog1027.DSView"
manifest := "flatpak/" + app + ".yml"
work     := "/var/tmp/dsview-fp"          # build state, outside $HOME
bundle   := "DSView.flatpak"

scope    := "system"
flatpak  := "flatpak"
sudo     := "sudo"

# List the recipes
[private]
default:
    @just --justfile {{justfile()}} --list

# Build, bundle and install
deploy: bundle install

# Build the app
build:
    flatpak-builder --disable-rofiles-fuse --force-clean \
        --state-dir={{work}}/state --repo={{work}}/repo \
        {{work}}/build {{manifest}}

# Export DSView.flatpak to the project dir
bundle: build
    flatpak build-bundle {{work}}/repo {{bundle}} {{app}}
    @ls -lh {{bundle}}

# Install (or replace) the bundle
install:
    {{flatpak}} install --{{scope}} --reinstall -y {{bundle}}

# Start the installed app
run *args:
    {{flatpak}} run {{app}} {{args}}

# Remove the app and any runtime no longer used (settings are kept)
uninstall:
    {{flatpak}} uninstall --{{scope}} -y {{app}}
    {{flatpak}} uninstall --{{scope}} -y --unused

# Install the USB udev rule (once; then replug the instrument)
udev:
    {{sudo}} install -m 644 DSView/DreamSourceLab.rules /etc/udev/rules.d/60-dreamsourcelab.rules
    {{sudo}} udevadm control --reload
    {{sudo}} udevadm trigger

# Drop the build dir and bundle (keeps downloaded sources)
clean:
    rm -rf {{work}}/build
    rm -f {{bundle}}

# Drop all build state (next build starts from scratch)
clean-all: clean
    rm -rf {{work}}
