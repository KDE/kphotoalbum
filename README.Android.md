# Compiling the android client

The android client is just a pure Qt/QML application.
Therefore, you can run and test it even without an android phone.

## Overview

### Directory `RemoteControl`

Contains the shared code between KPhotoAlbum and the remote client.

### Directory `AndroidRemoteControl`

The directory containing the remote client.
The required files from `RemoteControl` are included as symbolic links.

The user interface of the remote client is implemented using QML.
You can see the QML files in the `qml` subdirectory.
The C++ files are mostly for application logic.

## Building on the desktop

If you want to make changes to the remote client, it's best to use qtcreator and open the Qt project file:

    qtcreator AndroidRemoteControl/AndroidRemoteControl.pro

## Building for android

TODO
