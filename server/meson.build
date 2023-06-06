project('server', 'cpp')

dbus = dependency('dbus-1')
glib1 = dependency('dbus-glib-1')

include_dbus = include_directories('dbus/include')

executable('server', [
    'main.cpp',
    'dbus/server_dbus.cpp',
    ], 
    dependencies : [
        dbus, 
        glib1,
    ],
    include_directories : [
        include_dbus
    ]
)

# Ubuntu 23.04
# sudo apt-get install libdbus-glib-1-dev
