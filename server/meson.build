project('server', 'cpp')

dbus = dependency('dbus-1')
glib1 = dependency('dbus-glib-1')

include_test = include_directories('test/include')

executable('server', [
    'main.cpp',
    'test/test_dbus.cpp',
    ], 
    dependencies : [
        dbus, 
        glib1,
    ],
    include_directories : [
        include_test
    ]
)

# Ubuntu 23.04
# sudo apt-get install libdbus-glib-1-dev
