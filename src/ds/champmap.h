// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include <array>
#include <memory>
#include <optional>
#include <vector>

namespace champ
{
  // A persistent hash map based on the Compressed Hash-Array Mapped Preﬁx-tree
  // from 'Fast and Lean Immutable Multi-Maps on the JVM based on Heterogeneous
  // Hash-Array Mapped Tries' by Michael J. Steindorfer and Jurgen J. Vinju
  // (https://arxiv.org/pdf/1608.01036.pdf).

  using Hash = uint32_t;
  using SmallIndex = uint8_t;

  // 5 bits are masked off at each node to give an index. After 6 masks, only 2
  // bits of the hash are left to determine the bin in a `Collisions` node.
  constexpr SmallIndex collision_depth = 6;

  constexpr SmallIndex mask(Hash hash, SmallIndex depth)
  {
    return (hash >> ((Hash)depth * 5)) & 0b11111;
  }

  class Bitmap
  {
    uint32_t _bits;

  public:
    constexpr Bitmap() : _bits(0) {}

    constexpr Bitmap(uint32_t bits) : _bits(bits) {}

    constexpr Bitmap operator&(const Bitmap& other) const
    {
      return Bitmap(_bits & other._bits);
    }

    constexpr SmallIndex pop() const
    {
      return __builtin_popcount(_bits);
    }

    constexpr Bitmap set(SmallIndex idx) const
    {
      return Bitmap(_bits | ((uint32_t)1 << idx));
    }

    constexpr Bitmap clear(SmallIndex idx) const
    {
      return Bitmap(_bits & ~((uint32_t)1 << idx));
    }

    constexpr bool check(SmallIndex idx) const
    {
      return (_bits & ((uint32_t)1 << idx)) != 0;
    }
  };

  template <class V>
  std::optional<V> not_found()
  {
    return std::nullopt;
  }

  template <class K, class V, class H>
  struct SubNodes;

  template <class K, class V>
  struct Entry
  {
    K key;
    V value;

    Entry(K k, V v) : key(k), value(v) {}

    std::optional<V> get(const K& k) const
    {
      if (k == key)
        return value;
      else
        return not_found<V>();
    }
  };

  template <class K, class V, class H>
  using Node = std::shared_ptr<void>;

  template <class K, class V, class H>
  struct Collisions
  {
    std::array<std::vector<std::shared_ptr<Entry<K, V>>>, 4> bins;

    std::optional<V> get(Hash hash, const K& k) const
    {
      const auto idx = mask(hash, collision_depth);
      const auto& bin = bins[idx];
      for (const auto& node : bin)
      {
        if (k == node->key)
          return node->value;
      }
      return not_found<V>();
    }

    bool put_mut(Hash hash, const K& k, const V& v)
    {
      const auto idx = mask(hash, collision_depth);
      auto& bin = bins[idx];
      for (size_t i = 0; i < bin.size(); ++i)
      {
        const auto& entry = bin[i];
        if (k == entry->key)
        {
          bin[i] = std::make_shared<Entry<K, V>>(k, v);
          return false;
        }
      }
      bin.push_back(std::make_shared<Entry<K, V>>(k, v));
      return true;
    }

    template <class F>
    void foreach(F f) const
    {
      for (const auto& bin : bins)
      {
        for (const auto& entry : bin)
          f(entry->key, entry->value);
      }
    }
  };

  template <class K, class V, class H>
  struct SubNodes
  {
    std::vector<Node<K, V, H>> nodes;
    Bitmap node_map;
    Bitmap data_map;

    SubNodes() {}

    SubNodes(std::vector<Node<K, V, H>> ns) : nodes(ns) {}

    SubNodes(std::vector<Node<K, V, H>> ns, Bitmap nm, Bitmap dm) :
      nodes(ns),
      node_map(nm),
      data_map(dm)
    {}

    SmallIndex compressed_idx(SmallIndex idx) const
    {
      if (!node_map.check(idx) && !data_map.check(idx))
        return (SmallIndex)-1;

      const auto mask = Bitmap(~((uint32_t)-1 << idx));
      if (data_map.check(idx))
        return (data_map & mask).pop();

      return data_map.pop() + (node_map & mask).pop();
    }

    std::optional<V> get(SmallIndex depth, Hash hash, const K& k) const
    {
      const auto idx = mask(hash, depth);
      const auto c_idx = compressed_idx(idx);

      if (c_idx == (SmallIndex)-1)
        return not_found<V>();

      if (data_map.check(idx))
        return node_as<Entry<K, V>>(c_idx)->get(k);

      if (depth == (collision_depth - 1))
        return node_as<Collisions<K, V, H>>(c_idx)->get(hash, k);

      return node_as<SubNodes<K, V, H>>(c_idx)->get(depth + 1, hash, k);
    }

    bool put_mut(SmallIndex depth, Hash hash, const K& k, const V& v)
    {
      const auto idx = mask(hash, depth);
      auto c_idx = compressed_idx(idx);

      if (c_idx == (SmallIndex)-1)
      {
        data_map = data_map.set(idx);
        c_idx = compressed_idx(idx);
        nodes.insert(
          nodes.begin() + c_idx, std::make_shared<Entry<K, V>>(k, v));
        return true;
      }

      if (node_map.check(idx))
      {
        bool insert;
        if (depth < (collision_depth - 1))
        {
          auto sn = *node_as<SubNodes<K, V, H>>(c_idx);
          insert = sn.put_mut(depth + 1, hash, k, v);
          nodes[c_idx] = std::make_shared<SubNodes<K, V, H>>(std::move(sn));
        }
        else
        {
          auto sn = *node_as<Collisions<K, V, H>>(c_idx);
          insert = sn.put_mut(hash, k, v);
          nodes[c_idx] = std::make_shared<Collisions<K, V, H>>(std::move(sn));
        }
        return insert;
      }

      const auto& entry0 = node_as<Entry<K, V>>(c_idx);
      if (k == entry0->key)
      {
        nodes[c_idx] = std::make_shared<Entry<K, V>>(k, v);
        return false;
      }

      if (depth < (collision_depth - 1))
      {
        const auto hash0 = H()(entry0->key);
        const auto idx0 = mask(hash0, depth + 1);
        auto sub_node =
          SubNodes<K, V, H>({entry0}, Bitmap(0), Bitmap(0).set(idx0));
        sub_node.put_mut(depth + 1, hash, k, v);

        nodes.erase(nodes.begin() + c_idx);
        data_map = data_map.clear(idx);
        node_map = node_map.set(idx);
        c_idx = compressed_idx(idx);
        nodes.insert(
          nodes.begin() + c_idx,
          std::make_shared<SubNodes<K, V, H>>(std::move(sub_node)));
      }
      else
      {
        auto sub_node = Collisions<K, V, H>();
        const auto hash0 = H()(entry0->key);
        const auto idx0 = mask(hash0, collision_depth);
        sub_node.bins[idx0].push_back(entry0);
        const auto idx1 = mask(hash, collision_depth);
        sub_node.bins[idx1].push_back(std::make_shared<Entry<K, V>>(k, v));

        nodes.erase(nodes.begin() + c_idx);
        data_map = data_map.clear(idx);
        node_map = node_map.set(idx);
        c_idx = compressed_idx(idx);
        nodes.insert(
          nodes.begin() + c_idx,
          std::make_shared<Collisions<K, V, H>>(std::move(sub_node)));
      }
      return true;
    }

    std::pair<std::shared_ptr<SubNodes<K, V, H>>, bool> put(
      SmallIndex depth, Hash hash, const K& k, const V& v) const
    {
      auto node = *this;
      auto r = node.put_mut(depth, hash, k, v);
      return std::make_pair(
        std::make_shared<SubNodes<K, V, H>>(std::move(node)), r);
    }

    template <class F>
    void foreach(SmallIndex depth, F f) const
    {
      const auto entries = data_map.pop();
      for (SmallIndex i = 0; i < entries; ++i)
      {
        const auto& entry = node_as<Entry<K, V>>(i);
        f(entry->key, entry->value);
      }
      for (size_t i = entries; i < nodes.size(); ++i)
      {
        if (depth == (collision_depth - 1))
          node_as<Collisions<K, V, H>>(i)->foreach(f);
        else
          node_as<SubNodes<K, V, H>>(i)->foreach(depth + 1, f);
      }
    }

  private:
    template <class A>
    const std::shared_ptr<A>& node_as(SmallIndex c_idx) const
    {
      return reinterpret_cast<const std::shared_ptr<A>&>(nodes[c_idx]);
    }
  };

  template <class K, class V, class H = std::hash<K>>
  class Map
  {
  private:
    std::shared_ptr<SubNodes<K, V, H>> root;
    size_t _size = 0;

    Map(std::shared_ptr<SubNodes<K, V, H>> root_, size_t size_) :
      root(root_),
      _size(size_)
    {}

  public:
    Map() : root(std::make_shared<SubNodes<K, V, H>>()) {}

    size_t size() const
    {
      return _size;
    }

    bool empty() const
    {
      return _size == 0;
    }

    std::optional<V> get(const K& key) const
    {
      return root->get(0, H()(key), key);
    }

    const Map<K, V, H> put(const K& key, const V& value) const
    {
      auto r = root->put(0, H()(key), key, value);
      auto size_ = _size;
      if (r.second)
        size_++;

      return Map(std::move(r.first), size_);
    }

    template <class F>
    void foreach(F f) const
    {
      root->foreach(0, f);
    }
  };
}
