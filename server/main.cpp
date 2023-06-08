#include "server_dbus.h"

int main(void)
{
    return ServerDbus(
        "org.example.TestServer",
        "org.example.TestInterface",
        "/org/example/TestObject"
    ).run();
}
