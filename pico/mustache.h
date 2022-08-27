#ifndef __PICO_MUSTACHE_H__
#define __PICO_MUSTACHE_H__

#include <exception>
#include <fstream>
#include <json/json.h>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "config.h"
#include "logging.h"

namespace pico {
namespace mustache {


using context = Json::Value;

enum class ActionType
{
    Ignore,
    Tag,
    UnescapeTag,
    OpenBlock,
    CloseBlock,
    ElseBlock,
    Partial,
};

struct Action
{
    Action(ActionType type, size_t start, size_t end, size_t pos = 0)
        : start(start)
        , end(end)
        , pos(pos)
        , type(type) {}

    int start;
    int end;
    int pos;
    ActionType type;
};

class InvalidTemplateException : public std::exception
{
public:
    explicit InvalidTemplateException(const std::string& msg)
        : msg_(msg) {}
    virtual ~InvalidTemplateException() throw() {}
    virtual const char* what() const throw() { return msg_.c_str(); }

private:
    std::string msg_;
};

class RenderedTemplate
{
public:
    RenderedTemplate()
        : content_type_("text/html") {}
    explicit RenderedTemplate(const std::string& content)
        : content_(std::move(content))
        , content_type_("text/html") {}

    // setter
    void setContentType(const std::string& content_type) { content_type_ = content_type; }
    void setContent(const std::string& content) { content_ = content; }
    // getter
    const std::string& getContent() const { return content_; }
    const std::string& getContentType() const { return content_type_; }

    std::string dump() const { return content_; }

private:
    std::string content_;
    std::string content_type_;
};

class template_t
{
public:
    explicit template_t(std::string body)
        : m_template(std::move(body)) {
        // parse template
        // {{  {{#  {{^  {{>  {{=  {{!
        parse();
    }

    mustache::RenderedTemplate render() const;
    mustache::RenderedTemplate render(context& ctx) const;
    std::string render_string() const;
    std::string render_string(context& ctx) const;

private:
    std::string tag_name(const Action& action) const;
    auto find_context(const std::string& name, const std::vector<context*>& stack,
                      bool shouldUseOnlyFirstStackValue = false) const -> std::pair<bool, context&>;

    void escape(const std::string& str, std::string& out) const;

    bool isTagInsideObjectBlock(const int& current, const std::vector<context*>& stack) const;

    void render_internal(int action_start, int action_end, std::vector<context*>& stack,
                         std::string& out, int indent) const;

    void render_fragment(const std::pair<int, int> fragment, int indent, std::string& out) const;

private:
    void parse();

private:
    std::vector<std::pair<int, int>> m_fragments;
    std::vector<Action> m_actions;
    std::string m_template;
};

template_t load(const std::string& filename);

inline template_t compile(const std::string& body) {
    return template_t(body);
}

std::string default_loader(const std::string& filename);

namespace detail {
inline std::function<std::string(std::string)>& get_loader_ref() {
    static std::function<std::string(std::string)> loader = default_loader;
    return loader;
}
}   // namespace detail

inline void set_loader(std::function<std::string(std::string)> loader) {
    detail::get_loader_ref() = std::move(loader);
}

inline std::string load_text(const std::string& filename) {
    return detail::get_loader_ref()(filename);
}

inline template_t load(const std::string& filename) {
    return compile(detail::get_loader_ref()(filename));
}

}   // namespace mustache
}   // namespace pico

#endif