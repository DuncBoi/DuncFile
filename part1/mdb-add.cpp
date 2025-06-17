/*
 * mdb-add.cpp
 */

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include "mdb.h"

/*
 * DO NOT MODIFY this file.
 */

static void sanitize(char *s)
{
    while (*s) {
        if (!isprint(*s))
            *s = ' ';
        s++;
    }
}

int main(int argc, char **argv)
{
    /*
     * open the database file specified in the command line
     */

    if (argc != 2) {
        std::cerr << "usage: mdb-add <database_file>" << std::endl;
        std::exit(1);
    }

    char *filename = argv[1];
    try {
        Mdb mdb(filename);

        /*
         * read name
         */

        struct MdbRec r;
        std::string line;

        std::cout << "name please (will truncate to " << sizeof(r.name)-1 << " chars): " << std::flush;
        if (std::getline(std::cin, line).eof())
            throw std::runtime_error("could not read name");
        else if (!std::cin.good())
            die("std::getline");
        // must null-terminate the string manually after strncpy().
        std::strncpy(r.name, line.c_str(), sizeof(r.name) - 1);
        r.name[sizeof(r.name) - 1] = '\0';

        /*
         * read msg
         */

        std::cout << "msg please (will truncate to " << sizeof(r.msg)-1 << " chars): " << std::flush;
        if (std::getline(std::cin, line).eof())
            throw std::runtime_error("could not read msg");
        else if (!std::cin.good())
            die("std::getline");
        // must null-terminate the string manually after strncpy().
        std::strncpy(r.msg, line.c_str(), sizeof(r.msg) - 1);
        r.msg[sizeof(r.msg) - 1] = '\0';

        /*
         * add the record to Mdb object and print confirmation
         */

        // remove non-printable chars from the strings
        sanitize(r.name);
        sanitize(r.msg);

        std::cout << std::setw(4) << (mdb << r).size() << ": " << r << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "mdb-add: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
