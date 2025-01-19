#!/bin/bash
mkdir -p build_swift

swiftc src/SwiftCode.swift \
  -emit-objc-header-path ./build_swift/swift_addon-Swift.h \
  -emit-library -o ./build_swift/libSwiftCode.a \
  -emit-module -module-name swift_addon \
  -module-link-name SwiftCode

echo "Swift code built successfully"
