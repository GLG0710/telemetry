#include "parser.h"
#include "state/data.h"
#include "state/ack.h"

static char lineBuffer[400];
static uint16_t pos = 0;

static void processLine(char *line) {
  char *p = line;
  while (*p) {
    if (*p == ',') *p = ' ';
    p++;
  }

  // ACK
  if (strncmp(line, "ACK:", 4) == 0) {
    strncpy(ack.last, line + 4, sizeof(ack.last) - 1);
    ack.last[sizeof(ack.last) - 1] = '\0';
    ack.timestamp = millis();
    return;
  }

  char *saveptr;
  char *tok = strtok_r(line, " ", &saveptr);

  while (tok) {
    char *dp = strchr(tok, ':');
    if (dp) {
      *dp = '\0';
      char *k = tok;
      char *v = dp + 1;

      float val = atof(v);

      if      (strcmp(k, "V") == 0)      data.volts = val;
      else if (strcmp(k, "Pct") == 0)    data.pct = val;
      else if (strcmp(k, "Temp") == 0)   data.temp = (strcmp(v,"NaN")==0)?NAN:val;
      else if (strcmp(k, "Humi") == 0)   data.humi = (strcmp(v,"NaN")==0)?NAN:val;
      else if (strcmp(k, "RPM") == 0)    data.rpm = val;
      else if (strcmp(k, "Speed") == 0)  data.speed_kmh = val;
      else if (strcmp(k, "I") == 0)      data.current_bat_a = val;
      else if (strcmp(k, "IMOT") == 0)   data.current_mot_a = val;
      else if (strcmp(k, "MIN") == 0)    data.min_v = val;
      else if (strcmp(k, "MAX") == 0)    data.max_v = val;
      else if (strcmp(k, "WHEEL") == 0)  data.wheel_cm = val;
      else if (strcmp(k, "PPR") == 0)    data.ppr = (uint8_t)atoi(v);
      else if (strcmp(k, "OVR") == 0)    data.override_enabled = atoi(v) != 0;
      else if (strcmp(k, "OVRPCT") == 0) data.override_pct = val;
      else if (strcmp(k, "MAXPCT") == 0) data.max_pct = val;
    }

    tok = strtok_r(NULL, " ", &saveptr);
  }
}

void protocolFeedByte(char c) {
  if (c == '\n') {
    lineBuffer[pos] = '\0';
    processLine(lineBuffer);
    pos = 0;
    return;
  }

  if (c == '\r') return;

  if (pos < sizeof(lineBuffer) - 1) {
    lineBuffer[pos++] = c;
  } else {
    pos = 0; // overflow safety
  }
}