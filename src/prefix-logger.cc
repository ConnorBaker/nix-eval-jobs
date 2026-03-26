#include "prefix-logger.hh"

#include <format>
#include <memory>
#include <nix/util/error.hh>
#include <nix/util/logging.hh>
#include <nix/util/terminal.hh>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

PrefixLogger::PrefixLogger(std::unique_ptr<nix::Logger> wrapped)
    : wrapped(std::move(wrapped)) {}

void PrefixLogger::setAttrPath(std::string path) { attrPath = std::move(path); }

auto PrefixLogger::drainLogs() -> Logs {
    Logs logs{
        .traces = std::move(traceBuffer),
        .warnings = std::move(warningBuffer),
    };
    traceBuffer.clear();
    warningBuffer.clear();
    return logs;
}

void PrefixLogger::stop() { wrapped->stop(); }
void PrefixLogger::pause() { wrapped->pause(); }
void PrefixLogger::resume() { wrapped->resume(); }
auto PrefixLogger::isVerbose() -> bool { return wrapped->isVerbose(); }

void PrefixLogger::log(nix::Verbosity lvl, std::string_view msg) {
    if (!attrPath.empty()) {
        auto filtered = nix::filterANSIEscapes(msg, true);
        if (filtered.starts_with("trace:")) {
            traceBuffer.emplace_back(std::move(filtered));
        } else {
            warningBuffer.emplace_back(std::move(filtered));
        }
        wrapped->log(lvl, std::format("{}: {}", attrPath, msg));
    } else {
        wrapped->log(lvl, msg);
    }
}

void PrefixLogger::logEI(const nix::ErrorInfo &info) {
    if (!attrPath.empty()) {
        std::ostringstream oss;
        nix::showErrorInfo(oss, info, nix::loggerSettings.showTrace.get());
        warningBuffer.emplace_back(nix::filterANSIEscapes(oss.str(), true));
        auto modified = info;
        modified.traces.push_front(nix::Trace{
            .pos = {},
            .hint = nix::HintFmt("while evaluating attribute '%s'", attrPath),
            .print = nix::TracePrint::Always,
        });
        wrapped->logEI(modified);
    } else {
        wrapped->logEI(info);
    }
}

void PrefixLogger::startActivity(nix::ActivityId act, nix::Verbosity lvl,
                                 nix::ActivityType type,
                                 const std::string &text, const Fields &fields,
                                 nix::ActivityId parent) {
    if (attrPath.empty()) {
        wrapped->startActivity(act, lvl, type, text, fields, parent);
    } else {
        wrapped->startActivity(act, lvl, type,
                               std::format("{}: {}", attrPath, text), fields,
                               parent);
    }
}

void PrefixLogger::stopActivity(nix::ActivityId act) {
    wrapped->stopActivity(act);
}

void PrefixLogger::result(nix::ActivityId act, nix::ResultType type,
                          const Fields &fields) {
    wrapped->result(act, type, fields);
}

void PrefixLogger::writeToStdout(std::string_view data) {
    wrapped->writeToStdout(data);
}

auto PrefixLogger::ask(std::string_view msg) -> std::optional<char> {
    return wrapped->ask(msg);
}

void PrefixLogger::setPrintBuildLogs(bool printBuildLogs) {
    wrapped->setPrintBuildLogs(printBuildLogs);
}
