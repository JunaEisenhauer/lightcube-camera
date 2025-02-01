# Lightcube Camera

Lightcube uses gstreamer to create a camera stream to show a preview on the display. With external gpio buttons the brightness can be changed and a photo can be taken.

## Prerequisites

### Raspberry Pi Setup

https://www.raspberrypi.org/downloads/

- Download and install https://downloads.raspberrypi.org/imager/imager_1.5_amd64.deb
```
sudo dpkg -i imager_1.5_amd64.deb
```
- Run Raspberry Pi Imager
    - Operating System: Raspberry Pi OS Lite (32-bit)
    - SD Card -> Write

- Insert SD Card and start Raspberry Pi
- Login: pi:raspberry (en keyboard layout)

```
sudo raspi-config
```

Configure wifi, ssh, etc.

`/boot/config.txt` -> add:

```
hdmi_force_hotplug=1
hdmi_group=2
hdmi_mode=87
hdmi_cvt=800 480 60 6 0 0 0
hdmi_drive=1
```

#### GStreamer

```
sudo apt-get install libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-pulseaudio
```

#### wiringPi

```
sudo apt-get install wiringpi
```

#### Toolchain

```
sudo apt-get install gcc cmake
```

## Build

```
cmake -S . -B build
cd build
make
```

## Launch

Execute the built artifact in the build folder.

```
./lightcube
```

### Launch gstreamer pipeline manually

```
gst-launch-1.0 v4l2src extra-controls="v4l2,brightness=8" ! videoscale ! video/x-raw,framerate=30/1,width=800,height=480 ! tee name=t ! queue ! videoconvert ! gdkpixbufoverlay location=crosshair.png ! textoverlay text="Brightness: 8" font-desc="Sans, 24" ! fbdevsink t. ! fakesink
```

## Misc

### crosshair.png to crosshair.h

```
xxd -i crosshair.png > crosshair.h
```

### Backup sd card

Determine sd card:
```
sudo fdisk -l
```

Create compressed image file:
```
sudo dd bs=4M if=/dev/sdb | gzip > ~/image`date +%d%m%y`.gz
```

Use compressed image file:
```
sudo gzip -dc ~/imageXXXXXX.gz | dd bs=4M of=/dev/sdb
```
