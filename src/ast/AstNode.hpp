#pragma once
#include "../common/SourceLocation.hpp"

class AstVisitor;

class AstNode
{

public:
    SourceLocation loc;
    explicit AstNode(SourceLocation l = {0, 0}) : loc(l) {}
    virtual ~AstNode() = default;
    virtual void accept(AstVisitor& visitor) = 0;
};


