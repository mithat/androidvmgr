Android VM Manager
==================

Mithat Konar
2013-08-26

**Android VM Manager** is a desktop application designed to make launching [Android-x86](http://www.android-x86.org/) Virtual Machines and connecting them to the [Android Debug Bridge](https://developer.android.com/tools/help/adb.html) convenient. 

It supports [VirtualBox](https://www.virtualbox.org/) VMs and [Android-x86 4.4-r1 (KitKat-x86)](http://www.android-x86.org/releases/releasenote-4-4-r1). It has been tested on Linux. Mac OS is untested; Windows support may follow.

Android VM Manager is developed with [Qt widgets](http://qt-project.org/) and is licensed under the [GPLv3](https://www.gnu.org/licenses/gpl-3.0.txt).

Quickstart
-----------
###Configuration
#### Set the location of the adb executable
Start Android VM Manager and go to _Edit > Preferences..._. If you added the `platform-tools` directory to your `PATH` during installation of the Android SDK (or ADT), then check "ADB is on PATH" and you're done. If not, then use the "Browse" button to tell Android VM Manager where the `adb` executable is. (As of this writing, it's in `<Android-SDK-directory>/platform-tools`.) 

#### Set the name of the VM
In the "VM name" text field of Android VM Manager's main interface, enter the name of the virtual machine you want to use as it appears in Virtual Box.

#### Set the IP address of the VM
Two methods for configuring the IP address of the VM are described below: **NAT with port forwarding** and **Host only**. If the VM's documentation doesn't tell you what you should be, try NAT with port forwarding.

##### Using NAT with port forwarding
1. Open VirtualBox and configure the VM's settings as follows:
    * In the "Network" tab, set "Attached to" to "NAT".
    * Click the "Advanced" arrow to expose the options.
    * Set "Adapter Type" to "PCnet-Fast III (Am79C973)." (This might vary according to the specific VM, but it has been tested with Android-x86.)
    * Click the "Port Forwarding" button and add a forwarding rule:
        * Name: ADB forwarding rule
        * Protocol: TCP
        * Host IP: (leave blank)
        * Host Port: 5555
        * Guest IP: (leave blank)
        * Guest Port: 5555            
2. In Android VM Manager's main interface, enter 127.0.0.1 in the "VM IP address" text field.

##### Using Host-only adapter
1. Open VirtualBox and configure the VM's settings as follows:
    * Under the "Network" tab, set "Attached to" to "Host-only Adapter".
2. Consult the VM's documentation regarding the best way to learn the IP address it is using. If all else fails, you might get something useful from Android VM Manager's _File > VM info..._ command. If your VM has a terminal in it, you might try the `netcfg` command in it.

### Use
#### Starting the VM
Start the virtual machine with _Machine > Start VM_.

#### Connect the VM to the ADB
Connect the virtual machine to the Android Debug Bridge with _ADB > Connect VM_.

#### Running apps in the VM
Once the VM is running and connected, you should be able to run applications in the virtual machine from your development environment.

To be sure that it will launch in the VM rather than in the SDK's emulator, you might want to configure your development environment to prompt you for where to run your application on every run. In the Eclipse-based ADT, this is done via selecting _Run > Run configurations..._, clicking the "Target" tab, and selecting "Always prompt to pick device." 
