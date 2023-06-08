#include <iostream>
#include <functional>

#include "client_dbus.h"

ClientDbus::ClientDbus(std::string name, std::string interface, std::string path)
{
    GError *error = NULL;

    this->name = name;
    this->interface = interface;
    this->path = path;

    conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
	g_assert_no_error(error);

	proxy = g_dbus_proxy_new_sync(conn,
				      G_DBUS_PROXY_FLAGS_NONE,
				      NULL,
				      name.c_str(),
				      path.c_str(),
				      interface.c_str(),
				      NULL,
				      &error);

	g_assert_no_error(error);
}

bool ClientDbus::exit()
{
    GVariant *result;
	GError *error = NULL;

	result = g_dbus_proxy_call_sync(proxy,
					"Quit",
					NULL,
					G_DBUS_CALL_FLAGS_NONE,
					-1,
					NULL,
					&error);

	g_assert_no_error(error);
	g_variant_unref(result);
	g_object_unref(proxy);
	g_object_unref(conn);

    return error == NULL;
}

std::string ClientDbus::getVersion()
{
    GVariant *variant;
    const char *version;

	variant = g_dbus_proxy_get_cached_property(proxy, "Version");
	g_assert(variant != NULL);
	g_variant_get(variant, "s", &version);
	g_variant_unref(variant);
	
    return version;
}

std::string ClientDbus::ping()
{
	GVariant *call;
	GError *error = NULL;
	const gchar *str;

	call = g_dbus_proxy_call_sync(proxy,
					"Ping",
					NULL,
					G_DBUS_CALL_FLAGS_NONE,
					-1,
					NULL,
					&error);

	g_assert_no_error(error);
	g_variant_get(call, "(&s)", &str);
    std::string result(str);
	g_variant_unref(call);

    return result;
}

std::string ClientDbus::echo(std::string value)
{
	GVariant *call;
	GError *error = NULL;
	const gchar *str;

	call = g_dbus_proxy_call_sync(proxy,
					"Echo",
					g_variant_new("(s)", value.c_str()),
					G_DBUS_CALL_FLAGS_NONE,
					-1,
					NULL,
					&error);
	g_assert_no_error(error);
	g_variant_get(call, "(&s)", &str);
    std::string result(str);
	g_variant_unref(call);

    return result;
}

void ClientDbus::emitSignal(std::function<void()> listener)
{
	GError *error = NULL;
	guint id;
	GDBusConnection *conn;

    this->listener = listener;

	loop = g_main_loop_new(NULL, false);
	conn = g_dbus_proxy_get_connection(proxy);

	id = g_dbus_connection_signal_subscribe(conn,
						this->name.c_str(),
						this->interface.c_str(),
						"OnEmitSignal",
						this->path.c_str(),
						NULL,
						G_DBUS_SIGNAL_FLAGS_NONE,
						ClientDbus::callback,
						this,
						NULL);

	g_dbus_proxy_call_sync(proxy,
			       "EmitSignal",
			       NULL,
			       G_DBUS_CALL_FLAGS_NONE,
			       -1,
			       NULL,
			       &error);
	g_assert_no_error(error);

	g_main_loop_run(loop);
	g_dbus_connection_signal_unsubscribe(conn, id);
}

void ClientDbus::quitLoop()
{
    g_main_loop_quit(loop);
}

void ClientDbus::callback(GDBusConnection *conn,
			     const gchar *sender_name,
			     const gchar *object_path,
			     const gchar *interface_name,
			     const gchar *signal_name,
			     GVariant *parameters,
			     gpointer data)
{
    auto client = static_cast<ClientDbus *>(data);

    client->listener();
    client->quitLoop();
}
