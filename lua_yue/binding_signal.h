// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_YUE_BINDING_SIGNAL_H_
#define LUA_YUE_BINDING_SIGNAL_H_

#include <string>

#include "lua/lua.h"
#include "nativeui/signal.h"

namespace yue {

// A simple structure that records the signal pointer and owner reference.
template<typename Sig>
class SignalWrapper : public base::RefCounted<SignalWrapper<Sig>> {
 public:
  SignalWrapper(lua::State* state, int owner, nu::Signal<Sig>* signal)
      : signal_(signal) {
    lua::CreateWeakReference(state, this, owner);
  }

  bool IsOwnerAlive(lua::State* state) {
    return lua::WeakReferenceExists(state, this);
  }

  int Connect(lua::CallContext* context, const std::function<Sig>& slot) {
    if (!IsOwnerAlive(context->state)) {
      context->has_error = true;
      lua::Push(context->state, "Owner of signal is gone");
      return -1;
    }
    int id = signal_->Connect(slot);
    // self.__yuesignals[signal][id] = slot
    lua::PushWeakReference(context->state, this);
    lua::PushRefsTable(context->state, "__yuesignals", -1);
    lua::RawGetOrCreateTable(context->state, -1, static_cast<void*>(signal_));
    lua::RawSet(context->state, -1, id, lua::ValueOnStack(context->state, 2));
    return id;
  }

  void Disconnect(lua::CallContext* context, int id) {
    if (!IsOwnerAlive(context->state)) {
      context->has_error = true;
      lua::Push(context->state, "Owner of signal is gone");
      return;
    }
    signal_->Disconnect(id);
    // self.__yuesignals[signal][id] = nil
    lua::PushWeakReference(context->state, this);
    lua::PushRefsTable(context->state, "__yuesignals", -1);
    lua::RawGetOrCreateTable(context->state, -1, static_cast<void*>(signal_));
    lua::RawSet(context->state, -1, id, nullptr);
  }

  void DisconnectAll(lua::CallContext* context) {
    if (!IsOwnerAlive(context->state)) {
      context->has_error = true;
      lua::Push(context->state, "Owner of signal is gone");
      return;
    }
    signal_->DisconnectAll();
    // self.__yuesignals[signal] = {}
    lua::PushWeakReference(context->state, this);
    lua::PushRefsTable(context->state, "__yuesignals", -1);
    lua::RawSet(context->state, -1, static_cast<void*>(signal_), nullptr);
  }

 private:
  ~SignalWrapper() {}

  friend class base::RefCounted<SignalWrapper<Sig>>;

  nu::Signal<Sig>* signal_;
};

}  // namespace yue

namespace lua {

template<typename Sig>
struct Type<yue::SignalWrapper<Sig>> {
  static constexpr const char* name = "yue.Signal";
  static void BuildMetaTable(State* state, int metatable) {
    RawSet(state, metatable,
           "connect", &yue::SignalWrapper<Sig>::Connect,
           "disconnect", &yue::SignalWrapper<Sig>::Disconnect,
           "disconnectall", &yue::SignalWrapper<Sig>::DisconnectAll);
  }
};

template<typename Sig>
struct Type<nu::Signal<Sig>> {
  static constexpr const char* name = "yue.Signal";
};

// Define how the Signal member is converted.
template<typename Sig>
struct MemberTraits<nu::Signal<Sig>> {
  static const RefMode kRefMode = RefMode::FirstAssign;
  static inline void Push(State* state, int owner,
                          const nu::Signal<Sig>& signal) {
    lua::Push(state,
              new yue::SignalWrapper<Sig>(
                  state, owner, const_cast<nu::Signal<Sig>*>(&signal)));
  }
  static inline bool To(State* state, int owner, int value,
                        nu::Signal<Sig>* out) {
    if (lua::GetType(state, value) != lua::LuaType::Function)
      return false;
    std::function<Sig> callback;
    if (!lua::To(state, value, &callback))
      return false;
    int id = out->Connect(callback);
    // self.__yuesignals[signal][id] = slot
    lua::PushRefsTable(state, "__yuesignals", owner);
    lua::RawGetOrCreateTable(state, -1, static_cast<void*>(out));
    lua::RawSet(state, -1, id, lua::ValueOnStack(state, 3));
    return true;
  }
};

}  // namespace lua

#endif  // LUA_YUE_BINDING_SIGNAL_H_
