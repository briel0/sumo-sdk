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

  private:
    bool _modeRC = false;
    bool _modeAuto = false;
    bool _start = false;
};