// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "ds/json.h"
#include "ds/nonstd.h"

#include <http-parser/http_parser.h>
#include <nlohmann/json.hpp>
#include <string>

namespace ds
{
  /**
   * These structs contain the required fields to build the corresponding
   * OpenAPI objects. They do not contain every field, but should be trivially
   * extensible with any which are desired.
   */
  namespace openapi
  {
    struct Info
    {
      std::string title;
      std::string description;
      std::string version;
    };
    DECLARE_JSON_TYPE(Info);
    DECLARE_JSON_REQUIRED_FIELDS(Info, title, description, version);

    struct Server
    {
      std::string url;

      bool operator==(const Server& rhs) const
      {
        return url == rhs.url;
      }
    };
    DECLARE_JSON_TYPE(Server);
    DECLARE_JSON_REQUIRED_FIELDS(Server, url);

    struct MediaType
    {
      // May be a full in-place schema, but is generally a string containing a
      // reference to a schema stored elsewhere
      nlohmann::json schema;

      bool operator==(const MediaType& rhs) const
      {
        return schema == rhs.schema;
      }
    };
    DECLARE_JSON_TYPE_WITH_OPTIONAL_FIELDS(MediaType);
    DECLARE_JSON_REQUIRED_FIELDS(MediaType);
    DECLARE_JSON_OPTIONAL_FIELDS(MediaType, schema);

    struct Response
    {
      std::string description;
      std::map<std::string, MediaType> content;
    };
    DECLARE_JSON_TYPE_WITH_OPTIONAL_FIELDS(Response);
    DECLARE_JSON_REQUIRED_FIELDS(Response, description);
    DECLARE_JSON_OPTIONAL_FIELDS(Response, content);

    struct Operation
    {
      std::map<std::string, Response> responses;

      Response& operator[](http_status status)
      {
        // HTTP_STATUS_OK (aka an int with value 200) becomes the string "200"
        const auto s = std::to_string(status);
        return responses[s];
      }
    };
    DECLARE_JSON_TYPE(Operation);
    DECLARE_JSON_REQUIRED_FIELDS(Operation, responses);

    struct PathItem
    {
      std::map<std::string, Operation> operations;

      Operation& operator[](http_method verb)
      {
        // HTTP_GET becomes the string "get"
        std::string s = http_method_str(verb);
        nonstd::to_lower(s);
        return operations[s];
      }
    };
    DECLARE_JSON_TYPE(PathItem);
    DECLARE_JSON_REQUIRED_FIELDS(PathItem, operations);

    using Paths = std::map<std::string, PathItem>;

    struct Components
    {
      std::map<std::string, nlohmann::json> schemas;

      bool operator!=(const Components& rhs) const
      {
        return schemas != rhs.schemas;
      }
    };
    DECLARE_JSON_TYPE_WITH_OPTIONAL_FIELDS(Components);
    DECLARE_JSON_REQUIRED_FIELDS(Components);
    DECLARE_JSON_OPTIONAL_FIELDS(Components, schemas);

    struct Document
    {
      std::string openapi = "3.0.0";
      Info info;
      std::vector<Server> servers;
      Paths paths;
      Components components;

      void add_response_schema(
        const std::string& uri,
        http_method verb,
        http_status status,
        const std::string& content_type,
        const nlohmann::json& schema,
        const std::string& components_schema_name)
      {
        const auto schema_it = components.schemas.find(components_schema_name);
        if (schema_it != components.schemas.end())
        {
          // Check that the existing schema matches the new one being added with
          // the same name
          const auto& existing_schema = schema_it->second;
          if (schema != existing_schema)
          {
            throw std::logic_error(fmt::format(
              "Adding schema with name '{}'. Does not match previous schema "
              "registered with this name: {} vs {}",
              components_schema_name,
              schema.dump(),
              existing_schema.dump()));
          }
        }
        else
        {
          components.schemas.emplace(components_schema_name, schema);
        }

        paths[uri][verb][status].content[content_type].schema =
          fmt::format("#/components/schemas/{}", components_schema_name);
      }
    };
    DECLARE_JSON_TYPE_WITH_OPTIONAL_FIELDS(Document);
    DECLARE_JSON_REQUIRED_FIELDS(Document, openapi, info, paths);
    DECLARE_JSON_OPTIONAL_FIELDS(Document, servers, components);
  }
}
