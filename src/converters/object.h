/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

/*
 * This file defines functions for decomposing v8::Objects.
 */

#pragma once

#include <nan.h>
#include <node-addon-api/napi.h>
#include <v8.h>

#include "src/converters.h"
#include "src/functional/maybe.h"
#include "src/functional/validation.h"

namespace node_webrtc {

template <typename T>
static Validation<T> GetRequired(const v8::Local<v8::Object> object, const std::string& property) {
  return From<T>(object->Get(Nan::New(property).ToLocalChecked()));
}

template <typename T>
static Validation<Maybe<T>> GetOptional(const v8::Local<v8::Object> object, const std::string& property) {
  auto value = object->Get(Nan::New(property).ToLocalChecked());
  if (value->IsUndefined()) {
    return Pure(Maybe<T>::Nothing());
  }
  return From<T>(value).Map(&Maybe<T>::Just);
}

template <typename T>
static Validation<T> GetOptional(
    const v8::Local<v8::Object> object,
    const std::string& property,
    T default_value) {
  return GetOptional<T>(object, property).Map([default_value](auto maybeT) {
    return maybeT.FromMaybe(default_value);
  });
}

namespace napi {

template <typename T>
static Validation<T> GetRequired(const Napi::Object object, const std::string& property) {
  auto maybeValue = object.Get(property);
  return maybeValue.Env().IsExceptionPending()
      ? Validation<T>::Invalid(maybeValue.Env().GetAndClearPendingException().Message())
      : From<T>(maybeValue);
}

template <typename T>
static Validation<Maybe<T>> GetOptional(const Napi::Object object, const std::string& property) {
  auto maybeValue = object.Get(property);
  if (maybeValue.Env().IsExceptionPending()) {
    return Validation<Maybe<T>>::Invalid(maybeValue.Env().GetAndClearPendingException().Message());
  }
  return maybeValue.IsUndefined()
      ? Pure(MakeNothing<T>())
      : From<T>(maybeValue).Map(&MakeJust<T>);
}

template <typename T>
static Validation<T> GetOptional(
    const Napi::Object object,
    const std::string& property,
    T default_value) {
  return GetOptional<T>(object, property).Map([default_value](auto maybeT) {
    return maybeT.FromMaybe(default_value);
  });
}

}  // namespace napi

}  // namespace node_webrtc
