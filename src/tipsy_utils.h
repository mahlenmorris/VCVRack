#ifndef TIPSY_UTILS_H
#define TIPSY_UTILS_H

#include <queue>
#include <string>
#include "plugin.hpp"
#include <tipsy/tipsy.h>  // Library for sending text.


// The queue of Tipsy messages being sent out of a single OUTn port.
struct TextSender {
  std::queue<std::string> unsent_queue;
  tipsy::ProtocolEncoder encoder;
  std::string sending_message;

  void AddToQueue(const std::string &text) {
    // If there's enough of a backlog, we shed incoming load.
    // Not that it takes very long to send a message.
    if (unsent_queue.size() < 101) {
      unsent_queue.push(text);
    }
  }

  void ProcessEncoder(int port_index, std::vector<Output>* outputs) {
    // Do we need to do anything?
    if (unsent_queue.empty() && encoder.isDormant()) {
      outputs->at(port_index).setVoltage(0.0f);
      return;
    }
    if (!unsent_queue.empty() && encoder.isDormant()) {
      // Have message waiting, but not currently sending anything.
      sending_message = unsent_queue.front();
      unsent_queue.pop();
      // size() + 1 because the decoder needs the NUL at the end, I suspect.
      encoder.initiateMessage("text/plain", sending_message.size() + 1,
                              (unsigned char *)sending_message.c_str());
    }
    if (!encoder.isDormant()) {
      float f = 0.0f;  // Compiler complains otherwise.
      auto result = encoder.getNextMessageFloat(f);
      if (!encoder.isError(result)) {
          outputs->at(port_index).setVoltage(f);
      }
    }
  }
};



#endif // TIPSY_UTILS_H
