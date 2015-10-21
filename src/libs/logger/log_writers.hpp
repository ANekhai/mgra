//***************************************************************************
//* Copyright (c) 2011-2014 Saint-Petersburg Academic University
//* All Rights Reserved
//* See file LICENSE for details.
//****************************************************************************

#pragma once
#include "io/path_helper.hpp"
#include "logger.hpp"

#include <fstream>
#include <iostream>

namespace logging {

struct console_writer  : public writer {

  void write_msg(double time, size_t max_rss, level l, const char* file, size_t line_num, const char* source, const char* msg) {
    std::cout
        << str(boost::format("%14s %5s %6s %-24.24s (%-26.26s:%4d)   %s")
               % human_readable_time(time) % human_readable_memory(max_rss) % logging::level_name(l)
               % source % path::filename(file) % int(line_num) % msg)
        << std::endl;
  }

};

struct file_writer  : public writer {

	file_writer(std::string const & filename) 
	: out(filename.c_str())
	{
	}

  void write_msg(double time, size_t max_rss, level l, const char* file, size_t line_num, const char* source, const char* msg) {
    out << str(boost::format("%14s %5s %6s %-24.24s (%-26.26s:%4d)   %s")
               % human_readable_time(time) % human_readable_memory(max_rss) % logging::level_name(l)
               % source % path::filename(file) % int(line_num) % msg)
        << std::endl;
  }

private: 
	std::ofstream out;
};

} // logging
