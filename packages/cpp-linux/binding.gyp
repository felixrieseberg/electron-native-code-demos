{
  "targets": [
    {
      "target_name": "cpp_addon",
      "conditions": [
        ['OS=="linux"', {
          "sources": [
            "src/cpp_addon.cc",
            "src/cpp_code.cc"
          ],
          "include_dirs": [
            "<!@(node -p \"require('node-addon-api').include\")",
            "include",
            "<!@(pkg-config --cflags-only-I gtk4 | sed s/-I//g)"
          ],
          "cflags!": ["-fno-exceptions"],
          "cflags_cc!": ["-fno-exceptions"],
          "cflags": [
            "-fexceptions",
            "<!@(pkg-config --cflags gtk4)"
          ],
          "cflags_cc": [
            "-fexceptions",
            "<!@(pkg-config --cflags gtk4)"
          ],
          "defines": ["NODE_ADDON_API_CPP_EXCEPTIONS"],
          "libraries": [
            "<!@(pkg-config --libs gtk4)",
            "-luuid"
          ],
          "dependencies": [
            "<!(node -p \"require('node-addon-api').gyp\")"
          ]
        }]
      ]
    }
  ]
}
