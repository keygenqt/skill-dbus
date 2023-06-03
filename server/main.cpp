#include "test_dbus.h"

int main(void)
{
	auto serverTestDbus = TestDbus();
	serverTestDbus.run();

	return 0;
}
