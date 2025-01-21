# Electron & Native Code Demos

This repository demonstrates how you can integrate native code and even native UI elements into your Electron application. It's meant as both a tutorial and as a starting point for you own development. All packages implement:

1. A simple `helloWorld()` function. It can be called from JavaScript and is executed in native code. Use this as a starting point for simple calls - good examples are calling one specific operating system API.
2. A `helloGui()` function that launches a basic todo editor with all-native controls. Use this as a starting point to extend your Electron application with fully native user interfaces.

## Packages

 - `swift`: Uses Swift, SwiftUI, and Foundation. Bridging between JavaScript and Swift is done in Objective-C++.
 - `objective-c`: Uses Objective-C and Cocoa. Bridging between JavaScript and Objective-C is done in Objective-C++.
 - `cpp-win32`: Uses C++ and the `Windows Common Controls` (also known as Win32 API controls) in version 6. Bridging between the JavaScript and the Win32 world is done in C++.
