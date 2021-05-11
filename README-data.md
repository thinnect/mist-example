# Complex data packets

## Data arrays

The example application in `mist_mod_array.c` periodically sends data as an
array. Depending on configuration, it may send a number of sequential elements
or parallel elements from multiple "channels" of the same data source. The
example uses a temperature sensor and requires a sensor data rule to be applied.

### Parallel events

Data events of the same type sampled at the same time from multiple sources.

Data:
```C
{ 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0 }
```

Transmitted MoteXML (XML representation):
```XML
<xml_packet>
    <dt_data value="dt_temperature_C">
        <dt_value>
            <dt_array buffer="6400c8002c019001f4015802bc0220038403e803" value="10"/>
            <dt_exp value="-1"/>
        </dt_value>
        <dt_parallel/>
        <dt_timestamp_utc value="604401880"/>
    </dt_data>
</xml_packet>
```

Extracted XML:
```XML
<xml_packet>
    <dt_data value="dt_temperature_C_0">
        <dt_timestamp_utc value="604401880"/>
        <dt_value value="100">
            <dt_exp value="-1"/>
        </dt_value>
    </dt_data>
    <dt_data value="dt_temperature_C_1">
        <dt_timestamp_utc value="604401880"/>
        <dt_value value="200">
            <dt_exp value="-1"/>
        </dt_value>
    </dt_data>
    <dt_data value="dt_temperature_C_2">
        <dt_timestamp_utc value="604401880"/>
        <dt_value value="300">
            <dt_exp value="-1"/>
        </dt_value>
    </dt_data>
    <dt_data value="dt_temperature_C_3">
        <dt_timestamp_utc value="604401880"/>
        <dt_value value="400">
            <dt_exp value="-1"/>
        </dt_value>
    </dt_data>
    <dt_data value="dt_temperature_C_4">
        <dt_timestamp_utc value="604401880"/>
        <dt_value value="500">
            <dt_exp value="-1"/>
        </dt_value>
    </dt_data>
    <dt_data value="dt_temperature_C_5">
        <dt_timestamp_utc value="604401880"/>
        <dt_value value="600">
            <dt_exp value="-1"/>
        </dt_value>
    </dt_data>
    <dt_data value="dt_temperature_C_6">
        <dt_timestamp_utc value="604401880"/>
        <dt_value value="700">
            <dt_exp value="-1"/>
        </dt_value>
    </dt_data>
    <dt_data value="dt_temperature_C_7">
        <dt_timestamp_utc value="604401880"/>
        <dt_value value="800">
            <dt_exp value="-1"/>
        </dt_value>
    </dt_data>
    <dt_data value="dt_temperature_C_8">
        <dt_timestamp_utc value="604401880"/>
        <dt_value value="900">
            <dt_exp value="-1"/>
        </dt_value>
    </dt_data>
    <dt_data value="dt_temperature_C_9">
        <dt_timestamp_utc value="604401880"/>
        <dt_value value="1000">
            <dt_exp value="-1"/>
        </dt_value>
    </dt_data>
</xml_packet>
```

Converted JSON:
```JSON
{
	"data": {
		"values": [
			{
				"timestamp_production": 1551086680,
				"type": "dt_temperature_C_0",
				"value": 10.0
			},
			{
				"timestamp_production": 1551086680,
				"type": "dt_temperature_C_1",
				"value": 20.0
			},
			{
				"timestamp_production": 1551086680,
				"type": "dt_temperature_C_2",
				"value": 30.0
			},
			{
				"timestamp_production": 1551086680,
				"type": "dt_temperature_C_3",
				"value": 40.0
			},
			{
				"timestamp_production": 1551086680,
				"type": "dt_temperature_C_4",
				"value": 50.0
			},
			{
				"timestamp_production": 1551086680,
				"type": "dt_temperature_C_5",
				"value": 60.0
			},
			{
				"timestamp_production": 1551086680,
				"type": "dt_temperature_C_6",
				"value": 70.0
			},
			{
				"timestamp_production": 1551086680,
				"type": "dt_temperature_C_7",
				"value": 80.0
			},
			{
				"timestamp_production": 1551086680,
				"type": "dt_temperature_C_8",
				"value": 90.0
			},
			{
				"timestamp_production": 1551086680,
				"type": "dt_temperature_C_9",
				"value": 100.0
			}
		]
	}
}
```

### Sequentíal events

Data events sampled every 60 seconds - sampling period is precise in the example,
but does not have to be.

Data:
```C
{ 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0 }
```

Transmitted MoteXML (XML representation):
```XML
<xml_packet>
    <dt_data value="dt_temperature_C">
        <dt_value>
            <dt_array buffer="6400c8002c019001f4015802bc0220038403e803" value="10"/>
            <dt_exp value="-1"/>
        </dt_value>
        <dt_production_start>
            <dt_array buffer="c4ff88ff4cff10ffd4fe98fe5cfe20fee4fda8fd" value="10"/>
        </dt_production_start>
        <dt_timestamp_utc value="604401880"/>
    </dt_data>
</xml_packet>
```

Extracted XML:
```XML
<xml_packet>
    <dt_data value="dt_temperature_C">
        <dt_timestamp_utc value="604401880"/>
        <dt_value value="100">
            <dt_exp value="-1"/>
            <dt_production_start value="-60"/>
            <dt_production_duration value="0"/>
        </dt_value>
        <dt_value value="200">
            <dt_exp value="-1"/>
            <dt_production_start value="-120"/>
            <dt_production_duration value="0"/>
        </dt_value>
        <dt_value value="300">
            <dt_exp value="-1"/>
            <dt_production_start value="-180"/>
            <dt_production_duration value="0"/>
        </dt_value>
        <dt_value value="400">
            <dt_exp value="-1"/>
            <dt_production_start value="-240"/>
            <dt_production_duration value="0"/>
        </dt_value>
        <dt_value value="500">
            <dt_exp value="-1"/>
            <dt_production_start value="-300"/>
            <dt_production_duration value="0"/>
        </dt_value>
        <dt_value value="600">
            <dt_exp value="-1"/>
            <dt_production_start value="-360"/>
            <dt_production_duration value="0"/>
        </dt_value>
        <dt_value value="700">
            <dt_exp value="-1"/>
            <dt_production_start value="-420"/>
            <dt_production_duration value="0"/>
        </dt_value>
        <dt_value value="800">
            <dt_exp value="-1"/>
            <dt_production_start value="-480"/>
            <dt_production_duration value="0"/>
        </dt_value>
        <dt_value value="900">
            <dt_exp value="-1"/>
            <dt_production_start value="-540"/>
            <dt_production_duration value="0"/>
        </dt_value>
        <dt_value value="1000">
            <dt_exp value="-1"/>
            <dt_production_start value="-600"/>
            <dt_production_duration value="0"/>
        </dt_value>
    </dt_data>
</xml_packet>
```

Converted JSON:
```JSON
{
	"data": {
		"values": [
			{
				"values": [
					{
						"timestamp_production": 1551086080,
						"type": "dt_temperature_C",
						"value": 100.0
					},
					{
						"timestamp_production": 1551086140,
						"type": "dt_temperature_C",
						"value": 90.0
					},
					{
						"timestamp_production": 1551086200,
						"type": "dt_temperature_C",
						"value": 80.0
					},
					{
						"timestamp_production": 1551086260,
						"type": "dt_temperature_C",
						"value": 70.0
					},
					{
						"timestamp_production": 1551086320,
						"type": "dt_temperature_C",
						"value": 60.0
					},
					{
						"timestamp_production": 1551086380,
						"type": "dt_temperature_C",
						"value": 50.0
					},
					{
						"timestamp_production": 1551086440,
						"type": "dt_temperature_C",
						"value": 40.0
					},
					{
						"timestamp_production": 1551086500,
						"type": "dt_temperature_C",
						"value": 30.0
					},
					{
						"timestamp_production": 1551086560,
						"type": "dt_temperature_C",
						"value": 20.0
					},
					{
						"timestamp_production": 1551086620,
						"type": "dt_temperature_C",
						"value": 10.0
					}
				]
			}
		]
	}
}
```
