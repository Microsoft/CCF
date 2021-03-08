// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "ds/json.h"

#include <algorithm>
#include <cctype>
#include <msgpack/msgpack.hpp>
#include <string>

namespace ccf
{
  struct EntityId
  {
    // The underlying value type should be blit-serialisable so that it can be
    // written to and read from the ring buffer
    static constexpr size_t LENGTH = 64; // hex-encoded SHA-256 hash

    using Value =
      std::string; // < hex-encoded hash of caller's DER-encoded certificate
    Value id;

    EntityId() = default;
    EntityId(const Value& id_) : id(id_) {}

    void operator=(const EntityId& other)
    {
      id = other.id;
    }

    void operator=(const Value& id_)
    {
      id = id_;
    }

    bool operator==(const EntityId& other) const
    {
      return id == other.id;
    }

    bool operator!=(const EntityId& other) const
    {
      return !(*this == other);
    }

    bool operator<(const EntityId& other) const
    {
      return id < other.id;
    }

    operator std::string() const
    {
      return id;
    }

    auto& value() const
    {
      return id;
    }

    auto data() const
    {
      return id.data();
    }

    size_t size() const
    {
      return id.size();
    }

    MSGPACK_DEFINE(id);
  };

  inline void to_json(nlohmann::json& j, const EntityId& entity_id)
  {
    j = entity_id.id;
  }

  inline void from_json(const nlohmann::json& j, EntityId& entity_id)
  {
    if (j.is_string())
    {
      auto value = j.get<std::string>();
      if (value.length() != EntityId::LENGTH)
      {
        throw JsonParseError(fmt::format(
          "Entity id should be of length {} (found: {})",
          EntityId::LENGTH,
          value.length()));
      }

      if (!std::all_of(value.begin(), value.end(), [](unsigned char c) {
            return std::isxdigit(c);
          }))
      {
        throw JsonParseError(
          fmt::format("Entity id {} should be hex-encoded", value));
      }

      entity_id = value;
    }
    else
    {
      throw JsonParseError(
        fmt::format("Unable to parse entity id from this JSON: {}", j.dump()));
    }
  }

  inline std::string schema_name(const EntityId&)
  {
    return "EntityId";
  }

  inline void fill_json_schema(nlohmann::json& schema, const EntityId&)
  {
    schema["type"] = "string";

    // According to the spec, "format is an open value, so you can use any
    // formats, even not those defined by the OpenAPI Specification"
    // https://swagger.io/docs/specification/data-models/data-types/#format
    schema["format"] = "hex";
  }

  template <typename T>
  void add_schema_components(T&, nlohmann::json& j, const EntityId&)
  {
    j["type"] = "string";
    j["pattern"] = fmt::format("^[a-f0-9]{\\{}\\}$", EntityId::LENGTH);
  }

  using CallerId = EntityId;
  using MemberId = EntityId;
  using UserId = EntityId;
}

namespace std
{
  static inline std::ostream& operator<<(
    std::ostream& os, const ccf::EntityId& entity_id)
  {
    os << entity_id.id;
    return os;
  }

  template <>
  struct hash<ccf::EntityId>
  {
    size_t operator()(const ccf::EntityId& entity_id) const
    {
      return std::hash<std::string>{}(entity_id.id);
    }
  };
}