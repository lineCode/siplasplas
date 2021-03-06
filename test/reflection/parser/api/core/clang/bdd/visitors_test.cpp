#include <catch.hpp>
#include "../clangtest.hpp"
#include <siplasplas/reflection/parser/api/core/clang/visitor.hpp>
#include <siplasplas/reflection/parser/api/core/clang/recursivevisitor.hpp>
#include <siplasplas/reflection/parser/api/core/clang/kindvisitor.hpp>
#include <iostream>

using namespace ::cpp::reflection::parser::api::core::clang;

class CounterVisitor : public Visitor
{
public:
    bool visit(const Cursor& root) const
    {
        _count = 0;
        return Visitor::visit(root);
    }

    bool visit(const Cursor& root)
    {
        _count = 0;
        return Visitor::visit(root);
    }

    Visitor::Result onCursor(visitor_tags::Simple tag, const Cursor& current, const Cursor& parent) const override
    {
        _count++;
        std::cout << current << std::endl;
        return Visitor::onCursor(tag, current, parent);
    }

    Visitor::Result onCursor(visitor_tags::Simple tag, const Cursor& current, const Cursor& parent) override
    {
        _count++;
        std::cout << current << std::endl;
        return Visitor::onCursor(tag, current, parent);
    }

    std::size_t count() const
    {
        return _count;
    }

private:
    mutable std::size_t _count = 0;
};

SCENARIO("Visitors abort AST traversal by default")
{
    GIVEN("An index and a C++11 sourcefile to parse")
    {
        Index index;
        cpp::test::ClangTest::helloWorld("visitors_test.cpp");

        REQUIRE(!index.isNull());

        WHEN("The sourcefile is parsed with clang include dirs and C++11 option")
        {
            TranslationUnit tu = index.parse("visitors_test.cpp", CompileOptions()
                .I(SIPLASPLAS_LIBCLANG_INCLUDE_DIR)
                .I(SIPLASPLAS_LIBCLANG_SYSTEM_INCLUDE_DIR)
                .std("c++11")
            );

            REQUIRE(!tu.isNull());
            REQUIRE(tu.spelling().str().str() == "visitors_test.cpp");

            THEN("The visitation of the translation unit is aborted inmediately using default visitors")
            {
                CounterVisitor visitor;
                const CounterVisitor constVisitor;

                REQUIRE(tu.cursor().kind() == CursorKind::Kind::TranslationUnit);
                REQUIRE(tu.cursor().kind().str().str() == "TranslationUnit");

                REQUIRE(visitor.visit(tu.cursor()));
                REQUIRE(visitor.count() == 1);
                REQUIRE(constVisitor.visit(tu.cursor()));
                REQUIRE(constVisitor.count() == 1);
            }
        }
    }
}

SCENARIO("Kind visitors visit nodes of a specific kind only")
{
    GIVEN("An index and a C++11 sourcefile to parse")
    {
        Index index;
        cpp::test::ClangTest::writeFile("visitors_test.hpp",
        {
"#include <chrono>",
"",
"namespace foo",
"{",
"    class Foo {};",
"    namespace bar {}",
"    namespace quux {};",
"    namespace foobar",
"    {",
"        namespace foobarquux {}",
"    }",
"}"});

        REQUIRE(!index.isNull());

        WHEN("The sourcefile is parsed with clang include dirs and C++11 option")
        {
            TranslationUnit tu = index.parse("visitors_test.hpp", CompileOptions()
                .I(SIPLASPLAS_LIBCLANG_INCLUDE_DIR)
                .I(SIPLASPLAS_LIBCLANG_SYSTEM_INCLUDE_DIR)
                .std("c++11")
            );

            REQUIRE(!tu.isNull());
            REQUIRE(tu.spelling().str().str() == "visitors_test.hpp");

            THEN("A namespace kind visitor visits namespaces only")
            {
                std::unordered_map<std::string, bool> namespaces = {
                    { "foo", false },
                    { "bar", false },
                    { "quux", false },
                    { "foobar", false },
                    { "foobarquux", false },
                };

                class NamespaceVisitor : public KindVisitor<
                    CursorKind::Kind::Namespace,
                    RecursiveVisitor
                >
                {
                public:
                    NamespaceVisitor(std::unordered_map<std::string, bool>& namespaces) :
                        namespaces(namespaces)
                    {}

                    Visitor::Result onCursor(
                        visitor_tags::Kind<CursorKind::Kind::Namespace>,
                        const Cursor& current, const Cursor& parent) override
                    {
                        REQUIRE(current.kind() == CursorKind::Kind::Namespace);
                        std::cout << "On " << current << " (Parent: " << parent << ")" << std::endl;

                        auto it = namespaces.find(current.spelling().str().str());

                        if(it != namespaces.end())
                        {
                            it->second = true;
                        }

                        return Visitor::Result::Continue;
                    }

                private:
                    std::unordered_map<std::string, bool>& namespaces;
                };

                NamespaceVisitor visitor{namespaces};
                visitor.visit(tu.cursor());

                for(const auto& keyValue : namespaces)
                {
                    const auto& namespace_ = keyValue.first;
                    const auto& visited    = keyValue.second;

                    REQUIRE(visited);
                }
            }
        }
    }
}
