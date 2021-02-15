// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "ds/logger.h"

#include <memory>
#include <quickjs/quickjs-exports.h>
#include <quickjs/quickjs.h>

namespace js
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc99-extensions"

  extern JSValue js_print(
    JSContext* ctx, JSValueConst, int argc, JSValueConst* argv);

  extern void js_dump_error(JSContext* ctx);

  class JSAutoFreeRuntime
  {
    JSRuntime* rt;

  public:
    inline JSAutoFreeRuntime()
    {
      rt = JS_NewRuntime();
      if (rt == nullptr)
      {
        throw std::runtime_error("Failed to initialise QuickJS runtime");
      }
    }

    inline ~JSAutoFreeRuntime()
    {
      JS_FreeRuntime(rt);
    }

    inline operator JSRuntime*() const
    {
      return rt;
    }
  };

  class JSAutoFreeCtx
  {
    JSContext* ctx;

  public:
    inline JSAutoFreeCtx(JSRuntime* rt)
    {
      ctx = JS_NewContext(rt);
      if (ctx == nullptr)
      {
        throw std::runtime_error("Failed to initialise QuickJS context");
      }
      JS_SetContextOpaque(ctx, this);
    }

    inline ~JSAutoFreeCtx()
    {
      JS_FreeContext(ctx);
    }

    inline operator JSContext*() const
    {
      return ctx;
    }

    struct JSWrappedValue
    {
      inline JSWrappedValue(JSContext* ctx, JSValue&& val) :
        ctx(ctx),
        val(std::move(val))
      {}
      inline ~JSWrappedValue()
      {
        JS_FreeValue(ctx, val);
      }
      inline operator const JSValue&() const
      {
        return val;
      }
      JSContext* ctx;
      JSValue val;
    };

    struct JSWrappedCString
    {
      inline JSWrappedCString(JSContext* ctx, const char* cstr) :
        ctx(ctx),
        cstr(cstr)
      {}
      inline ~JSWrappedCString()
      {
        JS_FreeCString(ctx, cstr);
      }
      inline operator const char*() const
      {
        return cstr;
      }
      inline operator std::string() const
      {
        return std::string(cstr);
      }
      inline operator std::string_view() const
      {
        return std::string_view(cstr);
      }
      JSContext* ctx;
      const char* cstr;
    };

    inline JSWrappedValue operator()(JSValue&& val)
    {
      return JSWrappedValue(ctx, std::move(val));
    };

    inline JSWrappedCString operator()(const char* cstr)
    {
      return JSWrappedCString(ctx, cstr);
    };
  };

#pragma clang diagnostic pop

}