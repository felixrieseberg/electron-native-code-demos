{
  "targets": [
    {
      "target_name": "cpp_addon",
      "conditions": [
        ['OS=="win"', {
          "sources": [
            "src/cpp_addon.cc",
            "src/cpp_code.cc"
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
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1
            }
          },
          "defines": [
            "NODE_ADDON_API_CPP_EXCEPTIONS"
          ]
        }]
      ]
    }
  ]
}
