# Rule testing examples

## Button rules

The example application in `mist_mod_button.c` periodically sends new button
states and the lighting control application in `mist_mod_lighting.c` is able
to use these states as input if provided a suitable control rule. With 2
test devices 0000000000000001 and 0000000000000002, apply the following rule
to 0000000000000001 (`coreserver-rules apply https://BACKEND_URL/backend/api/v3 button_rule.json 0000000000000001`).

### button_rule.json
```
[
  {
    "rules": [
      {
        "type": "button",
        "state": 1,
        "value": 100
      },
      {
        "type": "button",
        "state": 2,
        "value": 50
      },
      {
        "type": "button",
        "state": 3,
        "value": 10
      },
      {
        "type": "default",
        "value": -1
      },
      {
        "type": "button_settings",
        "sensors": [
          "0000000000000002"
        ]
      }
    ]
  }
]
```


### Expected behaviour

With the rule applied, the expected behaviour is that 01 will periodically
change its output every minute.

```
2020-10-07 09:31:37.059 :I|m_m_l:  41|CONTROL 100
2020-10-07 09:32:37.036 :I|m_m_l:  41|CONTROL 50
2020-10-07 09:33:37.044 :I|m_m_l:  41|CONTROL 10
2020-10-07 09:34:37.053 :I|m_m_l:  41|CONTROL -1
2020-10-07 09:35:37.045 :I|m_m_l:  41|CONTROL 100
2020-10-07 09:36:37.069 :I|m_m_l:  41|CONTROL 50
2020-10-07 09:37:37.051 :I|m_m_l:  41|CONTROL 10
2020-10-07 09:38:37.059 :I|m_m_l:  41|CONTROL -1
```
