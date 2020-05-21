// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "ds/hash.h"
#include "kv_types.h"
#include "map.h"
#include "tx_view.h"

#include <vector>

namespace kv
{
  namespace experimental
  {
    using SerialisedRep = std::vector<uint8_t>;

    // TODO: I don't think this needs to be customisable? If the map is storing
    // _your_ types as keys, you might need to tell it how to compare them. But
    // we know how to compare byte-vectors, and this is an internal detail so
    // why would you change it?
    using RepHasher = std::hash<SerialisedRep>;

    using UntypedMap = kv::Map<SerialisedRep, SerialisedRep, RepHasher>;

    using UntypedOperationsView =
      kv::TxView<SerialisedRep, SerialisedRep, RepHasher>;

    using UntypedCommitter =
      kv::TxViewCommitter<SerialisedRep, SerialisedRep, RepHasher>;

    using UntypedState = kv::State<SerialisedRep, SerialisedRep, RepHasher>;

    template <typename K, typename V>
    struct MsgPackSerialiser
    {
    private:
      template <typename T>
      static SerialisedRep from_t(const T& t)
      {
        msgpack::sbuffer sb;
        msgpack::pack(sb, t);
        auto sb_data = reinterpret_cast<const uint8_t*>(sb.data());
        return SerialisedRep(sb_data, sb_data + sb.size());
      }

      template <typename T>
      static T to_t(const SerialisedRep& rep)
      {
        msgpack::object_handle oh = msgpack::unpack(
          reinterpret_cast<const char*>(rep.data()), rep.size());
        auto object = oh.get();
        return object.as<T>();
      }

    public:
      static SerialisedRep from_key(const K& k)
      {
        return from_t<K>(k);
      }

      static K to_key(const SerialisedRep& rep)
      {
        return to_t<K>(rep);
      }

      static SerialisedRep from_value(const V& v)
      {
        return from_t<V>(v);
      }

      static V to_value(const SerialisedRep& rep)
      {
        return to_t<V>(rep);
      }
    };

    template <typename K, typename V, typename S>
    class TxView : public UntypedCommitter
    {
    protected:
      // This _has_ a (non-visible, untyped) view, whereas the standard impl
      // _is_ a typed view
      UntypedOperationsView untyped_view;

    public:
      TxView(
        UntypedMap& m,
        size_t rollbacks,
        UntypedState& current_state,
        UntypedState& committed_state,
        Version v) :
        UntypedCommitter(m, rollbacks, current_state, committed_state, v),
        untyped_view(UntypedCommitter::change_set)
      {}

      std::optional<V> get(const K& key)
      {
        const auto k_rep = S::from_key(key);
        const auto opt_v_rep = untyped_view.get(k_rep);

        if (opt_v_rep.has_value())
        {
          return S::to_value(*opt_v_rep);
        }

        return std::nullopt;
      }

      std::optional<V> get_globally_committed(const K& key)
      {
        const auto k_rep = S::from_key(key);
        const auto opt_v_rep = untyped_view.get_globally_committed(k_rep);

        if (opt_v_rep.has_value())
        {
          return S::to_value(*opt_v_rep);
        }

        return std::nullopt;
      }

      bool put(const K& key, const V& value)
      {
        const auto k_rep = S::from_key(key);
        const auto v_rep = S::from_value(value);

        return untyped_view.put(k_rep, v_rep);
      }

      bool remove(const K& key)
      {
        const auto k_rep = S::from_key(key);

        return untyped_view.remove(k_rep);
      }

      template <class F>
      bool foreach(F&& f)
      {
        auto g = [&](const SerialisedRep& k_rep, const SerialisedRep& v_rep) {
          return f(S::to_key(k_rep), S::to_value(v_rep));
        };
        return untyped_view.foreach(g);
      }
    };

    // TODO: Inheritance doesn't work! Because the base typedefs aren't right!
    // Need to _have_ an untyped member, and _be_ an AbstractMap ourselves
    template <typename K, typename V, typename S = MsgPackSerialiser<K, V>>
    class Map : public AbstractMap
    {
    protected:
      using This = Map<K, V, S>;

      UntypedMap untyped_map;

      using __K_HASH = std::hash<K>;

    public:
      // Expose correct public aliases of types
      using VersionV = VersionV<V>;

      // TODO: We don't really have these! We really don't want to expose them
      // to you, they should be an implementation detail. Can we remove them
      // from commit hooks? This introduces a requirement that the type is
      // hashable! Unhappy!
      using State = State<K, V, __K_HASH>;

      using Write = Write<K, V, __K_HASH>;

      // TODO: Is this the correct choice? Or should we just gives hooks the
      // serialised forms and let them convert themselves?
      using CommitHook = CommitHook<K, V, __K_HASH>;

      using TxView = kv::experimental::TxView<K, V, S>;

      template <typename... Ts>
      Map(Ts&&... ts) : untyped_map(std::forward<Ts>(ts)...)
      {}

      bool operator==(const AbstractMap& that) const override
      {
        auto p = dynamic_cast<const This*>(&that);
        if (p == nullptr)
        {
          return false;
        }

        return untyped_map == p->untyped_map;
      }

      bool operator!=(const AbstractMap& that) const override
      {
        return !(*this == that);
      }

      AbstractStore* get_store() override
      {
        return untyped_map.get_store();
      }

      AbstractTxView* create_view(Version version) override
      {
        return untyped_map.create_view_internal<TxView>(version);
      }

      void serialise(
        const AbstractTxView* view,
        KvStoreSerialiser& s,
        bool include_reads) override
      {
        untyped_map.serialise(view, s, include_reads);
      }

      AbstractTxView* deserialise(
        KvStoreDeserialiser& d, Version version) override
      {
        return untyped_map.deserialise(d, version);
      }

      const std::string& get_name() const override
      {
        return untyped_map.get_name();
      }

      void compact(Version v) override
      {
        return untyped_map.compact(v);
      }

      void post_compact() override
      {
        return untyped_map.post_compact();
      }

      void rollback(Version v) override
      {
        untyped_map.rollback(v);
      }

      void lock() override
      {
        untyped_map.lock();
      }

      void unlock() override
      {
        untyped_map.unlock();
      }

      SecurityDomain get_security_domain() override
      {
        return untyped_map.get_security_domain();
      }

      bool is_replicated() override
      {
        return untyped_map.is_replicated();
      }

      void clear() override
      {
        untyped_map.clear();
      }

      AbstractMap* clone(AbstractStore* store) override
      {
        return untyped_map.clone(store);
      }

      void swap(AbstractMap* map) override
      {
        auto p = dynamic_cast<This*>(map);
        if (p == nullptr)
          throw std::logic_error(
            "Attempted to swap maps with incompatible types");

        untyped_map.swap(&p->untyped_map);
      }

      static UntypedMap::CommitHook wrap_commit_hook(const CommitHook& hook)
      {
        return
          [hook](
            Version v, const UntypedMap::State& s, const UntypedMap::Write& w) {
            // TODO: We're abandoning s for now. This is wrong!
            Write typed_w;
            for (const auto& [uk, version_uv] : w)
            {
              typed_w[S::to_key(uk)] =
                VersionV{version_uv.version, S::to_value(version_uv.value)};
            }
            hook(v, {}, typed_w);
          };
      }

      void set_local_hook(const CommitHook& hook)
      {
        untyped_map.set_local_hook(wrap_commit_hook(hook));
      }

      void unset_local_hook()
      {
        untyped_map.unset_local_hook();
      }

      void set_global_hook(const CommitHook& hook)
      {
        untyped_map.set_global_hook(wrap_commit_hook(hook));
      }

      void unset_global_hook()
      {
        untyped_map.unset_global_hook();
      }
    };
  }
}