#pragma once

class ProgramNode;
class BlockStmtNode;
class VarDeclStmtNode;
class AssignStmtNode;
class OutputStmtNode;
class InputStmtNode;
class IfStmtNode;
class WhileStmtNode;
class ForStmtNode;
class BreakStmtNode;
class ContinueStmtNode;

class LiteralExprNode;
class IdentifierExprNode;
class UnaryExprNode;
class BinaryExprNode;

class AstVisitor {
public:
    virtual ~AstVisitor() = default;

    virtual void visit(ProgramNode& node) = 0;
    virtual void visit(BlockStmtNode& node) = 0;

    virtual void visit(VarDeclStmtNode& node) = 0;
    virtual void visit(AssignStmtNode& node) = 0;
    virtual void visit(OutputStmtNode& node) = 0;
    virtual void visit(InputStmtNode& node) = 0;
    virtual void visit(IfStmtNode& node) = 0;
    virtual void visit(WhileStmtNode& node) = 0;
    virtual void visit(ForStmtNode& node) = 0;
    virtual void visit(BreakStmtNode& node) = 0;
    virtual void visit(ContinueStmtNode& node) = 0;

    virtual void visit(LiteralExprNode& node) = 0;
    virtual void visit(IdentifierExprNode& node) = 0;
    virtual void visit(UnaryExprNode& node) = 0;
    virtual void visit(BinaryExprNode& node) = 0;
};