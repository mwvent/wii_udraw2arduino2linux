# wii_udraw2arduino2linux
Use the wii udraw tablet with linux via an arduino interface


# Installation
Wiring to the Arduino as follows
    Yellow > A5/SCL
    Green > A4/SDA
    Blue > GND
    Brown > 5v

Upload the arduino sketch to your arudino

# To install as a service on systemd 
extract the linux_service folder contents to /opt/udraw
ln -s /opt/udraw/udraw.service /etc/systemd/system/udraw.service
systemctl enable udraw.service
systemctl daemon-reload
service udraw start
The service will continually try running the python script every 5 seconds, if the tablet is unplugged this will mean it will keep re-trying until it is plugged in again. Stderr is mapped to stdout to prevent tonnes of service errors being generated

# Just run
If you do not wish a systemd service just run driver.py.
To run as a normal user you need access to /dev/uniput (usually given by plugdev group) and access to the Arduino serial (usually dialout group)


There was little in the way in up to date udev tablet projects I could find online nor any docs for the Udraw tablets protocol. I very much hope this is usefull to somebody.
