# Build inside a distrobox, install and run on the host.
#
# Run from inside the box:   just box deploy
#
# Same recipes as the main justfile; build steps run in the box, and the
# Flatpak install/run and the udev rule go to the host via
# distrobox-host-exec.

main := "just --justfile " + justfile()
hostside := main + " host=distrobox-host-exec"

# List the recipes
[private]
default:
    @just --justfile {{source_file()}} --list --list-heading $'Inside the distrobox (just box <recipe>):\n'

[private]
_check:
    @test -f /run/.containerenv || { echo "just box: run this inside the distrobox"; exit 1; }

# Build in the box, install on the host
deploy: _check
    {{hostside}} deploy

# Build the app in the box
build: _check
    {{main}} build

# Export DSView.flatpak to the project dir
bundle: _check
    {{main}} bundle

# Install (or replace) the bundle on the host
install: _check
    {{hostside}} install

# Start the installed app on the host
run *args: _check
    {{hostside}} run {{args}}

# Remove the app from the host (settings are kept)
uninstall: _check
    {{hostside}} uninstall

# Install the USB udev rule on the host
udev: _check
    {{hostside}} udev

# Drop the build dir and bundle in the box
clean: _check
    {{main}} clean

# Drop all build state in the box
clean-all: _check
    {{main}} clean-all
