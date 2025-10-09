# LightController
Light controller for LED strip using MOSFET as dimmable switch and support for sensors.

Improvements:
Logging & diagnostics - make it easier to trace MQTT events, connection drops, and message flow (without spamming Serial) - NOT_DONE
Encapsulation & cohesion - simplify class boundaries: LightController shouldn’t need to know internals of MqttHandler - DONE
Unit-testability - add small seams (interfaces or mockable hooks) so behavior can be tested without hardware - NOT_DONE
Runtime resilience - graceful handling of invalid MQTT payloads, unknown topics, or JSON formatting errors - NOT_DONE
Home Assistant compliance polish - small protocol tweaks (retained flags, topic naming consistency, discovery expiry) - NOT_DONE
