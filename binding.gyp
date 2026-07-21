{
  "targets": [
    {
      "target_name": "node_llm_native",

      "sources": [
        "addon/native.cpp",
        "model/model.cpp"
      ],

      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "vendor/llama.cpp/include",
        "vendor/llama.cpp/ggml/include",
        "vendor/llama.cpp/common"
      ],

      "defines": [
        "NAPI_CPP_EXCEPTIONS"
      ],

      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],

      "conditions": [

        [
          "OS=='linux'",
          {
            "cflags_cc": [
              "-std=c++17",
              "-fexceptions"
            ],

            "libraries": [
              "-L<(module_root_dir)/vendor/llama.cpp/build/bin",
              "-Wl,-rpath,'$$ORIGIN/../../vendor/llama.cpp/build/bin'",
              "-lllama"
            ]
          }
        ],

        [
          "OS=='mac'",
          {
            "xcode_settings": {
              "CLANG_CXX_LANGUAGE_STANDARD": "c++17",
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
            },

            "libraries": [
              "-L<(module_root_dir)/vendor/llama.cpp/build/bin",
              "-Wl,-rpath,@loader_path/../../vendor/llama.cpp/build/bin",
              "-lllama"
            ]
          }
        ],

        [
          "OS=='win'",
          {
            "msvs_settings": {
              "VCCLCompilerTool": {
                "ExceptionHandling": 1,
                "AdditionalOptions": [
                  "/std:c++17"
                ]
              }
            },

            "libraries": [
              "<(module_root_dir)/vendor/llama.cpp/build/bin/llama.lib"
            ]
          }
        ]
      ]
    }
  ]
}