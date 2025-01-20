{
  "targets": [{
    "target_name": "swift_addon",
    "conditions": [
      ['OS=="mac"', {
        "sources": [
          "src/swift_addon.mm",
          "src/SwiftBridge.m",
          "src/SwiftCode.swift"
        ],
        "include_dirs": [
          "<!@(node -p \"require('node-addon-api').include\")",
          "include",
          "build_swift"
        ],
        "dependencies": [
          "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        "libraries": [
          "<(PRODUCT_DIR)/libSwiftCode.a"
        ],
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions" ],
        "xcode_settings": {
          "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
          "CLANG_ENABLE_OBJC_ARC": "YES",
          "SWIFT_OBJC_BRIDGING_HEADER": "include/SwiftBridge.h",
          "SWIFT_VERSION": "5.0",
          "SWIFT_OBJC_INTERFACE_HEADER_NAME": "swift_addon-Swift.h",
          "MACOSX_DEPLOYMENT_TARGET": "11.0",
          "OTHER_CFLAGS": [
            "-ObjC++",
            "-fobjc-arc"
          ],
          "OTHER_LDFLAGS": [
            "-Wl,-rpath,@loader_path",
            "-Wl,-install_name,@rpath/libSwiftCode.a"
          ],
          "HEADER_SEARCH_PATHS": [
            "$(SRCROOT)/include",
            "$(CONFIGURATION_BUILD_DIR)",
            "$(SRCROOT)/build/Release",
            "$(SRCROOT)/build_swift"
          ]
        },
        "actions": [
          {
            "action_name": "build_swift",
            "inputs": [
              "src/SwiftCode.swift"
            ],
            "outputs": [
              "build_swift/libSwiftCode.a",
              "build_swift/swift_addon-Swift.h"
            ],
            "action": [
              "swiftc",
              "src/SwiftCode.swift",
              "-emit-objc-header-path", "./build_swift/swift_addon-Swift.h",
              "-emit-library", "-o", "./build_swift/libSwiftCode.a",
              "-emit-module", "-module-name", "swift_addon",
              "-module-link-name", "SwiftCode"
            ]
          },
          {
            "action_name": "copy_swift_lib",
            "inputs": [
              "<(module_root_dir)/build_swift/libSwiftCode.a"
            ],
            "outputs": [
              "<(PRODUCT_DIR)/libSwiftCode.a"
            ],
            "action": [
              "sh",
              "-c",
              "cp -f <(module_root_dir)/build_swift/libSwiftCode.a <(PRODUCT_DIR)/libSwiftCode.a && install_name_tool -id @rpath/libSwiftCode.a <(PRODUCT_DIR)/libSwiftCode.a"
            ]
          }
        ]
      }]
    ]
  }]
}
