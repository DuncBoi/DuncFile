/*
 * mdb.h
 */

#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <system_error>
#include "xdb.h"

/*
 * DO NOT MODIFY this file.
 */

#define die(msg) \
    throw std::system_error(errno, std::generic_category(), msg)

struct MdbRec {
    char name[16];
    char  msg[24];

    MdbRec() { name[0] = msg[0] = '\0'; }
};

std::ostream& operator<<(std::ostream& output, const MdbRec& rec);
std::ofstream& operator<<(std::ofstream& output, const MdbRec& rec);
std::ifstream& operator>>(std::ifstream& output, MdbRec& rec);

using MdbReader = xdb::XdbReader<MdbRec>;
using MdbWriter = xdb::XdbWriter<MdbRec>;
using Mdb = xdb::Xdb<MdbRec>;
