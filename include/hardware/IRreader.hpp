#pragma once

class IRReader {
  public:
    void init(int pin);

    void update();

    bool ready() const {
        return _ready;
    }
    bool start() const {
        return _start;
    }
    bool stop() const {
        return _stop;
    }
    void shutdown();

  private:
    bool _ready = false;
    bool _start = false;
    bool _stop = false;
};