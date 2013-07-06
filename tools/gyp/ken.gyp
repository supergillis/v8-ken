{
  'includes': ['../../build/common.gypi'],
  'targets': [
    {
      'target_name': 'ken',
      'type': '<(library)',
      'cflags': [
        '-std=gnu99',
        '-g',
      ],
      'dependencies': [
        '../../deps/http-parser/http_parser.gyp:http_parser',
      ],
      'cflags!': [
        '-Wnon-virtual-dtor',
        '-Woverloaded-virtual',
        '-Werror',
        '-fno-rtti',
      ],
      'include_dirs': [
        '../../deps/ken',
        '../../deps/http-parser'
      ],
      'sources': [
        '../../deps/ken/ken.c',
        '../../deps/ken/kencom.c',
        '../../deps/ken/kencrc.c',
        '../../deps/ken/kenext.c',
        '../../deps/ken/kenpat.c',
        '../../deps/ken/kenvar.c',
        '../../deps/ken/kenhttp.c',
      ],
    },
  ]
}
