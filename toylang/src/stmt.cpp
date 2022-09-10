#include <toylang/stmt.hpp>

namespace toylang {
void StmtExpr::accept(Visitor& out) const { out.visit(*this); }
void StmtVar::accept(Visitor& out) const { out.visit(*this); }
void StmtBlock::accept(Visitor& out) const { out.visit(*this); }
void StmtIf::accept(Visitor& out) const { out.visit(*this); }
void StmtWhile::accept(Visitor& out) const { out.visit(*this); }
void StmtBreak::accept(Visitor& out) const { out.visit(*this); }
void StmtFn::accept(Visitor& out) const { out.visit(*this); }
void StmtReturn::accept(Visitor& out) const { out.visit(*this); }
void StmtStruct::accept(Visitor& out) const { out.visit(*this); }
} // namespace toylang
