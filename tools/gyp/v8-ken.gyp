{
  'includes': ['../../build/common.gypi'],
  'targets': [
    {
      'target_name': 'v8_ken_shell',
      'type': 'executable',
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
      ],
      'sources': [
        '../../src/v8-ken-main.cc',
        '../../src/v8-ken-data.cc',
      ],
    },
  ]
}
