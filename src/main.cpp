#include "nl_ops.h"
#include "util/nl.hpp"
#include "nl_ops.h"
#include "nl/log.hpp"

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

    CLI::App *log = app.add_subcommand("view_logs", "received logs from kernel module");
    log->callback([] {
        lfw::LogListener log;
        log.wait_for_messages();
    });

    CLI11_PARSE(app, argc, argv)

    return 0;
}
