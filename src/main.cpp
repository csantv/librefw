#include "nl_ops.h"
#include "util/nl.hpp"
#include "nl_ops.h"

#include <string>

#include <CLI/CLI.hpp>

int main(int argc, char *argv[])
{
    CLI::App app{"librefw client", "librefw"};
    app.set_version_flag("-V,--version", "0.0.1");
    app.require_subcommand(1);

    CLI::App *bogon = app.add_subcommand("set_bogon", "Set bogon list");

    std::string filename;
    bogon->add_option("-f,--filename", filename, "File containing bogon list")->required(true);
    bogon->callback([&] {
        lfw::nl::sock sk(LFW_NL_FAMILY_NAME);
        sk.send_bogon_list(filename);
    });

    CLI11_PARSE(app, argc, argv)

    return 0;
}
