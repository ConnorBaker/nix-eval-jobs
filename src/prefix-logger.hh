#pragma once
///@file

#include <nix/util/logging.hh>

#include <memory>
#include <string>

class PrefixLogger : public nix::Logger {
    std::unique_ptr<nix::Logger> wrapped;
    std::string attrPath;

  public:
    explicit PrefixLogger(std::unique_ptr<nix::Logger> wrapped);

    void setAttrPath(std::string path);

    void stop() override;
    void pause() override;
    void resume() override;
    bool isVerbose() override;

    void log(nix::Verbosity lvl, std::string_view msg) override;
    void logEI(const nix::ErrorInfo &info) override;

    void startActivity(nix::ActivityId act, nix::Verbosity lvl,
                       nix::ActivityType type, const std::string &text,
                       const Fields &fields, nix::ActivityId parent) override;
    void stopActivity(nix::ActivityId act) override;
    void result(nix::ActivityId act, nix::ResultType type,
                const Fields &fields) override;

    void writeToStdout(std::string_view data) override;
    std::optional<char> ask(std::string_view msg) override;
    void setPrintBuildLogs(bool printBuildLogs) override;
};
