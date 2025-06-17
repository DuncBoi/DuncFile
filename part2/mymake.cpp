#include <iostream>
#include <string>
#include <exception>
#include "exception.h"
#include "maker.h"

static void try_make(const mymake::Maker& maker, const char *target = nullptr)
{

	try {
		if (target)
			maker.make(target);
		else
			maker.make();
	}
	catch (const mymake::exception::NonFatal& e) {
		std::cout << e.what() << std::endl;
	}
}

int main(int argc, char **argv)
{
	try {
		const mymake::Maker maker;

		if (argc > 1 && std::string(argv[1]) == "-p") {
			std::cout << maker << std::flush;
			return 0;
		}

		if (argc > 1) {
			for (++argv; *argv; ++argv)
				try_make(maker, *argv);
		} else
			try_make(maker);
	}
	catch (const mymake::exception::TargetCommandFailed& e) {
		std::cerr << e.what() << std::endl;
		return e.exit_status();
	}
	catch (const mymake::exception::Fatal& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	catch (const std::exception& e) {
		std::cerr << "mymake: *** Uncaught exception: " << e.what() << ".  Stop." << std::endl;
		return 1;
	}

	return 0;
}
