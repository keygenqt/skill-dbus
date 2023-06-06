#include <cstdio>
#include <stdexcept>
#include <iostream>
#include <functional>
#include <chrono>
#include <thread>

#include "server_dbus.h"

using namespace std::chrono_literals;

DBusHandlerResult ServerDbus::messageFunction(DBusConnection *conn, DBusMessage *message, void *data)
{
	return static_cast<ServerDbus *>(data)->messageHandler(conn, message, data);
}

DBusHandlerResult ServerDbus::propertyHandler(const char *property, DBusConnection *conn, DBusMessage *reply)
{
	if (!strcmp(property, "Version"))
	{
		dbus_message_append_args(reply,
								 DBUS_TYPE_STRING,
								 &version,
								 DBUS_TYPE_INVALID);
	}
	else
	{
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	if (!dbus_connection_send(conn, reply, NULL))
	{
		return DBUS_HANDLER_RESULT_NEED_MEMORY;
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult ServerDbus::propertiesHandler(DBusConnection *conn, DBusMessage *reply)
{
	DBusMessageIter array, dict, iter, variant;

	const char *property = "Version";

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &array);
	dbus_message_iter_open_container(&array, DBUS_TYPE_DICT_ENTRY, NULL, &dict);
	dbus_message_iter_append_basic(&dict, DBUS_TYPE_STRING, &property);
	dbus_message_iter_open_container(&dict, DBUS_TYPE_VARIANT, "s", &variant);
	dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &version);
	dbus_message_iter_close_container(&dict, &variant);
	dbus_message_iter_close_container(&array, &dict);
	dbus_message_iter_close_container(&iter, &array);

	if (dbus_connection_send(conn, reply, NULL))
	{
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else
	{
		return DBUS_HANDLER_RESULT_NEED_MEMORY;
	}
}

DBusHandlerResult ServerDbus::methodCall(DBusConnection *conn, DBusMessage *message, std::function<void(DBusMessage *reply)> setArg)
{
	DBusMessage *reply = NULL;

	if (!(reply = dbus_message_new_method_return(message)))
	{
		throw std::runtime_error("Failed to create method");
	}
	else
	{
		setArg(reply);

		if (!dbus_connection_send(conn, reply, NULL))
		{
			throw std::runtime_error("Need more memory in order to return");
		}

		dbus_message_unref(reply);

		return DBUS_HANDLER_RESULT_HANDLED;
	}
}

DBusHandlerResult ServerDbus::messageHandler(DBusConnection *conn, DBusMessage *message, void *data)
{
	DBusHandlerResult result;
	DBusMessage *reply = NULL;
	DBusError err;

	std::cout << "Got D-Bus request: "
			  << dbus_message_get_interface(message)
			  << "."
			  << dbus_message_get_member(message)
			  << " on "
			  << dbus_message_get_path(message)
			  << std::endl;

	// init error
	dbus_error_init(&err);

	try
	{
		if (dbus_message_is_method_call(message, interface.c_str(), "Ping"))
		{
			return methodCall(conn, message, [this](DBusMessage *reply)
							  {
								  const char *pong = "Pong";
								  dbus_message_append_args(reply,
														   DBUS_TYPE_STRING, &pong,
														   DBUS_TYPE_INVALID); });
		}

		if (dbus_message_is_method_call(message, interface.c_str(), "Echo"))
		{
			return methodCall(conn, message, [this, message, &err](DBusMessage *reply)
							  { 
								const char *msg;
								dbus_message_get_args(message, &err,
													  DBUS_TYPE_STRING, &msg,
													  DBUS_TYPE_INVALID);
								dbus_message_append_args(reply,
														 DBUS_TYPE_STRING, &msg,
														 DBUS_TYPE_INVALID); });
		}

		if (dbus_message_is_method_call(message, interface.c_str(), "Quit"))
		{
			auto result = methodCall(conn, message, [this](DBusMessage *reply) {});
			std::cout << "Server exiting..." << std::endl;
			g_main_loop_quit(mainloop);
			return result;
		}

		if (dbus_message_is_method_call(message, interface.c_str(), "EmitSignal"))
		{
			if (!(reply = dbus_message_new_signal(path.c_str(), interface.c_str(), "OnEmitSignal")))
			{
				throw std::runtime_error("Failed to create signal");
			}

			if (!dbus_connection_send(conn, reply, NULL))
			{
				return DBUS_HANDLER_RESULT_NEED_MEMORY;
			}

			// delay
			for (int i : {1, 2, 3, 4, 5})
			{
				std::this_thread::sleep_for(1000ms);
				std::cout << "Loadin signal " << i << "s..." << std::endl;
			}

			// emit signal
			reply = dbus_message_new_method_return(message);
			dbus_connection_send(conn, reply, NULL);
			dbus_message_unref(reply);

			return DBUS_HANDLER_RESULT_HANDLED;
		}

		if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
		{
			return methodCall(conn, message, [this](DBusMessage *reply)
							  { dbus_message_append_args(reply,
														 DBUS_TYPE_STRING,
														 DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE + introspection,
														 DBUS_TYPE_INVALID); });
		}

		if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "GetAll"))
		{
			if (!(reply = dbus_message_new_method_return(message)))
			{
				throw std::runtime_error("Failed to create method");
			}

			result = propertiesHandler(conn, reply);

			dbus_message_unref(reply);

			return result;
		}

		if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "Get"))
		{
			const char *interface, *property;

			dbus_message_get_args(message, &err,
								  DBUS_TYPE_STRING, &interface,
								  DBUS_TYPE_STRING, &property,
								  DBUS_TYPE_INVALID);

			if (!(reply = dbus_message_new_method_return(message)))
			{
				throw std::runtime_error("Failed to create method");
			}

			result = propertyHandler(property, conn, reply);

			dbus_message_unref(reply);

			return result;
		}

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}
	catch (std::exception &e)
	{
		std::cout << "Request failed. Exception: " << e.what() << std::endl;

		return DBUS_HANDLER_RESULT_NEED_MEMORY;
	}
}

int ServerDbus::run()
{
	// init error
	dbus_error_init(&err);

	// connect to the daemon bus
	connect = dbus_bus_get(DBUS_BUS_SESSION, &err);

	// check connect
	if (!connect)
	{
		return EXIT_FAILURE;
	}

	// request registration name
	auto rn = dbus_bus_request_name(connect, name.c_str(), DBUS_NAME_FLAG_DO_NOT_QUEUE, &err);

	// check registration
	if (rn != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
	{
		return EXIT_FAILURE;
	}

	DBusObjectPathVTable vtable = {.message_function = messageFunction};

	// request registration path
	auto rp = dbus_connection_register_object_path(connect, path.c_str(), &vtable, this);

	// check registration
	if (!rp)
	{
		return EXIT_FAILURE;
	}

	// create glib event loop
	mainloop = g_main_loop_new(NULL, false);

	// set up the DBus connection to work in a GLib event loop
	dbus_connection_setup_with_g_main(connect, NULL);

	// start the glib event loop
	g_main_loop_run(mainloop);

	return EXIT_SUCCESS;
}
