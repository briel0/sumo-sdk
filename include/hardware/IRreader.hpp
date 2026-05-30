#pragma once

class IRReader {
  public:
    void init(int pin);

    void update();

    bool modeRC() const {
        return _modeRC;
    }
    bool modeAuto() const {
        return _modeAuto;
    }
    bool start() const {
        return _start;
    }
    bool stop() const {
        return _stop;
    }
    void shutdown();

  private:
    bool _modeRC = false;
    bool _modeAuto = false;
    bool _start = false;
    bool _stop = false;
};