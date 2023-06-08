#include <stdexcept>
#include <string>
#include <iostream>

#include "client_dbus.h"

int main(void)
{
    auto client = ClientDbus(
        "org.example.TestServer",
        "org.example.TestInterface",
        "/org/example/TestObject"
    );

    std::cout << "Version: " << client.getVersion() << std::endl;
    std::cout << "Ping: " << client.ping() << std::endl;
    std::cout << "Echo: " << client.echo("Client dbus!") << std::endl;

    client.emitSignal([]() { 
        std::cout << "Signal emit! " << std::endl;
    });

    std::cout << "Exit server, close client: " << (client.exit() ? "true" : "false") << std::endl;

    return EXIT_SUCCESS;
}
