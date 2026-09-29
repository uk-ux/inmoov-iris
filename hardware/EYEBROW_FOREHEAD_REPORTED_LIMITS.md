# User-reported movement limits

Latest calibration, 2026-09-15: exact EEPROM pulse values supplied by the user
supersede the earlier photographed position sheets.

Transcribed from the user's handwritten sheet on 2026-09-10. Values below
are treated as entered calibration command positions, not physical degrees.
Confirm this interpretation before using them as firmware limits: displayed
positions can differ from entered positions due to integer rounding.

| Joint | Channel | Lower pulse | Neutral pulse | Upper pulse |
| --- | ---: | ---: | ---: | ---: |
| Left eyebrow | 8 | 1677 us | 1500 us | 1322 us |
| Right eyebrow | 9 | 1322 us | 1500 us | 1700 us |
| Left forehead | 12 | 1344 us | 1455 us | 1700 us |
| Right forehead | 13 | 1611 us | 1566 us | 1322 us |

Lower and upper describe physical motion labels, so lower pulse may be greater
than upper pulse when a servo is mounted in the opposite direction. These exact
values are saved in the Uno EEPROM by the fast calibrator but have not yet been
copied into the combined animation firmware.
