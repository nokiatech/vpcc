# Video Point Cloud Coding (V-PCC) AR Demo

This software provides an experimental implementation for decoding and AR rendering of bitstreams encoded with the upcoming ISO/IEC standard for video-based point cloud compression (ISO/IEC 23090-5).

## Contents of the Repository

* Demo application source code
* Demo content

Note: We currently support only TMC 8.0 version (http://mpegx.int-evry.fr/software/MPEG/PCC/TM/mpeg-pcc-tmc2/tags/release-v8.0) and C2 intra coding conditions (lossy geometry, lossy attributes). The dimensions of the encoded video frames shall not change over an encoded sequence.

For more information on V-PCC, please refer to http://www.mpeg-pcc.org

## Building from source code:

### Common:

Follow guide to download & extract 3rd party dependencies: https://github.com/nokiatech/vpcc/blob/master/Sources/External/README.md

### Android:

#### Prerequisites
- Android Studio version 3.1 or higher with Android SDK Platform version 7.0 (API level 24) or higher (https://developer.android.com/studio/index.html)
- Prepare Android Studio for native development by installing Android Native Development Kit (NDK) and CMake with the SDK Manager (https://developer.android.com/ndk/guides/index.html)
- Make sure you have ARCore capable Android phone (https://developers.google.com/ar/discover/supported-devices)
- Install and update Google Play Services for AR from the Google Play Store (https://play.google.com/store/apps/details?id=com.google.ar.core).

Open the ARPlayer project under /Android into Android Studio and build the application.

## Install demo content:

### Android:

Install content (Windows):

```
install_content.bat
```

Install content (macOS):

```
./install_content.sh
```

## License:
Please see **[LICENSE.TXT](https://github.com/nokiatech/vpcc/blob/master/LICENSE.txt)** file for the terms of use of the contents of this repository.

For more information, please contact: <VPCC.contact@nokia.com>

### Copyright (c) 2018-2020 Nokia Corporation and/or its subsidiary(-ies).
### **All rights reserved.** 