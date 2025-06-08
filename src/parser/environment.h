#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
// An abstract class for conveying information from the module that an
// Expression might need to complete Compute().
//
// There is both a production and a test implementation of this class.

#include <string>

struct PortPointer {
  // Hard to imagine there will ever be more than these two, but being prepared.
  enum PortType {
    NOT_PORT,
    INPUT,
    OUTPUT
  };
  PortType port_type;
  // index into the relevant ProcessArgs array.
  // TODO: instead of setting INn variables in the process() loop, maybe
  // just let a VARIABLE expression read it this way?
  int index;

  PortPointer() :
      port_type{NOT_PORT}, index{-1} {
  }

  PortPointer(PortType port, int port_index) :
      port_type{port}, index{port_index} {
  }
};

class Environment {
 public:
  virtual float GetChannels(const PortPointer &port) = 0;
  virtual float GetVoltage(const PortPointer &port) = 0;
  // "channel" is a number from 1-16.
  virtual float GetVoltage(const PortPointer &port, int channel) = 0;
  virtual void SetChannels(const PortPointer &port, int channels) = 0;
  virtual void SetVoltage(const PortPointer &port, float value) = 0;
  // "channel" is a number from 1-16.
  virtual void SetVoltage(const PortPointer &port, int channel, float value) = 0;
  virtual float SampleRate() = 0;

  // Since a Port can only be constructed at compile time, it should just
  // become part of the function-running Expression.
  virtual float Connected(const PortPointer &port) = 0;

  // Hard to test these without making these overridable.
  virtual float Random(float min_value, float max_value) = 0;
  virtual float Normal(float mean, float std_dev) = 0;
  virtual float Time(bool millis) = 0;

  // These just change the operating environment.
  virtual void Clear() = 0;
  virtual void Reset() = 0;

  // Return true only when this sample is when the program was start/restarted/
  // reset.
  virtual bool Start() = 0;

  // Returns True only during the sample when this port has received a trigger.
  virtual bool Trigger(const PortPointer &port) = 0;

  // Start outputting a string via an OUTn port.
  virtual void Send(const PortPointer &port, const std::string &str) = 0;
};

#endif // ENVIRONMENT_H
