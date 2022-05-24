#include "mustache.h"
#include <iostream>

#include "util.h"

namespace pico {
namespace mustache {
mustache::RenderedTemplate template_t::render() const {
    context ctx;
    std::vector<context*> stack;
    stack.push_back(&ctx);

    std::string out;
    render_internal(0, m_fragments.size() - 1, stack, out, 0);
    return RenderedTemplate(std::move(out));
}

mustache::RenderedTemplate template_t::render(context& ctx) const {
    std::vector<context*> stack;
    stack.push_back(&ctx);

    std::string out;
    render_internal(0, m_fragments.size() - 1, stack, out, 0);
    return RenderedTemplate(std::move(out));
}

std::string template_t::render_string() const {
    context ctx;
    std::vector<context*> stack;
    stack.push_back(&ctx);

    std::string out;
    render_internal(0, m_fragments.size() - 1, stack, out, 0);
    return out;
}

std::string template_t::render_string(context& ctx) const {
    std::vector<context*> stack;
    stack.push_back(&ctx);

    std::string out;
    render_internal(0, m_fragments.size() - 1, stack, out, 0);
    return out;
}

std::string template_t::tag_name(const Action& action) const {
    return m_template.substr(action.start, action.end - action.start);
}

auto template_t::find_context(const std::string& name, const std::vector<context*>& stack,
                              bool shouldUseOnlyFirstStackValue) const
    -> std::pair<bool, context&> {
    if (name == ".") { return {true, *stack.back()}; }

    static Json::Value empty_value = Json::stringValue;

    int pos = name.find('.');
    if (pos == static_cast<int>(std::string::npos)) {
        for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
            if ((*it)->type() == Json::objectValue && (*it)->isMember(name)) {
                return {true, (**it)[name]};
            }
        }
    }
    else {
        std::vector<int> dot_positions;
        dot_positions.push_back(-1);
        while (pos != static_cast<int>(std::string::npos)) {
            dot_positions.push_back(pos);
            pos = name.find('.', pos + 1);
        }
        dot_positions.push_back(name.size());


        std::vector<std::string> names;
        names.reserve(dot_positions.size() - 1);
        for (int i = 1; i < static_cast<int>(dot_positions.size()); ++i) {
            names.emplace_back(
                name.substr(dot_positions[i - 1] + 1, dot_positions[i] - dot_positions[i - 1] - 1));
        }


        for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
            context* current_ctx = *it;
            bool found = true;
            for (auto& name : names) {
                if (current_ctx->type() == Json::objectValue && current_ctx->isMember(name)) {
                    current_ctx = &(*current_ctx)[name];
                }
                else {
                    if (shouldUseOnlyFirstStackValue) { return {false, empty_value}; }
                    found = false;
                    break;
                }
            }
            if (found) { return {true, *current_ctx}; }
        }
    }
    return {false, empty_value};
}

void template_t::escape(const std::string& in, std::string& out) const {
    for (auto c : in) {
        switch (c) {
        case '&': out += "&amp;"; break;
        case '<': out += "&lt;"; break;
        case '>': out += "&gt;"; break;
        case '"': out += "&quot;"; break;
        case '\'': out += "&#39;"; break;
        case '/': out += "&#x2F;"; break;
        case '`': out += "&#x60;"; break;
        case '=': out += "&#x3D;"; break;
        default: out += c; break;
        }
    }
}

bool template_t::isTagInsideObjectBlock(const int& current,
                                        const std::vector<context*>& stack) const {
    int openedBlock = 0;
    int totalBlocksBefore = 0;
    for (int i = current; i > 0; --i) {
        ++totalBlocksBefore;
        auto& action = m_actions[i - 1];

        if (action.type == ActionType::OpenBlock) {
            if (openedBlock == 0 && (*stack.rbegin())->type() == Json::objectValue) { return true; }
            --openedBlock;
        }
        else if (action.type == ActionType::CloseBlock) {
            ++openedBlock;
        }
    }

    return false;
}

void template_t::render_fragment(const std::pair<int, int> fragment, int indent,
                                 std::string& out) const {
    if (indent) {
        for (int i = fragment.first; i < fragment.second; i++) {
            out += m_template[i];
            if (m_template[i] == '\n' && i + 1 != static_cast<int>(m_template.size()))
                out.insert(out.size(), indent, ' ');
        }
    }
    else
        out.insert(out.size(), m_template, fragment.first, fragment.second - fragment.first);
}

void template_t::render_internal(int action_start, int action_end, std::vector<context*>& stack,
                                 std::string& out, int indent) const {
    int current = action_start;

    if (indent) { out.insert(out.size(), indent, ' '); }

    while (current < action_end) {
        auto& fragment = m_fragments[current];
        auto& action = m_actions[current];
        render_fragment(fragment, indent, out);
        switch (action.type) {
        case ActionType::Ignore:
            // do nothing
            break;
        case ActionType::Partial:
        {
            std::string partial_name = tag_name(action);
            auto partial_tpl = load(partial_name);
            int partial_indent = action.pos;
            partial_tpl.render_internal(
                0, partial_tpl.m_actions.size() - 1, stack, out, partial_indent);

        } break;
        case ActionType::UnescapeTag:
        case ActionType::Tag:
        {
            bool shouldUseOnlyFirstStackValue = false;
            if (isTagInsideObjectBlock(current, stack)) { shouldUseOnlyFirstStackValue = true; }
            auto optional_ctx = find_context(tag_name(action), stack, shouldUseOnlyFirstStackValue);
            auto& ctx = optional_ctx.second;
            switch (ctx.type()) {
            case Json::stringValue:
            {
                if (action.type == ActionType::Tag)
                    escape(ctx.asString(), out);
                else
                    out += ctx.asString();
            } break;
            case Json::intValue:
            case Json::uintValue:
            case Json::realValue:
            case Json::booleanValue:
            {
                out += Json2Str(ctx);
            } break;
            case Json::nullValue:
            {
                out += "";
            } break;
            default: throw std::runtime_error("Unsupported type " + std::to_string(ctx.type()));
            }
        } break;
        case ActionType::ElseBlock:
        {
            static context null_ctx;
            auto optional_ctx = find_context(tag_name(action), stack);
            if (!optional_ctx.first) {
                stack.emplace_back(&null_ctx);
                break;
            }
            auto& ctx = optional_ctx.second;
            switch (ctx.type()) {
            case Json::arrayValue:
                if (ctx.size()) { current = action.pos; }
                else {
                    stack.emplace_back(&null_ctx);
                }
                break;
            case Json::nullValue: stack.emplace_back(&null_ctx); break;
            case Json::booleanValue:
                if (!ctx.asBool()) { stack.emplace_back(&null_ctx); }
                else {
                    current = action.pos;
                }
                break;
            default: current = action.pos; break;
            }
        } break;
        case ActionType::OpenBlock:
        {
            auto opential_ctx = find_context(tag_name(action), stack);
            if (!opential_ctx.first) {
                current = action.pos;
                break;
            }
            auto& ctx = opential_ctx.second;
            switch (ctx.type()) {
            case Json::arrayValue:
                if (ctx.size()) {
                    for (auto& item : ctx) {
                        stack.push_back(&item);
                        render_internal(current + 1, action.pos, stack, out, indent);
                        stack.pop_back();
                    }
                }
                current = action.pos;
                break;
            case Json::objectValue:
            case Json::stringValue:
            case Json::intValue:
            case Json::uintValue:
            case Json::realValue: stack.push_back(&ctx); break;
            case Json::nullValue: current = action.pos; break;
            case Json::booleanValue:
                if (ctx.asBool()) { stack.push_back(&ctx); }
                else {
                    current = action.pos;
                }
                break;
            default:
                throw std::runtime_error("Unsupported type +" + std::to_string(ctx.type()));
                break;
            }
        } break;
        case ActionType::CloseBlock:
        {
            stack.pop_back();
        } break;
        default:
            throw std::runtime_error("Unsupported type +" +
                                     std::to_string(static_cast<int>(action.type)));
        }
        ++current;
    }
    auto& fragment = m_fragments[action_end];
    render_fragment(fragment, indent, out);
}

void template_t::parse() {
    std::string tag_open = "{{";
    std::string tag_close = "}}";

    std::vector<int> block_positions;

    size_t current = 0;

    while (1) {
        size_t index_open = m_template.find(tag_open, current);
        if (index_open == std::string::npos) {
            m_fragments.emplace_back(current, m_template.size());
            m_actions.emplace_back(ActionType::Ignore, 0, 0);
            break;
        }
        m_fragments.emplace_back(current, index_open);
        index_open += tag_open.size();
        size_t index_close = m_template.find(tag_close, index_open);
        if (index_close == index_open) { throw InvalidTemplateException("Invalid template: {{}}"); }
        if (index_close == std::string::npos) {
            throw InvalidTemplateException("not found close tag");
        }

        current = index_close + tag_close.size();
        switch (m_template[index_open]) {
        case '#':
        {
            index_open++;
            while (m_template[index_open] == ' ') { index_open++; }
            while (m_template[index_close - 1] == ' ') { index_close--; }
            block_positions.emplace_back(m_actions.size());
            m_actions.emplace_back(ActionType::OpenBlock, index_open, index_close);
        } break;
        case '/':
        {
            index_open++;
            while (m_template[index_open] == ' ') { index_open++; }
            while (m_template[index_close - 1] == ' ') { index_close--; }
            {
                auto& matched = m_actions[block_positions.back()];
                if (m_template.compare(index_open,
                                       index_close - index_open,
                                       m_template,
                                       matched.start,
                                       matched.end - matched.start) != 0) {
                    throw InvalidTemplateException(
                        "not matched {{# {{/ pair: " +
                        m_template.substr(matched.start, matched.end - matched.start) + ", " +
                        m_template.substr(index_open, index_close - index_open));
                }
                matched.pos = m_actions.size();
            }
            m_actions.emplace_back(
                ActionType::CloseBlock, index_open, index_close, block_positions.back());
            block_positions.pop_back();
        } break;
        case '^':
        {
            index_open++;
            while (m_template[index_open] == ' ') { index_open++; }
            while (m_template[index_close - 1] == ' ') { index_close--; }
            block_positions.emplace_back(m_actions.size());
            m_actions.emplace_back(ActionType::ElseBlock, index_open, index_close);
        } break;
        case '!':
            // ignore
            m_actions.emplace_back(ActionType::Ignore, index_open + 1, index_close);
            break;
        case '>':
        {
            index_open++;
            while (m_template[index_open] == ' ') { index_open++; }
            while (m_template[index_close - 1] == ' ') { index_close--; }
            m_actions.emplace_back(ActionType::Partial, index_open, index_close);
        } break;
        case '{':
        {
            if (tag_open != "{{" || tag_close != "}}") {
                throw InvalidTemplateException("cannot use triple mustache when delimiter changed");
            }
            index_open++;
            if (m_template[index_close + 2] != '}') {
                throw InvalidTemplateException("{{{: }}} not matched");
            }
            while (m_template[index_open] == ' ') { index_open++; }
            while (m_template[index_close - 1] == ' ') { index_close--; }
            m_actions.emplace_back(ActionType::UnescapeTag, index_open, index_close);
            current++;
        } break;
        case '&':
        {
            index_open++;
            while (m_template[index_open] == ' ') { index_open++; }
            while (m_template[index_close - 1] == ' ') { index_close--; }
            m_actions.emplace_back(ActionType::UnescapeTag, index_open, index_close);
        } break;
        case '=':
        {
            index_open++;
            m_actions.emplace_back(ActionType::Ignore, index_open, index_close);
            index_close--;
            if (m_template[index_close] != '=') {
                throw InvalidTemplateException(
                    "{{=: not matching = tag: " +
                    m_template.substr(index_open, index_close - index_open));
            }
            index_close--;
            while (m_template[index_open] == ' ') { index_open++; }
            while (m_template[index_close] == ' ') { index_close--; }
            index_close++;
            {
                bool success = false;
                for (size_t i = index_open; i < index_close; i++) {
                    if (m_template[i] == ' ') {
                        tag_open = m_template.substr(index_open, i - index_open);
                        while (m_template[i] == ' ') { i++; }
                        tag_close = m_template.substr(i, index_close - i);
                        if (tag_open.empty()) {
                            throw InvalidTemplateException("{{=: tag open is empty");
                        }
                        if (tag_close.empty()) {
                            throw InvalidTemplateException("{{=: tag close is empty");
                        }
                        if (tag_close.find(" ") != std::string::npos) {
                            throw InvalidTemplateException(
                                "{{=: invalid open/close tag: " + tag_open + " " + tag_close);
                        }
                        success = true;
                        break;
                    }
                }
                if (!success) {
                    throw InvalidTemplateException(
                        "{{=: cannot find space between new open/close tags");
                }
            }

        } break;
        default:
        {
            while (m_template[index_open] == ' ') { index_open++; }
            while (m_template[index_close - 1] == ' ') { index_close--; }
            m_actions.emplace_back(ActionType::Tag, index_open, index_close);
        } break;
        }
    }

    for (int i = m_actions.size() - 2; i >= 0; i--) {
        if (m_actions[i].type == ActionType::Tag || m_actions[i].type == ActionType::UnescapeTag) {
            continue;
        }
        auto& fragment_before = m_fragments[i];
        auto& fragment_after = m_fragments[i + 1];
        bool is_last_action = i == static_cast<int>(m_actions.size()) - 2;
        bool all_space_before = true;

        int j = fragment_before.second - 1, k = fragment_after.first;
        for (; j >= fragment_before.first; j--) {
            if (m_template[j] != ' ') {
                all_space_before = false;
                break;
            }
        }
        if (all_space_before && i > 0) { continue; }
        if (!all_space_before && m_template[j] != '\n') { continue; }

        bool all_space_after = true;
        for (; k < fragment_after.second && k < (int)m_template.size(); k++) {
            if (m_template[k] != ' ') {
                all_space_after = false;
                break;
            }
        }
        if (all_space_after && !is_last_action) { continue; }
        if (!all_space_after &&
            !(m_template[k] == '\n' ||
              (m_template[k] == '\r' && k + 1 < static_cast<int>(m_template.size()) &&
               m_template[k + 1] == '\n'))) {
            continue;
        }
        if (m_actions[i].type == ActionType::Partial) {
            m_actions[i].pos = fragment_before.second - j - 1;
        }
        fragment_before.second = j + 1;
        if (!all_space_after) {
            if (m_template[k] == '\n')
                k++;
            else
                k += 2;
            fragment_after.first = k;
        }
    }
}

pico::ConfigVar<std::string>::Ptr mustache_template_dir = pico::Config::Lookup<std::string>(
    "other.templates.dir", "./templates", "Mustache template directory");

std::string default_loader(const std::string& filename) {
    std::string path = mustache_template_dir->getValue();
    if (!path.empty() && path[path.size() - 1] != '/') { path += '/'; }
    path += filename;
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_WARN("Could not open template file: %s", path.c_str());
        return std::string();
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

}   // namespace mustache
}   // namespace pico