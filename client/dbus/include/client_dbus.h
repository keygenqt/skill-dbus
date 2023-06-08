#include <gio/gio.h>
#include <functional>

class ClientDbus
{
public:
    ClientDbus(std::string name, std::string interface, std::string path);

    std::string getVersion();
    std::string ping();
    std::string echo(std::string value);
    void emitSignal(std::function<void()>);
    void quitLoop();
    bool exit();

    static void callback(GDBusConnection *,
			     const gchar *,
			     const gchar *,
			     const gchar *,
			     const gchar *,
			     GVariant *,
			     gpointer);

private:
	GDBusProxy *proxy;
	GDBusConnection *conn;
	GMainLoop *loop;
    std::function<void()> listener;
    std::string name;
    std::string interface;
    std::string path;
};
