#include <glang_ir/passes.hpp>

#include <glang_ir/passes.hpp>

#include <anton/flat_hash_set.hpp>

#include <glang_core/ilist.hpp>
#include <glang_ir/ir.hpp>
#include <glang_ir/opcodes.hpp>

namespace glang {
  [[nodiscard]] static bool
  coalesce_load_store_load(ir::Instr_load* const load1)
  {
    if(load1->next->instr_kind != ir::Instr_Kind::store) {
      return false;
    }

    ir::Instr_store* const store = static_cast<ir::Instr_store*>(load1->next);
    if(store->next->instr_kind != ir::Instr_Kind::load) {
      return false;
    }

    bool progress = false;
    ir::Instr_load* const load2 = static_cast<ir::Instr_load*>(store->next);
    i64 const var_users = store->dst->referrers.size();
    if(store->src == load1 && load2->src == store->dst && var_users == 2) {
      store->src->remove_referrer(store);
      store->dst->remove_referrer(store);
      load2->src->remove_referrer(load2);
      ilist_erase(store);
      ilist_erase(load2);
      ir::replace_uses_with(load2, load1);
      progress = true;
    }
    return progress;
  }

  [[nodiscard]] static bool
  coalesce_load_store_load_load(ir::Instr_load* const load1)
  {
    if(load1->next->instr_kind != ir::Instr_Kind::store) {
      return false;
    }

    ir::Instr_store* const store = static_cast<ir::Instr_store*>(load1->next);
    if(store->next->instr_kind != ir::Instr_Kind::load) {
      return false;
    }

    ir::Instr_load* const load2 = static_cast<ir::Instr_load*>(store->next);
    if(load2->next->instr_kind != ir::Instr_Kind::load) {
      return false;
    }

    bool progress = false;
    ir::Instr_load* const load3 = static_cast<ir::Instr_load*>(load2->next);
    // The pattern of interest is
    //   b = load ma
    //       store mc b
    //   x = load my
    //   d = load mc
    // Load 2 is entirely irrelevant.
    i64 const var_users = store->dst->referrers.size();
    if(store->src == load1 && load3->src == store->dst && var_users == 2) {
      store->src->remove_referrer(store);
      store->dst->remove_referrer(store);
      load3->src->remove_referrer(load3);
      ilist_erase(store);
      ilist_erase(load3);
      ir::replace_uses_with(load3, load1);
      // Move load 1 to after load 2.
      ilist_erase(load1);
      ilist_insert_after(load2, load1);
      progress = true;
    }
    return progress;
  }

  [[nodiscard]] static bool
  coalesce_store_load_store(ir::Instr_store* const store1)
  {
    if(store1->next->instr_kind != ir::Instr_Kind::load) {
      return false;
    }

    ir::Instr_load* const load = static_cast<ir::Instr_load*>(store1->next);
    if(load->next->instr_kind != ir::Instr_Kind::store) {
      return false;
    }

    bool progress = false;
    ir::Instr_store* const store2 = static_cast<ir::Instr_store*>(load->next);
    i64 const var_users = store1->dst->referrers.size();
    if(store2->src == load && load->src == store1->dst && var_users == 2) {
      load->src->remove_referrer(load);
      store1->dst->remove_referrer(store1);
      store1->src->remove_referrer(store1);
      ilist_erase(load);
      ilist_erase(store1);
      ir::replace_uses_with(load, store1->src);
      progress = true;
    }
    return progress;
  }

  [[nodiscard]] static bool
  remove_redundant_temporaries(ir::Context& ctx, ir::Instr_store* const store)
  {
    if(store->next->instr_kind != ir::Instr_Kind::load) {
      return false;
    }

    i64 const var_users = store->dst->referrers.size();

    ir::Instr_load* const load = static_cast<ir::Instr_load*>(store->next);
    if(store->dst == load->src && var_users == 2) {
      load->src->remove_referrer(load);
      store->src->remove_referrer(store);
      store->dst->remove_referrer(store);
      ilist_erase(load);
      ilist_erase(store);
      ir::replace_uses_with(load, store->src);
      return true;
    }

    // store load load -> load_rhs
    if(load->next->instr_kind == ir::Instr_Kind::load) {
      ir::Instr_load* const load2 = static_cast<ir::Instr_load*>(load->next);
      if(store->dst == load2->src && var_users == 2) {
        auto const position = load->next;
        auto const load_rhs =
          make_instr_load_rhs(ctx, load->source_info, load->src);
        ilist_insert_before(position, load_rhs);

        ir::replace_uses_with(load2, store->src);
        ir::replace_uses_with(load, load_rhs);

        load->src->remove_referrer(load);
        load2->src->remove_referrer(load2);
        store->src->remove_referrer(store);
        store->dst->remove_referrer(store);
        ilist_erase(load);
        ilist_erase(load2);
        ilist_erase(store);
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] static bool process_block(ir::Context& ctx,
                                          ir::Block* const block)
  {
    bool progress = false;
    ir::Instr* current = block->get_first();
    ir::Instr* const end = block->get_last();
    for(; current != end; current = current->next) {
      if(current->instr_kind == ir::Instr_Kind::load) {
        ir::Instr_load* const load = static_cast<ir::Instr_load*>(current);
        progress |= coalesce_load_store_load(load);
        // progress |= coalesce_load_store_load_load(load);
      } else if(current->instr_kind == ir::Instr_Kind::store) {
        ir::Instr_store* const store = static_cast<ir::Instr_store*>(current);
        progress |= coalesce_store_load_store(store);
        progress |= remove_redundant_temporaries(ctx, store);
      }
    }
    return progress;
  }

  [[nodiscard]] static bool
  process_blocks(ir::Context& ctx, ir::Block* const block,
                 anton::Flat_Hash_Set<ir::Block*>& visited)
  {
    if(visited.find(block) != visited.end()) {
      return false;
    }

    bool progress = false;

    visited.emplace(block);
    progress |= process_block(ctx, block);
    ir::Instr* const terminator = block->get_last();
    ANTON_ASSERT(ir::is_branch(terminator), "terminator is not a branch");
    if(terminator->instr_kind == ir::Instr_Kind::branch) {
      auto const instr = static_cast<ir::Instr_branch*>(terminator);
      progress |= process_blocks(ctx, instr->target, visited);
    } else if(terminator->instr_kind == ir::Instr_Kind::brcond) {
      auto const instr = static_cast<ir::Instr_brcond*>(terminator);
      progress |= process_blocks(ctx, instr->then_target, visited);
      progress |= process_blocks(ctx, instr->else_target, visited);
    }
    return progress;
  }

  bool opt_coalesce_loads(ir::Context& ctx, ir::Function* const function)
  {
    anton::Flat_Hash_Set<ir::Block*> visited(ctx.allocator);
    bool const progress = process_blocks(ctx, function->entry_block, visited);

    auto current = function->locals.begin();
    auto end = function->locals.end();
    while(current != end) {
      ir::Instr_variable* local = *current;
      if(local->referrers.size() == 0) {
        function->locals.erase_unsorted(current);
        end = function->locals.end();
      } else {
        ++current;
      }
    }

    return progress;
  }
} // namespace glang
