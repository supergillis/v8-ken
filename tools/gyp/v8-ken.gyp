{
  'includes': ['../../build/common.gypi'],
  'targets': [
    {
      'target_name': 'v8_ken_shell',
      'type': 'executable',
      'dependencies': [
        'v8.gyp:v8',
      ],
      'cflags!': [
        '-Werror',
      ],
      'include_dirs': [
        '../../include',
        '../../deps/ken',
      ],
      'sources': [
        '../../src/v8-ken-main.cc',
      ],
    },
  ]
}
