#ifndef COMMAND_BUS_H
#define COMMAND_BUS_H

#include <string>
#include <functional>
#include <unordered_map>

class CommandBus {
private:
    std::unordered_map<std::string, std::function<void()>> commands;

public:
    // 发布命令
    void Publish(const std::string& commandName, const std::function<void()>& action) {
        commands[commandName] = action;
    }

    void Subscribe(const std::string& commandName, const std::function<void()>& action) {
        if (commands.find(commandName) != commands.end()) {
            commands[commandName] = action;
        }
    }

    void Execute(const std::string& commandName) {
        if (commands.find(commandName) != commands.end()) {
            commands[commandName]();
        }
    }
};
#endif // COMMAND_BUS_H