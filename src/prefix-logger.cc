#include "prefix-logger.hh"

#include <memory>
#include <nix/util/error.hh>

#include <nix/util/logging.hh>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

PrefixLogger::PrefixLogger(std::unique_ptr<nix::Logger> wrapped)
    : wrapped(std::move(wrapped)) {}

void PrefixLogger::setAttrPath(std::string path) { attrPath = std::move(path); }

auto PrefixLogger::getAttrPath() const -> const std::string & {
    return attrPath;
}

void PrefixLogger::stop() { wrapped->stop(); }
void PrefixLogger::pause() { wrapped->pause(); }
void PrefixLogger::resume() { wrapped->resume(); }
auto PrefixLogger::isVerbose() -> bool { return wrapped->isVerbose(); }

void PrefixLogger::log(nix::Verbosity lvl, std::string_view s) {
    if (attrPath.empty()) {
        wrapped->log(lvl, s);
    } else {
        wrapped->log(lvl, "[" + attrPath + "] " + std::string(s));
    }
}

void PrefixLogger::logEI(const nix::ErrorInfo &ei) {
    if (attrPath.empty()) {
        wrapped->logEI(ei);
    } else {
        auto modified = ei;
        modified.traces.push_front(nix::Trace{
            .pos = {},
            .hint = nix::HintFmt("while evaluating attribute '%s'", attrPath),
            .print = nix::TracePrint::Always,
        });
        wrapped->logEI(modified);
    }
}

void PrefixLogger::startActivity(nix::ActivityId act, nix::Verbosity lvl,
                                 nix::ActivityType type, const std::string &s,
                                 const Fields &fields, nix::ActivityId parent) {
    if (attrPath.empty()) {
        wrapped->startActivity(act, lvl, type, s, fields, parent);
    } else {
        wrapped->startActivity(act, lvl, type, "[" + attrPath + "] " + s,
                               fields, parent);
    }
}

void PrefixLogger::stopActivity(nix::ActivityId act) {
    wrapped->stopActivity(act);
}

void PrefixLogger::result(nix::ActivityId act, nix::ResultType type,
                          const Fields &fields) {
    wrapped->result(act, type, fields);
}

void PrefixLogger::writeToStdout(std::string_view s) {
    wrapped->writeToStdout(s);
}

auto PrefixLogger::ask(std::string_view s) -> std::optional<char> {
    return wrapped->ask(s);
}

void PrefixLogger::setPrintBuildLogs(bool printBuildLogs) {
    wrapped->setPrintBuildLogs(printBuildLogs);
}
