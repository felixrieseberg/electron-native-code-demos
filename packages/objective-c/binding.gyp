{
  "targets": [
    {
      "target_name": "objectivec_addon",
      "conditions": [
        ['OS=="mac"', {
          "sources": [
            "src/objectivec_addon.mm",
            "src/ObjCBridge.m",
            "src/TodoViewController.m"
          ],
          "include_dirs": [
            "<!@(node -p \"require('node-addon-api').include\")",
            "include"
          ],
          "dependencies": [
            "<!(node -p \"require('node-addon-api').gyp\")"
          ],
          "cflags!": [ "-fno-exceptions" ],
          "cflags_cc!": [ "-fno-exceptions" ],
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_ENABLE_OBJC_ARC": "YES",
            "MACOSX_DEPLOYMENT_TARGET": "11.0",
            "OTHER_CFLAGS": [
              "-ObjC++",
              "-fobjc-arc"
            ]
          }
        }]
      ]
    }
  ]
}
