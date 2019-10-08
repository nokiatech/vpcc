# Video Point Cloud Coding (V-PCC) AR Demo

This software provides an experimental implementation for decoding and AR rendering of bitstreams encoded with the upcoming ISO/IEC standard for video-based point cloud compression (ISO/IEC 23090-5).

## Contents of the Repository

* Demo application source code
* Prebuilt demo application
* Demo content

Note: We currently support only TMC 6.0 version.

## Install prebuilt demo:

### Android:
Make sure you have ARCore capable Android phone (https://developers.google.com/ar/discover/supported-devices)
Install and update Google Play Services for AR from the Google Play Store (https://play.google.com/store/apps/details?id=com.google.ar.core).

Install application and content (Windows):

```
install_demo.bat
install_content.bat
```

Install application and content (macOS):

```
./install_demo.sh
./install_content.sh
```

## Building from source code:

### Android:

####Prerequisites
- Android Studio version 3.1 or higher with Android SDK Platform version 7.0 (API level 24) or higher (https://developer.android.com/studio/index.html)
- Prepare Android Studio for native development by installing Android Native Development Kit (NDK) and CMake with the SDK Manager (https://developer.android.com/ndk/guides/index.html)

Open the ARPlayer project under /Android into Android Studio and build the application.

## License:
Please see **[LICENSE.TXT](https://github.com/nokiatech/vpcc/blob/master/LICENSE.txt)** file for the terms of use of the contents of this repository.

For more information, please contact: <vpcc@nokia.com>

### Copyright (c) 2018-2019 Nokia Corporation and/or its subsidiary(-ies).
### **All rights reserved.** 