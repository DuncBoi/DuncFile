/*
 * mdb.cpp
 */

#include <vector>
#include <fstream>

#include "mdb.h"

/*
 * DO NOT MODIFY this file.
 */

std::ostream& operator<<(std::ostream& output, const MdbRec& rec)
{
    output << "{" << rec.name << "} said {" << rec.msg << "}";
    return output;
}

std::ofstream& operator<<(std::ofstream& output, const MdbRec& rec)
{
    output.write(rec.name, sizeof(rec.name));
    output.write(rec.msg, sizeof(rec.msg));
    return output;
}

std::ifstream& operator>>(std::ifstream& input, MdbRec& rec)
{
    input.read(rec.name, sizeof(rec.name));
    input.read(rec.msg, sizeof(rec.msg));
    return input;
}
