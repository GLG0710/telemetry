struct AdvancedConfig {
	float pwm_hz = 1000.0f;
  	float start_min_pct = 8.0f;
  	float rapid_ms = 250.0f;
  	float rapid_up = 150.0f;
  	float slew_up = 40.0f;
  	float slew_dn = 60.0f;
  	float zero_timeout_ms = 600.0f;
};
extern AdvancedConfig config;