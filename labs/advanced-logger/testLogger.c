#include <stdio.h>
#include <stdlib.h>
#include "logger.h"

int main(int argc, char **argv)
{
	printf("%d\n", atoi(argv[1]));
	switch (atoi(argv[1]))
	{
	case 1:
		// default logging
		infof("INFO Message %d\n", 1);
		warnf("WARN Message %d\n", 2);
		errorf("ERROR Message %d\n", 2);
		break;
	case 2:
		// stdout logging
		initLogger("stdout");
		infof("INFO Message %d\n", 1);
		warnf("WARN Message %d\n", 2);
		errorf("ERROR Message %d\n", 2);
		break;
	case 3:
		// syslog logging
		initLogger("syslog");
		infof("INFO Message %d\n", 1);
		warnf("WARN Message %d\n", 2);
		errorf("ERROR Message %d\n", 2);
		break;
	default:
		errorf("Invalid test case\n");
		break;
	}

	return 0;
}
