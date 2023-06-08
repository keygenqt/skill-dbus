#include <string>
#include <functional>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

class ServerDbus
{
public:
    ServerDbus(std::string name, std::string interface, std::string path);
    int run();

public:
    const std::string version = "0.0.1";
    const std::string introspection = R"xml(
<node>
    <interface name='org.freedesktop.DBus.Introspectable'>
        <method name='Introspect'>
            <arg name='data' type='s' direction='out' />
        </method>
    </interface>
    
    <interface name='org.freedesktop.DBus.Properties'>
        <method name='Get'>
            <arg name='interface' type='s' direction='in' />
            <arg name='property' type='s' direction='in' />
            <arg name='value' type='s' direction='out' />
        </method>
        <method name='GetAll'>
            <arg name='interface' type='s' direction='in'/>
            <arg name='properties' type='a{sv}' direction='out'/>
        </method>
    </interface>

    <interface name='{Interface}'>
        <property name='Version' type='s' access='read' />
        <method name='Ping' >
            <arg type='s' direction='out' />
        </method>
        <method name='Echo'>
            <arg name='string' direction='in' type='s'/>
            <arg type='s' direction='out' />
        </method>
        <method name='EmitSignal'>
        </method>
        <method name='Quit'>
        </method>
        <signal name='OnEmitSignal'>
        </signal>
    </interface>
</node>
)xml";

private:
    static DBusHandlerResult messageFunction(DBusConnection *, DBusMessage *, void *);

    DBusHandlerResult messageHandler(DBusConnection *, DBusMessage *, void *);
    DBusHandlerResult propertyHandler(const char *, DBusConnection *, DBusMessage *);
    DBusHandlerResult propertiesHandler(DBusConnection *, DBusMessage *);
    DBusHandlerResult methodCall(DBusConnection *conn, DBusMessage *message, std::function<void(DBusMessage *reply)>);

    std::string name;
    std::string interface;
    std::string path;

private:
    DBusConnection *connect;
    GMainLoop *mainloop;
    DBusError err;
};
