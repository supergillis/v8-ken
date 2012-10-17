{
  'includes': ['../../build/common.gypi'],
  'targets': [
    {
      'target_name': 'ken',
      'type': '<(library)',
      'cflags': [
        '-std=gnu99',
      ],
      'cflags!': [
        '-Wnon-virtual-dtor',
        '-Woverloaded-virtual',
        '-Werror',
        '-fno-rtti',
      ],
      'include_dirs': [
        '../../deps/ken',
      ],
      'sources': [
        '../../deps/ken/ken.c',
        '../../deps/ken/kencom.c',
        '../../deps/ken/kencrc.c',
        '../../deps/ken/kenext.c',
        '../../deps/ken/kenpat.c',
        '../../deps/ken/kenvar.c',
      ],
    },
  ]
}
