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
    [[nodiscard]] auto getAttrPath() const -> const std::string &;

    void stop() override;
    void pause() override;
    void resume() override;
    bool isVerbose() override;

    void log(nix::Verbosity lvl, std::string_view s) override;
    void logEI(const nix::ErrorInfo &ei) override;

    void startActivity(nix::ActivityId act, nix::Verbosity lvl,
                       nix::ActivityType type, const std::string &s,
                       const Fields &fields, nix::ActivityId parent) override;
    void stopActivity(nix::ActivityId act) override;
    void result(nix::ActivityId act, nix::ResultType type,
                const Fields &fields) override;

    void writeToStdout(std::string_view s) override;
    std::optional<char> ask(std::string_view s) override;
    void setPrintBuildLogs(bool printBuildLogs) override;
};
