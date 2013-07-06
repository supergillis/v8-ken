{
  'includes': ['../../build/common.gypi'],
  'targets': [
    {
      'target_name': 'v8_ken_shell',
      'type': 'executable',
      'libraries': [
        '-lrt',
      ],
      'dependencies': [
        'v8.gyp:v8',
      ],
      'cflags': [
        '-g',
      ],
      'cflags!': [
        '-Werror',
      ],
      'include_dirs': [
        '../../src',
        '../../include',
        '../../deps/ken',
        '../../deps/http-parser',
      ],
      'sources': [
        '../../src/v8-ken.cc',
        '../../src/v8-ken-http.cc',
        '../../src/v8-ken-main.cc',
        '../../src/v8-ken-data.cc',
        '../../src/v8-ken-v8.cc',
      ],
    },
  ]
}
