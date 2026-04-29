#include "nl/command.hpp"
#include "nl/hcf.hpp"
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
        lfw::CommandDispatcher cmd;
        cmd.send_bogon_list(filename);
    });

    CLI::App *log = app.add_subcommand("view_logs", "received logs from kernel module");
    log->callback([] {
        lfw::LogListener log;
        log.wait_for_messages();
    });

    bool under_attack = false;
    CLI::App *under_attack_cmd = app.add_subcommand("under_attack", "enable/disable hop count filtering");
    under_attack_cmd->add_flag("--set", under_attack, "enable hop count filtering");
    under_attack_cmd->callback([&under_attack] {
        lfw::CommandDispatcher cmd;
        cmd.set_under_attack(under_attack);
    });

    CLI::App *hcf_cmd = app.add_subcommand("view_hcf", "receive logs from hcf in kernel module");
    hcf_cmd->callback([] {
        lfw::HcfListener hcf;
        hcf.wait_for_messages();
    });

    CLI::App *set_hcf_cmd = app.add_subcommand("set_hcf", "set hcf history");
    set_hcf_cmd->callback([] {
        lfw::HcfListener hcf;
        hcf.set_ip_history();
    });

    CLI11_PARSE(app, argc, argv)

    return 0;
}
