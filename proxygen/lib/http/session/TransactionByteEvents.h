/*
 *  Copyright (c) 2015-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include <proxygen/lib/http/session/ByteEvents.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>

namespace proxygen {

class TransactionByteEvent : public ByteEvent {
 public:
   TransactionByteEvent(uint64_t byteNo,
                        EventType eventType,
                        HTTPTransaction* txn)
       : ByteEvent(byteNo, eventType), txn_(txn) {
     txn_->incrementPendingByteEvents();
   }

   ~TransactionByteEvent() {
     txn_->decrementPendingByteEvents();
   }

  HTTPTransaction* getTransaction() override {
    return txn_;
  }

  HTTPTransaction* txn_;
};

/**
 * TimestampByteEvents are used to wait for TX and ACK timestamps.
 *
 * Contain a timeout that determines when the timestamp event expires (e.g., we
 * stop waiting to receive the timestamp from the system).
 */
class TimestampByteEvent
    : public TransactionByteEvent
    , public AsyncTimeoutSet::Callback {
 public:
  enum TimestampType {
    TX,
    ACK,
  };
  /**
   * The instances of TimestampByteEvent::Callback *MUST* outlive the ByteEvent
   * it is registered on.
   */
  class Callback {
   public:
    virtual ~Callback() {
    }
    virtual void timeoutExpired(TimestampByteEvent* event) noexcept = 0;
  };

  TimestampByteEvent(TimestampByteEvent::Callback* callback,
                     TimestampType timestampType,
                     uint64_t byteNo,
                     EventType eventType,
                     HTTPTransaction* txn)
      : TransactionByteEvent(byteNo, eventType, txn),
        timestampType_(timestampType),
        callback_(callback) {
  }

  void timeoutExpired() noexcept override {
    callback_->timeoutExpired(this);
  }

  const TimestampType timestampType_;

 private:
  Callback* callback_;
};

} // namespace proxygen
