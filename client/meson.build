project('client', 'cpp')

gio = dependency('gio-2.0')

include_dbus = include_directories('dbus/include')

executable('client', [
        'main.cpp',
        'dbus/client_dbus.cpp',
    ], 
    dependencies : [
        gio, 
    ],
    include_directories : [
        include_dbus
    ]
)
