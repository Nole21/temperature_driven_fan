# temperature_driven_fan
It is all about the software design curriculum. Detailed information is provided as below.
## Software/IDE
- IAR
- PROTEUS
## Hardware
- MSP430
- DS18B20
- LCD
- SPEAKER/BUZZER
## Requirements
- start the fan when the temperature is higher than a specified threshold
- distinguish the fan speed when temperature varies
- display temperature and fan speed in the lcd
- record the times when the fan starts to work and corresponding time, and the records can be reviewed in the lcd
## Notes
- !!! It is all about simulation, all so called hardwares are merely involved in the simulation.
- In the final design, the fan is simulated with a speaker. And the frequency represents the speed of the fan. So if you really want to drive a fan, do it youself after refering to my design.
- The time involved with the recording of the fan is also simulated, I do not really access the acute time using the clock.
- The display of the temperature is slightly unstable, maybe later I will fix it(maybe never).

