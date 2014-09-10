#ifndef UTILS_CODEPREPROCESSOR_H
#define UTILS_CODEPREPROCESSOR_H

#include <string>
#include <vector>
#include <memory>
#include <set>

#include "utils/debug.h"
#include "utils/stringUtils.h"

namespace sb {

struct Node
{
    std::string text;
    std::vector<Node> subNodes;

    Node(const Node&) = default;
    Node& operator =(const Node&) = default;
    Node(Node&&) = default;
    Node& operator =(Node&&) = default;

    Node() = default;
    Node(const std::string& line):
        text(line)
    {}

    bool isLine() const { return subNodes.empty(); }
    const std::string& getLine() const
    {
        sbAssert(isLine(), "node is not a line");
        return text;
    }

    const std::string& getIncludeFilename() const
    {
        sbAssert(!isLine(), "node is not an include");
        return text;
    }

    void dumpTree(const std::string &indent = "") const
    {
        if (isLine()) {
            gLog.trace("%s%s", indent.c_str(), getLine().c_str());
            return;
        }

        gLog.trace("%s+ %s", indent.c_str(), getIncludeFilename().c_str());
        for (const Node& node: subNodes) {
            node.dumpTree(indent + "  ");
        }
    }
};

inline std::ostream& operator <<(std::ostream& os,
                                 const Node& node)
{
    return os << node.subNodes.size() << " subnodes, "
              << (node.isLine() ? "line: " : "include: ") << node.text;
}

Node loadPreprocessed(const std::string &filename,
                      std::set<std::string> &includedFiles);

struct BacktraceNode
{
    std::string file;
    size_t line;
};

template<typename T>
struct NodeIterator
{
    T* node;
    size_t index;
    std::unique_ptr<NodeIterator<T>> nextLevel;

    //NodeIterator(): node(nullptr), index(0) {}
    NodeIterator(T& node):
        node(&node),
        index(0),
        nextLevel(node.subNodes.empty()
                      ? nullptr
                      : new NodeIterator(node.subNodes.front()))
    {
    }
    NodeIterator(T& node, std::nullptr_t):
        node(&node),
        index(-1),
        nextLevel(nullptr)
    {
    }

    NodeIterator(const NodeIterator& copy)
    {
        *this = copy;
    }

    NodeIterator& operator =(const NodeIterator& copy) {
        node = copy.node;
        index = copy.index;
        nextLevel.reset(copy.nextLevel ? new NodeIterator(*copy.nextLevel)
                                       : nullptr);
        return *this;
    }

    NodeIterator(NodeIterator&&) = default;
    NodeIterator& operator =(NodeIterator&&) = default;

    bool valid() const {
        return (index != (size_t)-1)
                && (!nextLevel
                    || (node && index < node->subNodes.size()));
    }

    void invalidate() {
        //gLog.debug("invalidating");
        index = (size_t)-1;
        nextLevel.reset(nullptr);
    }

    NodeIterator& operator ++()
    {
        //gLog.debug("%d: node %s", (int)index, node->text.c_str());
        sbAssert(valid(), "node not valid");

        if (!nextLevel) {
            //gLog.trace("no next level: %s", node->text.c_str());
            invalidate();
            return *this;
        }

        if (nextLevel->valid()) {
            ++*nextLevel;
        }

        while (!nextLevel->valid()) {
            ++index;

            if (!valid()) {
                //gLog.trace("no longer valid: %s", node->text.c_str());
                invalidate();
                return *this;
            }

            nextLevel.reset(new NodeIterator(node->subNodes[index]));
        }

        return *this;
    }

    NodeIterator operator ++(int)
    {
        //gLog.trace("post++: %s", node->text.c_str());
        NodeIterator ret = *this;
        ++*this;
        return ret;
    }

    T* operator ->() {
        if (!nextLevel) {
            return node;
        }
        return nextLevel->operator ->();
    }

    T& operator *() const {
        if (!nextLevel) {
            return *node;
        }
        return **nextLevel;
    }

    bool operator ==(const NodeIterator& i) const
    {
        //gLog.debug("op ==: node %p, i.node %p; nextLevel %p, i.nextLevel %p",
                   //node, i.node, nextLevel.get(), i.nextLevel.get());
        return node == i.node
                && index == i.index
                && ((!nextLevel && !i.nextLevel)
                    || (nextLevel && i.nextLevel && *nextLevel == *i.nextLevel));
    }

    bool operator !=(const NodeIterator& i) const
    {
        return !(*this == i);
    }

    std::vector<BacktraceNode> getBacktrace() const
    {
        if (node->isLine()) {
            return {};
        }

        sbAssert(nextLevel != nullptr,
                 "nextLevel must not be null on backtraceable include nodes");

        std::vector<BacktraceNode> ret = nextLevel->getBacktrace();
        ret.push_back({ node->getIncludeFilename(), index + 1 });

        return ret;
    }
};

inline std::ostream& operator <<(std::ostream& os,
                                 const BacktraceNode& node)
{
    return os << node.line << ':' << node.line;
}

template<typename T>
std::ostream& operator <<(std::ostream& os,
                          const NodeIterator<T>& it)
{
    auto backtrace = it.getBacktrace();
    return os << utils::join(backtrace, "\n");
}

class PreprocessedCode
{
public:
    PreprocessedCode(const std::string &filename)
    {
        std::set<std::string> includedFiles;
        root = loadPreprocessed(filename, includedFiles);

        initNumLines();
    }

    typedef NodeIterator<Node> iterator;
    typedef NodeIterator<const Node> const_iterator;
    iterator begin() { return iterator(root); }
    const_iterator begin() const { return const_iterator(root); }

    iterator end() { return iterator(root, nullptr); }
    const_iterator end() const { return const_iterator(root, nullptr); }

    const_iterator findLine(size_t lineNumber) const
    {
        const_iterator it = begin();
        while (lineNumber-- > 0) {
            ++it;
        }
        return it;
    }

    std::string getCode() const
    {
        std::vector<std::string> lines;

        for (const auto& node: *this) {
            if (node.isLine()) {
                lines.push_back(node.getLine());
            }
        }

        return utils::join(lines, "\n");
    }

    size_t getNumLines() const {
        return numLines;
    }

    void dumpTree() const {
        root.dumpTree();
    }

private:
    void initNumLines() {
        numLines = 0;
        for (const auto& node: *this) {
            //gLog.debug("node: %s", node.text.c_str());
            if (node.isLine()) {
                //gLog.debug("line node: %s", node.text.c_str());
                ++numLines;
            }
        }
    }

    Node root;
    size_t numLines;
};

} // namespace sb

#endif // UTILS_CODEPREPROCESSOR_H

