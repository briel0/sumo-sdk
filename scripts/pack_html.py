# -*- coding: utf-8 -*-
try:
    Import("env")
except:
    pass

with open('ui/index.html', 'r', encoding='utf-8') as f:
    html = f.read()

out = f'''#pragma once
#include <Arduino.h>

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
{html}
)rawliteral";
'''

with open('include/services/WebUI.hpp', 'w', encoding='utf-8') as f:
    f.write(out)
print('WebUI.hpp generated!')
