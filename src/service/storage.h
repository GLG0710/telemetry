#pragma once

#include <LittleFS.h>
#include <WiFi.h>
#include <cmath>

#include "storage.h"
#include "controller/control.h"
#include "core/telemetry.h"
#include "core/ack.h"
#include "config/adv_config.h"
#include "logger.h"

void appendCsvRow();
    