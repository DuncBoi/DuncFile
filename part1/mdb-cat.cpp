/*
 * mdb-cat.cpp
 */

#include <iostream>
#include <iomanip>
#include <regex>
#include <cstdlib>

#include "mdb.h"

/*
 * DO NOT MODIFY this file.
 */

int main(int argc, char **argv)
{
    /*
     * open each database file specified in the command line
     */

    if (argc < 2) {
        std::cerr << "usage: mdb-cat <database_files...>" << std::endl;
        std::exit(1);
    }

    int ret = 0;

    for (++argv; *argv; ++argv) {

        try {
            const MdbReader mdb(*argv);

            /*
             * dump entries
             */

            // traverse the Mdb, printing all matched records
            for (size_t i = 0; i < mdb.size(); ++i)
                std::cout << std::setw(4) << i + 1 << ": " << mdb[i] << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "mdb-cat: " << *argv << ": " << e.what() << std::endl;
            ret = 1;
        }

    }

    return ret;
}
