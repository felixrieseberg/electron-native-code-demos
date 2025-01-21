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
          "libraries": [
            "comctl32.lib",
            "shcore.lib"
          ],
          "dependencies": [
            "<!(node -p \"require('node-addon-api').gyp\")"
          ],
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1,
              "DebugInformationFormat": "OldStyle",
              "AdditionalOptions": [
                "/FS"
              ]
            },
            "VCLinkerTool": {
              "GenerateDebugInformation": "true"
            }
          },
          "defines": [
            "NODE_ADDON_API_CPP_EXCEPTIONS",
            "WINVER=0x0A00",
            "_WIN32_WINNT=0x0A00"
          ]
        }]
      ]
    }
  ]
}
