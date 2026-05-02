#include "parser.h"
#include "state/data.h"
#include "state/ack.h"

// Processa e atualiza os dados recebidos

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

  // Leitura do UART do MEGA
  while (tok) {
    char *dp = strchr(tok, ':');
    if (dp) {
      *dp = '\0';
      char *key = tok;
      char *value = dp + 1;

      float val = atof(value);

      if      (strcmp(key, "V") == 0)      data.volts = val;
      else if (strcmp(key, "Pct") == 0)    data.pct = val;
      else if (strcmp(key, "Temp") == 0)   data.temp = (strcmp(value,"NaN")==0)?NAN:val;
      else if (strcmp(key, "Humi") == 0)   data.humi = (strcmp(value,"NaN")==0)?NAN:val;
      else if (strcmp(key, "RPM") == 0)    data.rpm = val;
      else if (strcmp(key, "Speed") == 0)  data.speed_kmh = val;
      else if (strcmp(key, "I") == 0)      data.current_bat_a = val;
      else if (strcmp(key, "IMOT") == 0)   data.current_mot_a = val;
      else if (strcmp(key, "MIN") == 0)    data.min_v = val;
      else if (strcmp(key, "MAX") == 0)    data.max_v = val;
      else if (strcmp(key, "WHEEL") == 0)  data.wheel_cm = val;
      else if (strcmp(key, "PPR") == 0)    data.ppr = (uint8_t)atoi(value);
      else if (strcmp(key, "OVR") == 0)    data.override_enabled = atoi(value) != 0;
      else if (strcmp(key, "OVRPCT") == 0) data.override_pct = val;
      else if (strcmp(key, "MAXPCT") == 0) data.max_pct = val;
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
    pos = 0; 
  }
}