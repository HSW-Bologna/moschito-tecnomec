# Moschito

Application for Tecnomec's bug repellant erogator.

# Notes

 - There are two pumps for funnelling two different products (TRIACs but the pump itself cannot be controlled with phase cut) and an erogator (Relay), 4 input pins, two outputs, two analog sensors.
 - Erogation should be preceded by a warning "beep"
 - There should be a page to test the pumps
 - The products erogation percentage can be configured
 - There are two schedulers; the on precision time is in seconds, the off precision time is in minutes
 - one of the inputs is for a water level sensor (digital)
 - The button on the board is for fully turning off the device
 - One of the relais should be run for a fixed time when the erogation is over (configurable)

# TODO

 - move the homepage button always in the same place (top left?)
 - the setting for the timer should include starting time and duration (vs starting time and end time), to allow for seconds to be input
