//#pragma once

//#include <future>
//#include <deque>

//#include "c3/nu/concurrency/concurrent_queue.hpp"

//namespace c3::nu {
//  template<typename T, typename... ConsumeArgs>
//  class requester {

//  }

//  template<typename T>
//  class consumer;

//  template<typename T, typename... ConsumeArgs>
//  class producer;

//  template<typename T, typename... ConsumeArgs>
//  class producer {
//  private:
//    concurrent_queue<T> _values;
//    concurrent_queue<consumer<T>> _consumers;

//  public:
//    virtual void produce(T t) {/*
//      if (auto consumer = _consumers->pop())
//        consumer->provide(t);
//      else*/
//        _values.push(t);
//    }
//    virtual consumer<T> consume(ConsumeArgs... args) {

//    }
//  };
//}
